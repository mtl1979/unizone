/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleByteBuffer_h
#define MuscleByteBuffer_h

#include <string.h>
#include "util/FlatCountable.h"

namespace muscle {

/** This class is used to hold a raw buffer of untyped bytes, and is also Flattenable and RefCountable. */
class ByteBuffer : public FlatCountable
{
public:
   /** Constructs a ByteBuffer that holds the specified bytes.
     * @param numBytes Number of bytes to copy in (or just allocate, if (buffer) is NULL).  Defaults to zero bytes (i.e., don't allocate a buffer)
     * @param buffer Points to bytes that we will have in our internal buffer (see the copyBuffer parameter, below, for the exact semantics of this).
     *               If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
     * @param copyBuffer If true, (buffer) is copied into a separately allocated internal buffer, and we do not assume ownership of (buffer).
     *                   If false, (buffer) is not copied, but instead we assume ownership of (buffer) and will delete[] it when we are done with it.
     *                   Defaults to true.  If you set this false, then (buffer) must have been allocated with new[].
     */
   ByteBuffer(uint32 numBytes = 0, const void * buffer = NULL, bool copyBuffer = true) : _buffer(NULL), _numBytes(0) {(void) SetBuffer(numBytes, buffer, copyBuffer);}
  
   /** Copy Constructor. 
     * @param copyMe The ByteBuffer to become a copy of.
     */
   ByteBuffer(const ByteBuffer & copyMe) : FlatCountable(), _buffer(NULL), _numBytes(0) {*this = copyMe;}
  
   /** Destructor.  Deletes our held byte buffer. */
   virtual ~ByteBuffer() {Clear();}

   /** Assigment operator.  Copies the byte buffer from (rhs).  If there is an error copying (out of memory), we become an empty ByteBuffer. */
   ByteBuffer &operator=(const ByteBuffer & rhs) {if ((this != &rhs)&&(SetBuffer(rhs._numBytes, rhs._buffer) != B_NO_ERROR)) Clear(); return *this;}

   /** Accessor.  Returns a pointer to our held buffer, or NULL if we are not currently holding a buffer. */
   void * GetBuffer() const {return _buffer;}

   /** Convenience synonym for GetBuffer(). */
   void * operator()() const {return _buffer;}

   /** Returns the size of our held buffer, in bytes. */
   uint32 GetNumBytes() const {return _numBytes;}

   /** Returns true iff (rhs) is holding data that is byte-for-byte the same as our own data */
   bool operator ==(const ByteBuffer &rhs) const {return (this == &rhs) ? true : ((_numBytes == rhs._numBytes) ? (memcmp(_buffer, rhs._buffer, _numBytes) == 0) : false);}

   /** Returns true iff the data (rhs) is holding is different from our own (byte-for-byte). */
   bool operator !=(const ByteBuffer &rhs) const {return !(*this == rhs);}

   /** Sets our content using the given byte buffer.
     * @param numBytes Number of bytes to copy in (or just allocate, if (buffer) is NULL).  Defaults to zero bytes (i.e., don't allocate a buffer)
     * @param buffer Points to bytes that we will have in our internal buffer (see the copyBuffer parameter, below, for the exact semantics of this).
     *               If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
     * @param copyBuffer If true, (buffer) is copied into a separately allocated internal buffer, and we do not assume ownership of (buffer).
     *                   If false, (buffer) is not copied, but instead we assume ownership of (buffer) and will delete[] it when we are done with it.
     *                   Defaults to true.  If you set this false, then (buffer) must have been allocated with new[].
     * @param B_NO_ERROR on success, or B_ERROR on failure (out of memory--there are no side effects if this occurs)
     */ 
   status_t SetBuffer(uint32 numBytes = 0, const void * buffer = NULL, bool copyBuffer = true) 
   {
      Clear();
      if (copyBuffer) 
      {
         if (SetNumBytes(numBytes, false) != B_NO_ERROR) return B_ERROR;
         if ((buffer)&&(_buffer)) memcpy(_buffer, buffer, numBytes);
      }
      else 
      {
         _buffer   = (void *)buffer;
         _numBytes = numBytes;
      }
      return B_NO_ERROR;
   }

   /** Resets us to not holding any buffer */
   void Clear() {SetNumBytes(0, false);}

   /** Causes us to allocate/reallocate our buffer as necessary to be the given size.
     * @param newNumBytes New desired length for our buffer
     * @param retainData If true, we will take steps to ensure our current data is retained (as much as possible).
     *                   Otherwise, the contents of the new buffer will be uninitialized/undefined.
     * @return B_NO_ERROR on success, or B_ERROR on out-of-memory.
     */
   status_t SetNumBytes(uint32 newNumBytes, bool retainData)
   {
      if (newNumBytes != _numBytes)
      {
         uint8 * newBuf = NULL;
         if (newNumBytes > 0)
         {
            newBuf = newnothrow uint8[newNumBytes];
            if (newBuf == NULL) 
            {
               WARN_OUT_OF_MEMORY;
               return B_ERROR;
            }
         }
         if ((retainData)&&(newBuf)&&(_buffer)) memcpy(newBuf, _buffer, muscleMin(newNumBytes, _numBytes));
         delete [] ((uint8*)_buffer);
         _buffer   = newBuf;
         _numBytes = newNumBytes;
      }
      return B_NO_ERROR;
   }

   /** Causes us to forget the byte buffer we were holding, without deleting it.  Once this method 
     * is called, the calling code becomes responsible for array-deleting our (previously held) buffer.
     * So make sure you are holding a pointer to our data (via GetBuffer()) before you call this!
     */
   void ReleaseBuffer() {_buffer = NULL; _numBytes = 0;}

   // Flattenable interface
   virtual bool IsFixedSize() const {return false;}
   virtual type_code TypeCode() const {return B_RAW_TYPE;}
   virtual uint32 FlattenedSize() const {return _numBytes;}
   virtual void Flatten(uint8 *buffer) const {memcpy(buffer, _buffer, _numBytes);}
   virtual bool AllowsTypeCode(type_code) const {return true;}
   virtual status_t Unflatten(const uint8 *buf, uint32 size) {return SetBuffer(size, buf);}

protected:
   /** Overridden to unflatten directly from our buffer, for added efficiency */
   virtual status_t CopyToImplementation(Flattenable & copyTo) const {return copyTo.Unflatten((const uint8 *)_buffer, _numBytes);}
   
   /** Overridden to set our buffer directly from (copyFrom)'s Flatten() method */
   virtual status_t CopyFromImplementation(const Flattenable & copyFrom)
   {
      uint32 numBytes = copyFrom.FlattenedSize();
      if (SetNumBytes(numBytes, false) != B_NO_ERROR) return B_ERROR;
      copyFrom.Flatten((uint8*)_buffer);
      return B_NO_ERROR;
   }

private:
   void * _buffer;
   uint32 _numBytes;
};

typedef Ref<ByteBuffer> ByteBufferRef;

};  // end namespace muscle

#endif
