/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "dataio/FileDescriptorDataIO.h"

#ifdef __linux__
#include <linux/unistd.h>
#include <sys/types.h>
_syscall5(int, _llseek, uint, fd, ulong, hi, ulong, lo, loff_t *, res, uint, wh);  // scary --jaf
#endif  

namespace muscle {

FileDescriptorDataIO ::
FileDescriptorDataIO(int fd, bool blocking) : _fd(fd)
{
   SetBlockingIOEnabled(blocking);
}

FileDescriptorDataIO ::
~FileDescriptorDataIO() 
{
   if (_fd >= 0) close(_fd);
}

int32 FileDescriptorDataIO :: Read(void * buffer, uint32 size)  
{
   return (_fd >= 0) ? ConvertReturnValueToMuscleSemantics(read(_fd, buffer, size), size, _blocking) : -1;
}

int32 FileDescriptorDataIO :: Write(const void * buffer, uint32 size)
{
   return (_fd >= 0) ? ConvertReturnValueToMuscleSemantics(write(_fd, buffer, size), size, _blocking) : -1;
}

void FileDescriptorDataIO :: FlushOutput()
{
   // empty
}

status_t FileDescriptorDataIO :: SetBlockingIOEnabled(bool blocking)
{
   if (_fd >= 0)
   {
      if (fcntl(_fd, F_SETFL, blocking ? 0 : O_NONBLOCK) == 0)
      {
         _blocking = blocking;
         return B_NO_ERROR;
      }
      else
      {
         perror("FileDescriptorDataIO:SetBlockingIO failed");
         return B_ERROR;
      }
   }
   else return B_ERROR;
}

void FileDescriptorDataIO :: Shutdown()
{
   if (_fd >= 0) 
   {
      close(_fd);
      _fd = -1;
   }
}

status_t FileDescriptorDataIO :: Seek(int64 offset, int whence)
{
   if (_fd >= 0)
   {
      switch(whence)
      {
         case IO_SEEK_SET:  whence = SEEK_SET;  break;
         case IO_SEEK_CUR:  whence = SEEK_CUR;  break;
         case IO_SEEK_END:  whence = SEEK_END;  break;
         default:           return B_ERROR;
      }
#ifdef __linux__
      loff_t spot;
      return (_llseek(_fd, (uint32)((offset >> 32) & 0xFFFFFFFF), (uint32)(offset & 0xFFFFFFFF), &spot, whence) >= 0) ? B_NO_ERROR : B_ERROR;   
#else
      return (lseek(_fd, (off_t) offset, whence) >= 0) ? B_NO_ERROR : B_ERROR; 
#endif
   }
   return B_ERROR;
}

int64 FileDescriptorDataIO :: GetPosition() const
{
   if (_fd >= 0)
   {
#ifdef __linux__
      loff_t spot;
      return (_llseek(_fd, 0, 0, &spot, SEEK_CUR) == 0) ? spot : -1;
#else
      return lseek(_fd, 0, SEEK_CUR);
#endif
   }
   return -1;
}

};  // end namespace muscle
