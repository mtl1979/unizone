/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include <stdio.h>
#include "util/ByteBuffer.h"
#include "util/Queue.h"
#include "message/Message.h"

BEGIN_NAMESPACE(muscle);

static Message _emptyMessage;
const Message & GetEmptyMessage() {return _emptyMessage;}

static void DoIndents(uint32 num, String & s) {for (uint32 i=0; i<num; i++) s += ' ';}

static void ClearMsgFunc(Message * msg, void *) {msg->what = 0; msg->Clear();}
static MessageRef::ItemPool _messagePool(100, ClearMsgFunc);
MessageRef::ItemPool * GetMessagePool() {return &_messagePool;}

#define DECLARECLONE(X)                           \
   GenericRef X :: Clone() const                  \
   {                                              \
      GenericRef ref(NEWFIELD(X), FIELDPOOL(X));  \
      if (ref()) *((X*)ref()) = *this;            \
      return ref;                                 \
   }

#ifdef MUSCLE_DISABLE_MESSAGE_FIELD_POOLS
# define FIELDPOOL(X) NULL
# define NEWFIELD(X)  newnothrow X
# define DECLAREFIELDTYPE(X) DECLARECLONE(X)
#else
# define FIELDPOOL(X) (&_pool##X)
# define NEWFIELD(X)  _pool##X.ObtainObject()
# define DECLAREFIELDTYPE(X) static void Clear##X(X * obj, void *) {obj->Clear(obj->GetNumItems() > 100);} static ObjectPool<X> _pool##X(100, Clear##X); DECLARECLONE(X)
#endif

MessageRef GetMessageFromPool(uint32 what)
{
   MessageRef ref(_messagePool.ObtainObject(), &_messagePool);
   if (ref()) ref()->what = what;
   return ref;
}

MessageRef GetMessageFromPool(const Message & copyMe)
{
   MessageRef ref(_messagePool.ObtainObject(), &_messagePool);
   if (ref()) *(ref()) = copyMe;
   return ref;
}

/* This class is for the private use of the Message class only! 
 * It represents one "name" of the message, which can in turn represent 1 or more
 * data items of a single type. 
 */
class AbstractDataArray : public FlatCountable
{
public:
   // Should add the given item to our internal array.
   virtual status_t AddDataItem(const void * data, uint32 size) = 0;

   // Should remove the (index)'th item from our internal array.
   virtual status_t RemoveDataItem(uint32 index) = 0;

   // Prepends the given data item to the beginning of our array.
   virtual status_t PrependDataItem(const void * data, uint32 size) = 0;

   // Clears the array
   virtual void Clear(bool releaseDataBuffers) = 0;

   // Sets (setDataLoc) to point to the (index)'th item in our array.
   // Result is not guaranteed to remain valid after this object is modified.
   virtual status_t FindDataItem(uint32 index, const void ** setDataLoc) const = 0;

   // Should replace the (index)'th data item in the array with (data).
   virtual status_t ReplaceDataItem(uint32 index, const void * data, uint32 size) = 0;

   // Returns the size (in bytes) of the item in the (index)'th slot.
   virtual uint32 GetItemSize(uint32 index) const = 0;

   // Returns the number of items currently in the array
   virtual uint32 GetNumItems() const = 0;

   // Returns true iff all elements in the array have the same size
   virtual bool ElementsAreFixedSize() const = 0;

   // Flattenable interface
   virtual bool IsFixedSize() const {return false;}

   // returns a separate (deep) copy of this array
   virtual GenericRef Clone() const = 0;

   // Returns true iff this array should be included when flattening. 
   virtual bool IsFlattenable() const = 0;

   // For debugging:  returns a description of our contents as a String
   virtual void AddToString(String & s, bool recurse, int indent) const = 0;

   // Returns true iff this array is identical to (rhs).  If (compareContents) is false,
   // only the array lengths and type codes are checked, not the data itself.
   bool IsEqualTo(const AbstractDataArray * rhs, bool compareContents) const
   {
      if ((TypeCode() != rhs->TypeCode())||(GetNumItems() != rhs->GetNumItems())) return false;
      return compareContents ? AreContentsEqual(rhs) : true;
   }

protected:
   /** Must be implemented by each subclass to return true iff (rhs) is of the same type
    *  and has the same data as (*this).  The TypeCode() and GetNumItems() of (rhs) are
    *  guaranteed to equal those of this AbstractDataArray.  Called by IsEqualTo().
    */
   virtual bool AreContentsEqual(const AbstractDataArray * rhs) const = 0;
};

template <class DataType> class FixedSizeDataArray : public AbstractDataArray
{
public:
   FixedSizeDataArray() {/* empty */}
   virtual ~FixedSizeDataArray() {/* empty */}

   virtual uint32 GetNumItems() const {return _data.GetNumItems();}

   virtual void Clear(bool releaseDataBuffers) {_data.Clear(releaseDataBuffers);}

   virtual status_t AddDataItem(const void * item, uint32 size)
   {
      return (size == sizeof(DataType)) ? (item ? _data.AddTail(*((DataType *)item)) : _data.AddTail()) : B_ERROR;
   }

   virtual status_t PrependDataItem(const void * item, uint32 size)
   {
      return (size == sizeof(DataType)) ? (item ? _data.AddHead(*((DataType *)item)) : _data.AddHead()) : B_ERROR;
   }

   virtual uint32 GetItemSize(uint32 /*index*/) const {return sizeof(DataType);}

   virtual status_t RemoveDataItem(uint32 index) {return _data.RemoveItemAt(index);}

   virtual status_t FindDataItem(uint32 index, const void ** setDataLoc) const
   {
      if (index < _data.GetNumItems())
      {
         *setDataLoc = _data.GetItemAt(index);
         return (*setDataLoc) ? B_NO_ERROR : B_ERROR;
      }
      return B_ERROR;
   }

   virtual status_t ReplaceDataItem(uint32 index, const void * data, uint32 size)
   {
      return (size == sizeof(DataType)) ? _data.ReplaceItemAt(index, *((DataType *)data)) : B_ERROR;
   }

   virtual bool ElementsAreFixedSize() const {return true;}

   virtual bool IsFlattenable() const {return true;}

   const DataType & ItemAt(int i) const {return *_data.GetItemAt(i);}

   FixedSizeDataArray<DataType> & operator=(const FixedSizeDataArray<DataType> &rhs) 
   {
      if (this != &rhs) _data = rhs._data;
      return *this;
   }

protected:
   virtual bool AreContentsEqual(const AbstractDataArray * rhs) const
   {
      const FixedSizeDataArray<DataType> * trhs = dynamic_cast<const FixedSizeDataArray<DataType> *>(rhs);
      if (trhs == NULL) return false;

      for (int32 i=GetNumItems()-1; i>=0; i--) if (ItemAt(i) != trhs->ItemAt(i)) return false;
      return true;
   }

   Queue<DataType> _data;
};

// An array of ephemeral objects that won't be flattened
class TagDataArray : public FixedSizeDataArray<GenericRef>
{
public:
   TagDataArray() {/* empty */}
   virtual ~TagDataArray() {/* empty */}

   virtual void Flatten(uint8 *) const
   {
      MCRASH("Message::TagDataArray:Flatten()  This method should never be called!");
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const {return 0;}  // tags don't get flattened, so they take up no space

   virtual status_t Unflatten(const uint8 *, uint32)
   {
      MCRASH("Message::TagDataArray:Unflatten()  This method should never be called!");
      return B_NO_ERROR;  // just to keep the compiler happy
   }

   virtual uint32 TypeCode() const {return B_TAG_TYPE;}

   virtual GenericRef Clone() const; 

   virtual bool IsFlattenable() const {return false;}

protected:
   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char buf[128]; 
         sprintf(buf, "    %lu. %p\n", i, _data.GetItemAt(i)->GetItemPointer()); 
         s += buf;
      }
   }
};
DECLAREFIELDTYPE(TagDataArray);

// an array of flattened objects, where all flattened objects are guaranteed to be the same size flattened
template <class DataType> class FixedSizeFlatObjectArray : public FixedSizeDataArray<DataType>
{
public:
   FixedSizeFlatObjectArray() 
   {
      DataType temp;
      _typeCode = temp.TypeCode();
      _flatItemSize = temp.FlattenedSize();
   }

   virtual ~FixedSizeFlatObjectArray() 
   {
      // empty
   }

   virtual void Flatten(uint8 *buffer) const
   {
      uint32 numItems = _data.GetNumItems(); 
      for (uint32 i=0; i<numItems; i++) 
      {
         _data.GetItemAt(i)->Flatten(buffer);
         buffer += _flatItemSize;
      }
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const {return _data.GetNumItems() * _flatItemSize;}

   virtual status_t Unflatten(const uint8 *buffer, uint32 numBytes)
   {
      _data.Clear();
      if (numBytes % _flatItemSize) return B_ERROR;  // length must be an even multiple of item size, or something's wrong!

      DataType temp;
      uint32 numItems = numBytes / _flatItemSize;
      if (_data.EnsureSize(numItems) == B_NO_ERROR)
      {
         for (uint32 i=0; i<numItems; i++)
         {
            if ((temp.Unflatten(buffer, _flatItemSize) != B_NO_ERROR)||(_data.AddTail(temp) != B_NO_ERROR)) return B_ERROR;
            buffer += _flatItemSize;
         }
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }

   virtual uint32 TypeCode() const {return _typeCode;}

protected:
   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char buf[64]; 
         sprintf(buf, "    %lu. ", i);  
         s += buf;
         AddItemToString(s, _data[i]);
      }
   }

   virtual void AddItemToString(String & s, const DataType & item) const = 0;

private:
   uint32 _typeCode;
   uint32 _flatItemSize;
};

