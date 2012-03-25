/* This file is Copyright 2000-2011 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "iogateway/MessageIOGateway.h"
#include "reflector/StorageReflectConstants.h"  // for PR_COMMAND_PING, PR_RESULT_PONG
#include "dataio/TCPSocketDataIO.h"

namespace muscle {

MessageIOGateway :: MessageIOGateway(int32 encoding) :
   _maxIncomingMessageSize(MUSCLE_NO_LIMIT),
   _outgoingEncoding(encoding), 
   _aboutToFlattenCallback(NULL), _aboutToFlattenCallbackData(NULL),
   _flattenedCallback(NULL), _flattenedCallbackData(NULL),
   _unflattenedCallback(NULL), _unflattenedCallbackData(NULL)
#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
   , _sendCodec(NULL), _recvCodec(NULL)
#endif
   , _syncPingCounter(0), _pendingSyncPingCounter(-1)
{
   /* empty */
}

MessageIOGateway :: ~MessageIOGateway() 
{
#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
   delete _sendCodec;
   delete _recvCodec;
#endif
}

// For this method, B_NO_ERROR means "keep sending", and B_ERROR means "stop sending for now", and isn't fatal to the stream
// If there is a fatal error in the stream it will call SetHosed() to indicate that.
status_t 
MessageIOGateway :: SendData(TransferBuffer & buf, int32 & sentBytes, uint32 & maxBytes)
{
   TCHECKPOINT;

   const ByteBuffer * bb = buf._buffer();
   if (bb)
   {
      const uint8 * bytes = bb->GetBuffer();
      if (bytes)
      {
         int32 attemptSize = muscleMin(maxBytes, bb->GetNumBytes()-buf._offset);
         int32 numSent     = GetDataIO()()->Write(&bytes[buf._offset], attemptSize);

         if (numSent >= 0)
         {
            maxBytes    -= numSent;
            sentBytes   += numSent;
            buf._offset += numSent;
         }
         else SetHosed();

         if (buf._offset >= bb->GetNumBytes()) buf.Reset();  // when we've sent it all, forget about it
         if (numSent < attemptSize) return B_ERROR;  // assume this means buffers are full; we'll send the rest later
      }
      else buf.Reset();
   }
   return B_NO_ERROR;
}

status_t
MessageIOGateway ::
PopNextOutgoingMessage(MessageRef & retMsg)
{
   return GetOutgoingMessageQueue().RemoveHead(retMsg);
}

int32 
MessageIOGateway ::
DoOutputImplementation(uint32 maxBytes)
{
   TCHECKPOINT;

   int32 sentBytes = 0;
   while((maxBytes > 0)&&(IsHosed() == false))
   {
      // First, make sure our outgoing byte-buffer has data.  If it doesn't, fill it with the next outgoing message.
      if (_sendBodyBuffer._buffer() == NULL)
      {
         while(true)
         {
            MessageRef nextRef;
            if (PopNextOutgoingMessage(nextRef) != B_NO_ERROR) 
            {
               if ((GetFlushOnEmpty())&&(sentBytes > 0)) GetDataIO()()->FlushOutput();
               return sentBytes;  // nothing more to send, so we're done!
            }

            const Message * nextSendMsg = nextRef();
            if (nextSendMsg)
            {
               if (_aboutToFlattenCallback) _aboutToFlattenCallback(nextRef, _aboutToFlattenCallbackData);

               _sendHeaderBuffer._offset = 0;
               _sendHeaderBuffer._buffer = GetByteBufferFromPool(GetHeaderSize());
               if (_sendHeaderBuffer._buffer() == NULL) {SetHosed(); break;}
               
               _sendBodyBuffer._offset = 0;
               _sendBodyBuffer._buffer = FlattenMessage(nextRef, _sendHeaderBuffer._buffer()->GetBuffer());
               if (_sendBodyBuffer._buffer() == NULL) {SetHosed(); break;}

               if (_flattenedCallback) _flattenedCallback(nextRef, _flattenedCallbackData);

               break;  // now go on to the sending phase
            }
         }
      }

      if (IsHosed() == false)
      {
         if ((_sendHeaderBuffer._buffer() != NULL)&&(SendData(_sendHeaderBuffer, sentBytes, maxBytes) != B_NO_ERROR)) break;
         if ((_sendHeaderBuffer._buffer() == NULL)&&(SendData(_sendBodyBuffer,   sentBytes, maxBytes) != B_NO_ERROR)) break;
      }
   }
   return IsHosed() ? -1 : sentBytes;
}

