/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include <stdio.h>
#include "util/ByteBuffer.h"
#include "util/Queue.h"
#include "message/Message.h"

namespace muscle {

static Message _emptyMessage;
static MessageRef _emptyMessageRef(&_emptyMessage, false);

const Message & GetEmptyMessage() {return _emptyMessage;}
const MessageRef & GetEmptyMessageRef() {return _emptyMessageRef;}

static void DoIndents(uint32 num, String & s) {for (uint32 i=0; i<num; i++) s += ' ';}

static void ClearMsgFunc(Message * msg, void *) {msg->what = 0; msg->Clear();}
static MessageRef::ItemPool _messagePool(100, ClearMsgFunc);
MessageRef::ItemPool * GetMessagePool() {return &_messagePool;}

#define DECLARECLONE(X)                  \
   RefCountableRef X :: Clone() const    \
   {                                     \
      RefCountableRef ref(NEWFIELD(X));  \
      if (ref()) *((X*)ref()) = *this;   \
      return ref;                        \
   }

#ifdef MUSCLE_DISABLE_MESSAGE_FIELD_POOLS
# define NEWFIELD(X)  newnothrow X
# define DECLAREFIELDTYPE(X) DECLARECLONE(X)
#else
# define NEWFIELD(X)  _pool##X.ObtainObject()
# define DECLAREFIELDTYPE(X) static void Clear##X(X * obj, void *) {obj->Clear(obj->GetNumItems() > 100);} static ObjectPool<X> _pool##X(100, Clear##X); DECLARECLONE(X)
#endif

MessageRef GetMessageFromPool(uint32 what)
{
   MessageRef ref(_messagePool.ObtainObject());
   if (ref()) ref()->what = what;
   return ref;
}

MessageRef GetMessageFromPool(const Message & copyMe)
{
   MessageRef ref(_messagePool.ObtainObject());
   if (ref()) *(ref()) = copyMe;
   return ref;
}

MessageRef GetMessageFromPool(const uint8 * flatBytes, uint32 numBytes)
{
   MessageRef ref(_messagePool.ObtainObject());
   if ((ref())&&(ref()->Unflatten(flatBytes, numBytes) != B_NO_ERROR)) ref.Reset();
   return ref;
}

MessageRef GetMessageFromPool(ObjectPool<Message> & pool, uint32 what)
{
   MessageRef ref(pool.ObtainObject());
   if (ref()) ref()->what = what;
   return ref;
}

MessageRef GetMessageFromPool(ObjectPool<Message> & pool, const Message & copyMe)
{
   MessageRef ref(pool.ObtainObject());
   if (ref()) *(ref()) = copyMe;
   return ref;
}

MessageRef GetMessageFromPool(ObjectPool<Message> & pool, const uint8 * flatBytes, uint32 numBytes)
{
   MessageRef ref(pool.ObtainObject());
   if ((ref())&&(ref()->Unflatten(flatBytes, numBytes) != B_NO_ERROR)) ref.Reset();
   return ref;
}

MessageRef GetLightweightCopyOfMessageFromPool(const Message & copyMe)
{
   MessageRef ref(_messagePool.ObtainObject());
   if (ref()) ref()->BecomeLightweightCopyOf(copyMe);
   return ref;
}

MessageRef GetLightweightCopyOfMessageFromPool(ObjectPool<Message> & pool, const Message & copyMe)
{
   MessageRef ref(pool.ObtainObject());
   if (ref()) ref()->BecomeLightweightCopyOf(copyMe);
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

   // Normalizes the array
   virtual void Normalize() = 0;

   // Sets (setDataLoc) to point to the (index)'th item in our array.
   // Result is not guaranteed to remain valid after this object is modified.
   virtual status_t FindDataItem(uint32 index, const void ** setDataLoc) const = 0;

   // Should replace the (index)'th data item in the array with (data).
   virtual status_t ReplaceDataItem(uint32 index, const void * data, uint32 size) = 0;

   // Returns the size (in bytes) of the item in the (index)'th slot.
   virtual uint32 GetItemSize(uint32 index) const = 0;

   // Returns the number of items currently in the array
   virtual uint32 GetNumItems() const = 0;

   // Convenience methods
   bool HasItems() const {return (GetNumItems()>0);}
   bool IsEmpty() const {return (GetNumItems()==0);}

   // Returns a 32-bit checksum for this array
   virtual uint32 CalculateChecksum(bool countNonFlattenableFields) const = 0;

   // Returns true iff all elements in the array have the same size
   virtual bool ElementsAreFixedSize() const = 0;

   // Flattenable interface
   virtual bool IsFixedSize() const {return false;}

   // returns a separate (deep) copy of this array
   virtual RefCountableRef Clone() const = 0;

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
   virtual void Normalize() {_data.Normalize();}

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

   FixedSizeDataArray<DataType> & operator=(const FixedSizeDataArray<DataType> & rhs) 
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
class TagDataArray : public FixedSizeDataArray<RefCountableRef>
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

   virtual RefCountableRef Clone() const; 

   virtual bool IsFlattenable() const {return false;}

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      // This is the best we can do, since we don't know our elements' types
      return TypeCode() + GetNumItems();
   }

protected:
   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char buf[128]; 
         sprintf(buf, "    "UINT32_FORMAT_SPEC". %p\n", i, _data.GetItemAt(i)->GetItemPointer()); 
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

