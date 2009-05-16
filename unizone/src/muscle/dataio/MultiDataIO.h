/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleMultiDataIO_h
#define MuscleMultiDataIO_h

#include "dataio/DataIO.h"
#include "util/Queue.h"

namespace muscle {
 
/** This DataIO holds a list of one or more other DataIO objects, and passes any
  * calls made to all the sub-DataIOs.  If an error occurs on any of the sub-objects,
  * the call will error out.  This class can be useful when implementing RAID-like behavior.
  */
class MultiDataIO : public DataIO
{
public:
   /** Default Constructor.  Be sure to add some child DataIOs to our Queue of
     * DataIOs (as returned by GetChildDataIOs()) so that this object will do something useful!
     */
   MultiDataIO() {/* empty */}

   /** Virtual destructor, to keep C++ honest.  */
   virtual ~MultiDataIO() {/* empty */}

   /** Implemented to read data from the first held DataIO only.  The other DataIOs will
     * have Seek() called on them instead, to simulate a read without actually having
     * to read their data (since we only have one memory-buffer to place it in anyway).
     */
   virtual int32 Read(void * buffer, uint32 size);

   /** Calls Write() on all our held sub-DataIOs
     * have Seek() called on them instead, to simulate a read without actually having
     * to read their data.
     */
   virtual int32 Write(const void * buffer, uint32 size);

   virtual status_t Seek(int64 offset, int whence) {return SeekAll(0, offset, whence);}
   virtual int64 GetPosition() const {return HasChildren() ? GetFirstChild()->GetPosition():-1;}
   virtual uint64 GetOutputStallLimit() const {return HasChildren() ? GetFirstChild()->GetOutputStallLimit() : MUSCLE_TIME_NEVER;}

   virtual void FlushOutput() ;

   virtual void Shutdown() {_childIOs.Clear();}

   virtual const ConstSocketRef & GetSelectSocket() const {return (HasChildren()) ? GetFirstChild()->GetSelectSocket() : GetNullSocket();}

   virtual status_t GetReadByteTimeStamp(int32 whichByte, uint64 & retStamp) const {return HasChildren() ? GetFirstChild()->GetReadByteTimeStamp(whichByte, retStamp) : B_ERROR;}

   virtual bool HasBufferedOutput() const;
   virtual void WriteBufferedOutput();

   /** Returns a read-only reference to our list of child DataIO objects. */
   const Queue<DataIORef> & GetChildDataIOs() const {return _childIOs;}

   /** Returns a read/write reference to our list of child DataIO objects. */
   Queue<DataIORef> & GetChildDataIOs() {return _childIOs;}

private:
   bool HasChildren() const {return (_childIOs.HasItems());}
   DataIO * GetFirstChild() const {return (_childIOs.Head()());}
   status_t SeekAll(uint32 first, int64 offset, int whence);

   Queue<DataIORef> _childIOs;
};

}; // end namespace muscle

#endif
