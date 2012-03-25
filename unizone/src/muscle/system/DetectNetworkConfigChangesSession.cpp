#include "iogateway/SignalMessageIOGateway.h"
#include "system/DetectNetworkConfigChangesSession.h"

#ifdef __APPLE__
# include <SystemConfiguration/SystemConfiguration.h>
#elif WIN32
# include <Iphlpapi.h>
# define MY_INVALID_HANDLE_VALUE ((::HANDLE)(-1))  // bloody hell...
#endif

#ifdef __linux
# include <asm/types.h>
# include <sys/socket.h>
# include <linux/netlink.h>
# include <linux/rtnetlink.h>
#endif

namespace muscle {

DetectNetworkConfigChangesSession :: DetectNetworkConfigChangesSession() : 
#ifndef __linux__
   _threadKeepGoing(false), 
#endif
#ifdef __APPLE__
   _threadRunLoop(NULL), // paranoia
#elif WIN32
   _wakeupSignal(MY_INVALID_HANDLE_VALUE),
#endif
   _callbackTime(MUSCLE_TIME_NEVER),
   _enabled(true)
{
   // empty
}

void DetectNetworkConfigChangesSession :: ScheduleSendReport()
{
   // We won't actually send the report for a certain number of
   // seconds (OS-specific); that way any additional changes the OS
   // is making to the network config will have time to be reported
   // and we (hopefully) won't end up sending multiple reports in a row.
#ifdef WIN32
   const int hysteresisDelaySeconds = 5;  // Windows needs 5, it is lame
#else
   const int hysteresisDelaySeconds = 3;  // MacOS/X needs about 3 seconds
#endif
   _callbackTime = GetRunTime64() + SecondsToMicros(hysteresisDelaySeconds);
   InvalidatePulseTime();
}

void DetectNetworkConfigChangesSession :: NetworkInterfacesChanged()
{
   // default implementation is a no-op.
}

void DetectNetworkConfigChangesSession :: Pulse(const PulseArgs & pa)
{
   if (pa.GetCallbackTime() >= _callbackTime)
   {
      _callbackTime = MUSCLE_TIME_NEVER;
      if (_enabled) NetworkInterfacesChanged();
   }
   AbstractReflectSession::Pulse(pa);
}

ConstSocketRef DetectNetworkConfigChangesSession :: CreateDefaultSocket()
{
#ifdef __linux__
   struct sockaddr_nl sa; memset(&sa, 0, sizeof(sa));
   sa.nl_family = AF_NETLINK;
   sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV6_IFADDR;

   ConstSocketRef ret = GetConstSocketRefFromPool(socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE));
   return ((ret())&&(bind(ret()->GetFileDescriptor(), (struct sockaddr*)&sa, sizeof(sa)) == 0)&&(SetSocketBlockingEnabled(ret, false) == B_NO_ERROR)) ? ret : ConstSocketRef();
#else
   return GetOwnerWakeupSocket();
#endif
}

void DetectNetworkConfigChangesSession :: MessageReceivedFromGateway(const MessageRef & /*msg*/, void * /*ptr*/)
{
#ifndef __linux__
   bool sendReport = false;
   MessageRef ref;
   while(GetNextReplyFromInternalThread(ref) >= 0) sendReport = true;  // we only need to send one report, even for multiple Messages
   if (sendReport) ScheduleSendReport();
#endif
}

#ifdef __linux__

