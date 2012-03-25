/* This file is Copyright 2000-2011 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleFilePathInfo_h
#define MuscleFilePathInfo_h

#include "util/TimeUtilityFunctions.h"

namespace muscle {

/** A cross-platform API for examining the attributes of a particular file. */
class FilePathInfo
{
public:
   /** Default constructor:  creates an invalid FilePathInfo object.  */
   FilePathInfo() {Reset();}

   /** Constructor
     * @param optFilePath The path to the directory to open.  This is the same as calling SetFilePath(optFilePath).
     */
   FilePathInfo(const char * optFilePath) {SetFilePath(optFilePath);}

   /** Destructor.  */
   ~FilePathInfo() {/* empty */}

   /** Returns true iff something exists at the specified path. */
   bool Exists() const {return ((_flags & (1L<<FPI_FLAG_EXISTS)) != 0);}

   /** Returns true iff the item at the specified path is a regular data file. */
   bool IsRegularFile() const {return ((_flags & (1L<<FPI_FLAG_ISREGULARFILE)) != 0);}

   /** Returns true iff the item at the specified path is a directory. */
   bool IsDirectory()    const {return ((_flags & (1L<<FPI_FLAG_ISDIRECTORY)) != 0);}

   /** Returns true iff the item at the specified path is a directory. */
   bool IsSymLink()     const {return ((_flags & (1L<<FPI_FLAG_ISSYMLINK)) != 0);}

   /** Returns the current size of the file */
   uint64 GetFileSize() const {return _size;}

   /** Returns the most recent access time, in microseconds since 1970.  Note that
     * not all filesystems update this time, so it may not be correct.
     */
   uint64 GetAccessTime() const {return _atime;}

   /** Returns the most recent modification time, in microseconds since 1970.  */
   uint64 GetModificationTime() const {return _mtime;}

   /** Returns the most creation time, in microseconds since 1970.  */
   uint64 GetCreationTime() const {return _ctime;}

   /** Sets this object's state to reflect the item that exists at (optFilePath)
     * @param optFilePath The path to examine.  SetFilePath(NULL) is the same as calling Reset().
     */
   void SetFilePath(const char * optFilePath);

   /** Resets this object to its default/invalid state */
   void Reset();

private:
#ifdef WIN32
   uint64 InternalizeFileTime(const FILETIME & ft) const;
#else
   uint64 InternalizeTimeSpec(const struct timespec & ts) const {return ((((uint64)ts.tv_sec)*MICROS_PER_SECOND)+(((uint64)ts.tv_nsec)/1000));}
   uint64 InternalizeTimeT(time_t t) const {return (t==((time_t)-1)) ? 0 : (((uint64)t)*MICROS_PER_SECOND);}
#endif

   enum { 
      FPI_FLAG_EXISTS = 0,
      FPI_FLAG_ISREGULARFILE,
      FPI_FLAG_ISDIRECTORY,
      FPI_FLAG_ISSYMLINK,
      NUM_FPI_FLAGS
   };

   uint32 _flags;
   uint64 _size;   // file size
   uint64 _atime;  // access time
   uint64 _ctime;  // creation time
   uint64 _mtime;  // modification time
};

}; // end namespace muscle

#endif
