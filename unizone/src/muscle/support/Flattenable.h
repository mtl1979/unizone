/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

/******************************************************************************
/
/   File:      Flattenable.h
/
/   Description:    version of Be's BFlattenable base class.
/
******************************************************************************/

#ifndef MuscleFlattenable_h
#define MuscleFlattenable_h

#include <string.h>
#include "support/MuscleSupport.h"

namespace muscle {

/** This class is an interface representing an object that knows how
 *  to save itself into an array of bytes, and recover its state from
 *  an array of bytes.
 */
class Flattenable 
{
public:
   /** Constructor */
   Flattenable() {/* empty */}

   /** Destructor */
   virtual ~Flattenable() {/* empty */}

   /** Should return true iff every object of this type has a size that is known at compile time. */
   virtual bool IsFixedSize() const = 0;

   /** Should return the type code identifying this type of object.  */
   virtual type_code TypeCode() const = 0;

   /** Should return the number of bytes needed to store this object in its current state.  */
   virtual uint32 FlattenedSize() const = 0;

   /** 
    *  Should store this object's state into (buffer). 
    *  @param buffer The bytes to write this object's stat into.  Buffer must be at least FlattenedSize() bytes long.
    */
   virtual void Flatten(uint8 *buffer) const = 0;

   /** 
    *  Should return true iff a buffer with type_code (code) can be used to reconstruct
    *  this object's state.  Defaults implementation returns true iff (code) equals TypeCode() or B_RAW_DATA.
    *  @param code A type code constant, e.g. B_RAW_TYPE or B_STRING_TYPE, or something custom.
    *  @return True iff this object can Unflatten from a buffer of the given type, false otherwise.
    */
   virtual bool AllowsTypeCode(type_code code) const {return ((code == B_RAW_TYPE)||(code == TypeCode()));}

   /** 
    *  Should attempt to restore this object's state from the given buffer.
    *  @param buf The buffer of bytes to unflatten from.
    *  @param size Number of bytes in the buffer.
    *  @return B_NO_ERROR if the Unflattening was successful, else B_ERROR.
    */
   virtual status_t Unflatten(const uint8 *buf, uint32 size) = 0;

   /** 
    *  Causes (copyTo)'s state to set from this Flattenable, if possible. 
    *  Default implementation is not very efficient, since it has to flatten
    *  this object into a byte buffer, and then unflatten the bytes back into 
    *  (copyTo).  However, you can override CopyToImplementation() to provide 
    *  a more efficient implementation when possible.
    *  @param copyTo Object to make into the equivalent of this object.  (copyTo)
    *                May be any subclass of Flattenable.
    *  @return B_NO_ERROR on success, or B_ERROR on failure (typecode mismatch, out-of-memory, etc)
    */
   status_t CopyTo(Flattenable & copyTo) const 
   {
      return (this == &copyTo) ? B_NO_ERROR : ((copyTo.AllowsTypeCode(TypeCode())) ? CopyToImplementation(copyTo) : B_ERROR);
   }

   /** 
    *  Causes our state to be set from (copyFrom)'s state, if possible. 
    *  Default implementation is not very efficient, since it has to flatten
    *  (copyFrom) into a byte buffer, and then unflatten the bytes back into 
    *  (this).  However, you can override CopyFromImplementation() to provide 
    *  a more efficient implementation when possible.
    *  @param copyFrom Object to read from to set the state of this object.  
    *                  (copyFrom) may be any subclass of Flattenable.
    *  @return B_NO_ERROR on success, or B_ERROR on failure (typecode mismatch, out-of-memory, etc)
    */
   status_t CopyFrom(const Flattenable & copyFrom)
   {
      return (this == &copyFrom) ? B_NO_ERROR : ((AllowsTypeCode(copyFrom.TypeCode())) ? CopyFromImplementation(copyFrom) : B_ERROR);
   }

   /** 
    * Convenience method for writing data into a byte buffer.
    * Writes data consecutively into a byte buffer.  The output buffer is
    * assumed to be large enough to hold the data.
    * @param outBuf Flat buffer to write to
    * @param writeOffset Offset into buffer to read from.  Incremented by (blockSize) on success.
    * @param copyFrom memory location to copy bytes from
    * @param blockSize number of bytes to copy
    */
   static void WriteData(uint8 * outBuf, uint32 * writeOffset, const void * copyFrom, uint32 blockSize)
   {
      memcpy(&outBuf[*writeOffset], copyFrom, blockSize);
      *writeOffset += blockSize;
   };
    
