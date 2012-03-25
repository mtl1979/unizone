#ifndef DetectNetworkConfigChangesSession_h
#define DetectNetworkConfigChangesSession_h

#include "reflector/AbstractReflectSession.h"

#ifndef __linux__
# include "system/Thread.h"  // For Linux we can just listen directly on an AF_NETLINK socket, so no thread is needed
#endif

#ifdef __APPLE__
# include <CoreFoundation/CoreFoundation.h>
#endif

namespace muscle {

/** This class watches the set of available network interfaces and calls its 
  * NetworkInterfacesChanged() virtual method when a network-configuration change 
  * has been detected.  The default implementation of NetworkInterfacesChanged() is a no-op,
  * so you will want to subclass this class and implement your own version of 
  * NetworkInterfacesChanged() that does something useful (like posting a log message,
  * or tearing down and recreating any sockets that relied on the old networking config).
  *  
  * Note that this functionality is currently implemented for Linux, Windows, and MacOS/X only.
  * Note also that the Windows and MacOS/X implementations currently make use of the MUSCLE
  * Thread class, and therefore won't compile if -DMUSCLE_SINGLE_THREAD_ONLY is set.
  * 
  * @see tests/testnetconfigdetect.cpp for an example usage of this class.
  */
class DetectNetworkConfigChangesSession : public AbstractReflectSession, private CountedObject<DetectNetworkConfigChangesSession>
#ifndef __linux__
   , private Thread
#endif
{
public:
   DetectNetworkConfigChangesSession();

#ifndef __linux__
   virtual status_t AttachedToServer();
   virtual void AboutToDetachFromServer();
   virtual void EndSession();
   virtual AbstractMessageIOGatewayRef CreateGateway();
#endif

   virtual void MessageReceivedFromGateway(const MessageRef & msg, void * userData);

   virtual ConstSocketRef CreateDefaultSocket();
   virtual const char * GetTypeName() const {return "DetectNetworkConfigChanges";}

   virtual uint64 GetPulseTime(const PulseArgs & args) {return muscleMin(_callbackTime, AbstractReflectSession::GetPulseTime(args));}
   virtual void Pulse(const PulseArgs & args);

#ifdef __linux__
   virtual int32 DoInput(AbstractGatewayMessageReceiver & r, uint32 maxBytes);
#endif

   /** This method can be called to disable or enable this session.
     * A disabled session will not call NetworkInterfacesChanged(), even if a network interface change is detected. 
     * The default state of this session is enabled.
     * @param e True to enable calling of NetworkInterfacesChanged() when appropriate, or false to disable it.
     */
   void SetEnabled(bool e) {_enabled = e;}

   /** Returns true iff the calling of NetworkInterfaceChanged() is enabled.  Default value is true. */
   bool IsEnabled() const {return _enabled;}

protected:
#ifndef __linux__
   /** Overridden to do the signalling the Carbon way */
   virtual void SignalInternalThread();
#endif

   /** Called when a change in the local interfaces set is detected.
     * Default implementation calls FindAppropriateNetworkInterfaceIndices()
     * to update the process's interface list.  Subclass can augment
     * that behavior to include update various other objects that
     * need to be notified of the change.
     */
   virtual void NetworkInterfacesChanged();

private:
   void ScheduleSendReport();

#ifndef __linux__
   friend void SignalInterfacesChanged(DetectNetworkConfigChangesSession * s);
   virtual void InternalThreadEntry();

   volatile bool _threadKeepGoing;
# ifdef __APPLE__
   CFRunLoopRef _threadRunLoop;
# elif _WIN32
   ::HANDLE _wakeupSignal;
# endif
#endif // __linux__

   uint64 _callbackTime;
   bool _enabled;
};

};  // end namespace muscle

#endif