int32 DetectNetworkConfigChangesSession :: DoInput(AbstractGatewayMessageReceiver & /*r*/, uint32 /*maxBytes*/)
{
   int fd = GetSessionReadSelectSocket().GetFileDescriptor();
   if (fd < 0) return -1;

   bool sendReport = false;
   char buf[4096];
   struct iovec iov = {buf, sizeof(buf)};
   struct sockaddr_nl sa;
   struct msghdr msg = {(void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
   int len = recvmsg(fd, &msg, 0);
   for (struct nlmsghdr *nh = (struct nlmsghdr *)buf; ((sendReport == false)&&(NLMSG_OK(nh, len))); nh=NLMSG_NEXT(nh, len))
   {
      /* The end of multipart message. */
      if (nh->nlmsg_type == NLMSG_DONE) break;
      else
      {
         switch(nh->nlmsg_type)
         {
            case RTM_NEWLINK: case RTM_DELLINK: case RTM_NEWADDR: case RTM_DELADDR:
               sendReport = true;
            break; 

            default:
               // do nothing
            break;
         }
      }
   }
   if (sendReport) ScheduleSendReport();
   return len;
}

#else

status_t DetectNetworkConfigChangesSession :: AttachedToServer()
{
   _threadKeepGoing = true;
# ifdef __APPLE__
   _threadRunLoop = NULL;
# endif
# ifdef WIN32
   _wakeupSignal = CreateEvent(0, false, false, 0);
   if (_wakeupSignal == MY_INVALID_HANDLE_VALUE) return B_ERROR;
# endif
   return (AbstractReflectSession::AttachedToServer() == B_NO_ERROR) ? StartInternalThread() : B_ERROR;
}

void DetectNetworkConfigChangesSession :: EndSession()
{
   ShutdownInternalThread();  // do this ASAP, otherwise we get the occasional crash on shutdown :(
   AbstractReflectSession::EndSession();
}

void DetectNetworkConfigChangesSession :: AboutToDetachFromServer()
{
   ShutdownInternalThread();
#ifdef WIN32
   if (_wakeupSignal != MY_INVALID_HANDLE_VALUE)
   {
      CloseHandle(_wakeupSignal);
      _wakeupSignal = MY_INVALID_HANDLE_VALUE;
   }
#endif
   AbstractReflectSession::AboutToDetachFromServer();
}

AbstractMessageIOGatewayRef DetectNetworkConfigChangesSession :: CreateGateway()
{
   AbstractMessageIOGateway * gw = newnothrow SignalMessageIOGateway;
   if (gw == NULL) WARN_OUT_OF_MEMORY;
   return AbstractMessageIOGatewayRef(gw);
}

void DetectNetworkConfigChangesSession :: SignalInternalThread()
{
   _threadKeepGoing = false;
   Thread::SignalInternalThread();
# ifdef __APPLE__
   if (_threadRunLoop) CFRunLoopStop(_threadRunLoop);
# elif WIN32
   SetEvent(_wakeupSignal);
# endif
}

# ifdef __APPLE__

// MacOS/X Code taken from http://developer.apple.com/technotes/tn/tn1145.html
static OSStatus MoreSCErrorBoolean(Boolean success)
{
   OSStatus err = noErr;
   if (!success) 
   {
      int scErr = SCError();
      if (scErr == kSCStatusOK) scErr = kSCStatusFailed;
      err = scErr;
   }
   return err;
}

static OSStatus MoreSCError(const void *value) {return MoreSCErrorBoolean(value != NULL);}
static OSStatus CFQError(CFTypeRef cf) {return (cf == NULL) ? -1 : noErr;}
static void CFQRelease(CFTypeRef cf) {if (cf != NULL) CFRelease(cf);}

// Create a SCF dynamic store reference and a corresponding CFRunLoop source.  If you add the
// run loop source to your run loop then the supplied callback function will be called when local IP
// address list changes.
static OSStatus CreateIPAddressListChangeCallbackSCF(SCDynamicStoreCallBack callback, void *contextPtr, SCDynamicStoreRef *storeRef, CFRunLoopSourceRef *sourceRef)
{
   OSStatus                err;
   SCDynamicStoreContext   context = {0, NULL, NULL, NULL, NULL};
   SCDynamicStoreRef       ref = NULL;
   CFStringRef             patterns[2] = {NULL, NULL};
   CFArrayRef              patternList = NULL;
   CFRunLoopSourceRef      rls = NULL;

   assert(callback   != NULL);
   assert( storeRef  != NULL);
   assert(*storeRef  == NULL);
   assert( sourceRef != NULL);
   assert(*sourceRef == NULL);

   // Create a connection to the dynamic store, then create
   // a search pattern that finds all entities.
   context.info = contextPtr;
   ref = SCDynamicStoreCreate(NULL, CFSTR("AddIPAddressListChangeCallbackSCF"), callback, &context);
   err = MoreSCError(ref);
   if (err == noErr) 
   {
      // This pattern is "State:/Network/Service/[^/]+/IPv4".
      patterns[0] = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, kSCEntNetIPv4);  // FogBugz #6075
      err = MoreSCError(patterns[0]);
      if (err == noErr)
      {
         // This pattern is "State:/Network/Service/[^/]+/IPv6".
         patterns[1] = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, kSCEntNetIPv6);  // FogBugz #6075
         err = MoreSCError(patterns[1]);
      }
   }

   // Create a pattern list containing just one pattern,
   // then tell SCF that we want to watch changes in keys
   // that match that pattern list, then create our run loop
   // source.
   if (err == noErr) 
   {
       patternList = CFArrayCreate(NULL, (const void **) patterns, 2, &kCFTypeArrayCallBacks);
       err = CFQError(patternList);
   }
   if (err == noErr) err = MoreSCErrorBoolean(SCDynamicStoreSetNotificationKeys(ref, NULL, patternList));
   if (err == noErr) 
   {
       rls = SCDynamicStoreCreateRunLoopSource(NULL, ref, 0);
       err = MoreSCError(rls);
   }

   // Clean up.
   CFQRelease(patterns[0]);
   CFQRelease(patterns[1]);
   CFQRelease(patternList);
   if (err != noErr) 
   {
      CFQRelease(ref);
      ref = NULL;
   }
   *storeRef = ref;
   *sourceRef = rls;

   assert( (err == noErr) == (*storeRef  != NULL) );
   assert( (err == noErr) == (*sourceRef != NULL) );

   return err;
}

