/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleByteBuffer_h
#define MuscleByteBuffer_h

#include <string.h>
#include "util/FlatCountable.h"

BEGIN_NAMESPACE(muscle);

/** This class is used to hold a raw buffer of bytes, and is also Flattenable and RefCountable. */
class ByteBuffer : public FlatCountable
{
public:
   /** Constructs a ByteBuffer that holds the specified bytes.
     * @param numBytes Number of bytes to copy in (or just allocate, if (optBuffer) is NULL).  Defaults to zero bytes (i.e., don't allocate a buffer)
     * @param optBuffer May be set to point to an array of (numBytes) bytes that we should copy into our internal buffer.
     *                  If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
     */
   ByteBuffer(uint32 numBytes = 0, const uint8 * optBuffer = NULL) : _buffer(NULL), _numValidBytes(0), _numAllocatedBytes(0) {(void) SetBuffer(numBytes, optBuffer);}
  
   /** Copy Constructor. 
     * @param copyMe The ByteBuffer to become a copy of.
     */
   ByteBuffer(const ByteBuffer & copyMe) : FlatCountable(), _buffer(NULL), _numValidBytes(0), _numAllocatedBytes(0) {*this = copyMe;}
  
   /** Destructor.  Deletes our held byte buffer. */
   virtual ~ByteBuffer() {Clear(true);}

   /** Assigment operator.  Copies the byte buffer from (rhs).  If there is an error copying (out of memory), we become an empty ByteBuffer. */
   ByteBuffer &operator=(const ByteBuffer & rhs) {if ((this != &rhs)&&(SetBuffer(rhs.GetNumBytes(), rhs.GetBuffer()) != B_NO_ERROR)) Clear(); return *this;}

   /** Read/Write Accessor.  Returns a pointer to our held buffer, or NULL if we are not currently holding a buffer. */
   uint8 * GetBuffer() {return _buffer;}

   /** Read-only Accessor.  Returns a pointer to our held buffer, or NULL if we are not currently holding a buffer. */
   const uint8 * GetBuffer() const {return _buffer;}

   /** Convenience synonym for GetBuffer(). */
   const uint8 * operator()() const {return _buffer;}

   /** Convenience synonym for GetBuffer(). */
   uint8 * operator()() {return _buffer;}

   /** Returns the size of our held buffer, in bytes. */
   uint32 GetNumBytes() const {return _numValidBytes;}

   /** Returns the number of bytes we have allocated internally.  Note that this
    *  number may be larger than the number of bytes we officially contain (as returned by GetNumBytes())
    */
   uint32 GetNumAllocatedBytes() {return _numAllocatedBytes;}

   /** Returns true iff (rhs) is holding data that is byte-for-byte the same as our own data */
   bool operator ==(const ByteBuffer &rhs) const {return (this == &rhs) ? true : ((GetNumBytes() == rhs.GetNumBytes()) ? (memcmp(GetBuffer(), rhs.GetBuffer(), GetNumBytes()) == 0) : false);}

   /** Returns true iff the data (rhs) is holding is different from our own (byte-for-byte). */
   bool operator !=(const ByteBuffer &rhs) const {return !(*this == rhs);}

   /** Sets our content using the given byte buffer.
     * @param numBytes Number of bytes to copy in (or just to allocate, if (optBuffer) is NULL).  Defaults to zero bytes (i.e., don't allocate a buffer)
     * @param optBuffer May be set to point to an array of bytes to copy into our internal buffer.
     *                  If NULL, this ByteBuffer will contain (numBytess) uninitialized bytes.  Defaults to NULL.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory--there are no side effects if this occurs)
     */ 
   status_t SetBuffer(uint32 numBytes = 0, const uint8 * optBuffer = NULL);

   /** Resets this ByteBuffer to its empty state, i.e. not holding any buffer.
     * @param releaseBuffer If true, we will immediately muscleFree() any buffer we are holding; otherwise we will keep the buffer around for potential later re-use.
     */
   void Clear(bool releaseBuffer = false);

