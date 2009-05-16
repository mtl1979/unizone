/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "iogateway/AbstractMessageIOGateway.h"
#include "util/NetworkUtilityFunctions.h"

namespace muscle {

AbstractMessageIOGateway :: AbstractMessageIOGateway() : _hosed(false), _flushOnEmpty(true)
{
   // empty
}

AbstractMessageIOGateway :: ~AbstractMessageIOGateway() 
{
   // empty
}

// Handles buffer allocation and re-allocation
status_t
AbstractMessageIOGateway ::
EnsureBufferSize(uint8 ** bufPtr, uint32 * bufSize, uint32 desiredSize, uint32 copySize)
{
   uint8 * oldBuffer = *bufPtr;
   uint32 oldBufSize = *bufSize;

   if ((oldBuffer == NULL)||(oldBufSize < desiredSize))
   {
      uint8 * newBuffer = newnothrow_array(uint8, desiredSize);
      if (newBuffer == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}

      *bufPtr  = newBuffer;
      *bufSize = desiredSize;

      if (copySize > 0) memcpy(newBuffer, oldBuffer, copySize);
      delete [] oldBuffer;
   }
   return B_NO_ERROR;
}

void 
AbstractMessageIOGateway :: 
FreeLargeBuffer(uint8 ** buf, uint32 * size)
{
   if ((*buf)&&(*size > 10*1024))  // if the buffer is greater than 10KB, free it
   {
      delete [] (*buf);
      *buf = NULL;
      *size = 0;
   }
}

void
AbstractMessageIOGateway ::
SetFlushOnEmpty(bool f)
{
   _flushOnEmpty = f;
}

bool
AbstractMessageIOGateway ::
IsReadyForInput() const
{
   return true;
}

uint64
AbstractMessageIOGateway ::
GetOutputStallLimit() const
{
   return _ioRef() ? _ioRef()->GetOutputStallLimit() : MUSCLE_TIME_NEVER;
}

void
AbstractMessageIOGateway ::
Shutdown()
{
   if (_ioRef()) _ioRef()->Shutdown();
}

void
AbstractMessageIOGateway ::
Reset()
{
   _outgoingMessages.Clear();
   _hosed = false;
}

/** Used to funnel callbacks from DoInput() back through the AbstractMessageIOGateway's own API, so that subclasses can insert their logic as necessary */
class ScratchProxyReceiver : public AbstractGatewayMessageReceiver
{
public:
   ScratchProxyReceiver(AbstractMessageIOGateway * gw, AbstractGatewayMessageReceiver * r) : _gw(gw), _r(r) {/* empty */}

   virtual void MessageReceivedFromGateway(const MessageRef & msg, void * userData) {_gw->SynchronousMessageReceivedFromGateway(msg, userData, *_r);}
   virtual void AfterMessageReceivedFromGateway(const MessageRef & msg, void * userData) {_gw->SynchronousAfterMessageReceivedFromGateway(msg, userData, *_r);}
   virtual void BeginMessageReceivedFromGatewayBatch() {_gw->SynchronousBeginMessageReceivedFromGatewayBatch(*_r);}
   virtual void EndMessageReceivedFromGatewayBatch() {_gw->SynchronousEndMessageReceivedFromGatewayBatch(*_r);}

private:
   AbstractMessageIOGateway * _gw;
   AbstractGatewayMessageReceiver * _r;
};

status_t 
AbstractMessageIOGateway :: 
ExecuteSynchronousMessaging(AbstractGatewayMessageReceiver * optReceiver, uint64 timeoutPeriod)
{
   int fd = GetDataIO()() ? GetDataIO()()->GetSelectSocket().GetFileDescriptor() : -1;
   if (fd < 0) return B_ERROR;  // no socket to transmit or receive on!

   ScratchProxyReceiver scratchReceiver(this, optReceiver);
   bool hasTimeout = (timeoutPeriod != MUSCLE_TIME_NEVER);
   uint64 endTime = hasTimeout ? (GetRunTime64()+timeoutPeriod) : MUSCLE_TIME_NEVER;
   fd_set readSet, writeSet;
   while(IsStillAwaitingSynchronousMessagingReply())
   {
      uint64 now = GetRunTime64();
      if (now >= endTime) return B_ERROR;

      fd_set * rset = NULL;
      if (optReceiver)
      {
         rset = &readSet;
         FD_ZERO(rset);
         FD_SET(fd, rset);
      }

      fd_set * wset = NULL;
      if (HasBytesToOutput())
      {
         wset = &writeSet;
         FD_ZERO(wset);
         FD_SET(fd, wset);
      }

      struct timeval tv; 
      if (hasTimeout) Convert64ToTimeVal(muscleMax((int64)0, (int64)(endTime-now)), tv);
      if (select(fd+1, rset, wset, NULL, hasTimeout ? &tv : NULL) < 0)  return B_ERROR;
      if ((wset)&&(FD_ISSET(fd, wset))&&(DoOutput() < 0))               return B_ERROR;
      if ((rset)&&(FD_ISSET(fd, rset))&&(DoInput(scratchReceiver) < 0)) return B_ERROR;
   }
   return B_NO_ERROR;
}

}; // end namespace muscle