static void IPConfigChangedCallback(SCDynamicStoreRef /*store*/, CFArrayRef /*changedKeys*/, void * info)
{
   SignalInterfacesChanged((DetectNetworkConfigChangesSession *)info);
}

void SignalInterfacesChanged(DetectNetworkConfigChangesSession * s)
{
   s->SendMessageToOwner(MessageRef());
}

# endif  // __APPLE__

void DetectNetworkConfigChangesSession :: InternalThreadEntry()
{
# ifdef __APPLE__
   _threadRunLoop = CFRunLoopGetCurrent();

   SCDynamicStoreRef storeRef = NULL;
   CFRunLoopSourceRef sourceRef = NULL;
   if (CreateIPAddressListChangeCallbackSCF(IPConfigChangedCallback, this, &storeRef, &sourceRef) == noErr)
   {
      CFRunLoopAddSource(CFRunLoopGetCurrent(), sourceRef, kCFRunLoopDefaultMode);
      while(_threadKeepGoing) 
      {
         CFRunLoopRun();
         while(1)
         {
            MessageRef msgRef;
            int32 numLeft = WaitForNextMessageFromOwner(msgRef, 0);
            if (numLeft >= 0)
            {
               if (MessageReceivedFromOwner(msgRef, numLeft) != B_NO_ERROR) _threadKeepGoing = false;
            }
            else break; 
         }
      }
      CFRunLoopRemoveSource(CFRunLoopGetCurrent(), sourceRef, kCFRunLoopDefaultMode);
      CFRelease(storeRef);
      CFRelease(sourceRef);
   }
# elif WIN32
   OVERLAPPED olap; memset(&olap, 0, sizeof(olap));
   olap.hEvent = CreateEvent(NULL, false, false, NULL);
   if (olap.hEvent != NULL)
   {
      while(_threadKeepGoing)
      {
         ::HANDLE junk;
         int nacRet = NotifyAddrChange(&junk, &olap); 
         if ((nacRet == NO_ERROR)||(WSAGetLastError() == WSA_IO_PENDING))
         {
            ::HANDLE events[] = {olap.hEvent, _wakeupSignal};
            switch(WaitForMultipleObjects(ARRAYITEMS(events), events, false, INFINITE))
            {
               case WAIT_OBJECT_0: 
                  SendMessageToOwner(MessageRef());
               break;

               default:
                  (void) CancelIPChangeNotify(&olap);
                  _threadKeepGoing = false;
               break;
            }
         }
         else 
         {
            LogTime(MUSCLE_LOG_ERROR, "DetectNetworkConfigChangesSession:  NotifyAddrChange() failed, code %i (%i)\n", nacRet, WSAGetLastError());
            break;
         }
      }
      CloseHandle(olap.hEvent);
   }
   else LogTime(MUSCLE_LOG_ERROR, "DetectNetworkConfigChangesSession:  CreateEvent() failed\n");
# else
#  error "NetworkInterfacesSession:  OS not supported!"
# endif
}

#endif  // !__linux__

};  // end namespace muscle


