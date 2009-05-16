/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleByteBuffer_h
#define MuscleByteBuffer_h

#include "util/FlatCountable.h"

namespace muscle {

class IMemoryAllocationStrategy;

/** This class is used to hold a raw buffer of bytes, and is also Flattenable and RefCountable. */
class ByteBuffer : public FlatCountable
{
public:
   /** Constructs a ByteBuffer that holds the specified bytes.
     * @param numBytes Number of bytes to copy in (or just allocate, if (optBuffer) is NULL).  Defaults to zero bytes (i.e., don't allocate a buffer)
     * @param optBuffer May be set to point to an array of (numBytes) bytes that we should copy into our internal buffer.
     *                  If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
     * @param optAllocationStrategy If non-NULL, this object will be used to allocate and free bytes.  If left as NULL (the default),
     *                              then muscleAlloc(), muscleRealloc(), and/or muscleFree() will be called as necessary.
     */
   ByteBuffer(uint32 numBytes = 0, const uint8 * optBuffer = NULL, IMemoryAllocationStrategy * optAllocationStrategy = NULL) : _buffer(NULL), _numValidBytes(0), _numAllocatedBytes(0), _allocStrategy(optAllocationStrategy) {(void) SetBuffer(numBytes, optBuffer);}
  
   /** Copy Constructor. 
     * @param copyMe The ByteBuffer to become a copy of.  We will also use (copyMe)'s allocation strategy pointer.
     */
   ByteBuffer(const ByteBuffer & copyMe) : FlatCountable(), _buffer(NULL), _numValidBytes(0), _numAllocatedBytes(0), _allocStrategy(copyMe._allocStrategy) {*this = copyMe;}
  
   /** Destructor.  Deletes our held byte buffer. */
   virtual ~ByteBuffer() {Clear(true);}

   /** Assigment operator.  Copies the byte buffer from (rhs).  If there is an error copying (out of memory), we become an empty ByteBuffer.
    *  @note We do NOT adopt (rhs)'s allocation strategy pointer!
    */
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

   /** Prints the contents of this ByteBuffer to stdout, or to the specified file.  Useful for quick debugging.
     * @param maxBytesToPrint The maximum number of bytes we should actually print.  Defaults to MUSCLE_NO_LIMIT, meaning
     *                        that by default we will always print every byte held by this ByteBuffer.
     * @param numColumns The number of columns to format the bytes into.  Defaults to 16.  See the documentation for
     *                   the PrintHexBytes() function for further details.
     * @param optFile If specified, the bytes will be printed to this file.  Defaults to NULL, meaning that the bytes
     *                will be printed to stdout.
     */
   void PrintToStream(uint32 maxBytesToPrint = MUSCLE_NO_LIMIT, uint32 numColumns = 16, FILE * optFile = NULL) const;

   /** Sets our content using the given byte buffer.
     * @param numBytes Number of bytes to copy in (or just to allocate, if (optBuffer) is NULL).  Defaults to zero bytes (i.e., don't allocate a buffer)
     * @param optBuffer May be set to point to an array of bytes to copy into our internal buffer.
     *                  If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory--there are no side effects if this occurs)
     */ 
   status_t SetBuffer(uint32 numBytes = 0, const uint8 * optBuffer = NULL);

   /** This method is similar to SetBuffer(), except that instead of copying the bytes out of (optBuffer),
     * we simply assume ownership of (optBuffer) for ourself.  This means that this ByteBuffer object will
     * free the passed-in array-pointer later on, so you must be very careful to make sure that that is
     * the right thing to do!  If you aren't sure, call SetBuffer() instead.
     * @param numBytes Number of bytes that optBuffer points to.
     * @param optBuffer Pointer to an array to adopt.  Note that we take ownership of this array!
     */
   void AdoptBuffer(uint32 numBytes, uint8 * optBuffer);

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
     * @note Allocation strategy pointers get swapped by this operation.
     */
   void SwapContents(ByteBuffer & swapWith)
   {
      muscleSwap(_buffer,            swapWith._buffer);
      muscleSwap(_numValidBytes,     swapWith._numValidBytes);
      muscleSwap(_numAllocatedBytes, swapWith._numAllocatedBytes);
      muscleSwap(_allocStrategy,     swapWith._allocStrategy);
   }

   // Flattenable interface
   virtual bool IsFixedSize() const {return false;}
   virtual uint32 TypeCode() const {return B_RAW_TYPE;}
   virtual uint32 FlattenedSize() const {return _numValidBytes;}
   virtual void Flatten(uint8 *buffer) const {memcpy(buffer, _buffer, _numValidBytes);}
   virtual bool AllowsTypeCode(uint32 code) const {(void) code; return true;}
   virtual status_t Unflatten(const uint8 *buf, uint32 size) {return SetBuffer(size, buf);}