/* This class handles storage of all the primitive numeric types for us. */
template <class DataType> class PrimitiveTypeDataArray : public FixedSizeDataArray<DataType> 
{
public:
   PrimitiveTypeDataArray() {/* empty */}
   virtual ~PrimitiveTypeDataArray() {/* empty */}

   virtual void Flatten(uint8 *buffer) const
   {
      DataType * dBuf = (DataType *) buffer;
      uint32 numItems = _data.GetNumItems(); 
      for (uint32 i=0; i<numItems; i++) ConvertToNetworkByteOrder(&dBuf[i], _data.GetItemAt(i));
   }

   virtual status_t Unflatten(const uint8 *buffer, uint32 numBytes)
   {
      _data.Clear();
      if (numBytes % sizeof(DataType)) return B_ERROR;  // length must be an even multiple of item size, or something's wrong!

      uint32 numItems = numBytes / sizeof(DataType);
      if (_data.EnsureSize(numItems) == B_NO_ERROR)
      {
         DataType * dBuf = (DataType *) buffer;
         for (uint32 i=0; i<numItems; i++)
         {
            DataType next;
            ConvertFromNetworkByteOrder(&next, &dBuf[i]);
            if (_data.AddTail(next) != B_NO_ERROR) return B_ERROR;
         }
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const {return _data.GetNumItems() * sizeof(DataType);}

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const = 0;
   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const = 0;

   virtual const char * GetFormatString() const = 0;

   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char temp1[100]; sprintf(temp1, GetFormatString(), ItemAt(i));
         char temp2[150]; sprintf(temp2, "    %lu. [%s]\n", i, temp1);  s += temp2;
      }
   }
};

class PointDataArray : public FixedSizeFlatObjectArray<Point>
{
public:
   PointDataArray() {/* empty */}
   virtual ~PointDataArray() {/* empty */}
   virtual GenericRef Clone() const;
   virtual void AddItemToString(String & s, const Point & p) const 
   {
      char buf[128]; 
      sprintf(buf, "Point: %f %f\n", p.x(), p.y());
      s += buf;
   }
};
DECLAREFIELDTYPE(PointDataArray);

class RectDataArray : public FixedSizeFlatObjectArray<Rect>
{
public:
   RectDataArray() {/* empty */}
   virtual ~RectDataArray() {/* empty */}
   virtual GenericRef Clone() const;
   virtual void AddItemToString(String & s, const Rect & r) const 
   {
      char buf[256]; 
      sprintf(buf, "Rect: leftTop=(%f,%f) rightBottom=(%f,%f)\n", r.left(), r.top(), r.right(), r.bottom());
      s += buf;
   }
};
DECLAREFIELDTYPE(RectDataArray);

class Int8DataArray : public PrimitiveTypeDataArray<int8> 
{
public:
   Int8DataArray() {/* empty */}
   virtual ~Int8DataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_INT8_TYPE;}

   virtual const char * GetFormatString() const {return "%i";}
 
   virtual GenericRef Clone() const;

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      *((uint8*)writeToHere) = *((uint8*)readFromHere);  // no translation required, really
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      *((uint8*)writeToHere) = *((uint8*)readFromHere);  // no translation required, really
   }
};
DECLAREFIELDTYPE(Int8DataArray);

class BoolDataArray : public PrimitiveTypeDataArray<bool>
{
public:
   BoolDataArray() {/* empty */}
   virtual ~BoolDataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_BOOL_TYPE;}

   virtual const char * GetFormatString() const {return "%i";}

   virtual GenericRef Clone() const;

   virtual void Flatten(uint8 *buffer) const
   {
      uint8 * dBuf = (uint8 *) buffer;
      uint32 numItems = _data.GetNumItems(); 
      for (uint32 i=0; i<numItems; i++) dBuf[i] = (uint8) (_data[i] ? 1 : 0);
   }

   virtual status_t Unflatten(const uint8 *buffer, uint32 numBytes)
   {
      _data.Clear();

      if (_data.EnsureSize(numBytes) == B_NO_ERROR)
      {
         uint8 * dBuf = (uint8 *) buffer;
         for (uint32 i=0; i<numBytes; i++) if (_data.AddTail((dBuf[i] != 0) ? true : false) != B_NO_ERROR) return B_ERROR;
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const {return _data.GetNumItems()*sizeof(uint8);}  /* bools are always flattened into 1 byte each */

protected:
   virtual void ConvertToNetworkByteOrder(void *, const void *) const
   {
      MCRASH("BoolDataArray::ConvertToNetworkByteOrder should never be called");
   }

   virtual void ConvertFromNetworkByteOrder(void *, const void *) const
   {
      MCRASH("BoolDataArray::ConvertToNetworkByteOrder should never be called");
   }
};
DECLAREFIELDTYPE(BoolDataArray);

class Int16DataArray : public PrimitiveTypeDataArray<int16> 
{
public:
   Int16DataArray() {/* empty */}
   virtual ~Int16DataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_INT16_TYPE;}

   virtual const char * GetFormatString() const {return "%i";}

   virtual GenericRef Clone() const;

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      int16 val; muscleCopyIn(val, readFromHere);
      val = B_HOST_TO_LENDIAN_INT16(val); 
      muscleCopyOut(writeToHere, val);
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      int16 val; muscleCopyIn(val, readFromHere);
      val = B_LENDIAN_TO_HOST_INT16(val); 
      muscleCopyOut(writeToHere, val);
   }
};
DECLAREFIELDTYPE(Int16DataArray);

class Int32DataArray : public PrimitiveTypeDataArray<int32> 
{
public:
   Int32DataArray() {/* empty */}
   virtual ~Int32DataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_INT32_TYPE;}

   virtual const char * GetFormatString() const {return "%li";}

   virtual GenericRef Clone() const;

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      int32 val; muscleCopyIn(val, readFromHere);
      val = B_HOST_TO_LENDIAN_INT32(val);
      muscleCopyOut(writeToHere, val);
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      int32 val; muscleCopyIn(val, readFromHere);
      val = B_LENDIAN_TO_HOST_INT32(val);
      muscleCopyOut(writeToHere, val);
   }
};
DECLAREFIELDTYPE(Int32DataArray);

class Int64DataArray : public PrimitiveTypeDataArray<int64> 
{
public:
   Int64DataArray() {/* empty */}
   virtual ~Int64DataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_INT64_TYPE;}

   virtual const char * GetFormatString() const {return INT64_FORMAT_SPEC;}

   virtual GenericRef Clone() const;

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      int64 val; muscleCopyIn(val, readFromHere);
      val = B_HOST_TO_LENDIAN_INT64(val);
      muscleCopyOut(writeToHere, val);
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      int64 val; muscleCopyIn(val, readFromHere);
      val = B_LENDIAN_TO_HOST_INT64(val);
      muscleCopyOut(writeToHere, val);
   }
};
DECLAREFIELDTYPE(Int64DataArray);

class FloatDataArray : public PrimitiveTypeDataArray<float> 
{
public:
   FloatDataArray() {/* empty */}
   virtual ~FloatDataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_FLOAT_TYPE;}

   virtual const char * GetFormatString() const {return "%f";}

   virtual GenericRef Clone() const;

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      float val; muscleCopyIn(val, readFromHere);
      val = B_HOST_TO_LENDIAN_FLOAT(val);
      muscleCopyOut(writeToHere, val);
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      float val; muscleCopyIn(val, readFromHere);
      val = B_LENDIAN_TO_HOST_FLOAT(val);
      muscleCopyOut(writeToHere, val);
   }
};
DECLAREFIELDTYPE(FloatDataArray);

class DoubleDataArray : public PrimitiveTypeDataArray<double> 
{
public:
   DoubleDataArray() {/* empty */}
   virtual ~DoubleDataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_DOUBLE_TYPE;}

   virtual const char * GetFormatString() const {return "%lf";}

   virtual GenericRef Clone() const;

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      double val;  muscleCopyIn(val, readFromHere);
      val = B_HOST_TO_LENDIAN_DOUBLE(val);
      muscleCopyOut(writeToHere, val);
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      double val;  muscleCopyIn(val, readFromHere);
      val = B_LENDIAN_TO_HOST_DOUBLE(val);
      muscleCopyOut(writeToHere, val);
   }
};
DECLAREFIELDTYPE(DoubleDataArray);

