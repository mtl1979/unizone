/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleMemoryBufferDataIO_h
#define MuscleMemoryBufferDataIO_h

#include "dataio/DataIO.h"

namespace muscle {

/**
 *  Data I/O equivalent for reading from/writing in-memory arrays
 */
class MemoryBufferDataIO : public DataIO
{
public:
   /** Constructor.
    *  @param readBuf A byte buffer to read from.  May be NULL if no readable data is to be provided.  Ownership is retained by caller.
    *  @param readBufSize Number of bytes pointed to by (readBuf), or zero if (readBuf) is NULL.
    *  @param writeBuf A byte buffer to write to.  May be NULL if no write capability is to be provided.  Ownership is retained by caller.
    *  @param writeBufSize Number of bytes pointed to by (writeBuf), or zero if (writeBuf) is NULL.
    */
   MemoryBufferDataIO(const uint8 * readBuf, uint32 readBufSize, uint8 * writeBuf, uint32 writeBufSize) : _readBuf(readBuf), _readBufSize(readBufSize), _readBufOffset(0), _maxReadChunk(-1), _writeBuf(writeBuf), _writeBufSize(writeBufSize), _writeBufOffset(0), _maxWriteChunk(-1)  {/* empty */}

   /** Virtual Destructor, to keep C++ honest */
   virtual ~MemoryBufferDataIO() {/* empty */}

   /** 
    *  Copies bytes from our read buffer into (buffer).  If we have no read buffer, or no bytes left to read, returns -1.
    *  @param buffer Points to a buffer to read bytes into.
    *  @param size Number of bytes in the buffer.
    *  @return zero.
    */
   virtual int32 Read(void * buffer, uint32 size)  
   {
      int32 readBytesAvailable = _readBuf ? (_readBufSize - _readBufOffset) : -1;
      int32 copyBytes = (size < readBytesAvailable) ? size : readBytesAvailable;
      if (copyBytes > 0) 
      {
         if ((_maxReadChunk >= 0)&&(copyBytes > _maxReadChunk)) copyBytes = _maxReadChunk;
         memcpy(buffer, &_readBuf[_readBufOffset], copyBytes);
         _readBufOffset += copyBytes;
         return copyBytes;
      }
      return -1;
   }

   /** 
    *  Writes bytes into our write buffer.  If we have no write buffer, or there is no room left in the write buffer, returns -1.
    *  @param buffer Points to a buffer to write bytes from.
    *  @param size Number of bytes in the buffer.
    *  @return (size).
    */
   virtual int32 Write(const void * buffer, uint32 size) 
   {
      int32 writeBytesAvailable = _writeBuf ? (_writeBufSize - _writeBufOffset) : -1;
      int32 copyBytes = (size < writeBytesAvailable) ? size : writeBytesAvailable;
      if (copyBytes > 0) 
      {
         if ((_maxWriteChunk >= 0)&&(copyBytes > _maxWriteChunk)) copyBytes = _maxWriteChunk;
         memcpy(&_writeBuf[_writeBufOffset], buffer, copyBytes);
         _writeBufOffset += copyBytes;
         return copyBytes;
      }
      return -1;
   }

   /** Seeks to the specified point in any specified buffers (read and/or write).
    *  Note that only 32-bit seeks are supported in this implementation.
    *  @param offset Where to seek to.
    *  @param whence IO_SEEK_SET, IO_SEEK_CUR, or IO_SEEK_END. 
    *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. if neither read nor write buffers could seek successfully)
    */ 
   virtual status_t Seek(int64 offset, int whence)
   {
      int32 o = (int32) offset;
      int32 newReadOffset = -1, newWriteOffset = -1;
      if (_readBuf)
      {
         switch(whence)
         {
            case IO_SEEK_SET:  newReadOffset = o;                break;
            case IO_SEEK_CUR:  newReadOffset = _readBufOffset+o; break;
            case IO_SEEK_END:  newReadOffset = _readBufSize-o;   break;
            default:           return B_ERROR;
         }
         if (newReadOffset > _readBufSize) newReadOffset = -1;  // equal to _readBufSize is okay though
      }
      if (_writeBuf)
      {
         switch(whence)
         {
            case IO_SEEK_SET:  newWriteOffset = o;                 break;
            case IO_SEEK_CUR:  newWriteOffset = _writeBufOffset+o; break;
            case IO_SEEK_END:  newWriteOffset = _writeBufSize-o;   break;
            default:           return B_ERROR;
         }
         if (newWriteOffset > _writeBufSize) newWriteOffset = -1;  // equal to _writeBufSize is okay though
      }

      if ((newReadOffset < 0)&&(newWriteOffset < 0)) return B_ERROR;
      if (newReadOffset  >= 0) _readBufOffset  = newReadOffset;
      if (newWriteOffset >= 0) _writeBufOffset = newWriteOffset;
      return B_NO_ERROR;
   }
   
   virtual int64 GetPosition() const 
   {
           if (_readBuf)  return _readBufOffset;
      else if (_writeBuf) return _writeBufOffset;
      else                return -1;
   }

   /** 
    *  No-op method.
    *  This method doesn't do anything at all.
    */
   virtual void FlushOutput() {/* empty */}

   /** Disable us! */ 
   virtual void Shutdown() {_readBuf = _writeBuf = NULL;}

   /** Can't select on this one, sorry */
   virtual int GetSelectSocket() const {return -1;}

   /** For testing--you can call this to artificially limit the maximum number of bytes returnable in a single call to Read().
    *  @param mrc Maximum read chunk size, in bytes.  If less than zero, then no fixed limit is enforced (default behaviour)
    */
   void SetMaxReadChunk(int32 mrc) {_maxReadChunk = mrc;}

   /** For testing--you can call this to artificially limit the maximum number of bytes copyable in a single call to Write().
    *  @param mwc Maximum write chunk size, in bytes.  If less than zero, then no fixed limit is enforced (default behaviour)
    */
   void SetMaxWriteChunk(int32 mwc) {_maxWriteChunk = mwc;}

private:
   const uint8 * _readBuf;
   int32 _readBufSize;
   int32 _readBufOffset;
   int32 _maxReadChunk;

   uint8 * _writeBuf;
   int32 _writeBufSize;
   int32 _writeBufOffset;
   int32 _maxWriteChunk;
};

};  // end namespace muscle

#endif