   virtual void Flatten(uint8 * buffer) const
   {
      uint32 numItems = this->_data.GetNumItems(); 
      for (uint32 i=0; i<numItems; i++) 
      {
         this->_data.GetItemAt(i)->Flatten(buffer);
         buffer += _flatItemSize;
      }
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const {return this->_data.GetNumItems() * _flatItemSize;}

   virtual status_t Unflatten(const uint8 * buffer, uint32 numBytes)
   {
      this->_data.Clear();
      if (numBytes % _flatItemSize) return B_ERROR;  // length must be an even multiple of item size, or something's wrong!

      DataType temp;
      uint32 numItems = numBytes / _flatItemSize;
      if (this->_data.EnsureSize(numItems) == B_NO_ERROR)
      {
         for (uint32 i=0; i<numItems; i++)
         {
            if ((temp.Unflatten(buffer, _flatItemSize) != B_NO_ERROR)||(this->_data.AddTail(temp) != B_NO_ERROR)) return B_ERROR;
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
      uint32 numItems = this->GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char buf[64]; 
         sprintf(buf, "    "UINT32_FORMAT_SPEC". ", i);  
         s += buf;
         AddItemToString(s, this->_data[i]);
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

   virtual void Flatten(uint8 * buffer) const
   {
      DataType * dBuf = (DataType *) buffer;
      switch(this->_data.GetNumItems())
      {
         case 0:
            // do nothing
         break;

         case 1:
            ConvertToNetworkByteOrder(dBuf, this->_data.HeadPointer(), 1);
         break;

         default:
         {
            uint32 len0 = 0;
            const DataType * array0 = this->_data.GetArrayPointer(0, len0);
            if (array0) ConvertToNetworkByteOrder(dBuf, array0, len0);

            uint32 len1;
            const DataType * array1 = this->_data.GetArrayPointer(1, len1);
            if (array1) ConvertToNetworkByteOrder(&dBuf[len0], array1, len1); 
         }
         break;
      }
   }

   virtual status_t Unflatten(const uint8 * buffer, uint32 numBytes)
   {
      this->_data.Clear();
      if (numBytes % sizeof(DataType)) return B_ERROR;  // length must be an even multiple of item size, or something's wrong!

      uint32 numItems = numBytes / sizeof(DataType);
      if (this->_data.EnsureSize(numItems, true) == B_NO_ERROR)
      {  
         // Note that typically you can't rely on the contents of a Queue object to
         // be stored in a single, contiguous array like this, but in this case we've
         // called Clear() and then EnsureSize(), so we know that the array's headPointer
         // is at the front of the array, and we are safe to do this.  --jaf
         ConvertFromNetworkByteOrder(this->_data.HeadPointer(), (DataType *)buffer, numItems);
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const {return this->_data.GetNumItems() * sizeof(DataType);}

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const = 0;
   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const = 0;

   virtual const char * GetFormatString() const = 0;

   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = this->GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char temp1[100]; sprintf(temp1, GetFormatString(), this->ItemAt(i));
         char temp2[150]; sprintf(temp2, "    "UINT32_FORMAT_SPEC". [%s]\n", i, temp1);  s += temp2;
      }
   }
};

class PointDataArray : public FixedSizeFlatObjectArray<Point>
{
public:
   PointDataArray() {/* empty */}
   virtual ~PointDataArray() {/* empty */}
   virtual RefCountableRef Clone() const;
   virtual void AddItemToString(String & s, const Point & p) const 
   {
      char buf[128]; 
      sprintf(buf, "Point: %f %f\n", p.x(), p.y());
      s += buf;
   }

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += _data[i].CalculateChecksum();
      return ret;
   }
};
DECLAREFIELDTYPE(PointDataArray);

class RectDataArray : public FixedSizeFlatObjectArray<Rect>
{
public:
   RectDataArray() {/* empty */}
   virtual ~RectDataArray() {/* empty */}
   virtual RefCountableRef Clone() const;
   virtual void AddItemToString(String & s, const Rect & r) const 
   {
      char buf[256]; 
      sprintf(buf, "Rect: leftTop=(%f,%f) rightBottom=(%f,%f)\n", r.left(), r.top(), r.right(), r.bottom());
      s += buf;
   }

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += _data[i].CalculateChecksum();
      return ret;
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
 
   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += (uint32)_data[i];
      return ret;
   }

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      memcpy(writeToHere, readFromHere, numItems);  // no translation required, really
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      memcpy(writeToHere, readFromHere, numItems);  // no translation required, really
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

   virtual RefCountableRef Clone() const;

   virtual void Flatten(uint8 * buffer) const
   {
      uint8 * dBuf = (uint8 *) buffer;
      uint32 numItems = _data.GetNumItems(); 
      for (uint32 i=0; i<numItems; i++) dBuf[i] = (uint8) (_data[i] ? 1 : 0);
   }

   virtual status_t Unflatten(const uint8 * buffer, uint32 numBytes)
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

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += _data[i] ? 1 : 0;
      return ret;
   }

protected:
   virtual void ConvertToNetworkByteOrder(void *, const void *, uint32) const
   {
      MCRASH("BoolDataArray::ConvertToNetworkByteOrder should never be called");
   }

   virtual void ConvertFromNetworkByteOrder(void *, const void *, uint32) const
   {
      MCRASH("BoolDataArray::ConvertFromNetworkByteOrder should never be called");
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

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += (uint32)_data[i];
      return ret;
   }

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int16 * writeToHere16 = (int16 *) writeToHere;
      const int16 * readFromHere16 = (const int16 *) readFromHere;
      for (uint32 i=0; i<numItems; i++) muscleCopyOut(&writeToHere16[i], B_HOST_TO_LENDIAN_INT16(readFromHere16[i]));
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int16 * writeToHere16 = (int16 *) writeToHere;
      const int16 * readFromHere16 = (const int16 *) readFromHere;
      for (uint32 i=0; i<numItems; i++)
      {
         int16 val; muscleCopyIn(val, &readFromHere16[i]);
         writeToHere16[i] = B_LENDIAN_TO_HOST_INT16(val);
      }
   }
};
DECLAREFIELDTYPE(Int16DataArray);

class Int32DataArray : public PrimitiveTypeDataArray<int32> 
{
public:
   Int32DataArray() {/* empty */}
   virtual ~Int32DataArray() {/* empty */}

   virtual uint32 TypeCode() const {return B_INT32_TYPE;}

   virtual const char * GetFormatString() const {return INT32_FORMAT_SPEC;}

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += (uint32)_data[i];
      return ret;
   }

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int32 * writeToHere32 = (int32 *) writeToHere;
      const int32 * readFromHere32 = (const int32 *) readFromHere;
      for (uint32 i=0; i<numItems; i++) muscleCopyOut(&writeToHere32[i], B_HOST_TO_LENDIAN_INT32(readFromHere32[i]));
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int32 * writeToHere32 = (int32 *) writeToHere;
      const int32 * readFromHere32 = (const int32 *) readFromHere;
      for (uint32 i=0; i<numItems; i++)
      {
         int32 val; muscleCopyIn(val, &readFromHere32[i]);
         writeToHere32[i] = B_LENDIAN_TO_HOST_INT32(val);
      }
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

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += CalculateChecksumForUint64(_data[i]);
      return ret;
   }

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int64 * writeToHere64 = (int64 *) writeToHere;
      const int64 * readFromHere64 = (const int64 *) readFromHere;
      for (uint32 i=0; i<numItems; i++) muscleCopyOut(&writeToHere64[i], B_HOST_TO_LENDIAN_INT64(readFromHere64[i]));
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int64 * writeToHere64 = (int64 *) writeToHere;
      const int64 * readFromHere64 = (const int64 *) readFromHere;
      for (uint32 i=0; i<numItems; i++)
      {
         int64 val; muscleCopyIn(val, &readFromHere64[i]);
         writeToHere64[i] = B_LENDIAN_TO_HOST_INT64(val);
      }
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

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += CalculateChecksumForFloat(_data[i]);
      return ret;
   }

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int32 * writeToHere32 = (int32 *) writeToHere;  // yeah, they're really floats, but no need to worry about that here
      const int32 * readFromHere32 = (const int32 *) readFromHere;
      for (uint32 i=0; i<numItems; i++) muscleCopyOut(&writeToHere32[i], B_HOST_TO_LENDIAN_INT32(readFromHere32[i]));
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int32 * writeToHere32 = (int32 *) writeToHere;  // yeah, they're really floats, but no need to worry about that here
      const int32 * readFromHere32 = (const int32 *) readFromHere;
      for (uint32 i=0; i<numItems; i++)
      {
         int32 val; muscleCopyIn(val, &readFromHere32[i]);
         writeToHere32[i] = B_LENDIAN_TO_HOST_INT32(val);
      }
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

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += CalculateChecksumForDouble(_data[i]);
      return ret;
   }

