/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "reflector/AbstractReflectSession.h"
#include "system/SignalMultiplexer.h"

namespace muscle {

/** This session can be added to a ReflectServer in order to have the server
  * catch signals (e.g. SIGINT on Unix/MacOS, Console signals on Windows)
  * and react by initiating a controlled shutdown of the server.
  */
class SignalHandlerSession : public AbstractReflectSession, public ISignalHandler
{
public:
   /** Default constructor. */
   SignalHandlerSession() {/* empty */}
   virtual ~SignalHandlerSession() {/* empty */}

   virtual ConstSocketRef CreateDefaultSocket();
   virtual int32 DoInput(AbstractGatewayMessageReceiver &, uint32);
   virtual void MessageReceivedFromGateway(const MessageRef &, void *) {/* empty */}
   virtual status_t AttachedToServer();
   virtual void AboutToDetachFromServer();
   virtual const char * GetTypeName() const {return "SignalHandler";}
   virtual void SignalHandlerFunc(int whichSignal);

protected:
   /** This method is called in the main thread whenever a signal is received.
     * @param whichSignal the number of the signal received, as provided by the OS.
     *                    On POSIX OS's this might be SIGINT or SIGHUP; under Windows
     *                    it would be something like CTRL_CLOSE_EVENT or CTRL_LOGOFF_EVENT.
     * Default behavior is to always just call EndServer() so that the server process
     * will exit cleanly as soon as possible.
     */
   virtual void SignalReceived(int whichSignal);

private:
   ConstSocketRef _handlerSocket;
};

}; // end namespace muscle
