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
   if (IsByteInLocalBuffer(buffer))
   {
      // Special logic for handling it when the caller wants our bytes-array to become a subset of its former self.
      uint32 numReadableBytes = ((_buffer+_numValidBytes)-buffer);
      if (numBytes > numReadableBytes)
      {
         LogTime(MUSCLE_LOG_CRITICALERROR, "ByteBuffer::SetBuffer();  Attempted to read "UINT32_FORMAT_SPEC" bytes off the end of our internal buffer!\n", numBytes-numReadableBytes);
         return B_ERROR;
      }
      else
      {
         if (buffer > _buffer) memmove(_buffer, buffer, numBytes);
         return SetNumBytes(numBytes, true);
      }
   }
   else
   {
      Clear(numBytes<(_numAllocatedBytes/2));  // FogBugz #6933: if the new buffer takes up less than half of our current space, toss it
      if (SetNumBytes(numBytes, false) != B_NO_ERROR) return B_ERROR;
      if ((buffer)&&(_buffer)) memcpy(_buffer, buffer, numBytes);
      return B_NO_ERROR;
   }
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

status_t ByteBuffer :: AppendBytes(const uint8 * bytes, uint32 numBytes, bool allocExtra)
{
   if (numBytes == 0) return B_NO_ERROR;

   if ((bytes)&&(IsByteInLocalBuffer(bytes))&&((_numValidBytes+numBytes)>_numAllocatedBytes))
   {
      // Oh dear, caller wants us to add a copy of some of our own bytes to ourself, AND we'll need to perform a reallocation to do it!
      // So to avoid freeing (bytes) before we read from them, we're going to copy them over to a temporary buffer first.
      uint8 * tmpBuf = newnothrow uint8[numBytes];
      if (tmpBuf) memcpy(tmpBuf, bytes, numBytes);
             else {WARN_OUT_OF_MEMORY; return B_ERROR;}
      status_t ret = AppendBytes(tmpBuf, numBytes, allocExtra);
      delete [] tmpBuf;
      return ret;
   }

   uint32 oldTotalSize = _numValidBytes;  // save this value since SetNumBytes() will change it
   uint32 newTotalSize = _numValidBytes+numBytes;
   uint32 allocSize    = ((newTotalSize > _numAllocatedBytes)&&(allocExtra)) ? muscleMax(newTotalSize*4, (uint32)128) : newTotalSize;
   if (SetNumBytes(allocSize, true) != B_NO_ERROR) return B_ERROR;
   if (bytes != NULL) memcpy(_buffer+oldTotalSize, bytes, numBytes);
   _numValidBytes = newTotalSize;

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

ByteBuffer operator+(const ByteBuffer & lhs, const ByteBuffer & rhs)
{
   ByteBuffer ret;
   if (ret.SetNumBytes(lhs.GetNumBytes()+rhs.GetNumBytes(), false) == B_NO_ERROR)
   {
      memcpy(ret.GetBuffer(), lhs.GetBuffer(), lhs.GetNumBytes());
      memcpy(ret.GetBuffer()+lhs.GetNumBytes(), rhs.GetBuffer(), rhs.GetNumBytes());
   }
   return ret;
}

static ByteBufferRef::ItemPool _bufferPool;
ByteBufferRef::ItemPool * GetByteBufferPool() {return &_bufferPool;}
const ByteBuffer & GetEmptyByteBuffer() {return _bufferPool.GetDefaultObject();}

static const ConstByteBufferRef _emptyBufRef(&_bufferPool.GetDefaultObject(), false);
ConstByteBufferRef GetEmptyByteBufferRef() {return _emptyBufRef;}

ByteBufferRef GetByteBufferFromPool(uint32 numBytes, const uint8 * optBuffer) {return GetByteBufferFromPool(_bufferPool, numBytes, optBuffer);}
ByteBufferRef GetByteBufferFromPool(ObjectPool<ByteBuffer> & pool, uint32 numBytes, const uint8 * optBuffer)
{
   ByteBufferRef ref(pool.ObtainObject());
   if ((ref())&&(ref()->SetBuffer(numBytes, optBuffer) != B_NO_ERROR)) ref.Reset();  // return NULL ref on out-of-memory
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

