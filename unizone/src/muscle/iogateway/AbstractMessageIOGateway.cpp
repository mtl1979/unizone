/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "iogateway/AbstractMessageIOGateway.h"

BEGIN_NAMESPACE(muscle);

AbstractMessageIOGateway :: AbstractMessageIOGateway() : _hosed(false), _flushOnEmpty(true)
{
   // empty
}

AbstractMessageIOGateway :: ~AbstractMessageIOGateway() 
{
   // empty
}

// Adds the given message reference to our list of outgoing messages
// to send.  Never blocks.  Returns false iff for some reason
// the message can't be queued (out of memory?)
status_t 
AbstractMessageIOGateway ::
AddOutgoingMessage(const MessageRef & messageRef)
{
   return _hosed ? B_ERROR : _outgoingMessages.AddTail(messageRef);
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

END_NAMESPACE(muscle);
