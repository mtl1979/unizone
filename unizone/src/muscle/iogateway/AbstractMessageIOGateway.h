/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleAbstractMessageIOGateway_h
#define MuscleAbstractMessageIOGateway_h

#include "dataio/DataIO.h"
#include "message/Message.h"
#include "util/Queue.h"
#include "util/RefCount.h"
#include "util/NetworkUtilityFunctions.h"
#include "util/PulseNode.h"

namespace muscle {

/**
 *  Abstract base class representing an object that can send/receive 
 *  Messages via a DataIO byte-stream.
 */
class AbstractMessageIOGateway : public RefCountable, public PulseNode
{
public:
   /** Default Constructor. */
   AbstractMessageIOGateway();

   /** Destructor. */
   virtual ~AbstractMessageIOGateway();

   /**
    * Adds the given message reference to our list of outgoing messages to send.  Never blocks.  
    * @param messageRef A reference to the Message to send out through the gateway.
    * @return B_NO_ERROR on success, B_ERROR iff for some reason the message can't be queued (out of memory?)
    */
   status_t AddOutgoingMessage(const MessageRef & messageRef);

   /**
    * Retrieves the next message from the incoming messages queue.
    * @param setRef On success, this reference will point to the retrieved Message.
    * @return B_NO_ERROR on success, or B_ERROR iff the queue is empty.
    */
   status_t GetNextIncomingMessage(MessageRef & setRef);

   /**
    * Writes some of our outgoing message bytes to the wire.
    * Not guaranteed to write all outgoing messages (it will try not to block)
    * @param maxBytes optional limit on the number of bytes that should be sent out.
    *                 Defaults to MUSCLE_NO_LIMIT (which is a very large number)
    * @return The number of bytes written, or a negative value if the connection has been broken
    *         or some other catastrophic condition has occurred.
    */
   virtual int32 DoOutput(uint32 maxBytes = MUSCLE_NO_LIMIT) = 0;

   /**
    * Reads some more incoming message bytes from the wire.  
    * If enough bytes have been read to assemble one or more new Messages,
    * the new Messages can be retrieved after this method returns by
    * calling GetNextIncomingMessage().  
    * @param maxBytes optional limit on the number of bytes that should be read in.
    *                 Defaults to MUSCLE_NO_LIMIT (which is a very large number)
    * Tries not to block, but may (depending on implementation)
    * @return The number of bytes read, or a negative value if the connection has been broken
    *         or some other catastrophic condition has occurred.
    */
   virtual int32 DoInput(uint32 maxBytes = MUSCLE_NO_LIMIT) = 0;

   /**
    * Should return true if this gateway is willing to receive from bytes
    * from the wire.  Should return false if (for some reason) the gateway
    * doesn't want to read any bytes right now.  The default implementation
    * of this method always returns true.
    */
   virtual bool IsReadyForInput() const;

   /**
    * Should return true if this gateway has bytes that are queued up 
    * and waiting to be sent across the wire.  Should return false if
    * there are no bytes ready to send, or if the connection has been 
    * closed or hosed.
    */
   virtual bool HasBytesToOutput() const = 0;

   /** Returns the number of microseconds that output to this gateway's
    *  client should be allowed to stall for.  If the output stalls for
    *  longer than this amount of time, the connection will be closed.
    *  Return MUSCLE_TIME_NEVER to disable stall limit checking.  
    *  Default behaviour is to forward this call to the held DataIO object.
    */
   virtual uint64 GetOutputStallLimit() const;

   /** Shuts down the gateway.  Default implementation calls Shutdown() on
     * the held DataIO object.
     */
   virtual void Shutdown();

   /**
    * Returns true iff there are any incoming Messages ready to
    * be retrieved via GetNextIncomingMessage().
    */
   bool HasIncomingMessagesReady() const;

   /**
    * By default, the AbstractMessageIOGateway calls Flush() on its DataIO's
    * output stream whenever the last outgoing message in the outgoing message queue 
    * is sent.  Call SetFlushOnEmpty(false) to inhibit this behavior (e.g. for bandwidth
    * efficiency when low message latency is not a requirement).
    * @param flush If true, auto-flushing will be enabled.  If false, it will be disabled.
    */
   void SetFlushOnEmpty(bool flush);

   /** Accessor for the current state of the FlushOnEmpty flag */
   bool GetFlushOnEmpty() const {return _flushOnEmpty;}

   /** Returns A reference to our outgoing messages queue.  */
   Queue<MessageRef> & GetOutgoingMessageQueue() {return _outgoingMessages;}

   /** Returns A reference to our incoming messages queue.  */
   Queue<MessageRef> & GetIncomingMessageQueue() {return _incomingMessages;}

   /** Returns A const reference to our outgoing messages queue.  */
   const Queue<MessageRef> & GetOutgoingMessageQueue() const {return _outgoingMessages;}

   /** Returns A const reference to our incoming messages queue.  */
   const Queue<MessageRef> & GetIncomingMessageQueue() const {return _incomingMessages;}

   /** Installs (ref) as the DataIO object we will use for our I/O.
     * Typically called by the ReflectServer object.
     */
   void SetDataIO(DataIORef ref) {_ioRef = ref;}

protected:
   /**
    * Convenience method to allocate or reallocate a buffer.  When this method returns
    * successfully, (*bufPtr) will point to a buffer that is at least (desiredSize) bytes long.
    * @param bufPtr points to a pointer to the buffer.  (May point to a NULL
    *               pointer if no buffer is currently allocated)
    * @param bufSize points to the current size of the buffer.  On return, this value is changed to
    *                reflect the new buffer size.
    * @param desiredSize indicates the minimum new buffer size required.
    * @param copySize indicates the number of bytes to copy out of the
    *                 old buffer and into the new one, if a reallocation is necessary.
    * @return B_NO_ERROR on success, B_ERROR on failure (i.e. out of memory).
    */
   status_t EnsureBufferSize(uint8 ** bufPtr, uint32 * bufSize, uint32 desiredSize, uint32 copySize);

   /**
    * Convenience method; frees the given buffer and resets it to NULL, if the buffer is greater
    * than 10 kilobytes big.  (Buffers smaller than that are not effected, since it's probably cheaper
    * to keep them around and avoid having to deallocate and reallocate them all the time).
    * @param bufPtr points to a pointer to the buffer.  (May point to a NULL pointer if
    *               no buffer is currently allocated).  The pointer may be set to NULL if the buffer was freed.
    * @param bufSize points to the size-value of the buffer.  May be set to 0 if the buffer was freed.
    */
   void FreeLargeBuffer(uint8 ** bufPtr, uint32 * bufSize);

   /** Accessor for our held DataIO object */
   DataIO * GetDataIO() const {return _ioRef();}

   /** As above, but returns a reference instead of the raw pointer. */
   DataIORef GetDataIORef() const {return _ioRef;}

   /** Returns true iff we are hosed--that is, we've experienced an unrecoverable error. */
   bool IsHosed() const {return _hosed;}

   /** Call this method to flag this gateway as hosed--that is, to say that an unrecoverable error has occurred. */
   void SetHosed() {_hosed = true;}
  
private:
   Queue<MessageRef> _outgoingMessages;
   Queue<MessageRef> _incomingMessages;

   DataIORef _ioRef;

   bool _hosed;  // set true on error
   bool _flushOnEmpty;

   friend class ReflectServer;
};

typedef Ref<AbstractMessageIOGateway> AbstractMessageIOGatewayRef;

};  // end namespace muscle

#endif
