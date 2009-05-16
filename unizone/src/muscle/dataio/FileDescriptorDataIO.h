/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleFileDescriptorDataIO_h
#define MuscleFileDescriptorDataIO_h

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "dataio/DataIO.h"
#include "util/NetworkUtilityFunctions.h"

namespace muscle {

/**
 *  Data I/O to and from a file descriptor (useful for talking to Linux device drivers and the like)
 */
class FileDescriptorDataIO : public DataIO
{
public:
   /**
    *  Constructor.
    *  @param fd The file descriptor to use.  Becomes property of this FileDescriptorDataIO object.
    *  @param blocking determines whether to use blocking or non-blocking I/O.
    *  If you will be using this object with a AbstractMessageIOGateway,
    *  and/or select(), then it's usually better to set blocking to false.
    */
   FileDescriptorDataIO(const ConstSocketRef & fd, bool blocking);

   /** Destructor.
    *  close()'s the held file descriptor.
    */
   virtual ~FileDescriptorDataIO();

   /** Reads bytes from the file descriptor and places them into (buffer).
    *  @param buffer Buffer to write the bytes into
    *  @param size Number of bytes in the buffer.
    *  @return Number of bytes read, or -1 on error.
    *  @see DataIO::Read()
    */
   virtual int32 Read(void * buffer, uint32 size);

   /**
    * Reads bytes from (buffer) and sends them out to the file descriptor.
    *  @param buffer Buffer to read the bytes from.
    *  @param size Number of bytes in the buffer.
    *  @return Number of bytes written, or -1 on error.
    *  @see DataIO::Write()    
    */
   virtual int32 Write(const void * buffer, uint32 size);

   /**
    *  Implemented as a no-op (I don't believe file descriptors need flushing?)
    */
   virtual void FlushOutput();
   
   /**
    * Enables or disables blocking I/O on this file descriptor.
    * If this object is to be used by an AbstractMessageIOGateway,
    * then non-blocking I/O is usually better to use.
    * @param blocking If true, file descriptor is set to blocking I/O mode.  Otherwise, non-blocking I/O.
    * @return B_NO_ERROR on success, B_ERROR on error.
    */
   status_t SetBlockingIOEnabled(bool blocking);

   /** Returns true iff this object is using blocking I/O mode. */
   bool IsBlockingIOEnabled() const {return _blocking;}

   /** Clears our held ConstSocketRef. */
   virtual void Shutdown();

   /** Seeks to the specified point in the file stream.
    *  @param offset Where to seek to.
    *  @param whence IO_SEEK_SET, IO_SEEK_CUR, or IO_SEEK_END. 
    *  @return B_NO_ERROR on success, B_ERROR on failure.
    */ 
   virtual status_t Seek(int64 offset, int whence);
   
   /** Returns our current position in the file */
   virtual int64 GetPosition() const;

   /** Returns our file descriptor */
   virtual const ConstSocketRef & GetSelectSocket() const {return _fd;}

private:
   ConstSocketRef _fd;
   bool _blocking;
};

}; // end namespace muscle

#endif
