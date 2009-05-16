#include "util/ByteBuffer.h"
#include "util/MiscUtilityFunctions.h"
#include "system/GlobalMemoryAllocator.h"

namespace muscle {

void ByteBuffer :: AdoptBuffer(uint32 numBytes, uint8 * optBuffer)
{
   Clear(true);  // free any previously held array
   _buffer = optBuffer;
   _numValidBytes = _numAllocatedBytes = numBytes;
}

status_t ByteBuffer :: SetBuffer(uint32 numBytes, const uint8 * buffer)
{
   Clear();
   if (SetNumBytes(numBytes, false) != B_NO_ERROR) return B_ERROR;
   if ((buffer)&&(_buffer)) memcpy(_buffer, buffer, numBytes);
   return B_NO_ERROR;
}

status_t ByteBuffer :: SetNumBytes(uint32 newNumBytes, bool retainData)
{
   TCHECKPOINT;

   if (newNumBytes > _numAllocatedBytes)
   {
      if (retainData)
      {
         uint8 * newBuf = (uint8 *) (_allocStrategy ? _allocStrategy->Realloc(_buffer, newNumBytes, _numAllocatedBytes, true) : muscleRealloc(_buffer, newNumBytes));
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
            newBuf = (uint8 *) (_allocStrategy ? _allocStrategy->Malloc(newNumBytes) : muscleAlloc(newNumBytes));
            if (newBuf == NULL) 
            {
               WARN_OUT_OF_MEMORY;
               return B_ERROR;
            }
         }
         if (_allocStrategy) _allocStrategy->Free(_buffer, _numAllocatedBytes); else muscleFree(_buffer);
         _buffer = newBuf;
         _numAllocatedBytes = _numValidBytes = newNumBytes;
      }
   }
   else _numValidBytes = newNumBytes;  // truncating our array is easy!

   return B_NO_ERROR;
}

status_t ByteBuffer :: FreeExtraBytes()
{
   TCHECKPOINT;

   if (_numValidBytes < _numAllocatedBytes)
   {
      uint8 * newBuf = (uint8 *) (_allocStrategy ? _allocStrategy->Realloc(_buffer, _numValidBytes, _numAllocatedBytes, true) : muscleRealloc(_buffer, _numValidBytes));
      if ((_numValidBytes == 0)||(newBuf)) 
      {
         _buffer            = newBuf;
         _numAllocatedBytes = _numValidBytes;
      }
      else return B_ERROR;
   }
   return B_NO_ERROR;
}

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
      if (_allocStrategy) _allocStrategy->Free(_buffer, _numAllocatedBytes); else muscleFree(_buffer);
      _buffer = NULL;
      _numValidBytes = _numAllocatedBytes = 0;
   }
   else SetNumBytes(0, false);
}

void ByteBuffer :: PrintToStream(uint32 maxBytesToPrint, uint32 numColumns, FILE * optFile) const
{
   PrintHexBytes(GetBuffer(), muscleMin(maxBytesToPrint, GetNumBytes()), "ByteBuffer", numColumns, optFile);
}

static void ClearBufferFunc(ByteBuffer * buf, void *) {buf->Clear(buf->GetNumBytes() > 100);}
static ByteBufferRef::ItemPool _bufferPool(100, ClearBufferFunc);
ByteBufferRef::ItemPool * GetByteBufferPool() {return &_bufferPool;}

ByteBufferRef GetByteBufferFromPool(uint32 numBytes, const uint8 * optBuffer) {return GetByteBufferFromPool(_bufferPool, numBytes, optBuffer);}
ByteBufferRef GetByteBufferFromPool(ObjectPool<ByteBuffer> & pool, uint32 numBytes, const uint8 * optBuffer)
{
   ByteBufferRef ref(pool.ObtainObject());
   if ((ref())&&(ref()->SetBuffer(numBytes, optBuffer) != B_NO_ERROR)) ref.Reset();  // return NULL ref on out-of-memory
   return ref;
}

ByteBufferRef GetByteBufferFromPool(const Flattenable & flattenMe) {return GetByteBufferFromPool(_bufferPool, flattenMe);}
ByteBufferRef GetByteBufferFromPool(ObjectPool<ByteBuffer> & pool, const Flattenable & flattenMe)
{
   ByteBufferRef ref(pool.ObtainObject());
   if ((ref() == NULL)||(ref()->SetNumBytes(flattenMe.FlattenedSize(), false) != B_NO_ERROR)) return ByteBufferRef();
   flattenMe.FlattenToByteBuffer(*ref());
   return ref;
}

// These Flattenable methods are implemented here so that if you don't use them, you
// don't need to include ByteBuffer.o in your Makefile.  If you do use them, then you
// needed to include ByteBuffer.o in your Makefile anyway.

Ref<ByteBuffer> Flattenable :: FlattenToByteBuffer() const
{
   ByteBufferRef bufRef = GetByteBufferFromPool(FlattenedSize());
   if (bufRef()) Flatten(bufRef()->GetBuffer());
   return bufRef;
}

status_t Flattenable :: FlattenToByteBuffer(ByteBuffer & outBuf) const
{
   if (outBuf.SetNumBytes(FlattenedSize(), false) != B_NO_ERROR) return B_ERROR;
   Flatten(outBuf.GetBuffer());
   return B_NO_ERROR;
}

status_t Flattenable :: UnflattenFromByteBuffer(const ByteBuffer & buf) 
{
   return Unflatten(buf.GetBuffer(), buf.GetNumBytes());
}

status_t Flattenable :: UnflattenFromByteBuffer(const ConstRef<ByteBuffer> & buf)
{
   return buf() ? Unflatten(buf()->GetBuffer(), buf()->GetNumBytes()) : B_ERROR;
}

}; // end namespace muscle