class PointerDataArray : public PrimitiveTypeDataArray<void *> 
{
public:
   PointerDataArray() {/* empty */}
   virtual ~PointerDataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_POINTER_TYPE;}

   virtual const char * GetFormatString() const {return "%p";}

   virtual bool IsFlattenable() const {return false;}

   virtual GenericRef Clone() const;

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      // Just a straight value copy, since pointers across CPU types don't make any sense anyway
      void * val; muscleCopyIn(val, readFromHere); muscleCopyOut(writeToHere, val);
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere) const
   {
      // Just a straight value copy, since pointers across CPU types don't make any sense anyway
      void * val; muscleCopyIn(val, readFromHere); muscleCopyOut(writeToHere, val);
   }
};
DECLAREFIELDTYPE(PointerDataArray);


// An abstract array of FlatCountableRefs.
template <class RefType> class FlatCountableRefDataArray : public FixedSizeDataArray<RefType>
{
public:
   FlatCountableRefDataArray() : _typeCode(0) {/* empty */}
   FlatCountableRefDataArray(uint32 tc) : _typeCode(tc) {/* empty */}
   virtual ~FlatCountableRefDataArray() {/* empty */}

   /** Sets our type code.  Typically called after using the default ctor. */
   void SetTypeCode(uint32 tc) {_typeCode = tc;}

   virtual uint32 GetItemSize(uint32 index) const 
   {
      const FlatCountable * msg = ItemAt(index)();
      return msg ? msg->FlattenedSize() : 0;
   }

   virtual uint32 TypeCode() const {return _typeCode;}

   virtual bool ElementsAreFixedSize() const {return false;}

   /** Whether or not we should write the number-of-items element when we flatten this array.
     * Older versions of muscle didn't do this for MessageDataArray objects, so we need to
     * maintain that behaviour so that we don't break compatibility.  (bleah)
     */
   virtual bool ShouldWriteNumItems() const {return true;}

   virtual void Flatten(uint8 *buffer) const
   {
      uint32 writeOffset = 0;
      uint32 numItems = _data.GetNumItems(); 

      // Conditional to allow maintaining backwards compatibility with old versions of muscle's MessageDataArrays (sigh)
      if (ShouldWriteNumItems()) 
      {
         uint32 writeNumElements = B_HOST_TO_LENDIAN_INT32(numItems);
         WriteData(buffer, &writeOffset, &writeNumElements, sizeof(writeNumElements));
      }

      for (uint32 i=0; i<numItems; i++) 
      {
         const FlatCountable * next = ItemAt(i).GetItemPointer();
         if (next)
         {
            uint32 fs = next->FlattenedSize();
            uint32 writeFs = B_HOST_TO_LENDIAN_INT32(fs);
            WriteData(buffer, &writeOffset, &writeFs, sizeof(writeFs));
            next->Flatten(&buffer[writeOffset]);
            writeOffset += fs;
         }
      }
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const 
   {
      uint32 numItems = GetNumItems();
      uint32 count = (numItems+(ShouldWriteNumItems()?1:0))*sizeof(uint32);
      for (uint32 i=0; i<numItems; i++) count += GetItemSize(i);
      return count;
   }

private:
   uint32 _typeCode;
};

// An array of ByteBufferRefs.
class ByteBufferDataArray : public FlatCountableRefDataArray<FlatCountableRef>
{
public:
   ByteBufferDataArray() {/* empty */}
   ByteBufferDataArray(uint32 tc) : FlatCountableRefDataArray<FlatCountableRef>(tc) {/* empty */}
   virtual ~ByteBufferDataArray() {/* empty */}

   virtual status_t Unflatten(const uint8 *buffer, uint32 numBytes)
   {
      Clear(false);

      uint32 readOffset = 0;

      uint32 numItems;
      if (ReadData(buffer, numBytes, &readOffset, &numItems, sizeof(numItems)) != B_NO_ERROR) return B_ERROR;
      numItems = B_LENDIAN_TO_HOST_INT32(numItems);
   
      for (uint32 i=0; i<numItems; i++)
      {
         uint32 readFs;
         if (ReadData(buffer, numBytes, &readOffset, &readFs, sizeof(readFs)) != B_NO_ERROR) return B_ERROR;
         readFs = B_LENDIAN_TO_HOST_INT32(readFs);
         if (readOffset + readFs > numBytes) return B_ERROR;  // message size too large for our buffer... corruption?
         FlatCountableRef fcRef(GetByteBufferFromPool(readFs, &buffer[readOffset]).GetGeneric(), true);
         if ((fcRef())&&(AddDataItem(&fcRef, sizeof(fcRef)) == B_NO_ERROR)) readOffset += readFs;
                                                                       else return B_ERROR;
      }
      return B_NO_ERROR;
   }

   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++)
      {
         DoIndents(indent,s);

         char buf[100]; 
         sprintf(buf, "    %lu. ", i);
         s += buf;

         FlatCountable * fc = ItemAt(i)();
         ByteBuffer temp;
         ByteBuffer * bb = dynamic_cast<ByteBuffer *>(fc);
         if ((bb == NULL)&&(fc))
         {
            temp.SetNumBytes(fc->FlattenedSize(), false);
            if (temp())
            {
               fc->Flatten((uint8*)temp());
               bb = &temp;
            }
         }

         if (bb)
         {
            sprintf(buf, "[flattenedSize=%lu] ", bb->GetNumBytes()); 
            s += buf;
            uint32 printBytes = muscleMin(bb->GetNumBytes(), (uint32)10);
            if (printBytes > 0)
            {
               s += '[';
               for (uint32 j=0; j<printBytes; j++) 
               {
                  sprintf(buf, "%02x%s", (bb->GetBuffer())[j], (j<printBytes-1)?" ":"");
                  s += buf;
               }
               if (printBytes > 10) s += " ...";
               s += ']';
            }
         }
         else s += "[NULL]";

         s += '\n';
      }
   }

   virtual GenericRef Clone() const;

protected:
   /** Overridden to compare objects instead of merely the pointers to them */
   virtual bool AreContentsEqual(const AbstractDataArray * rhs) const
   {
      const ByteBufferDataArray * trhs = dynamic_cast<const ByteBufferDataArray *>(rhs);
      if (trhs == NULL) return false;

      for (int32 i=GetNumItems()-1; i>=0; i--) 
      {
         const ByteBuffer * myBuf  = (const ByteBuffer *) this->ItemAt(i)();
         const ByteBuffer * hisBuf = (const ByteBuffer *) trhs->ItemAt(i)();
         if (((myBuf != NULL)!=(hisBuf != NULL))||((myBuf)&&(*myBuf != *hisBuf))) return false;
      }
      return true;
   }
};

DECLAREFIELDTYPE(ByteBufferDataArray);

class MessageDataArray : public FlatCountableRefDataArray<MessageRef>
{
public:
   MessageDataArray() : FlatCountableRefDataArray<MessageRef>(B_MESSAGE_TYPE) {/* empty */}
   virtual ~MessageDataArray() {/* empty */}

   /** For backwards compatibility with older muscle streams */
   virtual bool ShouldWriteNumItems() const {return false;}

   virtual status_t Unflatten(const uint8 *buffer, uint32 numBytes)
   {
      Clear(false);

      uint32 readOffset = 0;
      while(readOffset < numBytes)
      {
         uint32 readFs;
         if (ReadData(buffer, numBytes, &readOffset, &readFs, sizeof(readFs)) != B_NO_ERROR) return B_ERROR;
         readFs = B_LENDIAN_TO_HOST_INT32(readFs);
         if (readOffset + readFs > numBytes) return B_ERROR;  // message size too large for our buffer... corruption?
         MessageRef nextMsg = GetMessageFromPool();
         if (nextMsg())
         {
            if (nextMsg()->Unflatten(&buffer[readOffset], readFs) != B_NO_ERROR) return B_ERROR;
            if (AddDataItem(&nextMsg, sizeof(nextMsg)) != B_NO_ERROR) return B_ERROR;
            readOffset += readFs;
         }
         else return B_ERROR;
      }
      return B_NO_ERROR;
   }

   virtual void AddToString(String & s, bool recurse, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++)
      {
         DoIndents(indent,s);  

         char buf[100]; 
         sprintf(buf, "    %lu. ", i); 
         s += buf;

         MessageRef * dataItem;
         uint32 itemSize = GetItemSize(i);
         if (FindDataItem(i, (const void **)&dataItem) == B_NO_ERROR)
         {
            Message * msg = dataItem->GetItemPointer();
            if (msg)
            {
               char tcbuf[5]; MakePrettyTypeCodeString(msg->what, tcbuf);
               sprintf(buf, "[what='%s' (%li/0x%lx), flattenedSize=%lu, numFields=%lu]\n", tcbuf, msg->what, msg->what, itemSize, msg->CountNames());
               s += buf;

               if (recurse) msg->AddToString(s, recurse, indent+3);
            }
            else s += "[NULL]\n";
         }
         else s += "[<Error!>]\n";
      }
   }

   virtual GenericRef Clone() const;