int32 
MessageIOGateway ::
DoInputImplementation(AbstractGatewayMessageReceiver & receiver, uint32 maxBytes)
{
   TCHECKPOINT;

   bool firstTime = true;  // always go at least once, to avoid live-lock
   int32 readBytes = 0;
   while((maxBytes > 0)&&(IsHosed() == false)&&((firstTime)||(IsSuggestedTimeSliceExpired() == false)))
   {
      firstTime = false;

      if (_recvHeaderBuffer._buffer() == NULL)
      {
         _recvHeaderBuffer._offset = 0;
         _recvHeaderBuffer._buffer = GetByteBufferFromPool(GetHeaderSize());
         if (_recvHeaderBuffer._buffer() == NULL) {SetHosed(); break;}
      }

      ByteBuffer * rHead = _recvHeaderBuffer._buffer();  // guaranteed not to be NULL!
      if ((_recvHeaderBuffer._offset < rHead->GetNumBytes())&&(ReadData(_recvHeaderBuffer, readBytes, maxBytes) != B_NO_ERROR)) break;
      if (_recvHeaderBuffer._offset == rHead->GetNumBytes())
      {
         ByteBuffer * rBody = _recvBodyBuffer._buffer();
         if (rBody)
         {
            if ((_recvBodyBuffer._offset < rBody->GetNumBytes())&&(ReadData(_recvBodyBuffer, readBytes, maxBytes) != B_NO_ERROR)) break;
            if (_recvBodyBuffer._offset == rBody->GetNumBytes())
            {
               // Finished receiving message bytes... now reconstruct that bad boy!
               MessageRef msg = UnflattenMessage(_recvBodyBuffer._buffer, _recvHeaderBuffer._buffer()->GetBuffer());
               _recvHeaderBuffer.Reset();  // reset our state for the next one!
               _recvBodyBuffer.Reset();
               if (msg() == NULL) {SetHosed(); break;}
               if (_unflattenedCallback) _unflattenedCallback(msg, _unflattenedCallbackData);
               receiver.CallMessageReceivedFromGateway(msg);
            }
         }
         else
         {
            // Now that we have the header, allocate space for the body bytes based on the info contained in the header
            int32 bodySize = GetBodySize(_recvHeaderBuffer._buffer()->GetBuffer());
            if ((bodySize >= 0)&&(((uint32)bodySize) <= _maxIncomingMessageSize))
            {
               _recvBodyBuffer._offset = 0;
               _recvBodyBuffer._buffer = GetByteBufferFromPool(bodySize);
               if (_recvBodyBuffer._buffer() == NULL) {SetHosed(); break;}
            }
            else {SetHosed(); break;}
         }
      }
   }
   return IsHosed() ? -1 : readBytes;
}
      