   /** 
    * Convenience method for safely reading bytes from a byte buffer.  (Checks to avoid buffer overrun problems)
    * @param inBuf Flat buffer to read bytes from
    * @param inputBufferBytes total size of the input buffer
    * @param readOffset Offset into buffer to read from.  Incremented by (blockSize) on success.
    * @param copyTo memory location to copy bytes to
    * @param blockSize number of bytes to copy
    * @return B_NO_ERROR if the data was successfully read, B_ERROR if the data couldn't be read (because the buffer wasn't large enough)
    */
   static status_t ReadData(const uint8 * inBuf, uint32 inputBufferBytes, uint32 * readOffset, void * copyTo, uint32 blockSize)
   {
      if ((*readOffset + blockSize) > inputBufferBytes) return B_ERROR;
      memcpy(copyTo, &inBuf[*readOffset], blockSize);
      *readOffset += blockSize;
      return B_NO_ERROR;
   };

protected:
   /** 
    *  Called by CopyTo().  Sets (copyTo) to set from this Flattenable, if possible. 
    *  Default implementation is not very efficient, since it has to flatten
    *  this object into a byte buffer, and then unflatten the bytes back into 
    *  (copyTo).  However, you can override CopyToImplementation() to provide 
    *  a more efficient implementation when possible.
    *  @param copyTo Object to make into the equivalent of this object.  (copyTo)
    *                May be any subclass of Flattenable, but has been pre-screened by CopyTo()
    *                to make sure it's not (&this), and that is allows our type code.
    *  @return B_NO_ERROR on success, or B_ERROR on failure (out-of-memory, etc)
    */
   virtual status_t CopyToImplementation(Flattenable & copyTo) const
   {
      uint8 smallBuf[256];
      uint8 * bigBuf = NULL;
      uint32 flatSize = FlattenedSize();
      if (flatSize > ARRAYITEMS(smallBuf)) 
      {
         bigBuf = newnothrow uint8[flatSize];
         if (bigBuf == NULL)
         {
            WARN_OUT_OF_MEMORY;
            return B_ERROR;
         }
      }
      Flatten(bigBuf ? bigBuf : smallBuf);
      status_t ret = copyTo.Unflatten(bigBuf ? bigBuf : smallBuf, flatSize);
      delete [] bigBuf;
      return ret;
   }

   /** 
    *  Called by CopyFrom().  Sets our state from (copyFrom) if possible. 
    *  Default implementation is not very efficient, since it has to flatten
    *  (copyFrom) into a byte buffer, and then unflatten the bytes back into 
    *  (this).  However, you can override CopyToImplementation() to provide 
    *  a more efficient implementation when possible.
    *  @param copyFrom Object to set this object's state from.
    *                  May be any subclass of Flattenable, but has been pre-screened by CopyFrom()
    *                  to make sure it's not (&this), and that we allow its type code.
    *  @return B_NO_ERROR on success, or B_ERROR on failure (out-of-memory, etc)
    */
   virtual status_t CopyFromImplementation(const Flattenable & copyFrom)
   {
      uint8 smallBuf[256];
      uint8 * bigBuf = NULL;
      uint32 flatSize = copyFrom.FlattenedSize();
      if (flatSize > ARRAYITEMS(smallBuf)) 
      {
         bigBuf = newnothrow uint8[flatSize];
         if (bigBuf == NULL)
         {
            WARN_OUT_OF_MEMORY;
            return B_ERROR;
         }
      }
      copyFrom.Flatten(bigBuf ? bigBuf : smallBuf);
      status_t ret = Unflatten(bigBuf ? bigBuf : smallBuf, flatSize);
      delete [] bigBuf;
      return ret;
   }
};

/*-------------------------------------------------------------*/
/*-------------------------------------------------------------*/

};  // end namespace muscle

#endif /* _MUSCLEFLATTENABLE_H */