protected:
   /** Overridden to compare objects instead of merely the pointers to them */
   virtual bool AreContentsEqual(const AbstractDataArray * rhs) const
   {
      const MessageDataArray * trhs = dynamic_cast<const MessageDataArray *>(rhs);
      if (trhs == NULL) return false;

      for (int32 i=GetNumItems()-1; i>=0; i--) 
      {
         const Message * myMsg  = (const Message *) this->ItemAt(i)();
         const Message * hisMsg = (const Message *) trhs->ItemAt(i)();
         if (((myMsg != NULL)!=(hisMsg != NULL))||((myMsg)&&(*myMsg != *hisMsg))) return false;
      }
      return true;
   }
};
DECLAREFIELDTYPE(MessageDataArray);

// An array of Flattenable objects which are *not* guaranteed to all have the same flattened size. 
template <class DataType> class VariableSizeFlatObjectArray : public FixedSizeDataArray<DataType>
{
public:
   VariableSizeFlatObjectArray() {/* empty */}
   virtual ~VariableSizeFlatObjectArray() {/* empty */}

   virtual uint32 GetItemSize(uint32 index) const {return ItemAt(index).FlattenedSize();}
   virtual bool ElementsAreFixedSize() const {return false;}

   virtual void Flatten(uint8 *buffer) const
   {
      // Format:  0. number of entries (4 bytes)
      //          1. entry size in bytes (4 bytes)
      //          2. entry data (n bytes)
      //          (repeat 1. and 2. as necessary)
      uint32 numElements = GetNumItems();
      uint32 networkByteOrder = B_HOST_TO_LENDIAN_INT32(numElements);
      uint32 writeOffset = 0;

      WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

      for (uint32 i=0; i<numElements; i++)
      {
         // write element size
         const Flattenable & s = ItemAt(i);
         uint32 nextElementBytes = s.FlattenedSize();
         networkByteOrder = B_HOST_TO_LENDIAN_INT32(nextElementBytes);
         WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

         // write element data
         s.Flatten(&buffer[writeOffset]);
         writeOffset += nextElementBytes;
      }
   }

   virtual uint32 FlattenedSize() const 
   {
      uint32 num = GetNumItems();
      uint32 sum = (num+1)*sizeof(uint32);  // 1 uint32 for the count, plus 1 per entry for entry-size
      for (uint32 i=0; i<num; i++) sum += ItemAt(i).FlattenedSize();
      return sum;
   }

   virtual status_t Unflatten(const uint8 *buffer, uint32 inputBufferBytes)
   {
      Clear(false);

      uint32 networkByteOrder;
      uint32 readOffset = 0;

      if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
      uint32 numElements = B_LENDIAN_TO_HOST_INT32(networkByteOrder);
      if (_data.EnsureSize(numElements) != B_NO_ERROR) return B_ERROR;
      for (uint32 i=0; i<numElements; i++)
      {
         if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
         uint32 elementSize = B_LENDIAN_TO_HOST_INT32(networkByteOrder);
         if (elementSize == 0) return B_ERROR;  // it should always have at least the trailing NUL byte!

         // read element data
         if ((readOffset + elementSize > inputBufferBytes)||(_data.AddTail() != B_NO_ERROR)||(_data.TailPointer()->Unflatten(&buffer[readOffset], elementSize) != B_NO_ERROR)) return B_ERROR;
         readOffset += elementSize;
      }
      return B_NO_ERROR;
   }
};

class StringDataArray : public VariableSizeFlatObjectArray<String>
{
public:
   StringDataArray() {/* empty */}
   virtual ~StringDataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_STRING_TYPE;}

   virtual GenericRef Clone() const;

   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char buf[50]; 
         sprintf(buf,"    %lu. [", i); 
         s += buf;
         s += ItemAt(i);
         s += "]\n";
      }
   }
};
DECLAREFIELDTYPE(StringDataArray);

MessageFieldNameIterator :: MessageFieldNameIterator() : _typeCode(B_ANY_TYPE)
{
   // empty
}

MessageFieldNameIterator :: MessageFieldNameIterator(const HashtableIterator<String, GenericRef> & iter, uint32 tc) : HashtableIterator<String, GenericRef>(iter), _typeCode(tc)
{
   // empty
}

MessageFieldNameIterator :: ~MessageFieldNameIterator() 
{
   // empty
}

bool MessageFieldNameIterator :: HasMoreFieldNames() const 
{
   return (PeekNextFieldNameString() != NULL);
}