// For this method, B_NO_ERROR means "keep sending", and B_ERROR means "stop sending for now", and isn't fatal to the stream
// If there is a fatal error in the stream it will call SetHosed() to indicate that.
status_t 
MessageIOGateway :: ReadData(TransferBuffer & buf, int32 & readBytes, uint32 & maxBytes)
{
   TCHECKPOINT;

   ByteBuffer * bb = buf._buffer();
   if (bb)
   {
      uint8 * bytes = bb->GetBuffer();
      if (bytes)
      {
         int32 attemptSize = muscleMin(maxBytes, bb->GetNumBytes()-buf._offset);
         int32 numRead     = GetDataIO()()->Read(&bytes[buf._offset], attemptSize);

         if (numRead >= 0)
         {
            maxBytes    -= numRead;
            readBytes   += numRead;
            buf._offset += numRead;
         }
         else SetHosed();

         if (numRead < attemptSize) return B_ERROR;  // assume this means buffers are empty; we'll read the rest later
      }
   }
   return B_NO_ERROR;  // no buffer:  try to get the next one from the Queue.
}

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
ZLibCodec * 
MessageIOGateway ::
GetCodec(int32 newEncoding, ZLibCodec * & setCodec) const
{
   TCHECKPOINT;

   if (muscleInRange(newEncoding, (int32)MUSCLE_MESSAGE_ENCODING_ZLIB_1, (int32)MUSCLE_MESSAGE_ENCODING_ZLIB_9))
   {
      int newLevel = (newEncoding-MUSCLE_MESSAGE_ENCODING_ZLIB_1)+1;
      if ((setCodec == NULL)||(newLevel != setCodec->GetCompressionLevel()))
      {
         delete setCodec;  // oops, encoding change!  Throw out the old codec, if any
         setCodec = newnothrow ZLibCodec(newLevel);
         if (setCodec == NULL) WARN_OUT_OF_MEMORY;
      }
      return setCodec;
   }
   return NULL;
}
#endif

ByteBufferRef 
MessageIOGateway ::
FlattenMessage(const MessageRef & msgRef, uint8 * headerBuf) const
{
   TCHECKPOINT;

   ByteBufferRef ret;
   if (msgRef())
   {
      ret = GetByteBufferFromPool(msgRef()->FlattenedSize());
      if (ret())
      {
         msgRef()->Flatten(ret()->GetBuffer());

         int32 encoding = MUSCLE_MESSAGE_ENCODING_DEFAULT;

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
         if (ret()->GetNumBytes() >= 32)  // below 32 bytes, the compression headers usually offset the benefits
         {
            ZLibCodec * enc = GetCodec(_outgoingEncoding, _sendCodec);
            if (enc)
            {
               ByteBufferRef compressedRef = enc->Deflate(*ret(), AreOutgoingMessagesIndependent());
               if (compressedRef())
               {
                  encoding = MUSCLE_MESSAGE_ENCODING_ZLIB_1+enc->GetCompressionLevel()-1;
                  ret = compressedRef;
               }
               else ret.Reset();  // uh oh, the compressor failed
            }
         }
#endif

         if (ret())
         {
            uint32 * lhb = (uint32 *) headerBuf;
            lhb[0] = B_HOST_TO_LENDIAN_INT32(ret()->GetNumBytes());
            lhb[1] = B_HOST_TO_LENDIAN_INT32(encoding);
         }
      }
   }
   return ret;
}

MessageRef 
MessageIOGateway ::
UnflattenMessage(const ByteBufferRef & bufRef, const uint8 * headerBuf) const
{
   TCHECKPOINT;

   MessageRef ret;
   if (bufRef())
   {
      ret = GetMessageFromPool();
      if (ret())
      {
         const uint32 * lhb = (const uint32 *) headerBuf;
         if ((uint32) B_LENDIAN_TO_HOST_INT32(lhb[0]) != bufRef()->GetNumBytes()) return MessageRef();

         int32 encoding = B_LENDIAN_TO_HOST_INT32(lhb[1]);

         const ByteBuffer * bb = bufRef();  // default; may be changed below

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
         ByteBufferRef expRef;  // must be declared outside the brackets below!
         ZLibCodec * enc = GetCodec(encoding, _recvCodec);
         if (enc) 
         {
            expRef = enc->Inflate(*bb);
            if (expRef()) bb = expRef();
         }
#else
         if (encoding != MUSCLE_MESSAGE_ENCODING_DEFAULT) bb = NULL;
#endif

         if ((bb == NULL)||(ret()->Unflatten(bb->GetBuffer(), bb->GetNumBytes()) != B_NO_ERROR)) ret.Reset();
      }
   }
   return ret;
}

