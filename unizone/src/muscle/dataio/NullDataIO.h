/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleNullDataIO_h
#define MuscleNullDataIO_h

#include "dataio/DataIO.h"

BEGIN_NAMESPACE(muscle);

/**
 *  Data I/O equivalent to /dev/null.  
 */
class NullDataIO : public DataIO
{
public:
   /** Default Constructor. */
   NullDataIO() : _shutdown(false) {/* empty */}

   /** Virtual Destructor, to keep C++ honest */
   virtual ~NullDataIO() {/* empty */}

   /** 
    *  No-op method, always returns zero (except if Shutdown() was called).
    *  @param buffer Points to a buffer to read bytes into (ignored).
    *  @param size Number of bytes in the buffer (ignored).
    *  @return zero.
    */
   virtual int32 Read(void * /*buffer*/, uint32 /*size*/)  {return _shutdown ? -1 : 0;}

   /** 
    *  No-op method, always returns (size) (except if Shutdown() was called).
    *  @param buffer Points to a buffer to write bytes from (ignored).
    *  @param size Number of bytes in the buffer (ignored).
    *  @return (size).
    */
   virtual int32 Write(const void * /*buffer*/, uint32 size) {return _shutdown ? -1 : size;}

   /**
    *  This method always returns B_ERROR.
    */
   virtual status_t Seek(int64 /*seekOffset*/, int /*whence*/) {return B_ERROR;}

   /** 
    *  This method always return -1.
    */
   virtual int64 GetPosition() const {return -1;}

   /** 
    *  No-op method.
    *  This method doesn't do anything at all.
    */
   virtual void FlushOutput() {/* empty */}

   /** Disable us! */ 
   virtual void Shutdown() {_shutdown = true;}

   /** Can't select on this one, sorry */
   virtual int GetSelectSocket() const {return -1;}

private:
   bool _shutdown;
};

END_NAMESPACE(muscle);

#endif
