/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleNullDataIO_h
#define MuscleNullDataIO_h

#include "dataio/DataIO.h"

namespace muscle {

/**
 *  Data I/O equivalent to /dev/null.  
 */
class NullDataIO : public DataIO
{
public:
   /** Constructor. 
     * @param selectSocket Optional ConstSocketRef to return in GetSelectSocket().  Defaults to a NULL ref.
     */
   NullDataIO(const ConstSocketRef & selectSocket = ConstSocketRef()) : _selectSocket(selectSocket), _shutdown(false) {/* empty */}

   /** Virtual Destructor, to keep C++ honest */
   virtual ~NullDataIO() {/* empty */}

   /** 
    *  No-op method, always returns zero (except if Shutdown() was called).
    *  @param buffer Points to a buffer to read bytes into (ignored).
    *  @param size Number of bytes in the buffer (ignored).
    *  @return zero.
    */
   virtual int32 Read(void * buffer, uint32 size)  {(void) buffer; (void) size; return _shutdown ? -1 : 0;}

   /** 
    *  No-op method, always returns (size) (except if Shutdown() was called).
    *  @param buffer Points to a buffer to write bytes from (ignored).
    *  @param size Number of bytes in the buffer (ignored).
    *  @return (size).
    */
   virtual int32 Write(const void * buffer, uint32 size) {(void) buffer; return _shutdown ? -1 : (int32)size;}

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

   /** Returns the socket specified in our constructor (if any) */
   virtual const ConstSocketRef & GetSelectSocket() const {return _selectSocket;}

private:
   ConstSocketRef _selectSocket;
   bool _shutdown;
};

}; // end namespace muscle

#endif
