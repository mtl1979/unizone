/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "iogateway/MessageIOGateway.h"

namespace muscle {

MessageIOGateway :: MessageIOGateway(int32 encoding) :
   _sendBuffer(NULL), _sendBufferSize(0), _sendBufferOffset(-1),
   _recvBuffer(NULL), _recvBufferSize(0), _recvBufferOffset(-1),
   _maxIncomingMessageSize(MUSCLE_NO_LIMIT),
   _encoding(encoding), 
   _aboutToFlattenCallback(NULL), _aboutToFlattenCallbackData(NULL),
   _flattenedCallback(NULL), _flattenedCallbackData(NULL),
   _unflattenedCallback(NULL), _unflattenedCallbackData(NULL)
{
   /* empty */
}

MessageIOGateway :: ~MessageIOGateway() 
{
   delete [] _sendBuffer;
   delete [] _recvBuffer;
}

// Writes some of our outgoing messages to the wire.
// Not guaranteed to write all outgoing messages (it will
// try not to block)
int32 
MessageIOGateway ::
DoOutput(uint32 maxBytes)
{
   if (IsHosed()) return -1;

   int32 ret = 0;
   bool sentBytes = false;
   while(maxBytes > 0)
   {
      // First, make sure our outgoing byte-buffer has data.  If it doesn't, fill it with the next outgoing message.
      if (_sendBufferOffset < 0)
      {
         while(true)
         {
            MessageRef nextRef;
            if (GetOutgoingMessageQueue().RemoveHead(nextRef) != B_NO_ERROR) 
            {
               if ((GetFlushOnEmpty())&&(sentBytes)) GetDataIO()->FlushOutput();
               FreeLargeBuffer(&_sendBuffer, &_sendBufferSize);
               return ret;  // nothing more to send, so we're done!
            }

            if (_aboutToFlattenCallback) _aboutToFlattenCallback(nextRef, _aboutToFlattenCallbackData);
            const Message * nextSendMsg = nextRef.GetItemPointer();
            if (nextSendMsg)
            {
               _currentSendMessageSize = GetHeaderSize() + GetFlattenedMessageBodySize(*nextSendMsg);
               if (EnsureBufferSize(&_sendBuffer, &_sendBufferSize, _currentSendMessageSize, 0) != B_NO_ERROR) {SetHosed(); return -1;}
               if (FlattenMessage(*nextSendMsg, _sendBuffer, &_sendBuffer[GetHeaderSize()]) == B_NO_ERROR)
               {
                  if (_flattenedCallback) _flattenedCallback(nextRef, _flattenedCallbackData);
                  _sendBufferOffset = 0;
                  break;  // go on to sending phase
               }
            }
         }
      }

      // Send whatever data we have pending...
      int attemptSize = muscleMin(_currentSendMessageSize-_sendBufferOffset, maxBytes);
      int32 writtenBytes = GetDataIO()->Write(&_sendBuffer[_sendBufferOffset], attemptSize);
           if (writtenBytes < 0) {SetHosed(); return -1;}  // trouble!
      else if (writtenBytes > 0)
      {
         maxBytes -= writtenBytes;
         ret += writtenBytes;
         _sendBufferOffset += writtenBytes;
         sentBytes = true;
      }
      if (writtenBytes < attemptSize) return ret;  // assume this means buffers are full; will send the rest later
      if ((uint32) _sendBufferOffset == _currentSendMessageSize) _sendBufferOffset = -1;  // trigger flattening of next message
   }
   return ret;
}

int32 
MessageIOGateway ::
DoInput(uint32 maxBytes)
{
   if (IsHosed()) return -1;

   status_t errorCode = B_ERROR;
   int32 ret = 0;
   while(maxBytes > 0)
   {
      // First, make sure our incoming byte-buffer has at least enough bytes for the header...
      if (_recvBufferOffset < 0)
      {
         _currentRecvMessageSize = GetHeaderSize();
         if (EnsureBufferSize(&_recvBuffer, &_recvBufferSize, _currentRecvMessageSize, 0) != B_NO_ERROR) {SetHosed(); return (errorCode == B_NO_ERROR) ? ret : -1;}
         _recvBufferOffset = 0;
      }

      // Recv whatever data we have pending...
      int attemptSize = muscleMin(_currentRecvMessageSize-_recvBufferOffset, maxBytes);
      int32 readBytes = GetDataIO()->Read(&_recvBuffer[_recvBufferOffset], attemptSize);
           if (readBytes < 0) {SetHosed(); return (errorCode == B_NO_ERROR) ? ret : -1;}  // trouble, connection closed!
      else if (readBytes > 0)
      {
         _recvBufferOffset += readBytes;
         ret += readBytes;
         maxBytes -= readBytes;
      }
      if (readBytes < attemptSize) return ret;  // assume this means input buffers are empty; we'll read more later
 
      uint32 hs = GetHeaderSize();
      if ((_currentRecvMessageSize == hs)&&((uint32) _recvBufferOffset == _currentRecvMessageSize))
      {
         // Now that we have the entire header block, we can determine the size of the message body
         uint32 bodySize;
         if (GetBodySize(_recvBuffer, bodySize) != B_NO_ERROR) 
         {
            // header format error--see if we can recover
            uint32 firstValidByte = hs;
            if (RecoverFromInvalidHeader(_recvBuffer, hs) == B_NO_ERROR)
            {
               int j = 0;
               for (uint32 i=firstValidByte; i<hs; i++) _recvBuffer[j++] = _recvBuffer[i];
               _recvBufferOffset = j;
               return ret;
            }
            else
            {
               SetHosed(); 
               return (errorCode == B_NO_ERROR) ? ret : -1;
            }
         }
         if (bodySize > _maxIncomingMessageSize) {SetHosed(); return (errorCode == B_NO_ERROR) ? ret : -1;}
         _currentRecvMessageSize = GetHeaderSize() + bodySize;
         if (EnsureBufferSize(&_recvBuffer, &_recvBufferSize, _currentRecvMessageSize, GetHeaderSize()) != B_NO_ERROR) {SetHosed(); return (errorCode == B_NO_ERROR) ? ret : -1;}
      }
      if ((uint32) _recvBufferOffset == _currentRecvMessageSize) 
      {
         // Finished receiving message bytes... now reconstruct that bad boy!
         MessageRef newMsg = GetMessageFromPool();
         if (newMsg() == NULL) {WARN_OUT_OF_MEMORY; SetHosed(); return (errorCode == B_NO_ERROR) ? ret : -1;}
         status_t unflatRet = UnflattenMessage(*newMsg(), _recvBuffer, &_recvBuffer[GetHeaderSize()], _currentRecvMessageSize-GetHeaderSize());
         if (unflatRet != B_NO_ERROR)
         {
            newMsg.Reset();
            if (RecoverFromInvalidMessage(_recvBuffer, _currentRecvMessageSize) == B_NO_ERROR)
            {
               _recvBufferOffset = -1;
               return ret;
            }
            else
            {
               SetHosed();
               return (errorCode == B_NO_ERROR) ? ret : -1;
            }
         }
         if (_unflattenedCallback) _unflattenedCallback(newMsg, _unflattenedCallbackData);
         if (GetIncomingMessageQueue().AddTail(newMsg) == B_NO_ERROR) errorCode = B_NO_ERROR;  // force successful result so that user knows to get his pending messages.
                                                                 else {SetHosed(); return (errorCode == B_NO_ERROR) ? ret : -1;}
         _recvBufferOffset = -1;  // go back to virgin state for next message
      }
   }
   if (_recvBufferOffset < 0) FreeLargeBuffer(&_recvBuffer, &_recvBufferSize);
   return ret;
}

// Pass-throughs to the Message Flattenable
// interface.  Broken out into methods here so that subclasses
// can modify the standard flattening behavior...
uint32 
MessageIOGateway ::
GetFlattenedMessageBodySize(const Message & msg) const 
{ 
   return msg.FlattenedSize();
}

// Flattens (msg) into (buffer) and (headerBuf).
// (buffer) is GetFlattenedMessageBodySize() bytes long.  This should have header info written to it (including body length!)
// (headerBuf) is GetHeaderSize() bytes long.  This should have the body data written into it
status_t 
MessageIOGateway ::
FlattenMessage(const Message & msg, uint8 * headerBuf, uint8 * buffer) const
{
   uint32 * lhb = (uint32 *) headerBuf;
   lhb[0] = B_HOST_TO_LENDIAN_INT32(msg.FlattenedSize());
   lhb[1] = B_HOST_TO_LENDIAN_INT32(_encoding); 
   msg.Flatten(buffer);
   return B_NO_ERROR;
}

// Unflattens (msg) from (buffer) and (headerBuf).
// (buffer) is (bufSize) bytes long.
// (headerBuf) is GetHeaderSize() bytes long.
status_t 
MessageIOGateway ::
UnflattenMessage(Message & setMsg, const uint8 * headerBuf, const uint8 * buf, uint32 bufSize) const
{
   const uint32 * header = (const uint32 *) headerBuf;
   if (header[0] != B_LENDIAN_TO_HOST_INT32(bufSize)) return B_ERROR;
   if (header[1] != B_LENDIAN_TO_HOST_INT32(_encoding)) return B_ERROR;
   return setMsg.Unflatten(buf, bufSize);
}

// Returns the size of the pre-flattened-message header section, in bytes.
// The default format has an 8-byte header (4 bytes for encoding ID, 4 bytes for message length)
uint32 
MessageIOGateway ::
GetHeaderSize() const
{
   return 2 * sizeof(uint32);  // one long for the encoding ID, and one long for the body length 
}

// Extracts and returns (in setSize) the dynamic buffer size from the given header.
// Note that the returned size should NOT include the header bytes!
// (headerBuf) is GetHeaderSize() bytes long.
// Returns B_NO_ERROR iff successful.
status_t 
MessageIOGateway ::
GetBodySize(const uint8 * headerBuf, uint32 & setSize) const 
{
   const uint32 * header = (const uint32 *) headerBuf;
   if (B_LENDIAN_TO_HOST_INT32(header[1]) != (uint32) _encoding) return B_ERROR;
   setSize = (uint32) B_LENDIAN_TO_HOST_INT32(header[0]);
   return B_NO_ERROR;
}

bool 
MessageIOGateway ::
HasBytesToOutput() const
{
   if (IsHosed()) return false;
   return ((_sendBufferOffset >= 0)||(GetOutgoingMessageQueue().GetNumItems() > 0));
}


status_t
MessageIOGateway ::
RecoverFromInvalidHeader(const uint8 * /*headerBuf*/, uint32 & /*setFirstValidByte*/) const
{
   return B_ERROR;  // default behavior:  just give up and die, don't try to recover
}

status_t
MessageIOGateway ::
RecoverFromInvalidMessage(const uint8 * /*messageBuf*/, uint32 /*messageBufSize*/) const
{
   return B_ERROR;  // default behavior:  just give up and die, don't try to recover
}

};  // end namespace muscle