protected:
   virtual void ConvertToNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int64 * writeToHere64 = (int64 *) writeToHere;  // yeah, they're really doubles, but no need to worry about that here
      const int64 * readFromHere64 = (const int64 *) readFromHere;
      for (uint32 i=0; i<numItems; i++) muscleCopyOut(&writeToHere64[i], B_HOST_TO_LENDIAN_INT64(readFromHere64[i]));
   }

   virtual void ConvertFromNetworkByteOrder(void * writeToHere, const void * readFromHere, uint32 numItems) const
   {
      int64 * writeToHere64 = (int64 *) writeToHere;  // yeah, they're really doubles, but no need to worry about that here
      const int64 * readFromHere64 = (const int64 *) readFromHere;
      for (uint32 i=0; i<numItems; i++)
      {
         int64 val; muscleCopyIn(val, &readFromHere64[i]);
         writeToHere64[i] = B_LENDIAN_TO_HOST_INT64(val);
      }
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

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      return TypeCode() + GetNumItems();  // Best we can do, since pointer equivalence is not well defined
   }

protected:
   virtual void ConvertToNetworkByteOrder(void *, const void *, uint32) const
   {
      MCRASH("PointerDataArray::ConvertToNetworkByteOrder should never be called");
   }

   virtual void ConvertFromNetworkByteOrder(void *, const void *, uint32) const
   {
      MCRASH("PointerDataArray::ConvertFromNetworkByteOrder should never be called");
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
      const FlatCountable * msg = this->ItemAt(index)();
      return msg ? msg->FlattenedSize() : 0;
   }

   virtual uint32 TypeCode() const {return _typeCode;}

   virtual bool ElementsAreFixedSize() const {return false;}

   /** Whether or not we should write the number-of-items element when we flatten this array.
     * Older versions of muscle didn't do this for MessageDataArray objects, so we need to
     * maintain that behaviour so that we don't break compatibility.  (bleah)
     */
   virtual bool ShouldWriteNumItems() const {return true;}

   virtual void Flatten(uint8 * buffer) const
   {
      uint32 writeOffset = 0;
      uint32 numItems = this->_data.GetNumItems(); 

      // Conditional to allow maintaining backwards compatibility with old versions of muscle's MessageDataArrays (sigh)
      if (ShouldWriteNumItems()) 
      {
         uint32 writeNumElements = B_HOST_TO_LENDIAN_INT32(numItems);
         this->WriteData(buffer, &writeOffset, &writeNumElements, sizeof(writeNumElements));
      }

      for (uint32 i=0; i<numItems; i++) 
      {
         const FlatCountable * next = this->ItemAt(i)();
         if (next)
         {
            uint32 fs = next->FlattenedSize();
            uint32 writeFs = B_HOST_TO_LENDIAN_INT32(fs);
            this->WriteData(buffer, &writeOffset, &writeFs, sizeof(writeFs));
            next->Flatten(&buffer[writeOffset]);
            writeOffset += fs;
         }
      }
   }

   // Flattenable interface
   virtual uint32 FlattenedSize() const 
   {
      uint32 numItems = this->GetNumItems();
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

   virtual status_t Unflatten(const uint8 * buffer, uint32 numBytes)
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
         FlatCountableRef fcRef(GetByteBufferFromPool(readFs, &buffer[readOffset]).GetRefCountableRef(), true);
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
         sprintf(buf, "    "UINT32_FORMAT_SPEC". ", i);
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
            sprintf(buf, "[flattenedSize="UINT32_FORMAT_SPEC"] ", bb->GetNumBytes()); 
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

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--)
      {
         const ByteBuffer * buf = dynamic_cast<ByteBuffer *>(_data[i]());  // TODO: possibly make this a static cast?
         if (buf) ret += buf->CalculateChecksum();
      }
      return ret;
   }

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

   virtual status_t Unflatten(const uint8 * buffer, uint32 numBytes)
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
         sprintf(buf, "    "UINT32_FORMAT_SPEC". ", i); 
         s += buf;

         const void * vp;
         uint32 itemSize = GetItemSize(i);
         if (FindDataItem(i, &vp) == B_NO_ERROR)
         {
            MessageRef * dataItem = (MessageRef *) vp;
            Message * msg = dataItem->GetItemPointer();
            if (msg)
            {
               char tcbuf[5]; MakePrettyTypeCodeString(msg->what, tcbuf);
               sprintf(buf, "[what='%s' ("INT32_FORMAT_SPEC"/0x"XINT32_FORMAT_SPEC"), flattenedSize="UINT32_FORMAT_SPEC", numFields="UINT32_FORMAT_SPEC"]\n", tcbuf, msg->what, msg->what, itemSize, msg->GetNumNames());
               s += buf;

               if (recurse) msg->AddToString(s, recurse, indent+3);
            }
            else s += "[NULL]\n";
         }
         else s += "[<Error!>]\n";
      }
   }

   virtual RefCountableRef Clone() const;

   virtual uint32 CalculateChecksum(bool countNonFlattenableFields) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--)
      {
         const MessageRef & msg = _data[i];
         if (msg()) ret += msg()->CalculateChecksum(countNonFlattenableFields);
      }
      return ret;
   }

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

   virtual uint32 GetItemSize(uint32 index) const {return this->ItemAt(index).FlattenedSize();}
   virtual bool ElementsAreFixedSize() const {return false;}

   virtual void Flatten(uint8 * buffer) const
   {
      // Format:  0. number of entries (4 bytes)
      //          1. entry size in bytes (4 bytes)
      //          2. entry data (n bytes)
      //          (repeat 1. and 2. as necessary)
      uint32 numElements = this->GetNumItems();
      uint32 networkByteOrder = B_HOST_TO_LENDIAN_INT32(numElements);
      uint32 writeOffset = 0;

      this->WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

      for (uint32 i=0; i<numElements; i++)
      {
         // write element size
         const Flattenable & s = this->ItemAt(i);
         uint32 nextElementBytes = s.FlattenedSize();
         networkByteOrder = B_HOST_TO_LENDIAN_INT32(nextElementBytes);
         this->WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

         // write element data
         s.Flatten(&buffer[writeOffset]);
         writeOffset += nextElementBytes;
      }
   }

   virtual uint32 FlattenedSize() const 
   {
      uint32 num = this->GetNumItems();
      uint32 sum = (num+1)*sizeof(uint32);  // 1 uint32 for the count, plus 1 per entry for entry-size
      for (uint32 i=0; i<num; i++) sum += this->ItemAt(i).FlattenedSize();
      return sum;
   }

   virtual status_t Unflatten(const uint8 * buffer, uint32 inputBufferBytes)
   {
      this->Clear(false);

      uint32 networkByteOrder;
      uint32 readOffset = 0;

      if (this->ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
      uint32 numElements = B_LENDIAN_TO_HOST_INT32(networkByteOrder);
      if (this->_data.EnsureSize(numElements) != B_NO_ERROR) return B_ERROR;
      for (uint32 i=0; i<numElements; i++)
      {
         if (this->ReadData(buffer, inputBufferBytes, &readOffset, &networkByteOrder, sizeof(networkByteOrder)) != B_NO_ERROR) return B_ERROR;
         uint32 elementSize = B_LENDIAN_TO_HOST_INT32(networkByteOrder);
         if (elementSize == 0) return B_ERROR;  // it should always have at least the trailing NUL byte!

         // read element data
         if ((readOffset + elementSize > inputBufferBytes)||(this->_data.AddTail() != B_NO_ERROR)||(this->_data.TailPointer()->Unflatten(&buffer[readOffset], elementSize) != B_NO_ERROR)) return B_ERROR;
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

   virtual RefCountableRef Clone() const;

   virtual void AddToString(String & s, bool, int indent) const
   {
      uint32 numItems = GetNumItems();
      for (uint32 i=0; i<numItems; i++) 
      {
         DoIndents(indent,s); 
         char buf[50]; 
         sprintf(buf,"    "UINT32_FORMAT_SPEC". [", i); 
         s += buf;
         s += ItemAt(i);
         s += "]\n";
      }
   }

   virtual uint32 CalculateChecksum(bool /*countNonFlattenableFields*/) const
   {
      uint32 ret = TypeCode() + GetNumItems();
      for (int32 i=GetNumItems()-1; i>=0; i--) ret += _data[i].CalculateChecksum();
      return ret;
   }
};
DECLAREFIELDTYPE(StringDataArray);

status_t MessageFieldNameIterator :: GetNextFieldName(String & fieldName)
{
   const String * ret = GetNextFieldNameString();
   if (ret)
   {
      fieldName = *ret;
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t MessageFieldNameIterator :: PeekNextFieldName(String & fieldName) const
{
   const String * ret = PeekNextFieldNameString();
   if (ret)
   {
      fieldName = *ret;
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
      const RefCountableRef * array;
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
      const RefCountableRef * array;
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

Message & Message :: operator=(const Message & rhs) 
{
   TCHECKPOINT;

   if (this != &rhs)
   {
      Clear();
      what = rhs.what;

      // Copy all his entries!
      HashtableIterator<String, RefCountableRef> it(rhs._entries, HTIT_FLAG_NOREGISTER);
      const String * nextKey;
      while((nextKey = it.GetNextKey()) != NULL) 
      {
         RefCountableRef clone = ((const AbstractDataArray *)(*it.GetNextValue())())->Clone();
         if (clone()) (void) _entries.Put(*nextKey, clone);
      }
   }
   return *this;
}

status_t Message :: GetInfo(const String & fieldName, uint32 * type, uint32 * c, bool * fixedSize) const
{
   const AbstractDataArray * array = GetArray(fieldName, B_ANY_TYPE);
   if (array == NULL) return B_ERROR;
   if (type)      *type      = array->TypeCode();
   if (c)         *c         = array->GetNumItems();
   if (fixedSize) *fixedSize = array->ElementsAreFixedSize();
   return B_NO_ERROR;
}

uint32 Message :: GetNumNames(uint32 type) const 
{
   if (type == B_ANY_TYPE) return _entries.GetNumItems();

   // oops, gotta count just the entries of the given type
   uint32 total = 0;
   HashtableIterator<String, RefCountableRef> it(_entries, HTIT_FLAG_NOREGISTER);
   const RefCountableRef * nextValue;
   while((nextValue = it.GetNextValue()) != NULL) if (((const AbstractDataArray*)nextValue->GetItemPointer())->TypeCode() == type) total++;
   return total;
}

void Message :: PrintToStream(FILE * optFile, bool recurse, int indent) const 
{
   String s; AddToString(s, recurse, indent);
   fprintf(optFile?optFile:stdout, "%s", s());
}

String Message :: ToString(bool recurse, int indent) const 
{
   String s;  
   AddToString(s, recurse, indent);
   return s;
}

void Message :: AddToString(String & s, bool recurse, int indent) const 
{
   TCHECKPOINT;

   String ret;

   char prettyTypeCodeBuf[5];
   MakePrettyTypeCodeString(what, prettyTypeCodeBuf);

   char buf[128];
   DoIndents(indent,s); 
   sprintf(buf, "Message:  what='%s' ("INT32_FORMAT_SPEC"/0x"XINT32_FORMAT_SPEC"), entryCount="INT32_FORMAT_SPEC", flatSize="UINT32_FORMAT_SPEC" checksum="UINT32_FORMAT_SPEC"\n", prettyTypeCodeBuf, what, what, GetNumNames(B_ANY_TYPE), FlattenedSize(), CalculateChecksum());
   s += buf;

   for (HashtableIterator<String, RefCountableRef> iter(_entries, HTIT_FLAG_NOREGISTER); iter.HasMoreKeys(); iter++)
   {
      const AbstractDataArray * nextValue = static_cast<const AbstractDataArray *>(iter.GetValue()());
      uint32 tc = nextValue->TypeCode();
      MakePrettyTypeCodeString(tc, prettyTypeCodeBuf);
      DoIndents(indent,s); 
      s += "  Entry: Name=[";
      s += iter.GetKey();
      sprintf(buf, "], GetNumItems()="INT32_FORMAT_SPEC", TypeCode()='%s' ("INT32_FORMAT_SPEC") flatSize="UINT32_FORMAT_SPEC" checksum="UINT32_FORMAT_SPEC"\n", nextValue->GetNumItems(), prettyTypeCodeBuf, tc, nextValue->FlattenedSize(), nextValue->CalculateChecksum(false));
      s += buf;
      nextValue->AddToString(s, recurse, indent);
   }
}

// Returns an pointer to a held array of the given type, if it exists.  If (tc) is B_ANY_TYPE, then any type array is acceptable.
AbstractDataArray * Message :: GetArray(const String & arrayName, uint32 tc) const
{
   RefCountableRef * array;
   return (((array = _entries.Get(arrayName)) != NULL)&&((tc == B_ANY_TYPE)||(tc == ((const AbstractDataArray *)array->GetItemPointer())->TypeCode()))) ? ((AbstractDataArray *)array->GetItemPointer()) : NULL;
}

// Returns an pointer to a held array of the given type, if it exists.  If (tc) is B_ANY_TYPE, then any type array is acceptable.
RefCountableRef Message :: GetArrayRef(const String & arrayName, uint32 tc) const
{
   RefCountableRef array;
   if ((_entries.Get(arrayName, array) == B_NO_ERROR)&&(tc != B_ANY_TYPE)&&(tc != ((const AbstractDataArray *)array())->TypeCode())) array.Reset();
   return array;
}

AbstractDataArray * Message :: GetOrCreateArray(const String & arrayName, uint32 tc)
{
   TCHECKPOINT;

   {
      AbstractDataArray * nextEntry = GetArray(arrayName, tc);
      if (nextEntry) return nextEntry;
   }

   // Make sure the problem isn't that there already exists an array, but of the wrong type...
   // If that's the case, we can't create a like-names array of a different type, so fail.
   if (_entries.ContainsKey(arrayName)) return NULL;

   // Oops!  This array doesn't exist; better create it!
   RefCountableRef newEntry;
   switch(tc)
   {
      case B_BOOL_TYPE:    newEntry.SetRef(NEWFIELD(BoolDataArray));    break;
      case B_DOUBLE_TYPE:  newEntry.SetRef(NEWFIELD(DoubleDataArray));  break;
      case B_POINTER_TYPE: newEntry.SetRef(NEWFIELD(PointerDataArray)); break;
      case B_POINT_TYPE:   newEntry.SetRef(NEWFIELD(PointDataArray));   break;
      case B_RECT_TYPE:    newEntry.SetRef(NEWFIELD(RectDataArray));    break;
      case B_FLOAT_TYPE:   newEntry.SetRef(NEWFIELD(FloatDataArray));   break;
      case B_INT64_TYPE:   newEntry.SetRef(NEWFIELD(Int64DataArray));   break;
      case B_INT32_TYPE:   newEntry.SetRef(NEWFIELD(Int32DataArray));   break;
      case B_INT16_TYPE:   newEntry.SetRef(NEWFIELD(Int16DataArray));   break;
      case B_INT8_TYPE:    newEntry.SetRef(NEWFIELD(Int8DataArray));    break;
      case B_MESSAGE_TYPE: newEntry.SetRef(NEWFIELD(MessageDataArray)); break;
      case B_STRING_TYPE:  newEntry.SetRef(NEWFIELD(StringDataArray));  break;
      case B_TAG_TYPE:     newEntry.SetRef(NEWFIELD(TagDataArray));     break;
      default:             
         newEntry.SetRef(NEWFIELD(ByteBufferDataArray));
         if (newEntry()) ((ByteBufferDataArray*)newEntry())->SetTypeCode(tc);
         break;
   }
   return ((newEntry())&&(_entries.Put(arrayName, newEntry) == B_NO_ERROR)) ? (AbstractDataArray*)newEntry() : NULL;
}

status_t Message :: Rename(const String & oldFieldName, const String & newFieldName) 
{
   RefCountableRef oldArray = GetArrayRef(oldFieldName, B_ANY_TYPE);
   if (oldArray()) 
   {
      (void) RemoveName(newFieldName);             // destructive rename... remove anybody in our way
      (void) _entries.Remove(oldFieldName);        // remove from under old name
      return _entries.Put(newFieldName, oldArray); // add to under new name
   }
   return B_ERROR;
}

uint32 Message :: FlattenedSize() const 
{
   uint32 sum = 3 * sizeof(uint32);  // For the message header:  4 bytes for the protocol revision #, 4 bytes for the number-of-entries field, 4 bytes for what code

   // For each flattenable field: 4 bytes for the name length, name data, 4 bytes for entry type code, 4 bytes for entry data length, entry data
   HashtableIterator<String, RefCountableRef> it(_entries, HTIT_FLAG_NOREGISTER);
   const String * nextKey;
   while((nextKey = it.GetNextKey()) != NULL)  
   {
      const AbstractDataArray * nextValue = (const AbstractDataArray *)(*it.GetNextValue())();
      if (nextValue->IsFlattenable()) sum += sizeof(uint32) + nextKey->FlattenedSize() + sizeof(uint32) + sizeof(uint32) + nextValue->FlattenedSize();
   }
   return sum;
}

uint32 Message :: CalculateChecksum(bool countNonFlattenableFields) const 
{
   uint32 ret = what;

   // Calculate the number of flattenable entries (may be less than the total number of entries!)
   const RefCountableRef * next;
   HashtableIterator<String, RefCountableRef> it(_entries, HTIT_FLAG_NOREGISTER);
   while((next = it.GetNextValue()) != NULL) 
   {
      const AbstractDataArray * a = static_cast<const AbstractDataArray *>(next->GetItemPointer());
      if ((countNonFlattenableFields)||(a->IsFlattenable())) ret += a->CalculateChecksum(countNonFlattenableFields);
   }
   return ret;
}

void Message :: Flatten(uint8 * buffer) const 
{
   TCHECKPOINT;

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
      const RefCountableRef * next;
      HashtableIterator<String, RefCountableRef> it(_entries, HTIT_FLAG_NOREGISTER);
      while((next = it.GetNextValue()) != NULL) if (((const AbstractDataArray *)next->GetItemPointer())->IsFlattenable()) numFlattenableEntries++;
   }

   // Write number of entries
   networkByteOrder = B_HOST_TO_LENDIAN_INT32(numFlattenableEntries);
   WriteData(buffer, &writeOffset, &networkByteOrder, sizeof(networkByteOrder));

   // Write entries
   HashtableIterator<String, RefCountableRef> it(_entries, HTIT_FLAG_NOREGISTER);
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

status_t Message :: Unflatten(const uint8 * buffer, uint32 inputBufferBytes) 
{
   TCHECKPOINT;

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

status_t Message :: AddFlatBuffer(const String & fieldName, const Flattenable & flat, uint32 tc, bool prepend)
{
   FlatCountableRef fcRef(GetByteBufferFromPool(flat.FlattenedSize()).GetRefCountableRef(), true);
   if ((fcRef())&&(fcRef()->CopyFrom(flat) == B_NO_ERROR))
   {
      AbstractDataArray * array = GetOrCreateArray(fieldName, tc);
      return (array) ? (prepend ? array->PrependDataItem(&fcRef, sizeof(fcRef)) : array->AddDataItem(&fcRef, sizeof(fcRef))) : B_ERROR;
   }
   return B_ERROR;
}

status_t Message :: AddFlatRef(const String & fieldName, const FlatCountableRef & ref, uint32 tc, bool prepend)
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, tc);
   return (array) ? (prepend ? array->PrependDataItem(&ref, sizeof(ref)) : array->AddDataItem(&ref, sizeof(ref))) : B_ERROR;
}

status_t Message :: AddString(const String & fieldName, const String & val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_STRING_TYPE);
   status_t ret = array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
   return ret;
}

status_t Message :: AddInt8(const String & fieldName, int8 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT8_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddInt16(const String & fieldName, int16 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT16_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddInt32(const String & fieldName, int32 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT32_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddInt64(const String & fieldName, int64 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT64_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddBool(const String & fieldName, bool val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_BOOL_TYPE);
   status_t ret = array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
   return ret;
}

status_t Message :: AddFloat(const String & fieldName, float val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_FLOAT_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddDouble(const String & fieldName, double val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_DOUBLE_TYPE);
   return array ? array->AddDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: AddPointer(const String & fieldName, const void * ptr) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_POINTER_TYPE);
   return array ? array->AddDataItem(&ptr, sizeof(ptr)) : B_ERROR;
}

status_t Message :: AddPoint(const String & fieldName, const Point & point) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_POINT_TYPE);
   return array ? array->AddDataItem(&point, sizeof(point)) : B_ERROR;
}

status_t Message :: AddRect(const String & fieldName, const Rect & rect) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_RECT_TYPE);
   return array ? array->AddDataItem(&rect, sizeof(rect)) : B_ERROR;
}

status_t Message :: AddTag(const String & fieldName, const RefCountableRef & tag)
{
   if (tag() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_TAG_TYPE);
   return array ? array->AddDataItem(&tag, sizeof(tag)) : B_ERROR;
}

status_t Message :: AddMessage(const String & fieldName, const MessageRef & ref)
{
   if (ref() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_MESSAGE_TYPE);
   return (array) ? array->AddDataItem(&ref, sizeof(ref)) : B_ERROR;
}

status_t Message :: AddFlat(const String & fieldName, const Flattenable &obj) 
{
   switch(obj.TypeCode())
   {
      case B_STRING_TYPE:  return (dynamic_cast<const String *>(&obj))  ? AddString(fieldName,  (const String  &)obj) : B_ERROR;
      case B_MESSAGE_TYPE: return (dynamic_cast<const Message *>(&obj)) ? AddMessage(fieldName, (const Message &)obj) : B_ERROR;
      default:             return AddFlatBuffer(fieldName, obj, obj.TypeCode(), false);
   }
}

status_t Message :: AddFlat(const String & fieldName, const FlatCountableRef & ref) 
{
   FlatCountable * fc = ref();
   if (fc)
   {
      uint32 tc = fc->TypeCode();
      switch(tc)
      {
         case B_MESSAGE_TYPE: return AddMessage(fieldName, MessageRef(ref.GetRefCountableRef(), true));
         case B_STRING_TYPE:  return B_ERROR;  // sorry, can't do that (Strings aren't FlatCountables)
         default:             return AddFlatRef(fieldName, ref, tc, false);
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

status_t Message :: AddDataAux(const String & fieldName, const void * data, uint32 numBytes, uint32 tc, bool prepend)
{
   if (numBytes == 0) return B_ERROR;   // can't add 0 bytes, that's silly
   if (tc == B_STRING_TYPE) 
   {
      String temp((const char *)data);  // kept separate to avoid BeOS gcc optimizer bug (otherwise -O3 crashes here)
      return prepend ? PrependString(fieldName, temp) : AddString(fieldName, temp);
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

   AbstractDataArray * array = GetOrCreateArray(fieldName, tc);
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
         fcRef.SetFromRefCountableRef(bufRef.GetRefCountableRef());
         dataToAdd = &fcRef;
         addSize = sizeof(fcRef);
      }
      if ((prepend ? array->PrependDataItem(dataToAdd, addSize) : array->AddDataItem(dataToAdd, addSize)) != B_NO_ERROR) return B_ERROR;
   }
   return B_NO_ERROR;
}

void * Message :: GetPointerToNormalizedFieldData(const String & fieldName, uint32 * retNumItems, uint32 typeCode) 
{
   RefCountableRef * e = _entries.Get(fieldName);
   if (e)
   {
      AbstractDataArray * a = static_cast<AbstractDataArray *>(e->GetItemPointer());
      if ((typeCode == B_ANY_TYPE)||(typeCode == a->TypeCode()))
      {
         a->Normalize();

         const void * ptr;
         if (a->FindDataItem(0, &ptr) == B_NO_ERROR)  // must be called AFTER a->Normalize()
         {
            if (retNumItems) *retNumItems = a->GetNumItems();
            return const_cast<void *>(ptr);
         }
      }
   }
   return NULL;
}

status_t Message :: EnsureFieldIsPrivate(const String & fieldName) 
{
   RefCountableRef * e = _entries.Get(fieldName);
   return e ? (e->IsRefPrivate() ? B_NO_ERROR : CopyName(fieldName, *this)) : B_ERROR;  // Copying the field to ourself replaces the field with an unshared clone
}

status_t Message :: RemoveName(const String & fieldName) 
{
   return _entries.Remove(fieldName);
}

status_t Message :: RemoveData(const String & fieldName, uint32 index) 
{
   AbstractDataArray * array = GetArray(fieldName, B_ANY_TYPE);
   if (array) 
   {
      status_t ret = array->RemoveDataItem(index);
      return (array->IsEmpty()) ? RemoveName(fieldName) : ret;
   }
   else return B_ERROR;
}

status_t Message :: FindString(const String & fieldName, uint32 index, const char * & setMe) const
{
   const StringDataArray * ada = (const StringDataArray *)GetArray(fieldName, B_STRING_TYPE);
   if ((ada)&&(index < ada->GetNumItems()))
   {
      setMe = ada->ItemAt(index)();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: FindString(const String & fieldName, uint32 index, const String ** setMe) const 
{
   const StringDataArray * ada = (const StringDataArray *)GetArray(fieldName, B_STRING_TYPE);
   if ((ada)&&(index < ada->GetNumItems()))
   {
      *setMe = &ada->ItemAt(index);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: FindString(const String & fieldName, uint32 index, String & str) const 
{
   const StringDataArray * ada = (const StringDataArray *)GetArray(fieldName, B_STRING_TYPE);
   if ((ada)&&(index < ada->GetNumItems()))
   {
      str = ada->ItemAt(index);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: FindFlat(const String & fieldName, uint32 index, Flattenable & flat) const
{
   TCHECKPOINT;

   const AbstractDataArray * ada = GetArray(fieldName, B_ANY_TYPE);
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
                  const void * data;
                  status_t ret = ada->FindDataItem(index, &data);
                  if (ret == B_NO_ERROR) return flat.Unflatten((const uint8 *)data, ada->GetItemSize(index));
               }
            }
            break;
         }
      }
   }
   return B_ERROR;
}

status_t Message :: FindFlat(const String & fieldName, uint32 index, FlatCountableRef & ref) const
{
   TCHECKPOINT;

   const AbstractDataArray * array = GetArray(fieldName, B_ANY_TYPE);
   if ((array)&&(index < array->GetNumItems()))
   {
      const ByteBufferDataArray * bbda = dynamic_cast<const ByteBufferDataArray *>(array);
      if (bbda)
      {
         ref = bbda->ItemAt(index);
         return B_NO_ERROR;
      }
      const MessageDataArray * mda = dynamic_cast<const MessageDataArray *>(array);
      if (mda) return ref.SetFromRefCountableRef(mda->ItemAt(index).GetRefCountableRef());
   }
   return B_ERROR;
}

status_t Message :: FindData(const String & fieldName, uint32 tc, uint32 index, const void ** data, uint32 * setSize) const
{
   TCHECKPOINT;

   const AbstractDataArray * array = GetArray(fieldName, tc);
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

status_t Message :: FindDataItemAux(const String & fieldName, uint32 index, uint32 tc, void * setValue, uint32 valueSize) const
{
   const AbstractDataArray * array = GetArray(fieldName, tc);
   if (array == NULL) return B_ERROR;
   const void * addressOfValue;
   status_t ret = array->FindDataItem(index, &addressOfValue);
   if (ret != B_NO_ERROR) return ret;
   memcpy(setValue, addressOfValue, valueSize);
   return B_NO_ERROR;
}

status_t Message :: FindPoint(const String & fieldName, uint32 index, Point & point) const 
{
   const PointDataArray * array = (const PointDataArray *)GetArray(fieldName, B_POINT_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   point = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindRect(const String & fieldName, uint32 index, Rect & rect) const 
{
   const RectDataArray * array = (const RectDataArray *)GetArray(fieldName, B_RECT_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   rect = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindTag(const String & fieldName, uint32 index, RefCountableRef & tag) const 
{
   const TagDataArray * array = (const TagDataArray*)GetArray(fieldName, B_TAG_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   tag = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindMessage(const String & fieldName, uint32 index, Message & msg) const 
{
   MessageRef msgRef;
   if (FindMessage(fieldName, index, msgRef) == B_NO_ERROR)
   {
      const Message * m = msgRef();
      if (m) 
      {
         msg = *m;
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t Message :: FindMessage(const String & fieldName, uint32 index, MessageRef & ref) const
{
   const MessageDataArray * array = (const MessageDataArray *) GetArray(fieldName, B_MESSAGE_TYPE);
   if ((array == NULL)||(index >= array->GetNumItems())) return B_ERROR;
   ref = array->ItemAt(index);
   return B_NO_ERROR;
}

status_t Message :: FindDataPointer(const String & fieldName, uint32 tc, uint32 index, void ** data, uint32 * setSize) const 
{
   const void * dataLoc;
   status_t ret = FindData(fieldName, tc, index, &dataLoc, setSize);
   if (ret == B_NO_ERROR)
   {
      *data = (void *) dataLoc;  // breaks const correctness, but oh well
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: ReplaceString(bool okayToAdd, const String & fieldName, uint32 index, const String & string) 
{
   AbstractDataArray * array = GetArray(fieldName, B_STRING_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddString(fieldName, string);
   return array ? array->ReplaceDataItem(index, &string, sizeof(string)) : B_ERROR;
}

status_t Message :: ReplaceDataAux(bool okayToAdd, const String & fieldName, uint32 index, void * dataBuf, uint32 bufSize, uint32 tc)
{
   AbstractDataArray * array = GetArray(fieldName, tc);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddDataAux(fieldName, dataBuf, bufSize, tc, false);
   return array ? array->ReplaceDataItem(index, dataBuf, bufSize) : B_ERROR;
}

status_t Message :: ReplaceInt8(bool okayToAdd, const String & fieldName, uint32 index, int8 val) 
{
   AbstractDataArray * array = GetArray(fieldName, B_INT8_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt8(fieldName, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceInt16(bool okayToAdd, const String & fieldName, uint32 index, int16 val) 
{
   AbstractDataArray * array = GetArray(fieldName, B_INT16_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt16(fieldName, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceInt32(bool okayToAdd, const String & fieldName, uint32 index, int32 val) 
{
   AbstractDataArray * array = GetArray(fieldName, B_INT32_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt32(fieldName, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceInt64(bool okayToAdd, const String & fieldName, uint32 index, int64 val) 
{
   AbstractDataArray * array = GetArray(fieldName, B_INT64_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddInt64(fieldName, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceBool(bool okayToAdd, const String & fieldName, uint32 index, bool val) 
{
   AbstractDataArray * array = GetArray(fieldName, B_BOOL_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddBool(fieldName, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceFloat(bool okayToAdd, const String & fieldName, uint32 index, float val) 
{
   AbstractDataArray * array = GetArray(fieldName, B_FLOAT_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddFloat(fieldName, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplaceDouble(bool okayToAdd, const String & fieldName, uint32 index, double val) 
{
   AbstractDataArray * array = GetArray(fieldName, B_DOUBLE_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddDouble(fieldName, val);
   return array ? array->ReplaceDataItem(index, &val, sizeof(val)) : B_ERROR;
}

status_t Message :: ReplacePointer(bool okayToAdd, const String & fieldName, uint32 index, const void * ptr) 
{
   AbstractDataArray * array = GetArray(fieldName, B_POINTER_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddPointer(fieldName, ptr);
   return array ? array->ReplaceDataItem(index, &ptr, sizeof(ptr)) : B_ERROR;
}

status_t Message :: ReplacePoint(bool okayToAdd, const String & fieldName, uint32 index, const Point &point) 
{
   AbstractDataArray * array = GetArray(fieldName, B_POINT_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddPoint(fieldName, point);
   return array ? array->ReplaceDataItem(index, &point, sizeof(point)) : B_ERROR;
}

status_t Message :: ReplaceRect(bool okayToAdd, const String & fieldName, uint32 index, const Rect &rect) 
{
   AbstractDataArray * array = GetArray(fieldName, B_RECT_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddRect(fieldName, rect);
   return array ? array->ReplaceDataItem(index, &rect, sizeof(rect)) : B_ERROR;
}

status_t Message :: ReplaceTag(bool okayToAdd, const String & fieldName, uint32 index, const RefCountableRef & tag) 
{
   if (tag() == NULL) return B_ERROR;
   AbstractDataArray * array = GetArray(fieldName, B_TAG_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddTag(fieldName, tag);
   return array ? array->ReplaceDataItem(index, &tag, sizeof(tag)) : B_ERROR;
}

status_t Message :: ReplaceMessage(bool okayToAdd, const String & fieldName, uint32 index, const MessageRef & msgRef)
{
   if (msgRef() == NULL) return B_ERROR;
   AbstractDataArray * array = GetArray(fieldName, B_MESSAGE_TYPE);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddMessage(fieldName, msgRef);
   if (array) return array->ReplaceDataItem(index, &msgRef, sizeof(msgRef));
   return B_ERROR;
}

status_t Message :: ReplaceFlat(bool okayToAdd, const String & fieldName, uint32 index, const Flattenable &obj) 
{
   switch(obj.TypeCode())
   {
      case B_MESSAGE_TYPE: return (dynamic_cast<const Message *>(&obj)) ? ReplaceMessage(okayToAdd, fieldName, index, (const Message &)obj) : B_ERROR;
      case B_POINT_TYPE:   return (dynamic_cast<const Point *>(&obj))   ? ReplacePoint(  okayToAdd, fieldName, index, (const Point &)  obj) : B_ERROR;
      case B_RECT_TYPE:    return (dynamic_cast<const Rect *>(&obj))    ? ReplaceRect(   okayToAdd, fieldName, index, (const Rect &)   obj) : B_ERROR;
      case B_STRING_TYPE:  return (dynamic_cast<const String *>(&obj))  ? ReplaceString( okayToAdd, fieldName, index, (const String &) obj) : B_ERROR;
      default:
      {
         FlatCountableRef fcRef(GetByteBufferFromPool(obj.FlattenedSize()).GetRefCountableRef(), true);
         return ((fcRef())&&(fcRef()->CopyFrom(obj) == B_NO_ERROR)) ? ReplaceDataAux(okayToAdd, fieldName, index, &fcRef, sizeof(fcRef), obj.TypeCode()) : B_ERROR;
      }
   }
}

status_t Message :: ReplaceFlat(bool okayToAdd, const String & fieldName, uint32 index, const FlatCountableRef & ref) 
{
   const FlatCountable * fc = ref();
   if (fc)
   {
      uint32 tc = fc->TypeCode();
      AbstractDataArray * array = GetArray(fieldName, tc);
      if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddFlat(fieldName, ref);
      if (array)
      { 
         switch(tc)
         {
            case B_MESSAGE_TYPE:  
               return (dynamic_cast<const Message *>(fc)) ? ReplaceMessage(okayToAdd, fieldName, index, MessageRef(ref.GetRefCountableRef(), true)) : B_ERROR;

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

status_t Message :: ReplaceData(bool okayToAdd, const String & fieldName, uint32 type, uint32 index, const void * data, uint32 numBytes) 
{
   TCHECKPOINT;

   if (type == B_STRING_TYPE) 
   {   
      String temp((const char *)data);  // temp to avoid gcc optimizer bug
      return ReplaceString(okayToAdd, fieldName, index, temp);
   }
   AbstractDataArray * array = GetArray(fieldName, type);
   if ((okayToAdd)&&((array == NULL)||(index >= array->GetNumItems()))) return AddDataAux(fieldName, data, numBytes, type, false);
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
         ref.SetFromRefCountableRef(GetByteBufferFromPool(elementSize, (const uint8 *)dataToAdd).GetRefCountableRef());
         if (ref() == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
         dataToAdd = &ref;
         addSize = sizeof(ref);
      }
      if (array->ReplaceDataItem(i, dataToAdd, addSize) != B_NO_ERROR) return B_ERROR;
   }
   return B_NO_ERROR;
}

uint32 Message :: GetNumValuesInName(const String & fieldName, uint32 type) const
{
   const AbstractDataArray * array = GetArray(fieldName, type);
   return array ? array->GetNumItems() : 0;
}

status_t Message :: CopyName(const String & oldFieldName, Message & copyTo, const String & newFieldName) const
{
   const AbstractDataArray * array = GetArray(oldFieldName, B_ANY_TYPE);
   if (array)
   {
      RefCountableRef clone = array->Clone();
      if ((clone())&&(copyTo._entries.Put(newFieldName, clone) == B_NO_ERROR)) return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t Message :: ShareName(const String & oldFieldName, Message & shareTo, const String & newFieldName) const
{
   const RefCountableRef * gRef = _entries.Get(oldFieldName);
   return gRef ? shareTo._entries.Put(newFieldName, *gRef) : B_ERROR;
}

status_t Message :: MoveName(const String & oldFieldName, Message & moveTo, const String & newFieldName)
{
   if (ShareName(oldFieldName, moveTo) == B_NO_ERROR)
   {
      (void) _entries.Remove(newFieldName);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Message :: PrependString(const String & fieldName, const String & val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_STRING_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt8(const String & fieldName, int8 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT8_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt16(const String & fieldName, int16 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT16_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt32(const String & fieldName, int32 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT32_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependInt64(const String & fieldName, int64 val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_INT64_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependBool(const String & fieldName, bool val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_BOOL_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependFloat(const String & fieldName, float val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_FLOAT_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependDouble(const String & fieldName, double val) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_DOUBLE_TYPE);
   return array ? array->PrependDataItem(&val, sizeof(val)) : B_ERROR;
}

status_t Message :: PrependPointer(const String & fieldName, const void * ptr) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_POINTER_TYPE);
   return array ? array->PrependDataItem(&ptr, sizeof(ptr)) : B_ERROR;
}

status_t Message :: PrependPoint(const String & fieldName, const Point & point) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_POINT_TYPE);
   return array ? array->PrependDataItem(&point, sizeof(point)) : B_ERROR;
}

status_t Message :: PrependRect(const String & fieldName, const Rect & rect) 
{
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_RECT_TYPE);
   return array ? array->PrependDataItem(&rect, sizeof(rect)) : B_ERROR;
}

status_t Message :: PrependTag(const String & fieldName, const RefCountableRef & tag) 
{
   if (tag() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_TAG_TYPE);
   return array ? array->PrependDataItem(&tag, sizeof(tag)) : B_ERROR;
}

status_t Message :: PrependMessage(const String & fieldName, const MessageRef & ref)
{
   if (ref() == NULL) return B_ERROR;
   AbstractDataArray * array = GetOrCreateArray(fieldName, B_MESSAGE_TYPE);
   return (array) ? array->PrependDataItem(&ref, sizeof(ref)) : B_ERROR;
}

status_t Message :: PrependFlat(const String & fieldName, const Flattenable & obj) 
{
   switch(obj.TypeCode())
   {
      case B_STRING_TYPE:  return (dynamic_cast<const String *>(&obj))  ? PrependString(fieldName,  (const String &) obj) : B_ERROR;
      case B_MESSAGE_TYPE: return (dynamic_cast<const Message *>(&obj)) ? PrependMessage(fieldName, (const Message &)obj) : B_ERROR;
      default:             return AddFlatBuffer(fieldName, obj, obj.TypeCode(), true);
   }
}

status_t Message :: PrependFlat(const String & fieldName, const FlatCountableRef & ref)
{
   FlatCountable * fc = ref();
   if (fc)
   {
      uint32 tc = fc->TypeCode();
      switch(tc)
      {
         case B_STRING_TYPE:  return B_ERROR;  // sorry, can't do that (Strings aren't FlatCountables)
         case B_MESSAGE_TYPE: return PrependMessage(fieldName, MessageRef(ref.GetRefCountableRef(), true)); 
         default:             return AddFlatRef(fieldName, ref, tc, true);
      }
   }
   return B_ERROR;   
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
   return ((this == &rhs)||((what == rhs.what)&&(GetNumNames() == rhs.GetNumNames())&&(FieldsAreSubsetOf(rhs, true))));
}

bool Message :: FieldsAreSubsetOf(const Message & rhs, bool compareContents) const
{
   TCHECKPOINT;

   // Returns true iff every one of our fields has a like-named, liked-typed, equal-length field in (rhs).
   HashtableIterator<String, RefCountableRef> iter(_entries, HTIT_FLAG_NOREGISTER);
   const String * nextKey;
   const RefCountableRef * myNextValue;
   while(iter.GetNextKeyAndValue(nextKey, myNextValue) == B_NO_ERROR)
   {
      const RefCountableRef * hisNextValue = rhs._entries.Get(*nextKey);
      if ((hisNextValue == NULL)||(((const AbstractDataArray*)myNextValue->GetItemPointer())->IsEqualTo((const AbstractDataArray*)(hisNextValue->GetItemPointer()), compareContents) == false)) return false;
   }
   return true;
}

void Message :: SwapContents(Message & swapWith)
{
   muscleSwap(what, swapWith.what);
   _entries.SwapContents(swapWith._entries);
}

}; // end namespace muscle
