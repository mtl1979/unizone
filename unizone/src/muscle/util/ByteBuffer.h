/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleByteBuffer_h
#define MuscleByteBuffer_h

#include <string.h>
#include "util/FlatCountable.h"

namespace muscle {

/** This class is used to hold a raw buffer of bytes, and is also Flattenable and RefCountable. */
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
   ByteBuffer(uint32 numBytes = 0, const uint8 * buffer = NULL, bool copyBuffer = true) : _buffer(NULL), _numValidBytes(0), _numAllocatedBytes(0) {(void) SetBuffer(numBytes, buffer, copyBuffer);}
  
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
     * @param numBytes Number of bytes to copy in (or just allocate, if (buffer) is NULL).  Defaults to zero bytes (i.e., don't allocate a buffer)
     * @param buffer Points to bytes that we will have in our internal buffer (see the copyBuffer parameter, below, for the exact semantics of this).
     *               If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
     * @param copyBuffer If true, (buffer) is copied into a separately allocated internal buffer, and we do not assume ownership of (buffer).
     *                   If false, (buffer) is not copied, but instead we assume ownership of (buffer) and will delete[] it when we are done with it.
     *                   Defaults to true.  If you set this false, then (buffer) must have been allocated with new[].
     * @param B_NO_ERROR on success, or B_ERROR on failure (out of memory--there are no side effects if this occurs)
     */ 
   status_t SetBuffer(uint32 numBytes = 0, const uint8 * buffer = NULL, bool copyBuffer = true);

   /** Resets us to not holding any buffer
     * @param releaseBuffer If true, we will delete any memory we are holding; otherwise we will keep it for later re-use.
     */
   void Clear(bool releaseBuffer = false);

   /** Causes us to allocate/reallocate our buffer as necessary to be the given size.
     * @param newNumBytes New desired length for our buffer
     * @param retainData If true, we will take steps to ensure our current data is retained (as much as possible).
     *                   Otherwise, the contents of the new buffer will be uninitialized/undefined.
     * @return B_NO_ERROR on success, or B_ERROR on out-of-memory.
     */
   status_t SetNumBytes(uint32 newNumBytes, bool retainData);

   /** Causes us to forget the byte buffer we were holding, without deleting it.  Once this method 
     * is called, the calling code becomes responsible for array-deleting our (previously held) buffer.
     * So make sure you are holding a pointer to our data (via GetBuffer()) before you call this!
     */
   void ReleaseBuffer() {_buffer = NULL; _numValidBytes = _numAllocatedBytes = 0;}

   // Flattenable interface
   virtual bool IsFixedSize() const {return false;}
   virtual uint32 TypeCode() const {return B_RAW_TYPE;}
   virtual uint32 FlattenedSize() const {return _numValidBytes;}
   virtual void Flatten(uint8 *buffer) const {memcpy(buffer, _buffer, _numValidBytes);}
   virtual bool AllowsTypeCode(uint32) const {return true;}
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
 *  ByteBuffer allocations and deletions by recycling the ByteBuffer objects.
 */
ByteBufferRef::ItemPool * GetByteBufferPool();

/** Convenience method:  Gets a ByteBuffer from the ByteBuffer pool, makes it equal to (copyMe), and returns a reference to it.
 *  @param numBytes Number of bytes to copy in (or just allocate, if (buffer) is NULL).  Defaults to zero bytes (i.e. allocate an empty buffer)
 *  @param buffer Points to bytes that we will have in our internal buffer (see the copyBuffer parameter, below, for the exact semantics of this).
 *                If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
 *  @param copyBuffer If true, (buffer) is copied into a separately allocated internal buffer, and we do not assume ownership of (buffer).
 *                    If false, (buffer) is not copied, but instead we assume ownership of (buffer) and will delete[] it when we are done with it.
 *                    Defaults to true.  If you set this false, then (buffer) must have been allocated with new[].
 *  @return Reference to a ByteBuffer object as specified, or a NULL ref on failure (out of memory).
 */
ByteBufferRef GetByteBufferFromPool(uint32 numBytes = 0, const uint8 * buffer = NULL, bool copyBuffer = true);

/** Convenience method:  Gets a ByteBuffer from the ByteBuffer pool, makes it equal to (copyMe), and returns a reference to it.
 *  @param copyMe A ByteBuffer to clone.
 *  @return Reference to a ByteBuffer object as specified, or a NULL ref on failure (out of memory).
 */
ByteBufferRef GetByteBufferFromPool(const ByteBuffer & copyMe);               

};  // end namespace muscle

#endif
