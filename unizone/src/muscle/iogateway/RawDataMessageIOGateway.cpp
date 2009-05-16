/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "iogateway/RawDataMessageIOGateway.h"

namespace muscle {

RawDataMessageIOGateway ::
RawDataMessageIOGateway(uint32 minChunkSize, uint32 maxChunkSize) : _recvScratchSpace(NULL), _minChunkSize(minChunkSize), _maxChunkSize(maxChunkSize)
{
   // empty
}

RawDataMessageIOGateway ::
~RawDataMessageIOGateway()
{
   delete [] _recvScratchSpace;
}

int32 
RawDataMessageIOGateway ::
DoOutputImplementation(uint32 maxBytes)
{
   TCHECKPOINT;

   const Message * msg = _sendMsgRef();
   if (msg == NULL) 
   {
      // try to get the next message from our queue
      (void) GetOutgoingMessageQueue().RemoveHead(_sendMsgRef);
      msg = _sendMsgRef();
      _sendBufLength = _sendBufIndex = _sendBufByteOffset = -1;
   }
   if (msg)
   {
      if ((_sendBufByteOffset < 0)||(_sendBufByteOffset >= _sendBufLength))
      {
         // Try to get the next field from our message message
         if (msg->FindData(PR_NAME_DATA_CHUNKS, B_ANY_TYPE, ++_sendBufIndex, &_sendBuf, (uint32*)(&_sendBufLength)) == B_NO_ERROR) _sendBufByteOffset = 0;
         else 
         {
            _sendMsgRef.Reset();  // no more data available?  Go to the next message then.
            return DoOutputImplementation(maxBytes);
         }
      }
      if ((_sendBufByteOffset >= 0)&&(_sendBufByteOffset < _sendBufLength))
      {
         // Send as much as we can of the current text line
         int32 bytesWritten = GetDataIO()()->Write(&((char *)_sendBuf)[_sendBufByteOffset], muscleMin(maxBytes, (uint32) (_sendBufLength-_sendBufByteOffset)));
              if (bytesWritten < 0) return -1;
         else if (bytesWritten > 0)
         {
            _sendBufByteOffset += bytesWritten;
            int32 subRet = DoOutputImplementation(maxBytes-bytesWritten);
            return (subRet >= 0) ? subRet+bytesWritten : -1;
         }
      }
   }
   return 0;
}


int32 
RawDataMessageIOGateway ::
DoInputImplementation(AbstractGatewayMessageReceiver & receiver, uint32 maxBytes)
{
   TCHECKPOINT;

   int32 ret = 0;
   Message * inMsg = _recvMsgRef();
   if (_minChunkSize > 0)
   {
      // Minimum-chunk-size mode:  we read bytes directly into the Message's data field until it is full, then 
      // forward that message on to the user code and start the next.  Advantage of this is:  no data-copying necessary!
      if (inMsg == NULL)
      {
         _recvMsgRef = GetMessageFromPool(PR_COMMAND_RAW_DATA);
         inMsg = _recvMsgRef();
         if (inMsg)
         {
            if ((inMsg->AddData(PR_NAME_DATA_CHUNKS, B_RAW_TYPE, NULL, _minChunkSize)                         == B_NO_ERROR)&&
                (inMsg->FindDataPointer(PR_NAME_DATA_CHUNKS, B_RAW_TYPE, &_recvBuf, (uint32*)&_recvBufLength) == B_NO_ERROR)) _recvBufByteOffset = 0;
            else
            {
               _recvMsgRef.Reset();
               return -1;  // oops, no mem?
            }
         }
      }
      if (inMsg)
      {
         int32 bytesRead = GetDataIO()()->Read(&((char*)_recvBuf)[_recvBufByteOffset], muscleMin(maxBytes, (uint32)(_recvBufLength-_recvBufByteOffset)));
              if (bytesRead < 0) return -1;
         else if (bytesRead > 0)
         {
            ret += bytesRead;
            _recvBufByteOffset += bytesRead;
            if (_recvBufByteOffset == _recvBufLength)
            {
               // This buffer is full... forward it on to the user, and start receiving the next one.
               receiver.CallMessageReceivedFromGateway(_recvMsgRef);
               _recvMsgRef.Reset();
               int32 subRet = IsSuggestedTimeSliceExpired() ? 0 : DoInputImplementation(receiver, maxBytes-bytesRead);
               return (subRet >= 0) ? (ret+subRet) : -1;
            }
         }
      }
   }
   else
   {
      // Immediate-forward mode... Read data into a temporary buffer, and immediately forward it to the user.
      if (_recvScratchSpace == NULL) 
      {
         // demand-allocate a scratch buffer
         const uint32 maxScratchSpaceSize = 8192;  // we probably won't ever get more than this much at once anyway
         _recvScratchSpaceSize = (_maxChunkSize < maxScratchSpaceSize) ? _maxChunkSize : maxScratchSpaceSize;
         _recvScratchSpace = newnothrow_array(uint8, _recvScratchSpaceSize);
         if (_recvScratchSpace == NULL)
         {
            WARN_OUT_OF_MEMORY;
            return -1;
         }
      }

      int32 bytesRead = GetDataIO()()->Read(_recvScratchSpace, muscleMin(_recvScratchSpaceSize, maxBytes));
           if (bytesRead < 0) return -1;
      else if (bytesRead > 0)
      {
         ret += bytesRead;
         MessageRef ref = GetMessageFromPool();
         if ((ref())&&(ref()->AddData(PR_NAME_DATA_CHUNKS, B_RAW_TYPE, _recvScratchSpace, bytesRead) == B_NO_ERROR)) receiver.CallMessageReceivedFromGateway(ref);
         // note:  don't recurse here!  It would be bad (tm) on a fast feed since we might never return
      }
   }
   return ret;
}

bool 
RawDataMessageIOGateway ::
HasBytesToOutput() const
{
   return ((_sendMsgRef())||(GetOutgoingMessageQueue().HasItems()));
}

void
RawDataMessageIOGateway ::
Reset()
{
   TCHECKPOINT;

   AbstractMessageIOGateway::Reset();
   _sendMsgRef.Reset();
   _recvMsgRef.Reset();
}

}; // end namespace muscle
