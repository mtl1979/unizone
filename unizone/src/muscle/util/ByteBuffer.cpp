#include "util/ByteBuffer.h"
#include "system/GlobalMemoryAllocator.h"

BEGIN_NAMESPACE(muscle);

status_t ByteBuffer :: SetBuffer(uint32 numBytes, const uint8 * buffer)
{
   Clear();
   if (SetNumBytes(numBytes, false) != B_NO_ERROR) return B_ERROR;
   if ((buffer)&&(_buffer)) memcpy(_buffer, buffer, numBytes);
   return B_NO_ERROR;
}

status_t ByteBuffer :: SetNumBytes(uint32 newNumBytes, bool retainData)
{
   if (newNumBytes > _numAllocatedBytes)
   {
      if (retainData)
      {
         uint8 * newBuf = (uint8 *)muscleRealloc(_buffer, newNumBytes);
         if (newBuf)
         {
            _buffer = newBuf;
            _numAllocatedBytes = _numValidBytes = newNumBytes;
         }
         else
         {
            WARN_OUT_OF_MEMORY;
            return B_ERROR;
         }
      }
      else
      {
         uint8 * newBuf = NULL;
         if (newNumBytes > 0)
         {
            newBuf = (uint8 *) muscleAlloc(newNumBytes);
            if (newBuf == NULL) 
            {
               WARN_OUT_OF_MEMORY;
               return B_ERROR;
            }
         }
         muscleFree(_buffer);
         _buffer = newBuf;
         _numAllocatedBytes = _numValidBytes = newNumBytes;
      }
   }
   else _numValidBytes = newNumBytes;  // truncating our array is easy!

   return B_NO_ERROR;
}

status_t ByteBuffer :: FreeExtraBytes()
{
   if (_numValidBytes < _numAllocatedBytes)
   {
      uint8 * newBuf = (uint8 *) muscleRealloc(_buffer, _numValidBytes);
      if ((_numValidBytes == 0)||(newBuf)) 
      {
         _buffer            = newBuf;
         _numAllocatedBytes = _numValidBytes;
      }
      else return B_ERROR;  // huh?  Why would this happen?  Why?  Why?
   }
   return B_NO_ERROR;
}

/** Overridden to unflatten directly from our buffer, for added efficiency */
status_t ByteBuffer :: CopyToImplementation(Flattenable & copyTo) const {return copyTo.Unflatten(_buffer, _numValidBytes);}
   
/** Overridden to set our buffer directly from (copyFrom)'s Flatten() method */
status_t ByteBuffer :: CopyFromImplementation(const Flattenable & copyFrom)
{
   uint32 numBytes = copyFrom.FlattenedSize();
   if (SetNumBytes(numBytes, false) != B_NO_ERROR) return B_ERROR;
   copyFrom.Flatten(_buffer);
   return B_NO_ERROR;
}

void ByteBuffer :: Clear(bool releaseBuffers)
{
   if (releaseBuffers)
   {
      muscleFree(_buffer);
      _buffer = NULL;
      _numValidBytes = _numAllocatedBytes = 0;
   }
   else SetNumBytes(0, false);
}

static void ClearBufferFunc(ByteBuffer * buf, void *) {buf->Clear(buf->GetNumBytes() > 100);}
static ByteBufferRef::ItemPool _bufferPool(100, ClearBufferFunc);
ByteBufferRef::ItemPool * GetByteBufferPool() {return &_bufferPool;}

ByteBufferRef GetByteBufferFromPool(uint32 numBytes, const uint8 * optBuffer)
{
   ByteBufferRef ref(_bufferPool.ObtainObject(), &_bufferPool);
   if ((ref())&&(ref()->SetBuffer(numBytes, optBuffer) != B_NO_ERROR)) ref.Reset();  // return NULL ref on out-of-memory
   return ref;
}

ByteBufferRef GetByteBufferFromPool(const ByteBuffer & copyMe)
{
   ByteBufferRef ref(_bufferPool.ObtainObject(), &_bufferPool);
   if (ref())
   {
      *(ref()) = copyMe;
      if ((copyMe())&&((*ref())() == NULL)) ref.Reset();  // return NULL ref on out-of-memory
   }
   return ref;
}

END_NAMESPACE(muscle);