   /** Returns a 32-bit checksum corresponding to this ByteBuffer's contetns.
     * Note that this method is O(N).
     */
   uint32 CalculateChecksum() const {return muscle::CalculateChecksum(_buffer, _numValidBytes);}

   /** Sets our allocation strategy pointer.  Note that you should be careful when you call this,
    *  as changing strategies can lead to allocation/deallocation method mismatches.
    *  @param imas Pointer to the new allocation strategy to use from now on.
    */
   void SetMemoryAllocationStrategy(IMemoryAllocationStrategy * imas) {_allocStrategy = imas;}

   /** Returns the current value of our allocation strategy pointer (may be NULL if the default strategy is in use) */
   IMemoryAllocationStrategy * GetMemoryAllocationStrategy() const {return _allocStrategy;}

protected:
   /** Overridden to set our buffer directly from (copyFrom)'s Flatten() method */
   virtual status_t CopyFromImplementation(const Flattenable & copyFrom);

private:
   uint8 * _buffer;            // pointer to our byte array (or NULL if we haven't got one)
   uint32 _numValidBytes;      // number of bytes the user thinks we have
   uint32 _numAllocatedBytes;  // number of bytes we actually have
   IMemoryAllocationStrategy * _allocStrategy;
};
DECLARE_REFTYPES(ByteBuffer);

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

/** As above, except that the byte buffer is obtained from the specified pool instead of from the default ByteBuffer pool.
 *  @param pool the ObjectPool to allocate the ByteBuffer from.
 *  @param numBytes Number of bytes to copy in (or just allocate, if (optBuffer) is NULL).  Defaults to zero bytes (i.e. retrieve an empty buffer)
 *  @param optBuffer If non-NULL, points to an array of (numBytes) bytes to copy in to our internal buffer. 
 *                   If NULL, this ByteBuffer will contain (numBytes) uninitialized bytes.  Defaults to NULL.
 *  @return Reference to a ByteBuffer object that has been initialized as specified, or a NULL ref on failure (out of memory).
 */
ByteBufferRef GetByteBufferFromPool(ObjectPool<ByteBuffer> & pool, uint32 numBytes = 0, const uint8 * optBuffer = NULL);

/** Convenience method:  Gets a ByteBuffer from the default ByteBuffer pool, flattens (flattenMe) into the byte buffer, and 
 *  returns a reference to the new ByteBuffer.
 *  @param flattenMe A Flattenable object to flatten.
 *  @return Reference to a ByteBuffer object as specified, or a NULL ref on failure (out of memory).
 */
ByteBufferRef GetByteBufferFromPool(const Flattenable & flattenMe);               

/** Convenience method:  Gets a ByteBuffer from the specified ByteBuffer pool, flattens (flattenMe) into the byte buffer, and 
 *  returns a reference to the new ByteBuffer.
 *  @param pool The ObjectPool to retrieve the new ByteBuffer object from.
 *  @param flattenMe A Flattenable object to flatten.
 *  @return Reference to a ByteBuffer object as specified, or a NULL ref on failure (out of memory).
 */
ByteBufferRef GetByteBufferFromPool(ObjectPool<ByteBuffer> & pool, const Flattenable & flattenMe);

/** This interface is used to represent any object that knows how to allocate, reallocate, and free memory in a special way. */
class IMemoryAllocationStrategy 
{
public:
   /** Default constructor */
   IMemoryAllocationStrategy() {/* empty */}

   /** Destructor */
   virtual ~IMemoryAllocationStrategy() {/* empty */}

   /** Called when a ByteBuffer needs to allocate a memory buffer.  This method should be implemented to behave similarly to malloc().
    *  @param size Number of bytes to allocate
    *  @returns A pointer to the allocated bytes on success, or NULL on failure.
    */
   virtual void * Malloc(size_t size) = 0;

   /** Called when a ByteBuffer needs to resize a memory buffer.  This method should be implemented to behave similarly to realloc().
    *  @param ptr Pointer to the buffer to resize, or NULL if there is no current buffer.
    *  @param newSize Desired new size of the buffer
    *  @param oldSize Current size of the buffer
    *  @param retainData If false, the returned buffer need not retain the contents of the old buffer.
    *  @returns A pointer to the new buffer on success, or NULL on failure (or if newSize == 0)
    */
   virtual void * Realloc(void * ptr, size_t newSize, size_t oldSize, bool retainData) = 0;

   /** Called when a ByteBuffer needs to free a memory buffer.  This method should be implemented to behave similarly to free().
    *  @param ptr Pointer to the buffer to free.
    *  @param size Number of byes in the buffer.
    */
   virtual void Free(void * ptr, size_t size) = 0;
};

}; // end namespace muscle

#endif