// Returns the size of the pre-flattened-message header section, in bytes.
// The default format has an 8-byte header (4 bytes for encoding ID, 4 bytes for message length)
uint32 
MessageIOGateway ::
GetHeaderSize() const
{
   return 2 * sizeof(uint32);  // one long for the encoding ID, and one long for the body length 
}

int32 
MessageIOGateway ::
GetBodySize(const uint8 * headerBuf) const 
{
   const uint32 * h = (const uint32 *) headerBuf;
   return (muscleInRange((uint32)B_LENDIAN_TO_HOST_INT32(h[1]), (uint32)MUSCLE_MESSAGE_ENCODING_DEFAULT, (uint32)MUSCLE_MESSAGE_ENCODING_END_MARKER-1)) ? (int32)(B_LENDIAN_TO_HOST_INT32(h[0])) : -1;
}

bool 
MessageIOGateway ::
HasBytesToOutput() const
{
   return ((IsHosed() == false)&&((_sendBodyBuffer._buffer())||(GetOutgoingMessageQueue().HasItems())));
}

void
MessageIOGateway ::
Reset()
{
   TCHECKPOINT;

   AbstractMessageIOGateway::Reset();

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
   delete _sendCodec; _sendCodec = NULL;
   delete _recvCodec; _recvCodec = NULL;
#endif

   _sendHeaderBuffer.Reset();
   _sendBodyBuffer.Reset();
   _recvHeaderBuffer.Reset();
   _recvBodyBuffer.Reset();
}

MessageRef MessageIOGateway :: CreateSynchronousPingMessage(uint32 syncPingCounter) const
{
   MessageRef pingMsg = GetMessageFromPool(PR_COMMAND_PING);
   return ((pingMsg())&&(pingMsg()->AddInt32("_miosp", syncPingCounter) == B_NO_ERROR)) ? pingMsg : MessageRef();
}

status_t MessageIOGateway :: ExecuteSynchronousMessaging(AbstractGatewayMessageReceiver * optReceiver, uint64 timeoutPeriod)
{
   const DataIO * dio = GetDataIO()();
   if ((dio == NULL)||(dio->GetReadSelectSocket().GetFileDescriptor() < 0)||(dio->GetWriteSelectSocket().GetFileDescriptor() < 0)) return B_ERROR;

   MessageRef pingMsg = CreateSynchronousPingMessage(_syncPingCounter);
   if ((pingMsg())&&(AddOutgoingMessage(pingMsg) == B_NO_ERROR))
   {
      _pendingSyncPingCounter = _syncPingCounter;
      _syncPingCounter++;
      return AbstractMessageIOGateway::ExecuteSynchronousMessaging(optReceiver, timeoutPeriod);
   }
   else return B_ERROR;
}

void MessageIOGateway :: SynchronousMessageReceivedFromGateway(const MessageRef & msg, void * userData, AbstractGatewayMessageReceiver & r) 
{
   if ((_pendingSyncPingCounter >= 0)&&(IsSynchronousPongMessage(msg, _pendingSyncPingCounter)))
   {
      // Yay, we found our pong Message, so we are no longer waiting for one.
      _pendingSyncPingCounter = -1;
   }
   else AbstractMessageIOGateway::SynchronousMessageReceivedFromGateway(msg, userData, r);
}

bool MessageIOGateway :: IsSynchronousPongMessage(const MessageRef & msg, uint32 pendingSyncPingCounter) const
{
   return ((msg()->what == PR_RESULT_PONG)&&((uint32)msg()->GetInt32("_miosp", -1) == pendingSyncPingCounter));
}