status_t MessageFieldNameIterator :: GetNextFieldName(String & name)
{
   const String * ret = GetNextFieldNameString();
   if (ret)
   {
      name = *ret;
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t MessageFieldNameIterator :: PeekNextFieldName(String & name) const
{
   const String * ret = PeekNextFieldNameString();
   if (ret)
   {
      name = *ret;
      return B_NO_ERROR;
   }
   return B_ERROR;
}

const String * MessageFieldNameIterator :: GetNextFieldNameString()
{
   if (_typeCode == B_ANY_TYPE) return GetNextKey();
   else
   {
      // Read until the array is of type (_typeCode)...
      const GenericRef * array;
      while((array = GetNextValue()) != NULL)
      {
         const String * ret = GetNextKey();
         if (((const AbstractDataArray*)array->GetItemPointer())->TypeCode() == _typeCode) return ret;
      }
      return NULL;
   }
}

const String * MessageFieldNameIterator :: PeekNextFieldNameString() const
{
   if (_typeCode == B_ANY_TYPE) return PeekNextKey();
   else
   {
      // Read until the next array to get is of type (_typeCode)...
      const GenericRef * array;
      while((array = PeekNextValue()) != NULL)
      {
         if (((const AbstractDataArray *)array->GetItemPointer())->TypeCode() == _typeCode) return PeekNextKey();

         // iterate along... it's okay to cast away const here as this iteration won't be visible to the calling code.
         MessageFieldNameIterator * nonConstThis = const_cast<MessageFieldNameIterator *>(this);
         (void)nonConstThis->GetNextKey();
         (void)nonConstThis->GetNextValue();
      }
      return NULL;
   }
}

Message :: Message() : what(0)
{
   // empty
}

Message :: Message(uint32 w) : what(w)
{
   // empty
}

Message :: Message(const Message &him) : FlatCountable()
{
   *this = him;
}

Message :: ~Message() 
{
   Clear();
}

Message & Message :: operator=(const Message &rhs) 
{
   if (this != &rhs)
   {
      Clear();
      what = rhs.what;

      // Copy all his entries!
      HashtableIterator<String, GenericRef> it = rhs._entries.GetIterator();
      const String * nextKey;
      while((nextKey = it.GetNextKey()) != NULL) 
      {
         GenericRef clone = ((const AbstractDataArray *)(*it.GetNextValue())())->Clone();
         if (clone()) (void) _entries.Put(*nextKey, clone);
      }
   }
   return *this;
}

status_t Message :: GetInfo(const String & name, uint32 * type, uint32 * c, bool * fixedSize) const
{
   const AbstractDataArray * array = GetArray(name, B_ANY_TYPE);
   if (array == NULL) return B_ERROR;
   if (type)      *type      = array->TypeCode();
   if (c)         *c         = array->GetNumItems();
   if (fixedSize) *fixedSize = array->ElementsAreFixedSize();
   return B_NO_ERROR;
}

uint32 Message :: CountNames(uint32 type) const 
{
   if (type == B_ANY_TYPE) return _entries.GetNumItems();

   // oops, gotta count just the entries of the given type
   uint32 total = 0;
   HashtableIterator<String, GenericRef> it = _entries.GetIterator();
   const GenericRef * nextValue;
   while((nextValue = it.GetNextValue()) != NULL) if (((const AbstractDataArray*)nextValue->GetItemPointer())->TypeCode() == type) total++;
   return total;
}

bool Message :: IsEmpty() const 
{
   return (_entries.GetNumItems() == 0);
}

void Message :: PrintToStream(bool recurse, int indent) const 
{
   String s;  
   AddToString(s, recurse, indent);
   printf("%s", s());
}

String Message :: ToString(bool recurse, int indent) const 
{
   String s;  
   AddToString(s, recurse, indent);
   return s;
}

void Message :: AddToString(String & s, bool recurse, int indent) const 
{
   String ret;

   char prettyTypeCodeBuf[5];
   MakePrettyTypeCodeString(what, prettyTypeCodeBuf);

   char buf[150];
   DoIndents(indent,s); 
   sprintf(buf, "Message:  this=%p, what='%s' (%li/0x%lx), entryCount=%li, flatSize=%lu\n", this, prettyTypeCodeBuf, what, what, CountNames(B_ANY_TYPE), (uint32)FlattenedSize()); 
   s += buf;

   HashtableIterator<String, GenericRef> it = _entries.GetIterator();
   const String * nextKey;
   while((nextKey = it.GetNextKey()) != NULL) 
   {
       const AbstractDataArray * nextValue = (const AbstractDataArray *)((*it.GetNextValue())());
       uint32 tc = nextValue->TypeCode();
       MakePrettyTypeCodeString(tc, prettyTypeCodeBuf);
       DoIndents(indent,s); 
       sprintf(buf, "  Entry: Name=[%s], GetNumItems()=%li, TypeCode()='%s' (%li) flatSize=%lu\n", nextKey->Cstr(), nextValue->GetNumItems(), prettyTypeCodeBuf, tc, (uint32)nextValue->FlattenedSize()); 
       s += buf;
       nextValue->AddToString(s, recurse, indent);
   }
}

// Returns an pointer to a held array of the given type, if it exists.  If (tc) is B_ANY_TYPE, then any type array is acceptable.
AbstractDataArray * Message :: GetArray(const String & arrayName, uint32 tc) const
{
   GenericRef * array;
   return (((array = _entries.Get(arrayName)) != NULL)&&((tc == B_ANY_TYPE)||(tc == ((const AbstractDataArray *)array->GetItemPointer())->TypeCode()))) ? ((AbstractDataArray *)array->GetItemPointer()) : NULL;
}

// Returns an pointer to a held array of the given type, if it exists.  If (tc) is B_ANY_TYPE, then any type array is acceptable.
GenericRef Message :: GetArrayRef(const String & arrayName, uint32 tc) const
{
   GenericRef array;
   if ((_entries.Get(arrayName, array) == B_NO_ERROR)&&(tc != B_ANY_TYPE)&&(tc != ((const AbstractDataArray *)array())->TypeCode())) array.Reset();
   return array;
}

AbstractDataArray * Message :: GetOrCreateArray(const String & arrayName, uint32 tc)
{
   {
      AbstractDataArray * nextEntry = GetArray(arrayName, tc);
      if (nextEntry) return nextEntry;
   }

   // Make sure the problem isn't that there already exists an array, but of the wrong type...
   // If that's the case, we can't create a like-names array of a different type, so fail.
   if (_entries.ContainsKey(arrayName)) return NULL;

   // Oops!  This array doesn't exist; better create it!
   GenericRef newEntry;
   switch(tc)
   {
      case B_BOOL_TYPE:    newEntry.SetRef(NEWFIELD(BoolDataArray),    FIELDPOOL(BoolDataArray));    break;
      case B_DOUBLE_TYPE:  newEntry.SetRef(NEWFIELD(DoubleDataArray),  FIELDPOOL(DoubleDataArray));  break;
      case B_POINTER_TYPE: newEntry.SetRef(NEWFIELD(PointerDataArray), FIELDPOOL(PointerDataArray)); break;
      case B_POINT_TYPE:   newEntry.SetRef(NEWFIELD(PointDataArray),   FIELDPOOL(PointDataArray));   break;
      case B_RECT_TYPE:    newEntry.SetRef(NEWFIELD(RectDataArray),    FIELDPOOL(RectDataArray));    break;
      case B_FLOAT_TYPE:   newEntry.SetRef(NEWFIELD(FloatDataArray),   FIELDPOOL(FloatDataArray));   break;
      case B_INT64_TYPE:   newEntry.SetRef(NEWFIELD(Int64DataArray),   FIELDPOOL(Int64DataArray));   break;
      case B_INT32_TYPE:   newEntry.SetRef(NEWFIELD(Int32DataArray),   FIELDPOOL(Int32DataArray));   break;
      case B_INT16_TYPE:   newEntry.SetRef(NEWFIELD(Int16DataArray),   FIELDPOOL(Int16DataArray));   break;
      case B_INT8_TYPE:    newEntry.SetRef(NEWFIELD(Int8DataArray),    FIELDPOOL(Int8DataArray));    break;
      case B_MESSAGE_TYPE: newEntry.SetRef(NEWFIELD(MessageDataArray), FIELDPOOL(MessageDataArray)); break;
      case B_STRING_TYPE:  newEntry.SetRef(NEWFIELD(StringDataArray),  FIELDPOOL(StringDataArray));  break;
      case B_TAG_TYPE:     newEntry.SetRef(NEWFIELD(TagDataArray),     FIELDPOOL(TagDataArray));     break;
      default:             
         newEntry.SetRef(NEWFIELD(ByteBufferDataArray), FIELDPOOL(ByteBufferDataArray));
         if (newEntry()) ((ByteBufferDataArray*)newEntry())->SetTypeCode(tc);
         break;
   }
   return ((newEntry())&&(_entries.Put(arrayName, newEntry) == B_NO_ERROR)) ? (AbstractDataArray*)newEntry() : NULL;
}


status_t Message :: Rename(const String & old_entry, const String & new_entry) 
{
   GenericRef oldArray = GetArrayRef(old_entry, B_ANY_TYPE);
   if (oldArray()) 
   {
      (void) RemoveName(new_entry);             // destructive rename... remove anybody in our way
      (void) _entries.Remove(old_entry);        // remove from under old name
      return _entries.Put(new_entry, oldArray); // add to under new name
   }
   return B_ERROR;
}


uint32 Message :: FlattenedSize() const 
{
   uint32 sum = 3 * sizeof(uint32);  // For the message header:  4 bytes for the protocol revision #, 4 bytes for the number-of-entries field, 4 bytes for what code

   // For each flattenable field: 4 bytes for the name length, name data, 4 bytes for entry type code, 4 bytes for entry data length, entry data
   HashtableIterator<String, GenericRef> it = _entries.GetIterator();
   const String * nextKey;
   while((nextKey = it.GetNextKey()) != NULL)  
   {
      const AbstractDataArray * nextValue = (const AbstractDataArray *)(*it.GetNextValue())();
      if (nextValue->IsFlattenable()) sum += sizeof(uint32) + nextKey->FlattenedSize() + sizeof(uint32) + sizeof(uint32) + nextValue->FlattenedSize();
   }
   return sum;
}

void Message :: Flatten(uint8 *buffer) const 
{
   // Format:  0. Protocol revision number (4 bytes, always set to CURRENT_PROTOCOL_VERSION)
   //          1. 'what' code (4 bytes)
   //          2. Number of entries (4 bytes)
   //          3. Entry name length (4 bytes)
   //          4. Entry name string (flattened String)
   //          5. Entry type code (4 bytes)
   //          6. Entry data length (4 bytes)
   //          7. Entry data (n bytes)
   //          8. loop to 3 as necessary

   // Write current protocol version
   uint32 writeOffset = 0;
   uint32 networkByteOrder = B_HOST_TO_LENDIAN_INT32(CURRENT_PROTOCOL_VERSION);
   WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

   // Write 'what' code
   networkByteOrder = B_HOST_TO_LENDIAN_INT32(what);
   WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

   // Calculate the number of flattenable entries (may be less than the total number of entries!)
   uint32 numFlattenableEntries = 0;
   {
      const GenericRef * next;
      HashtableIterator<String, GenericRef> it = _entries.GetIterator();
      while((next = it.GetNextValue()) != NULL) if (((const AbstractDataArray *)next->GetItemPointer())->IsFlattenable()) numFlattenableEntries++;
   }

   // Write number of entries
   networkByteOrder = B_HOST_TO_LENDIAN_INT32(numFlattenableEntries);
   WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

   // Write entries
   HashtableIterator<String, GenericRef> it = _entries.GetIterator();
   const String * nextKey;
   while((nextKey = it.GetNextKey()) != NULL)
   {
      const AbstractDataArray * nextValue = (const AbstractDataArray *)(*it.GetNextValue())(); 
      if (nextValue->IsFlattenable())
      {
         // Write entry name length
         uint32 keyNameSize = nextKey->FlattenedSize();
         networkByteOrder = B_HOST_TO_LENDIAN_INT32(keyNameSize);
         WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

         // Write entry name
         nextKey->Flatten(&buffer[writeOffset]);
         writeOffset += keyNameSize;

         // Write entry type code
         networkByteOrder = B_HOST_TO_LENDIAN_INT32(nextValue->TypeCode());
         WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

         // Write entry data length
         uint32 dataSize = nextValue->FlattenedSize();
         networkByteOrder = B_HOST_TO_LENDIAN_INT32(dataSize);
         WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));
         
         // Write entry data
         nextValue->Flatten(&buffer[writeOffset]);
         writeOffset += dataSize;
      }
   }
}