   /** Causes us to allocate/reallocate our buffer as necessary to be the given size.
     * @param newNumBytes New desired length for our buffer
     * @param retainData If true, we will take steps to ensure our current data is retained (as much as possible).
     *                   Otherwise, the contents of the resized buffer will be undefined.
     * @return B_NO_ERROR on success, or B_ERROR on out-of-memory.
     */
   status_t SetNumBytes(uint32 newNumBytes, bool retainData);

   /** If we contain any extra bytes that are not being used to hold actual data (i.e. if GetNumAllocatedBytes()
    *  is returning a valud greater than GetNumBytes(), this method can be called to free up the unused bytes.
    *  This method calls muscleRealloc(), so it should be quite efficient.  After this method returns successfully,
    *  the number of allocated bytes will be equal to the number of used bytes.
    *  @returns B_NO_ERROR on success or B_ERROR on failure (although I can't imagine why muscleRealloc() would ever fail)
    */
   status_t FreeExtraBytes();

   /** Causes us to forget the byte buffer we were holding, without freeing it.  Once this method 
     * is called, the calling code becomes responsible for calling muscleFree() on our (previously held) buffer.
     * @returns a pointer to our data bytes.  It becomes the responsibility of the caller to muscleFree() this buffer
     *          when he is done with it!
     */
   const uint8 * ReleaseBuffer() {const uint8 * ret = _buffer; _buffer = NULL; _numValidBytes = _numAllocatedBytes = 0; return ret;}

   /** Swaps our contents with those of the specified ByteBuffer.  This is an efficient O(1) operation.
     * @param swapWith ByteBuffer to swap contents with.
     */
   void SwapContents(ByteBuffer & swapWith)
   {
      muscleSwap(_buffer,            swapWith._buffer);
      muscleSwap(_numValidBytes,     swapWith._numValidBytes);
      muscleSwap(_numAllocatedBytes, swapWith._numAllocatedBytes);
   }

   // Flattenable interface
   virtual bool IsFixedSize() const {return false;}
   virtual uint32 TypeCode() const {return B_RAW_TYPE;}
   virtual uint32 FlattenedSize() const {return _numValidBytes;}
   virtual void Flatten(uint8 *buffer) const {memcpy(buffer, _buffer, _numValidBytes);}
   virtual bool AllowsTypeCode(uint32 /*code*/) const {return true;}
   virtual status_t Unflatten(const uint8 *buf, uint32 size) {return SetBuffer(size, buf);}

protected:
   /** Overridden to unflatten directly from our buffer, for added efficiency */
   virtual status_t CopyToImplementation(Flattenable & copyTo) const;
   
   /** Overridden to set our buffer directly from (copyFrom)'s Flatten() method */
   virtual status_t CopyFromImplementation(const Flattenable & copyFrom);

private:
   uint8 * _buffer;            // pointer to our byte array (or NULL if we haven't got one)
   uint32 _numValidBytes;      // number of bytes the user thinks we have
   uint32 _numAllocatedBytes;  // number of bytes we actually have
};

typedef Ref<ByteBuffer> ByteBufferRef;

/** This function returns a pointer to a singleton ObjectPool that can be used to minimize the number of 
 *  ByteBuffer allocations and frees by recycling the ByteBuffer objects.
 */
ByteBufferRef::ItemPool * GetByteBufferPool();

/** Convenience method:  Gets a ByteBuffer from the ByteBuffer pool, makes sure it holds the specified number of bytes, and returns it.
 *  @param numBytes Number of bytes to copy in (or just allocate, if (optBuffer) is NULL).  Defaults to zero bytes (i.e. retrieve an empty buffer)
 *  @param optBuffer If non-NULL, points to an array of (numBytes) bytes to copy in to our internal buffer. 
 *                   If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
 *  @return Reference to a ByteBuffer object that has been initialized as specified, or a NULL ref on failure (out of memory).
 */
ByteBufferRef GetByteBufferFromPool(uint32 numBytes = 0, const uint8 * optBuffer = NULL);

/** Convenience method:  Gets a ByteBuffer from the ByteBuffer pool, makes it equal to (copyMe), and returns a reference to it.
 *  @param copyMe A ByteBuffer to clone.
 *  @return Reference to a ByteBuffer object as specified, or a NULL ref on failure (out of memory).
 */
ByteBufferRef GetByteBufferFromPool(const ByteBuffer & copyMe);               

END_NAMESPACE(muscle);

#endif