MessageRef MessageIOGateway :: ExecuteSynchronousMessageRPCCall(const Message & requestMessage, const IPAddressAndPort & targetIAP, uint64 timeoutPeriod)
{
   MessageRef ret;

   uint64 timeBeforeConnect = GetRunTime64();
   ConstSocketRef s = Connect(targetIAP, NULL, NULL, true, timeoutPeriod);
   if (s())
   {
      if (timeoutPeriod != MUSCLE_TIME_NEVER)
      {
         uint64 connectDuration = GetRunTime64()-timeBeforeConnect;
         timeoutPeriod = (timeoutPeriod > connectDuration) ? (timeoutPeriod-connectDuration) : 0;
      }
 
      DataIORef oldIO = GetDataIO();
      TCPSocketDataIO tsdio(s, false);
      SetDataIO(DataIORef(&tsdio, false));
      QueueGatewayMessageReceiver receiver;
      if ((AddOutgoingMessage(MessageRef(const_cast<Message *>(&requestMessage), false)) == B_NO_ERROR)&&(ExecuteSynchronousMessaging(&receiver, timeoutPeriod) == B_NO_ERROR)) ret = receiver.HasItems() ? receiver.Head() : GetMessageFromPool();
      SetDataIO(oldIO);  // restore any previous I/O
   }
   return ret;
}

status_t MessageIOGateway :: ExecuteSynchronousMessageSend(const Message & requestMessage, const IPAddressAndPort & targetIAP, uint64 timeoutPeriod)
{
   status_t ret = B_ERROR;
   uint64 timeBeforeConnect = GetRunTime64();
   ConstSocketRef s = Connect(targetIAP, NULL, NULL, true, timeoutPeriod);
   if (s())
   {
      if (timeoutPeriod != MUSCLE_TIME_NEVER)
      {
         uint64 connectDuration = GetRunTime64()-timeBeforeConnect;
         timeoutPeriod = (timeoutPeriod > connectDuration) ? (timeoutPeriod-connectDuration) : 0;
      }
 
      DataIORef oldIO = GetDataIO();
      TCPSocketDataIO tsdio(s, false);
      SetDataIO(DataIORef(&tsdio, false));
      QueueGatewayMessageReceiver receiver;
      if (AddOutgoingMessage(MessageRef(const_cast<Message *>(&requestMessage), false)) == B_NO_ERROR) 
      {
         NestCountGuard ncg(_noRPCReply);  // so that we'll return as soon as we've sent the request Message, and not wait for a reply Message.
         ret = ExecuteSynchronousMessaging(&receiver, timeoutPeriod);
      }
      SetDataIO(oldIO);
   }
   return ret;
}

CountedMessageIOGateway :: CountedMessageIOGateway(int32 outgoingEncoding) : MessageIOGateway(outgoingEncoding), _outgoingByteCount(0)
{
   // empty
}

status_t CountedMessageIOGateway :: AddOutgoingMessage(const MessageRef & messageRef)
{
   if (MessageIOGateway::AddOutgoingMessage(messageRef) != B_NO_ERROR) return B_ERROR;

   uint32 msgSize = messageRef()?messageRef()->FlattenedSize():0;
   if (GetOutgoingMessageQueue().GetNumItems() > 1) _outgoingByteCount += msgSize;
                                               else _outgoingByteCount  = msgSize;  // semi-paranoia about meddling via GetOutgoingMessageQueue() access
   return B_NO_ERROR;
}

void CountedMessageIOGateway :: Reset()
{
   MessageIOGateway::Reset();
   _outgoingByteCount = 0;
}

status_t CountedMessageIOGateway :: PopNextOutgoingMessage(MessageRef & ret)
{
   if (MessageIOGateway::PopNextOutgoingMessage(ret) != B_NO_ERROR) return B_ERROR;

   if (GetOutgoingMessageQueue().HasItems())
   {
      uint32 retSize = ret()?ret()->FlattenedSize():0;
      _outgoingByteCount = (retSize<_outgoingByteCount) ? (_outgoingByteCount-retSize) : 0;  // paranoia to avoid underflow
   }
   else _outgoingByteCount = 0;  // semi-paranoia about meddling via GetOutgoingMessageQueue() access

   return B_NO_ERROR;
}

}; // end namespace muscle