status_t Message :: Unflatten(const uint8 *buffer, uint32 inputBufferBytes) 
{
   Clear();

   uint32 readOffset = 0;
   
   // Read and check protocol version number
   uint32 networkByteOrder;
   if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;

   uint32 messageProtocolVersion = B_LENDIAN_TO_HOST_INT32(networkByteOrder);
   if ((messageProtocolVersion < OLDEST_SUPPORTED_PROTOCOL_VERSION) ||
       (messageProtocolVersion > CURRENT_PROTOCOL_VERSION)) return B_ERROR;
   
   // Read 'what' code
   if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
   what = B_LENDIAN_TO_HOST_INT32(networkByteOrder);

   // Read number of entries
   if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
   uint32 numEntries = B_LENDIAN_TO_HOST_INT32(networkByteOrder);

   // Read entries
   for (uint32 i=0; i<numEntries; i++)
   {
      // Read entry name length
      if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
      uint32 nameLength = B_LENDIAN_TO_HOST_INT32(networkByteOrder);
      if (nameLength > inputBufferBytes-readOffset) return B_ERROR;

      // Read entry name
      String entryName;
      if (entryName.Unflatten(&buffer[readOffset], nameLength) != B_NO_ERROR) return B_ERROR;
      readOffset += nameLength;

      // Read entry type code
      if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
      uint32 tc = B_LENDIAN_TO_HOST_INT32(networkByteOrder);

      // Read entry data length
      if (ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
      uint32 eLength = B_LENDIAN_TO_HOST_INT32(networkByteOrder);
      if (eLength > inputBufferBytes-readOffset) return B_ERROR;
   
      AbstractDataArray * nextEntry = GetOrCreateArray(entryName, tc);
      if (nextEntry == NULL) return B_ERROR;

      if (nextEntry->Unflatten(&buffer[readOffset], eLength) != B_NO_ERROR) 
      {
         Clear();  // fix for occasional crash bug; we were deleting nextEntry here, *and* in the destructor!
         return B_ERROR;
      }
      readOffset += eLength;
   }
   return B_NO_ERROR;
}

status_t Message :: AddFlatBuffer(const String & name, const Flattenable & flat, uint32 tc, bool prepend)
{
   FlatCountableRef fcRef(GetByteBufferFromPool(flat.FlattenedSize()).GetGeneric(), true);
   if ((fcRef())&&(fcRef()->CopyFrom(flat) == B_NO_ERROR))
   {
      AbstractDataArray * array = GetOrCreateArray(name, tc);
      return (array) ? (prepend ? array->PrependDataItem(&fcRef, sizeof(fcRef)) : array->AddDataItem(&fcRef, sizeof(fcRef))) : B_ERROR;
   }
   return B_ERROR;
}

status_t Message :: AddFlatRef(const String & name, FlatCountableRef ref, uint32 tc, bool prepend)
{
   AbstractDataArray * array = GetOrCreateArray(name, tc);
   return (array) ? (prepend ? array->PrependDataItem(&ref, sizeof(ref)) : array->AddDataItem(&ref, sizeof(ref))) : B_ERROR;
}

status_t Message :: AddString(const String &name, const String & val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_STRING_TYPE);
   status_t ret = array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
   return ret;
}

status_t Message :: AddInt8(const String &name, int8 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT8_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddInt16(const String &name, int16 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT16_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddInt32(const String &name, int32 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT32_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddInt64(const String &name, int64 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT64_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddBool(const String &name, bool val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_BOOL_TYPE);
   status_t ret = array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
   return ret;
}

status_t Message :: AddFloat(const String &name, float val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_FLOAT_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddDouble(const String &name, double val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_DOUBLE_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddPointer(const String &name, const void * ptr) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_POINTER_TYPE);
   return array ? array->AddDataItem(&ptr, sizeof(ptr)) : B_ERROR;
}

status_t Message :: AddPoint(const String &name, const Point & point) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_POINT_TYPE);
   return array ? array->AddDataItem(&point, sizeof(point)) : B_ERROR;
}

status_t Message :: AddRect(const String &name, const Rect & rect) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_RECT_TYPE);
   return array ? array->AddDataItem(&rect, sizeof(rect)) : B_ERROR;
}

