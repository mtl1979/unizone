/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleSignalMessageIOGateway_h
#define MuscleSignalMessageIOGateway_h

#include "iogateway/AbstractMessageIOGateway.h"

namespace muscle {

/** 
 * This gateway is simple almost to the point of being crippled... all it does
 * is read data from its socket, and whenever it has read some data, it
 * will add a user-specified MessageRef to its incoming Message queue.
 * It's useful primarily for thread synchronization purposes.
 */
class SignalMessageIOGateway : public AbstractMessageIOGateway
{
public:
   /** Constructor.  Creates a SignalMessageIOGateway with a NULL signal message reference.  
     */
   SignalMessageIOGateway() {/* empty */}

   /** Constructor
     * @param signalMessage The message to send out when we have read some incoming data.
     */
   SignalMessageIOGateway(MessageRef signalMessage) : _signalMessage(signalMessage) {/* empty */}

   /** Destructor */
   virtual ~SignalMessageIOGateway() {/* empty */}

   /** DoOutput is a no-op for this gateway... all messages are simply eaten and dropped. */
   virtual int32 DoOutput(uint32 maxBytes = MUSCLE_NO_LIMIT)
   {
      // Just eat and drop ... we don't really support outgoing messages
      while(GetOutgoingMessageQueue().RemoveHead() == B_NO_ERROR) {/* keep doing it */}
      return maxBytes;
   }

   /** Overridden to enqeue a (signalMessage) whenever data is read. */
   virtual int32 DoInput(uint32 maxBytes = MUSCLE_NO_LIMIT)
   {
      char buf[256];
      int32 bytesRead = GetDataIO()->Read(buf, muscleMin(maxBytes, (uint32)sizeof(buf)));
      if (bytesRead > 0) (void) GetIncomingMessageQueue().AddTail(_signalMessage);
      return bytesRead;
   }

   /** Always returns false. */
   virtual bool HasBytesToOutput() const {return false;}

   /** Returns a reference to our current signal message */
   MessageRef GetSignalMessage() const {return _signalMessage;}

   /** Sets our current signal message reference. */
   void SetSignalMessage(MessageRef r) {_signalMessage = r;}

private:
   MessageRef _signalMessage;
};

};  // end namespace muscle

#endif
