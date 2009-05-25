/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "reflector/ReflectServer.h"
# include "reflector/StorageReflectConstants.h"
#ifndef MUSCLE_AVOID_SIGNAL_HANDLING
#include "reflector/SignalHandlerSession.h"
#include "system/SetupSystem.h"  // for IsCurrentThreadMainThread()
#endif
#include "util/NetworkUtilityFunctions.h"
#include "util/MemoryAllocator.h"

#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
# include "system/GlobalMemoryAllocator.h"
#endif

namespace muscle {

extern bool _mainReflectServerCatchSignals;  // from SetupSystem.cpp

status_t
ReflectServer ::
AddNewSession(const AbstractReflectSessionRef & ref, const ConstSocketRef & ss)
{
   TCHECKPOINT;

   AbstractReflectSession * newSession = ref();
   if (newSession == NULL) return B_ERROR;

   newSession->SetOwner(this);  // in case CreateGateway() needs to use the owner

   ConstSocketRef s = ss;
   if (s() == NULL) s = ref()->CreateDefaultSocket();

   // Create the gateway object for this session, if it isn't already set up
   if (s())
   {
      AbstractMessageIOGatewayRef gatewayRef = newSession->GetGateway();
      if (gatewayRef() == NULL) gatewayRef = newSession->CreateGateway();
      if (gatewayRef())  // don't combine these ifs!
      {
         if (gatewayRef()->GetDataIO()() == NULL)
         {
            // create the new DataIO for the gateway; this must always be done on the fly
            // since it depends on the socket being used.
            DataIORef io = newSession->CreateDataIO(s);
            if (io()) 
            {
               gatewayRef()->SetDataIO(io);
               newSession->SetGateway(gatewayRef);
            }
            else {newSession->SetOwner(NULL); return B_ERROR;}
         }
      }
      else {newSession->SetOwner(NULL); return B_ERROR;}
   }

   TCHECKPOINT;

   // Set our hostname (IP address) string if it isn't already set
   if (newSession->_hostName.IsEmpty())
   {
      const ConstSocketRef & sock = newSession->GetSessionSelectSocket();
      if (sock.GetFileDescriptor() >= 0)
      {
         ip_address ip = GetPeerIPAddress(sock, true);
         const String * remapString = _remapIPs.Get(ip);
         char ipbuf[64]; Inet_NtoA(ip, ipbuf);
         newSession->_hostName = remapString ? *remapString : ((ip != invalidIP) ? String(ipbuf) : newSession->GetDefaultHostName());
      }
      else newSession->_hostName = newSession->GetDefaultHostName();
   }

        if (AttachNewSession(ref) == B_NO_ERROR) return B_NO_ERROR;
   else if (newSession) newSession->SetOwner(NULL);

   TCHECKPOINT;

   return B_ERROR;
}

status_t
ReflectServer ::
AddNewConnectSession(const AbstractReflectSessionRef & ref, const ip_address & destIP, uint16 port, uint64 autoReconnectDelay)
{
   AbstractReflectSession * session = ref();
   if (session)
   {
      ConstSocketRef sock = ConnectAsync(destIP, port, session->_isConnected);

      // FogBugz #5256:  If ConnectAsync() fails, we want to act as if it succeeded, so that the calling
      //                 code still uses its normal asynchronous-connect-failure code path.  That way the
      //                 caller doesn't have to worry about synchronous failure as a separate case.
      bool isHack = false;
      if (sock() == NULL)
      {
         ConstSocketRef tempSockRef;  // tempSockRef represents the closed remote end of the failed connection and is intentionally closed ASAP
         if (CreateConnectedSocketPair(sock, tempSockRef) == B_NO_ERROR) 
         {
            session->_isConnected = false;
            isHack = true;
         }
      }

      if (sock())
      {
         session->_asyncConnectDest = IPAddressAndPort(destIP, port);
         session->_connectingAsync  = ((isHack == false)&&(session->_isConnected == false));

         char ipbuf[64]; Inet_NtoA(destIP, ipbuf);
         session->_hostName = (destIP != invalidIP) ? ipbuf : "<unknown>";

         status_t ret = AddNewSession(ref, sock);
         if (ret == B_NO_ERROR)
         {
            if (autoReconnectDelay != MUSCLE_TIME_NEVER) session->SetAutoReconnectDelay(autoReconnectDelay);
            if (session->_isConnected) 
            {
               session->_wasConnected = true;
               session->AsyncConnectCompleted();
            }
            return B_NO_ERROR;
         }
         else
         {
            session->_asyncConnectDest.Reset();
            session->_hostName.Clear();
            session->_isConnected = session->_connectingAsync = false;
         }
      }
   }
   return B_ERROR;
}

status_t
ReflectServer ::
AddNewDormantConnectSession(const AbstractReflectSessionRef & ref, const ip_address & destIP, uint16 port, uint64 autoReconnectDelay)
{
   AbstractReflectSession * session = ref();
   if (session)
   {
      session->_asyncConnectDest = IPAddressAndPort(destIP, port);
      char ipbuf[64]; Inet_NtoA(destIP, ipbuf);
      session->_hostName = (destIP != invalidIP) ? ipbuf : "<unknown>";
      status_t ret = AddNewSession(ref, ConstSocketRef());
      if (ret == B_NO_ERROR)
      {
         if (autoReconnectDelay != MUSCLE_TIME_NEVER) session->SetAutoReconnectDelay(autoReconnectDelay);
         return B_NO_ERROR;
      }
      else
      {
         session->_asyncConnectDest.Reset();
         session->_hostName.Clear();
      }
   }
   return B_ERROR;
}

status_t
ReflectServer ::
AttachNewSession(const AbstractReflectSessionRef & ref)
{
   AbstractReflectSession * newSession = ref();
   if ((newSession)&&(_sessions.Put(&newSession->GetSessionIDString(), ref) == B_NO_ERROR))
   {
      newSession->SetOwner(this);
      if (newSession->AttachedToServer() == B_NO_ERROR)
      {
         if (_doLogging) LogTime(MUSCLE_LOG_DEBUG, "New %s ("UINT32_FORMAT_SPEC" total)\n", newSession->GetSessionDescriptionString()(), _sessions.GetNumItems());
         return B_NO_ERROR;
      }
      else 
      {
         newSession->AboutToDetachFromServer();  // well, it *was* attached, if only for a moment
         newSession->DoOutput(MUSCLE_NO_LIMIT);  // one last chance for him to send any leftover data!
         if (_doLogging) LogTime(MUSCLE_LOG_DEBUG, "%s aborted startup ("UINT32_FORMAT_SPEC" left)\n", newSession->GetSessionDescriptionString()(), _sessions.GetNumItems()-1);
      }
      newSession->SetOwner(NULL);
      (void) _sessions.Remove(&newSession->GetSessionIDString());
   }
   return B_ERROR;
}


ReflectServer :: ReflectServer(MemoryAllocator * optMemoryUsageTracker) : _keepServerGoing(true), _serverStartedAt(0), _doLogging(true), _serverSessionID(GetCurrentTime64()+GetRunTime64()+rand()), _watchMemUsage(optMemoryUsageTracker)
{
   if (_serverSessionID == 0) _serverSessionID++;  // paranoia:  make sure 0 can be used as a guard value

   // make sure _lameDuckSessions has plenty of memory available in advance (we need might need it in a tight spot later!)
   _lameDuckSessions.EnsureSize(256);
   _sessions.SetKeyCompareFunction(StringCompareFunc);
}

ReflectServer :: ~ReflectServer()
{
   // empty
}

void
ReflectServer :: Cleanup()
{
   // Detach all sessions
   {
      HashtableIterator<const String *, AbstractReflectSessionRef> iter(GetSessions());
      const String * nextKey;
      AbstractReflectSessionRef nextValue;
      while(iter.GetNextKeyAndValue(nextKey, nextValue) == B_NO_ERROR)
      {
         if (nextValue()) 
         {
            nextValue()->AboutToDetachFromServer();
            nextValue()->DoOutput(MUSCLE_NO_LIMIT);  // one last chance for him to send any leftover data!
            nextValue()->SetOwner(NULL);
            _lameDuckSessions.AddTail(nextValue);  // we'll delete it below
            _sessions.Remove(nextKey);  // but prevent other sessions from accessing it now that it's detached
         }
      }
   }
  
   // Detach all factories
   RemoveAcceptFactory(0);

   // This will dereference everything so they can be safely deleted here
   _lameDuckSessions.Clear();
   _lameDuckFactories.Clear();
}

status_t
ReflectServer ::
ReadyToRun()
{
   return B_NO_ERROR;
}

const char *
ReflectServer ::
GetServerName() const
{
   return "MUSCLE";
}

/** Makes sure the given policy has its BeginIO() called, if necessary, and returns it */
uint32
ReflectServer :: 
CheckPolicy(Hashtable<AbstractSessionIOPolicyRef, bool> & policies, const AbstractSessionIOPolicyRef & policyRef, const PolicyHolder & ph, uint64 now) const
{
   AbstractSessionIOPolicy * p = policyRef();
   if (p)
   {
      // Any policy that is found attached to a session goes into our temporary policy set
       policies.Put(policyRef, true);

      // If the session is ready, and BeginIO() hasn't been called on this policy already, do so now
      if ((ph.GetSession())&&(p->_hasBegun == false))
      {
         CallSetCycleStartTime(*p, now);
         p->BeginIO(now); 
         p->_hasBegun = true;
      }
   }
   return ((ph.GetSession())&&((p == NULL)||(p->OkayToTransfer(ph)))) ? MUSCLE_NO_LIMIT : 0;
}

void ReflectServer :: CheckForOutOfMemory(const AbstractReflectSessionRef & optSessionRef)
{
   if ((_watchMemUsage)&&(_watchMemUsage->HasAllocationFailed()))
   {
      _watchMemUsage->SetAllocationHasFailed(false);  // clear the memory-failed flag
      if (optSessionRef())
      {
         if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Low Memory!  Aborting %s to get some back!\n", optSessionRef()->GetSessionDescriptionString()());
         AddLameDuckSession(optSessionRef);
      }
      else if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Low Memory!  Dumping bogged sessions!\n");

      DumpBoggedSessions();       // see what other cleanup we can do
   }
}

status_t 
ReflectServer ::
ServerProcessLoop()
{
   TCHECKPOINT;

   _serverStartedAt = GetRunTime64();

   if (_doLogging)
   {
      LogTime(MUSCLE_LOG_DEBUG, "This %s server was compiled on " __DATE__ " " __TIME__ "\n", GetServerName());
#ifdef MUSCLE_USE_IPV6
      const char * verb = "enabled";
#else
      const char * verb = "disabled";
#endif
      LogTime(MUSCLE_LOG_DEBUG, "The server was compiled with MUSCLE version %s.  IPv6 support is %s.\n", MUSCLE_VERSION_STRING, verb);
      LogTime(MUSCLE_LOG_DEBUG, "This server's session ID is "UINT64_FORMAT_SPEC".\n", GetServerSessionID());
   }

#ifndef MUSCLE_AVOID_SIGNAL_HANDLING
   if ((_mainReflectServerCatchSignals)&&(IsCurrentThreadMainThread()))
   {
      SignalHandlerSession * shs = newnothrow SignalHandlerSession;
      if (shs == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
      if (AddNewSession(AbstractReflectSessionRef(shs)) != B_NO_ERROR)
      {
         LogTime(MUSCLE_LOG_CRITICALERROR, "ReflectServer::ReadyToRun:  Couldn't install SignalHandlerSession!\n");
         return B_ERROR;
      }
   }
#endif

   if (ReadyToRun() != B_NO_ERROR) 
   {
      if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Server:  ReadyToRun() failed, aborting.\n");
      return B_ERROR;
   }

   TCHECKPOINT;

   // Print an informative startup message
   if ((_doLogging)&&(GetMaxLogLevel() >= MUSCLE_LOG_DEBUG))
   {
      if (_factories.HasItems())
      {
         bool listeningOnAll = false;
         for (HashtableIterator<IPAddressAndPort, ReflectSessionFactoryRef> iter(_factories); iter.HasMoreKeys(); iter++)
         {
            const IPAddressAndPort & iap = iter.GetKey();
            LogTime(MUSCLE_LOG_DEBUG, "%s is listening on port %u ", GetServerName(), iap.GetPort());
            if (iap.GetIPAddress() == invalidIP) 
            {
               Log(MUSCLE_LOG_DEBUG, "on all network interfaces.\n");
               listeningOnAll = true;
            }
            else Log(MUSCLE_LOG_DEBUG, "on network interface %s\n", Inet_NtoA(iap.GetIPAddress())()); 
         }

         if (listeningOnAll)
         {
            Queue<NetworkInterfaceInfo> ifs;
            if ((GetNetworkInterfaceInfos(ifs) == B_NO_ERROR)&&(ifs.HasItems()))
            {
               LogTime(MUSCLE_LOG_DEBUG, "This host's network interface addresses are as follows:\n");
               for (uint32 i=0; i<ifs.GetNumItems(); i++) 
               {
                  LogTime(MUSCLE_LOG_DEBUG, "- %s (%s)\n", Inet_NtoA(ifs[i].GetLocalAddress())(), ifs[i].GetName()());
               }
            }
            else LogTime(MUSCLE_LOG_ERROR, "Couldn't retrieve this server's network interface addresses list.\n"); 
         }
      }
      else LogTime(MUSCLE_LOG_DEBUG, "Server is not listening on any ports.\n");
   }

   // The primary event loop for any MUSCLE-based server!
   // These variables are used as scratch space, but are declared outside the loop to avoid having to reinitialize them all the time.
   Hashtable<AbstractSessionIOPolicyRef, bool> policies;
   fd_set readSet;
   fd_set writeSet;
#ifdef WIN32
   fd_set exceptionSet;
#endif

   while(ClearLameDucks() == B_NO_ERROR)
   {
      TCHECKPOINT;

      fd_set * exceptionSetPtr = NULL;  // set to point to exceptionSet if we should be watching for exceptions

      // Initialize our fd sets of events-to-watch-for
      FD_ZERO(&readSet);
      FD_ZERO(&writeSet);
      int maxSocket = -1;
      uint64 nextPulseAt = MUSCLE_TIME_NEVER; // running minimum of everything that wants to be Pulse()'d

      // Set up fd set entries and Pulse() timing info for all our different components
      {
         const uint64 now = GetRunTime64(); // nothing in this scope is supposed to take a significant amount of time to execute, so just calculate this once

         TCHECKPOINT;

         // Set up the session factories so we can be notified when a new connection is received
         if (_factories.HasItems())
         {
            HashtableIterator<IPAddressAndPort, ReflectSessionFactoryRef> iter(_factories);
            const IPAddressAndPort * nextKey;
            ReflectSessionFactoryRef * nextValue;
            while(iter.GetNextKeyAndValue(nextKey, nextValue) == B_NO_ERROR)
            {
               ConstSocketRef * nextAcceptSocket = nextValue->GetItemPointer()->IsReadyToAcceptSessions() ? _factorySockets.Get(*nextKey) : NULL;
               int nfd = nextAcceptSocket ? nextAcceptSocket->GetFileDescriptor() : -1;
               if (nfd >= 0)
               {
                  FD_SET(nfd, &readSet);
                  if (nfd > maxSocket) maxSocket = nfd;
               }
               CallGetPulseTimeAux(*nextValue->GetItemPointer(), now, nextPulseAt);
            }
         }

         TCHECKPOINT;

         // Set up the sessions, their associated IO-gateways, and their IOPolicies
         if (_sessions.HasItems())
         {
            AbstractReflectSessionRef * nextRef;
            HashtableIterator<const String *, AbstractReflectSessionRef> iter(GetSessions());
            while((nextRef = iter.GetNextValue()) != NULL)
            {
               AbstractReflectSession * session = nextRef->GetItemPointer();
               if (session)
               {
                  session->_maxInputChunk = session->_maxOutputChunk = 0;
                  AbstractMessageIOGateway * g = session->GetGateway()();
                  if (g)
                  {
                     int sfd = session->GetSessionSelectSocket().GetFileDescriptor();
                     if (sfd >= 0)
                     {
                        bool in, out;
                        if (session->_connectingAsync) 
                        {
                           in  = false;
                           out = true;  // so we can watch for the async-connect event

#ifdef WIN32
                           // Under windows, failed asynchronous connect()'s are communicated via the exceptions fd_set
                           if (exceptionSetPtr == NULL)
                           {
                              FD_ZERO(&exceptionSet);
                              exceptionSetPtr = &exceptionSet;
                           }
                           FD_SET(sfd, &exceptionSet);
#endif
                        }
                        else
                        {
                           session->_maxInputChunk  = CheckPolicy(policies, session->GetInputPolicy(),  PolicyHolder(session->IsReadyForInput()  ? session : NULL, true),  now);
                           session->_maxOutputChunk = CheckPolicy(policies, session->GetOutputPolicy(), PolicyHolder(session->HasBytesToOutput() ? session : NULL, false), now);
                           in  = (session->_maxInputChunk  > 0);
                           out = ((session->_maxOutputChunk > 0)||((g->GetDataIO()())&&(g->GetDataIO()()->HasBufferedOutput())));
                        }

                        if (in) FD_SET(sfd, &readSet);
                        if (out) 
                        {
                           FD_SET(sfd, &writeSet);
                           if (session->_lastByteOutputAt == 0) session->_lastByteOutputAt = now;  // the bogged-session-clock starts ticking when we first want to write...
                           if (session->_outputStallLimit != MUSCLE_TIME_NEVER) nextPulseAt = muscleMin(nextPulseAt, session->_lastByteOutputAt+session->_outputStallLimit);
                        }
                        else session->_lastByteOutputAt = 0;  // If we no longer want to write, then the bogged-session-clock-timeout is cancelled

                        if (((in)||(out))&&(sfd > maxSocket)) maxSocket = sfd;
                     }
                     TCHECKPOINT;
                     CallGetPulseTimeAux(*g, now, nextPulseAt);
                     TCHECKPOINT;
                  }
                  TCHECKPOINT;
                  CallGetPulseTimeAux(*session, now, nextPulseAt);
                  TCHECKPOINT;
               }
            }
         }

         TCHECKPOINT;
         CallGetPulseTimeAux(*this, now, nextPulseAt);
         TCHECKPOINT;

         // Set up the Session IO Policies
         if (policies.HasItems())
         {
            // Now that the policies know *who* amongst their policyholders will be reading/writing,
            // let's ask each activated policy *how much* each policyholder should be allowed to read/write.
            {
               AbstractReflectSessionRef * nextRef;
               HashtableIterator<const String *, AbstractReflectSessionRef> iter(GetSessions());
               while((nextRef = iter.GetNextValue()) != NULL)
               {
                  AbstractReflectSession * session = nextRef->GetItemPointer();
                  if (session)
                  {
                     AbstractSessionIOPolicy * inPolicy  = session->GetInputPolicy()();
                     AbstractSessionIOPolicy * outPolicy = session->GetOutputPolicy()();
                     if ((inPolicy)&&( session->_maxInputChunk  > 0)) session->_maxInputChunk  = inPolicy->GetMaxTransferChunkSize(PolicyHolder(session, true));
                     if ((outPolicy)&&(session->_maxOutputChunk > 0)) session->_maxOutputChunk = outPolicy->GetMaxTransferChunkSize(PolicyHolder(session, false));
                  }
               }
            }

            // Now that all is prepared, calculate all the policies' wakeup times
            {
               TCHECKPOINT;
               HashtableIterator<AbstractSessionIOPolicyRef, bool> iter(policies);
               const AbstractSessionIOPolicyRef * next;
               while((next = iter.GetNextKey()) != NULL) CallGetPulseTimeAux(*(next->GetItemPointer()), now, nextPulseAt);
               TCHECKPOINT;
            }
         }
      }

      TCHECKPOINT;

      // This block is the center of the MUSCLE server's universe -- where we wait for the next event, inside select()
      {
         // Calculate timeout value for the next call to Pulse() (if any)
         struct timeval waitTime;
         if (nextPulseAt != MUSCLE_TIME_NEVER) 
         {
            uint64 now = GetRunTime64();
            uint64 waitTime64 = (nextPulseAt > now) ? (nextPulseAt - now) : 0;
            Convert64ToTimeVal(waitTime64, waitTime);
         }

#ifdef WIN32
         if (maxSocket < 0)
         {
            TCHECKPOINT;

            // Stupid Win32 can't handle it when all three fd_sets are empty!
            // So we have to do some special-case logic for that scenario here
            Sleep((nextPulseAt == MUSCLE_TIME_NEVER) ? INFINITE : ((waitTime.tv_sec*1000)+(waitTime.tv_usec/1000)));
         }
         else
#endif
         {
            TCHECKPOINT;

            // We sleep here until the next I/O or pulse event becomes ready
            int selResult = select(maxSocket+1, (maxSocket >= 0) ? &readSet : NULL, (maxSocket >= 0) ? &writeSet : NULL, exceptionSetPtr, (nextPulseAt == MUSCLE_TIME_NEVER) ? NULL : &waitTime);
            if (selResult < 0)
            {
               if (PreviousOperationWasInterrupted()) 
               {
                  // If select() was interrupted, then assume nothing is ready
                  FD_ZERO(&readSet);
                  FD_ZERO(&writeSet);
                  if (exceptionSetPtr) FD_ZERO(exceptionSetPtr);
               }
               else 
               {
                  if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "select() failed, aborting!\n");
                  ClearLameDucks();
                  return B_ERROR;
               }
            }
         }
      }

      // Each event-loop cycle officially "starts" as soon as select() returns
      CallSetCycleStartTime(*this, GetRunTime64());

      TCHECKPOINT;

      // Before we do any session I/O, make sure there hasn't been a generalized memory failure
      CheckForOutOfMemory(AbstractReflectSessionRef());

      TCHECKPOINT;

      // Do I/O for each of our attached sessions
      {
         AbstractReflectSessionRef sessionRef;
         HashtableIterator<const String *, AbstractReflectSessionRef> iter(GetSessions());
         while(iter.GetNextValue(sessionRef) == B_NO_ERROR)
         {
            TCHECKPOINT;

            AbstractReflectSession * session = sessionRef();
            if (session)
            {
               if (_watchMemUsage) (void) _watchMemUsage->SetAllocationHasFailed(false);  // (session)'s responsibility for starts here!  If we run out of mem on his watch, he's history

               TCHECKPOINT;

               CallSetCycleStartTime(*session, GetRunTime64());
               TCHECKPOINT;
               CallPulseAux(*session, session->GetCycleStartTime());
               {
                  AbstractMessageIOGateway * gateway = session->GetGateway()();
                  if (gateway) 
                  {
                     TCHECKPOINT;

                     CallSetCycleStartTime(*gateway, GetRunTime64());
                     CallPulseAux(*gateway, gateway->GetCycleStartTime());
                  }
               }

               TCHECKPOINT;

               int sock = session->GetSessionSelectSocket().GetFileDescriptor();
               if (sock >= 0)
               {
                  int32 readBytes = 0, wroteBytes = 0;
                  if (FD_ISSET(sock, &readSet))
                  {
                     readBytes = session->DoInput(*session, session->_maxInputChunk);  // session->MessageReceivedFromGateway() gets called here
                     AbstractSessionIOPolicy * p = session->GetInputPolicy()();
                     if ((p)&&(readBytes >= 0)) p->BytesTransferred(PolicyHolder(session, true), (uint32)readBytes);
                  }

                  TCHECKPOINT;

                  if (FD_ISSET(sock, &writeSet))
                  {
                     if (session->_connectingAsync) wroteBytes = (FinalizeAsyncConnect(sessionRef) == B_NO_ERROR) ? 0 : -1;
                     else
                     {
#ifdef MUSCLE_MAX_OUTPUT_CHUNK
                        // HACK: Certain BeOS configurations (i.e. behind routers??) don't handle large outgoing send()'s very
                        // well.  If this is the case with your machine, one easy fix is to put -DMUSCLE_MAX_OUTPUT_CHUNK=1000
                        // or so in your Makefile; that will force all send sizes to be 1000 bytes or less.
                        // (shatty reports the problem with a net_server -> Ethernet -> linksys -> pppoe -> dsl config)
                        if (session->_maxOutputChunk > MUSCLE_MAX_OUTPUT_CHUNK) session->_maxOutputChunk = MUSCLE_MAX_OUTPUT_CHUNK;
#endif
                        
                        // if the session's DataIO object is still has bytes buffered for output, try to send them now
                        {
                           AbstractMessageIOGateway * g = session->GetGateway()();
                           if (g)
                           {
                              DataIO * io = g->GetDataIO()();
                              if (io) io->WriteBufferedOutput();
                           }
                        }

                        wroteBytes = session->DoOutput(session->_maxOutputChunk);

                        AbstractSessionIOPolicy * p = session->GetOutputPolicy()();
                        if ((p)&&(wroteBytes >= 0)) p->BytesTransferred(PolicyHolder(session, false), (uint32)wroteBytes);
                     }
                  }
#ifdef WIN32
                  if ((exceptionSetPtr)&&(FD_ISSET(sock, exceptionSetPtr))) wroteBytes = -1;  // async connect() failed!
#endif

                  TCHECKPOINT;

                  if ((readBytes < 0)||(wroteBytes < 0))
                  {
                     bool wasConnecting = session->_connectingAsync;
                     if ((DisconnectSession(session) == false)&&(_doLogging)) LogTime(MUSCLE_LOG_DEBUG, "Connection for %s %s.\n", session->GetSessionDescriptionString()(), wasConnecting?"failed":"was severed");
                  }
                  else if (session->_lastByteOutputAt > 0)
                  {
                     // Check for output stalls
                     const uint64 now = GetRunTime64();
                          if ((wroteBytes > 0)||(session->_maxOutputChunk == 0)) session->_lastByteOutputAt = now;  // reset the moribundness-timer
                     else if (now-session->_lastByteOutputAt > session->_outputStallLimit)
                     {
                        if (_doLogging) LogTime(MUSCLE_LOG_WARNING, "Connection for %s timed out (output stall, no data movement for "UINT64_FORMAT_SPEC" seconds).\n", session->GetSessionDescriptionString()(), (session->_outputStallLimit/1000000));
                        (void) DisconnectSession(session);
                     }
                  }
               }
            }
            TCHECKPOINT;
            CheckForOutOfMemory(sessionRef);  // if the session caused a memory error, give him the boot
         }
      }

      TCHECKPOINT;

      // Pulse() our other PulseNode objects, as necessary
      {
         // Tell the session policies we're done doing I/O (for now)
         if (policies.HasItems())
         {
            const AbstractSessionIOPolicyRef * next;
            HashtableIterator<AbstractSessionIOPolicyRef, bool> iter(policies);
            while((next = iter.GetNextKey()) != NULL) 
            {
               AbstractSessionIOPolicy * p = next->GetItemPointer();
               if (p->_hasBegun)
               {
                  p->EndIO(GetRunTime64());
                  p->_hasBegun = false;
               }
            }
         }

         // Pulse the Policies
         if (policies.HasItems())
         {
            const AbstractSessionIOPolicyRef * next;
            HashtableIterator<AbstractSessionIOPolicyRef, bool> iter(policies);
            while((next = iter.GetNextKey()) != NULL) CallPulseAux(*(next->GetItemPointer()), GetRunTime64());
         }

         // Pulse the Server
         CallPulseAux(*this, GetRunTime64());
      }
      policies.Clear();

      TCHECKPOINT;

      // Lastly, check our accepting ports to see if anyone is trying to connect...
      if (_factories.HasItems())
      {
         HashtableIterator<IPAddressAndPort, ReflectSessionFactoryRef> iter(_factories);
         const IPAddressAndPort * iap;
         ReflectSessionFactoryRef * fref;
         while(iter.GetNextKeyAndValue(iap, fref) == B_NO_ERROR)
         {
            ReflectSessionFactory * factory = fref->GetItemPointer();
            CallSetCycleStartTime(*factory, GetRunTime64());
            CallPulseAux(*factory, factory->GetCycleStartTime());

            if (factory->IsReadyToAcceptSessions())
            {
               ConstSocketRef * as = _factorySockets.Get(*iap);
               int fd = as ? as->GetFileDescriptor() : -1;
               if ((fd >= 0)&&(FD_ISSET(fd, &readSet))) (void) DoAccept(*iap, *as, factory);
            }
         }
      }

      TCHECKPOINT;
   }

   TCHECKPOINT;

   (void) ClearLameDucks();  // get rid of any leftover ducks

   TCHECKPOINT;
   return B_NO_ERROR;
}

void ReflectServer :: ShutdownIOFor(AbstractReflectSession * session)
{
   AbstractMessageIOGateway * gw = session->GetGateway()();
   if (gw) 
   {
      DataIO * io = gw->GetDataIO()();
      if (io) io->Shutdown();  // so we won't try to do I/O on this one anymore
   }
}

status_t ReflectServer :: ClearLameDucks()
{
   // Delete any factories that were previously marked for deletion
   _lameDuckFactories.Clear();

   // Remove any sessions that were previously marked for removal
   AbstractReflectSessionRef duckRef;
   while(_lameDuckSessions.RemoveHead(duckRef) == B_NO_ERROR)
   {
      AbstractReflectSession * duck = duckRef();
      if (duck)
      {
         const String & id = duck->GetSessionIDString();
         if (_sessions.ContainsKey(&id))
         {
            duck->AboutToDetachFromServer();
            duck->DoOutput(MUSCLE_NO_LIMIT);  // one last chance for him to send any leftover data!
            if (_doLogging) LogTime(MUSCLE_LOG_DEBUG, "Closed %s ("UINT32_FORMAT_SPEC" left)\n", duck->GetSessionDescriptionString()(), _sessions.GetNumItems()-1);
            duck->SetOwner(NULL);
            (void) _sessions.Remove(&id);
         }
      }
   }

   return _keepServerGoing ? B_NO_ERROR : B_ERROR;
}

void ReflectServer :: LogAcceptFailed(int lvl, const char * desc, const char * ipbuf, const IPAddressAndPort & iap)
{
   if (_doLogging)
   {
      if (iap.GetIPAddress() == invalidIP) LogTime(lvl, "%s for [%s] on port %u.\n", desc, ipbuf?ipbuf:"???", iap.GetPort());
                                      else LogTime(lvl, "%s for [%s] on port %u on interface [%s].\n", desc, ipbuf?ipbuf:"???", iap.GetPort(), Inet_NtoA(iap.GetIPAddress())());
   }
}

status_t ReflectServer :: DoAccept(const IPAddressAndPort & iap, const ConstSocketRef & acceptSocket, ReflectSessionFactory * optFactory)
{
   // Accept a new connection and try to start up a session for it
   ip_address acceptedFromIP;
   ConstSocketRef newSocket = Accept(acceptSocket, &acceptedFromIP);
   if (newSocket())
   {
      IPAddressAndPort nip(acceptedFromIP, iap.GetPort());
      ip_address remoteIP = GetPeerIPAddress(newSocket, true);
      if (remoteIP == invalidIP) LogAcceptFailed(MUSCLE_LOG_DEBUG, "GetPeerIPAddress() failed", NULL, nip);
      else
      {
         char ipbuf[64]; Inet_NtoA(remoteIP, ipbuf);

         AbstractReflectSessionRef newSessionRef;
         if (optFactory) newSessionRef = optFactory->CreateSession(ipbuf, nip);

         if (newSessionRef()) 
         {
            newSessionRef()->_ipAddressAndPort = iap;
            newSessionRef()->_isConnected      = true;
            if (AddNewSession(newSessionRef, newSocket) == B_NO_ERROR) 
            {
               newSessionRef()->_wasConnected = true;   
               return B_NO_ERROR;  // success!
            }
            else
            {
               newSessionRef()->_isConnected = false;   
               newSessionRef()->_ipAddressAndPort.Reset();
            }
         }
         else if (optFactory) LogAcceptFailed(MUSCLE_LOG_DEBUG, "Session creation denied", ipbuf, nip);
      }
   }
   else LogAcceptFailed(MUSCLE_LOG_DEBUG, "Accept() failed", NULL, iap);

   return B_ERROR;
}

void ReflectServer :: DumpBoggedSessions()
{
   TCHECKPOINT;

   // New for v1.82:  also find anyone whose outgoing message queue is getting too large.
   //                 (where "too large", for now, is more than 5 megabytes)
   // This could happen if someone has a really slow Internet connection, or has decided to
   // stop reading from their end indefinitely for some reason.
   const int MAX_MEGABYTES = 5;
   AbstractReflectSessionRef * lRef;
   HashtableIterator<const String *, AbstractReflectSessionRef> xiter(GetSessions());
   while((lRef = xiter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * asr = lRef->GetItemPointer();
      if (asr)
      {
         AbstractMessageIOGateway * gw = asr->GetGateway()();
         if (gw)
         {
            uint32 qSize = 0;
            const Queue<MessageRef> & q = gw->GetOutgoingMessageQueue();
            for (int k=q.GetNumItems()-1; k>=0; k--)
            {
               const Message * qmsg = q[k]();
               if (qmsg) qSize += qmsg->FlattenedSize();
            }
            if (qSize > MAX_MEGABYTES*1024*1024)
            {
               if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Low Memory!  Aborting %s to get some back!\n", asr->GetSessionDescriptionString()());
               AddLameDuckSession(*lRef);
            }
         }
      }
   }
}

AbstractReflectSessionRef
ReflectServer ::
GetSession(const String & name) const
{
   AbstractReflectSessionRef ref;
   (void) _sessions.Get(&name, ref); 
   return ref;
}

AbstractReflectSessionRef
ReflectServer ::
GetSession(uint32 id) const
{
   char buf[64]; sprintf(buf, UINT32_FORMAT_SPEC, id);
   return GetSession(buf);
}

ReflectSessionFactoryRef
ReflectServer ::
GetFactory(uint16 port, const ip_address & optInterfaceIP) const
{
   ReflectSessionFactoryRef ref;
   (void) _factories.Get(IPAddressAndPort(optInterfaceIP, port), ref); 
   return ref;
}

status_t
ReflectServer ::
ReplaceSession(const AbstractReflectSessionRef & newSessionRef, AbstractReflectSession * oldSession)
{
   TCHECKPOINT;

   // move the gateway from the old session to the new one...
   AbstractReflectSession * newSession = newSessionRef();
   if (newSession == NULL) return B_ERROR;

   newSession->SetGateway(oldSession->GetGateway());
   newSession->_hostName = oldSession->_hostName;
   newSession->_ipAddressAndPort = oldSession->_ipAddressAndPort;

   if (AttachNewSession(newSessionRef) == B_NO_ERROR)
   {
      oldSession->SetGateway(AbstractMessageIOGatewayRef());   /* gateway now belongs to newSession */
      EndSession(oldSession);
      return B_NO_ERROR;
   }
   else
   {
       // Oops, rollback changes and error out
       newSession->SetGateway(AbstractMessageIOGatewayRef());
       newSession->_hostName.Clear();
       newSession->_ipAddressAndPort.Reset();
       return B_ERROR;
   }
}

bool ReflectServer :: DisconnectSession(AbstractReflectSession * session)
{
   session->_isConnected = session->_connectingAsync = false;  // he's not connecting anymore, by gum!
   session->_scratchReconnected = false;  // if the session calls Reconnect() this will be set to true below

   AbstractMessageIOGateway * oldGW = session->GetGateway()();
   DataIO * oldIO = oldGW ? oldGW->GetDataIO()() : NULL;

   bool ret = session->ClientConnectionClosed(); 

   AbstractMessageIOGateway * newGW = session->GetGateway()();
   DataIO * newIO = newGW ? newGW->GetDataIO()() : NULL;

        if (ret) AddLameDuckSession(session);
   else if ((session->_scratchReconnected == false)&&(newGW == oldGW)&&(newIO == oldIO)) ShutdownIOFor(session);

   return ret;
}

void
ReflectServer ::
EndSession(AbstractReflectSession * who)
{
   AddLameDuckSession(who);
}

void
ReflectServer ::
EndServer()
{
   _keepServerGoing = false;
}

status_t
ReflectServer ::
PutAcceptFactory(uint16 port, const ReflectSessionFactoryRef & factoryRef, const ip_address & optInterfaceIP, uint16 * optRetPort)
{
   if (port > 0) (void) RemoveAcceptFactory(port, optInterfaceIP); // Get rid of any previous acceptor on this port...

   ReflectSessionFactory * f = factoryRef();
   if (f)
   {
      ConstSocketRef acceptSocket = CreateAcceptingSocket(port, 20, &port, optInterfaceIP);
      if (acceptSocket())
      {
         IPAddressAndPort iap(optInterfaceIP, port);
         if ((SetSocketBlockingEnabled(acceptSocket, false) == B_NO_ERROR)&&(_factories.Put(iap, factoryRef) == B_NO_ERROR))
         {
            if (_factorySockets.Put(iap, acceptSocket) == B_NO_ERROR)
            {
               f->SetOwner(this);
               if (optRetPort) *optRetPort = port;
               if (f->AttachedToServer() == B_NO_ERROR) return B_NO_ERROR;
               else
               {
                  (void) RemoveAcceptFactory(port, optInterfaceIP);
                  return B_ERROR;
               }
            }
            else _factories.Remove(iap); // roll back!
         }
      }
   }
   return B_ERROR;
}

status_t
ReflectServer ::
RemoveAcceptFactoryAux(const IPAddressAndPort & iap)
{
   ReflectSessionFactoryRef ref;
   if (_factories.Get(iap, ref) == B_NO_ERROR)  // don't remove it yet, in case AboutToDetachFromServer() calls a method on us
   {
      ref()->AboutToDetachFromServer();
      _lameDuckFactories.AddTail(ref);  // we'll actually have (factory) deleted later on, since at the moment 
                                        // we could be in the middle of one of (ref())'s own method calls!
      (void) _factories.Remove(iap);  // must call this AFTER the AboutToDetachFromServer() call!

      // See if there are any other instances of this factory still present.
      // if there aren't, we'll clear the factory's owner-pointer so he can't access us anymore
      if (_factories.IndexOfValue(ref) < 0) ref()->SetOwner(NULL);

      (void) _factorySockets.Remove(iap);

      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t
ReflectServer ::
RemoveAcceptFactory(uint16 port, const ip_address & optInterfaceIP)
{
   if (port > 0) return RemoveAcceptFactoryAux(IPAddressAndPort(optInterfaceIP, port));
   else
   {
      HashtableIterator<IPAddressAndPort, ReflectSessionFactoryRef> iter(_factories);
      const IPAddressAndPort * nextKey;
      while(iter.GetNextKey(nextKey) == B_NO_ERROR) (void) RemoveAcceptFactoryAux(*nextKey);
      return B_NO_ERROR;
   }
}

status_t 
ReflectServer :: 
FinalizeAsyncConnect(const AbstractReflectSessionRef & ref)
{
   AbstractReflectSession * session = ref();
   if ((session)&&(muscle::FinalizeAsyncConnect(session->GetSessionSelectSocket()) == B_NO_ERROR))
   {
      session->_connectingAsync = false;  // we're legit now!  :^)
      session->_isConnected = session->_wasConnected = true;
      session->AsyncConnectCompleted();
      return B_NO_ERROR;
   }
   return B_ERROR;
}

uint64
ReflectServer ::
GetServerUptime() const
{
   uint64 now = GetRunTime64();
   return (_serverStartedAt <= now) ? (now - _serverStartedAt) : 0;
}
                        
uint32 
ReflectServer ::
GetNumAvailableBytes() const
{
   return (_watchMemUsage) ? (uint32)_watchMemUsage->GetNumAvailableBytes(GetNumUsedBytes()) : MUSCLE_NO_LIMIT;
}
 
uint32 
ReflectServer ::
GetMaxNumBytes() const
{
   return _watchMemUsage ? (uint32)_watchMemUsage->GetMaxNumBytes() : MUSCLE_NO_LIMIT;
}
 
uint32 
ReflectServer ::
GetNumUsedBytes() const
{
#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
   return (uint32) GetNumAllocatedBytes();
#else
   return 0;  // if we're not tracking, there is no way to know!
#endif
}

void
ReflectServer :: AddLameDuckSession(const AbstractReflectSessionRef & ref)
{
   if ((_lameDuckSessions.IndexOf(ref) < 0)&&(_lameDuckSessions.AddTail(ref) != B_NO_ERROR)&&(_doLogging)) LogTime(MUSCLE_LOG_CRITICALERROR, "Server:  AddLameDuckSession() failed, I'm REALLY in trouble!  Aggh!\n");
}

void
ReflectServer ::
AddLameDuckSession(AbstractReflectSession * who)
{
   TCHECKPOINT;

   AbstractReflectSessionRef * lRef; 
   HashtableIterator<const String *, AbstractReflectSessionRef> xiter(GetSessions());
   while((lRef = xiter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * session = lRef->GetItemPointer();
      if (session == who)
      {
         AddLameDuckSession(*lRef);
         break;
      }
   }
}

}; // end namespace muscle