status_t Message :: AddTag(const String &name, GenericRef tag)
{
   if (tag() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(name, B_TAG_TYPE);
   return array ? array->AddDataItem(&tag, sizeof(tag)) : B_ERROR;
}

status_t Message :: AddMessage(const String & name, MessageRef ref)
{
   if (ref() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(name, B_MESSAGE_TYPE);
   return (array) ? array->AddDataItem(&ref, sizeof(ref)) : B_ERROR;
}

status_t Message :: AddFlat(const String &name, const Flattenable &obj) 
{
   switch(obj.TypeCode())
   {
      case B_STRING_TYPE:  return (dynamic_cast<const String *>(&obj))  ? AddString(name,  (const String  &)obj) : B_ERROR;
      case B_MESSAGE_TYPE: return (dynamic_cast<const Message *>(&obj)) ? AddMessage(name, (const Message &)obj) : B_ERROR;
      default:             return AddFlatBuffer(name, obj, obj.TypeCode(), false);
   }
}

status_t Message :: AddFlat(const String &name, FlatCountableRef ref) 
{
   FlatCountable * fc = ref();
   if (fc)
   {
      uint32 tc = fc->TypeCode();
      switch(tc)
      {
         case B_MESSAGE_TYPE: return AddMessage(name, MessageRef(ref.GetGeneric(), true));
         case B_STRING_TYPE:  return B_ERROR;  // sorry, can't do that (Strings aren't FlatCountables)
         default:             return AddFlatRef(name, ref, tc, false);
      }
   }
   return B_ERROR;   
}

uint32 Message :: GetElementSize(uint32 type) const
{
   switch(type)
   {
      case B_BOOL_TYPE:    return sizeof(bool);
      case B_DOUBLE_TYPE:  return sizeof(double);
      case B_POINTER_TYPE: return sizeof(void *);
      case B_POINT_TYPE:   return sizeof(Point);
      case B_RECT_TYPE:    return sizeof(Rect);
      case B_FLOAT_TYPE:   return sizeof(float);
      case B_INT64_TYPE:   return sizeof(int64);
      case B_INT32_TYPE:   return sizeof(int32);
      case B_INT16_TYPE:   return sizeof(int16);
      case B_INT8_TYPE:    return sizeof(int8);
      case B_MESSAGE_TYPE: return sizeof(MessageRef);
      case B_STRING_TYPE:  return sizeof(String);
      default:             return 0;
   }
}

status_t Message :: AddDataAux(const String &name, const void *data, uint32 numBytes, uint32 tc, bool prepend)
{
   if (numBytes == 0) return B_ERROR;   // can't add 0 bytes, that's silly
   if (tc == B_STRING_TYPE) 
   {
      String temp((const char *)data);  // kept separate to avoid BeOS gcc optimizer bug (otherwise -O3 crashes here)
      return prepend ? PrependString(name, temp) : AddString(name, temp);
   }

   // for primitive types, we do this:
   bool isVariableSize = false;
   uint32 elementSize = GetElementSize(tc);
   if (elementSize == 0) 
   {
       // zero indicates a variable-sized data item; we will use a ByteBuffer to hold it.
       isVariableSize = true;
       elementSize    = numBytes;
       if (elementSize == 0) return B_ERROR;
   }
   if (numBytes % elementSize) return B_ERROR;  // Can't add half an element, silly!

   AbstractDataArray * array = GetOrCreateArray(name, tc);
   if (array == NULL) return B_ERROR;

   uint32 numElements = numBytes/elementSize;
   const uint8 * dataBuf = (const uint8 *) data;
   for (uint32 i=0; i<numElements; i++) 
   {
      FlatCountableRef fcRef;
      const void * dataToAdd = dataBuf ? &dataBuf[i*elementSize] : NULL;
      uint32 addSize = elementSize;
      if (isVariableSize)
      {
         ByteBufferRef bufRef = GetByteBufferFromPool(elementSize, (const uint8 *)dataToAdd);
         if (bufRef() == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
         fcRef.SetFromGeneric(bufRef.GetGeneric());
         dataToAdd = &fcRef;
         addSize = sizeof(fcRef);
      }
      if ((prepend ? array->PrependDataItem(dataToAdd, addSize) : array->AddDataItem(dataToAdd, addSize)) != B_NO_ERROR) return B_ERROR;
   }
   return B_NO_ERROR;
}

status_t Message :: RemoveName(const String & name) 
{
   return _entries.Remove(name);
}

status_t Message :: RemoveData(const String &name, uint32 index) 
{
   AbstractDataArray * array = GetArray(name, B_ANY_TYPE);
   if (array) 
   {
      status_t ret = array->RemoveDataItem(index);
      return (array->GetNumItems() == 0) ? RemoveName(name) : ret;
   }
   else return B_ERROR;
}

status_t Message :: FindString(const String &name, uint32 index, const char ** setMe) const 
{
   const StringDataArray * ada = (const StringDataArray *)GetArray(name, B_STRING_TYPE);
   if ((ada)&&(index < ada->GetNumItems()))
   {
      *setMe = ada->ItemAt(index)();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: FindString(const String &name, uint32 index, String & str) const 
{
   const StringDataArray * ada = (const StringDataArray *)GetArray(name, B_STRING_TYPE);
   if ((ada)&&(index < ada->GetNumItems()))
   {
      str = ada->ItemAt(index);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: FindFlat(const String & name, uint32 index, Flattenable & flat) const
{
   const AbstractDataArray * ada = GetArray(name, B_ANY_TYPE);
   if ((ada)&&(index < ada->GetNumItems()))
   {
      uint32 arrayType = ada->TypeCode();
      if (flat.AllowsTypeCode(arrayType))
      {
         switch(arrayType)
         {
            case B_STRING_TYPE:
            {
               const String & str = ((const StringDataArray *)ada)->ItemAt(index);
               return str.CopyTo(flat);
            }
            // not fall thru since we always return, above

            case B_MESSAGE_TYPE:
            {
               const MessageRef & mref = ((const MessageDataArray *)ada)->ItemAt(index);
               return (mref()) ? mref()->CopyTo(flat) : B_ERROR;
            }
            // not fall thru since we always return, above

            default:
            {
               const ByteBufferDataArray * bbda = dynamic_cast<const ByteBufferDataArray *>(ada);
               if (bbda)
               {
                  FlatCountable * aflat = bbda->ItemAt(index)();
                  return (aflat) ? aflat->CopyTo(flat) : B_ERROR;
               }
               else
               {
                  const uint8 * data;
                  status_t ret = ada->FindDataItem(index, (const void **) &data);
                  if (ret == B_NO_ERROR) return flat.Unflatten(data, ada->GetItemSize(index));
               }
            }
            break;
         }
      }
   }
   return B_ERROR;
}

status_t Message :: FindFlat(const String &name, uint32 index, FlatCountableRef &ref) const
{
   const AbstractDataArray * array = GetArray(name, B_ANY_TYPE);
   if ((array)&&(index < array->GetNumItems()))
   {
      const ByteBufferDataArray * bbda = dynamic_cast<const ByteBufferDataArray *>(array);
      if (bbda)
      {
         ref = bbda->ItemAt(index);
         return B_NO_ERROR;
      }
      const MessageDataArray * mda = dynamic_cast<const MessageDataArray *>(array);
      if (mda) return ref.SetFromGeneric(mda->ItemAt(index).GetGeneric());
   }
   return B_ERROR;
}

status_t Message :: FindData(const String & name, uint32 tc, uint32 index, const void ** data, uint32 * setSize) const
{
   const AbstractDataArray * array = GetArray(name, tc);
   if ((array)&&(array->FindDataItem(index, data) == B_NO_ERROR))
   {
      switch(tc)
      {
         case B_STRING_TYPE:  
         {
            const String * str = (const String *) (*data);
            *data = str->Cstr();  
            if (setSize) *setSize = str->FlattenedSize();
         }
         break;

         default:
         {
            uint32 es = GetElementSize(tc);
            if (es > 0) 
            {
               if (setSize) *setSize = es;
            }
            else
            {
               // But for user-generated types, we need to get a pointer to the actual data, not just the ref
               const FlatCountableRef * fcRef = (const FlatCountableRef *)(*data);
               const ByteBuffer * buf = dynamic_cast<ByteBuffer *>(fcRef->GetItemPointer());
               if (buf)
               {
                  const void * b = buf->GetBuffer();
                  if (b)
                  {
                     *data = b;
                     if (setSize) *setSize = buf->GetNumBytes();
                     return B_NO_ERROR;
                  }
               }
               return B_ERROR;
            }
         }
         break;
      }
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: FindDataItemAux(const String & name, uint32 index, uint32 tc, void * setValue, uint32 valueSize) const
{
   const AbstractDataArray * array = GetArray(name, tc);
   if (array == NULL) return B_ERROR;
   const void * addressOfValue;
   status_t ret = array->FindDataItem(index, &addressOfValue);
   if (ret != B_NO_ERROR) return ret;
   memcpy(setValue, addressOfValue, valueSize);
   return B_NO_ERROR;
}

status_t Message :: FindPoint(const String &name, uint32 index, Point & point) const 
{
   const PointDataArray * array = (const PointDataArray *)GetArray(name, B_POINT_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   point = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindRect(const String &name, uint32 index, Rect & rect) const 
{
   const RectDataArray * array = (const RectDataArray *)GetArray(name, B_RECT_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   rect = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindTag(const String &name, uint32 index, GenericRef & tag) const 
{
   const TagDataArray * array = (const TagDataArray*)GetArray(name, B_TAG_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   tag = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindMessage(const String &name, uint32 index, Message &msg) const 
{
   MessageRef msgRef;
   if (FindMessage(name, index, msgRef) == B_NO_ERROR)
   {
      const Message * m = msgRef.GetItemPointer();
      if (m) 
      {
         msg = *m;
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t Message :: FindMessage(const String & name, uint32 index, MessageRef & ref) const
{
   const MessageDataArray * array = (const MessageDataArray *) GetArray(name, B_MESSAGE_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   ref = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindDataPointer(const String &name, uint32 tc, uint32 index, void **data, uint32 *setSize) const 
{
   const void * dataLoc;
   status_t ret = FindData(name, tc, index, &dataLoc, setSize);
   if (ret == B_NO_ERROR)
   {
      *data = (void *) dataLoc;  // breaks const correctness, but oh well
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: ReplaceString(bool okayToAdd, const String &name, uint32 index, const String & string) 
{
   AbstractDataArray * array = GetArray(name, B_STRING_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddString(name, string);
   return array ? array->ReplaceDataItem(index, &string, sizeof(string)) : B_ERROR;
}

status_t Message :: ReplaceDataAux(bool okayToAdd, const String & name, uint32 index, void * dataBuf, uint32 bufSize, uint32 tc)
{
   AbstractDataArray * array = GetArray(name, tc);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddDataAux(name, dataBuf, bufSize, tc, false);
   return array ? array->ReplaceDataItem(index, dataBuf, bufSize) : B_ERROR;
}

status_t Message :: ReplaceInt8(bool okayToAdd, const String &name, uint32 index, int8 val) 
{
   AbstractDataArray * array = GetArray(name, B_INT8_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt8(name, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceInt16(bool okayToAdd, const String &name, uint32 index, int16 val) 
{
   AbstractDataArray * array = GetArray(name, B_INT16_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt16(name, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceInt32(bool okayToAdd, const String &name, uint32 index, int32 val) 
{
   AbstractDataArray * array = GetArray(name, B_INT32_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt32(name, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceInt64(bool okayToAdd, const String &name, uint32 index, int64 val) 
{
   AbstractDataArray * array = GetArray(name, B_INT64_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt64(name, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceBool(bool okayToAdd, const String &name, uint32 index, bool val) 
{
   AbstractDataArray * array = GetArray(name, B_BOOL_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddBool(name, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceFloat(bool okayToAdd, const String &name, uint32 index, float val) 
{
   AbstractDataArray * array = GetArray(name, B_FLOAT_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddFloat(name, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceDouble(bool okayToAdd, const String &name, uint32 index, double val) 
{
   AbstractDataArray * array = GetArray(name, B_DOUBLE_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddDouble(name, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplacePointer(bool okayToAdd, const String &name, uint32 index, const void * ptr) 
{
   AbstractDataArray * array = GetArray(name, B_POINTER_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddPointer(name, ptr);
   return array ? array->ReplaceDataItem(index, &ptr, sizeof(ptr)) : B_ERROR;
}

status_t Message :: ReplacePoint(bool okayToAdd, const String &name, uint32 index, const Point &point) 
{
   AbstractDataArray * array = GetArray(name, B_POINT_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddPoint(name, point);
   return array ? array->ReplaceDataItem(index, &point, sizeof(point)) : B_ERROR;
}

status_t Message :: ReplaceRect(bool okayToAdd, const String &name, uint32 index, const Rect &rect) 
{
   AbstractDataArray * array = GetArray(name, B_RECT_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddRect(name, rect);
   return array ? array->ReplaceDataItem(index, &rect, sizeof(rect)) : B_ERROR;
}

status_t Message :: ReplaceTag(bool okayToAdd, const String &name, uint32 index, const GenericRef tag) 
{
   if (tag() == NULL) return B_ERROR;
   AbstractDataArray * array = GetArray(name, B_TAG_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddTag(name, tag);
   return array ? array->ReplaceDataItem(index, &tag, sizeof(tag)) : B_ERROR;
}

status_t Message :: ReplaceMessage(bool okayToAdd, const String & name, uint32 index, MessageRef msgRef)
{
   if (msgRef() == NULL) return B_ERROR;
   AbstractDataArray * array = GetArray(name, B_MESSAGE_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddMessage(name, msgRef);
   if (array) return array->ReplaceDataItem(index, &msgRef, sizeof(msgRef));
   return B_ERROR;
}

status_t Message :: ReplaceFlat(bool okayToAdd, const String &name, uint32 index, const Flattenable &obj) 
{
   switch(obj.TypeCode())
   {
      case B_MESSAGE_TYPE: return (dynamic_cast<const Message *>(&obj)) ? ReplaceMessage(okayToAdd, name, index, (const Message &)obj) : B_ERROR;
      case B_POINT_TYPE:   return (dynamic_cast<const Point *>(&obj))   ? ReplacePoint(  okayToAdd, name, index, (const Point &)  obj) : B_ERROR;
      case B_RECT_TYPE:    return (dynamic_cast<const Rect *>(&obj))    ? ReplaceRect(   okayToAdd, name, index, (const Rect &)   obj) : B_ERROR;
      case B_STRING_TYPE:  return (dynamic_cast<const String *>(&obj))  ? ReplaceString( okayToAdd, name, index, (const String &) obj) : B_ERROR;
      default:
      {
         FlatCountableRef fcRef(GetByteBufferFromPool(obj.FlattenedSize()).GetGeneric(), true);
         return ((fcRef())&&(fcRef()->CopyFrom(obj) == B_NO_ERROR)) ? ReplaceDataAux(okayToAdd, name, index, &fcRef, sizeof(fcRef), obj.TypeCode()) : B_ERROR;
      }
   }
}

status_t Message :: ReplaceFlat(bool okayToAdd, const String &name, uint32 index, FlatCountableRef ref) 
{
   const FlatCountable * fc = ref();
   if (fc)
   {
      uint32 tc = fc->TypeCode();
      AbstractDataArray * array = GetArray(name, tc);
      if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddFlat(name, ref);
      if (array)
      { 
         switch(tc)
         {
            case B_MESSAGE_TYPE:  
               return (dynamic_cast<const Message *>(fc)) ? ReplaceMessage(okayToAdd, name, index, MessageRef(ref.GetGeneric(), true)) : B_ERROR;

            default:              
               if (GetElementSize(tc) == 0)
               {
                  ByteBufferDataArray * bbda = dynamic_cast<ByteBufferDataArray *>(array);
                  if (bbda) return bbda->ReplaceDataItem(index, &ref, sizeof(ref));
               }
            break;
         }
      }
   }
   return B_ERROR;
}

status_t Message :: ReplaceData(bool okayToAdd, const String &name, uint32 type, uint32 index, const void *data, uint32 numBytes) 
{
   if (type == B_STRING_TYPE) 
   {   
      String temp((const char *)data);  // temp to avoid gcc optimizer bug
      return ReplaceString(okayToAdd, name, index, temp);
   }
   AbstractDataArray * array = GetArray(name, type);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddDataAux(name, data, numBytes, type, false);
   if (array == NULL) return B_ERROR;
   
   // for primitive types, we do this:
   bool isVariableSize = false;
   uint32 elementSize = GetElementSize(type);
   if (elementSize == 0) 
   {
      // zero indicates a variable-sized data item
      isVariableSize = true;
      elementSize = numBytes;
      if (elementSize == 0) return B_ERROR;
   }

   if (numBytes % elementSize) return B_ERROR;  // Can't add half an element, silly!
   uint32 numElements = numBytes / elementSize;
   const uint8 * dataBuf = (const uint8 *) data;
   for (uint32 i=index; i<index+numElements; i++) 
   {
      FlatCountableRef ref;
      const void * dataToAdd = &dataBuf[i*elementSize];
      uint32 addSize = elementSize;
      if (isVariableSize)
      {
         ref.SetFromGeneric(GetByteBufferFromPool(elementSize, (const uint8 *)dataToAdd).GetGeneric());
         if (ref() == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
         dataToAdd = &ref;
         addSize = sizeof(ref);
      }
      if (array->ReplaceDataItem(i, dataToAdd, addSize) != B_NO_ERROR) return B_ERROR;
   }
   return B_NO_ERROR;
}

status_t Message :: MoveName(const String & name, Message &moveTo)
{
   GenericRef array = GetArrayRef(name, B_ANY_TYPE);
   if (array())
   {
      (void) moveTo.RemoveName(name);  // clear out any previous data he had under that name
      (void) _entries.Remove(name);
      return moveTo._entries.Put(name, array);
   }
   return B_ERROR;
}

uint32 Message :: GetNumValuesInName(const String & name, uint32 type) const
{
   const AbstractDataArray * array = GetArray(name, type);
   return array ? array->GetNumItems() : 0;
}

status_t Message :: CopyName(const String & name, Message &copyTo) const
{
   const AbstractDataArray * array = GetArray(name, B_ANY_TYPE);
   if (array)
   {
      GenericRef clone = array->Clone();
      if (clone())
      {
         (void) copyTo.RemoveName(name);  // clear out any previous data he had under that name
         if (copyTo._entries.Put(name, clone) == B_NO_ERROR) return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t Message :: PrependString(const String & name, const String & val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_STRING_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt8(const String &name, int8 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT8_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt16(const String &name, int16 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT16_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt32(const String &name, int32 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT32_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt64(const String &name, int64 val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_INT64_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependBool(const String &name, bool val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_BOOL_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependFloat(const String &name, float val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_FLOAT_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependDouble(const String &name, double val) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_DOUBLE_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependPointer(const String &name, const void * ptr) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_POINTER_TYPE);
   return array ? array->PrependDataItem(&ptr, sizeof(ptr)) : B_ERROR;
}

status_t Message :: PrependPoint(const String &name, const Point & point) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_POINT_TYPE);
   return array ? array->PrependDataItem(&point, sizeof(point)) : B_ERROR;
}

status_t Message :: PrependRect(const String &name, const Rect & rect) 
{
   AbstractDataArray * array = GetOrCreateArray(name, B_RECT_TYPE);
   return array ? array->PrependDataItem(&rect, sizeof(rect)) : B_ERROR;
}

status_t Message :: PrependTag(const String &name, GenericRef tag) 
{
   if (tag() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(name, B_TAG_TYPE);
   return array ? array->PrependDataItem(&tag, sizeof(tag)) : B_ERROR;
}

status_t Message :: PrependMessage(const String & name, MessageRef ref)
{
   if (ref() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(name, B_MESSAGE_TYPE);
   return (array) ? array->PrependDataItem(&ref, sizeof(ref)) : B_ERROR;
}

status_t Message :: PrependFlat(const String &name, const Flattenable &obj) 
{
   switch(obj.TypeCode())
   {
      case B_STRING_TYPE:  return (dynamic_cast<const String *>(&obj))  ? PrependString(name,  (const String &) obj) : B_ERROR;
      case B_MESSAGE_TYPE: return (dynamic_cast<const Message *>(&obj)) ? PrependMessage(name, (const Message &)obj) : B_ERROR;
      default:             return AddFlatBuffer(name, obj, obj.TypeCode(), true);
   }
}

status_t Message :: PrependFlat(const String &name, FlatCountableRef ref)
{
   FlatCountable * fc = ref();
   if (fc)
   {
      uint32 tc = fc->TypeCode();
      switch(tc)
      {
         case B_STRING_TYPE:  return B_ERROR;  // sorry, can't do that (Strings aren't FlatCountables)
         case B_MESSAGE_TYPE: return PrependMessage(name, MessageRef(ref.GetGeneric(), true)); 
         default:             return AddFlatRef(name, ref, tc, true);
      }
   }
   return B_ERROR;   
}

status_t Message :: CopyToImplementation(Flattenable & copyTo) const
{
   Message * cMsg = dynamic_cast<Message *>(&copyTo);
   if (cMsg)
   {
      *cMsg = *this;
      return B_NO_ERROR;
   }
   else return FlatCountable::CopyToImplementation(copyTo);
}

status_t Message :: CopyFromImplementation(const Flattenable & copyFrom)
{
   const Message * cMsg = dynamic_cast<const Message *>(&copyFrom);
   if (cMsg)
   {
      *this = *cMsg;
      return B_NO_ERROR;
   }
   else return FlatCountable::CopyFromImplementation(copyFrom);
}

bool Message :: operator == (const Message & rhs) const
{
   return ((this == &rhs)||((what == rhs.what)&&(CountNames() == rhs.CountNames())&&(FieldsAreSubsetOf(rhs, true))));
}

bool Message :: FieldsAreSubsetOf(const Message & rhs, bool compareContents) const
{
   // Returns true iff every one of our fields has a like-named, liked-typed, equal-length field in (rhs).
   HashtableIterator<String, GenericRef> iter = _entries.GetIterator();
   const String * nextKey;
   const GenericRef * myNextValue;
   while(iter.GetNextKeyAndValue(nextKey, myNextValue) == B_NO_ERROR)
   {
      const GenericRef * hisNextValue = rhs._entries.Get(*nextKey);
      if ((hisNextValue == NULL)||(((const AbstractDataArray*)myNextValue->GetItemPointer())->IsEqualTo((const AbstractDataArray*)(hisNextValue->GetItemPointer()), compareContents) == false)) return false;
   }
   return true;
}

END_NAMESPACE(muscle);
