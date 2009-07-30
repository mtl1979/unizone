/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

/******************************************************************************
/
/   File:       Message.h
/
/   Description:   OS-independent version of Be's BMessage class.
/         Doesn't contain all of BMessage's functionality,
/         but keeps enough of it to be useful as a portable
/         data object (dictionary) in cross-platform apps.
/         Also adds some functionality that BMessage doesn't have,
/         such as iterators and optional direct (non-copying) access to internal
/         data (for efficiency)
/
*******************************************************************************/

#ifndef MuscleMessage_h
#define MuscleMessage_h

#include "support/Point.h"
#include "support/Rect.h"
#include "util/ByteBuffer.h"
#include "util/String.h"
#include "util/Hashtable.h"

namespace muscle {

class Message;
DECLARE_REFTYPES(Message);

/** Very trivial function, just returns a reference to an empty/default Message object.
  * Useful in cases where you need to refer to an empty Message and don't wish to be
  * constantly constructing/destroying it.
  */
const Message & GetEmptyMessage();

/** Same as GetEmptyMessage(), except it returns a MessageRef instead of a Message. */
const MessageRef & GetEmptyMessageRef();

/** This function returns a pointer to a singleton ObjectPool that can be 
 *  used to minimize the number of Message allocations and deletions by
 *  recycling the Message objects.  
 */
MessageRef::ItemPool * GetMessagePool();

/** Convenience method:  Gets a Message from the Message pool and returns a reference to it.
 *  @param what The 'what' code to set in the returned Message.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory).
 */
MessageRef GetMessageFromPool(uint32 what = 0L);

/** As above, except that the Message is obtained from the specified pool instead of from the default Message pool.
 *  @param pool the ObjectPool to allocate the Message from.
 *  @param what The 'what' code to set in the returned Message.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory).
 */
MessageRef GetMessageFromPool(ObjectPool<Message> & pool, uint32 what = 0L);

/** Convenience method:  Gets a Message from the message pool, makes it equal to (copyMe), and returns a reference to it.
 *  @param copyMe A Message to clone.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory).
 */
MessageRef GetMessageFromPool(const Message & copyMe);

/** As above, except that the Message is obtained from the specified pool instead of from the default Message pool.
 *  @param pool the ObjectPool to allocate the Message from.
 *  @param copyMe A Message to clone.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory).
 */
MessageRef GetMessageFromPool(ObjectPool<Message> & pool, const Message & copyMe);

/** Convenience method:  Gets a Message from the message pool, populates it using the flattened Message
 *  bytes at (flatBytes), and returns it.
 *  @param flatBytes The flattened Message bytes (as previously produced by Message::Flatten()) to unflatten from
 *  @param numBytes The number of bytes that (flatBytes) points to.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory or unflattening error)
 */
MessageRef GetMessageFromPool(const uint8 * flatBytes, uint32 numBytes);

/** As above, except that the Message is obtained from the specified pool instead of from the default Message pool.
 *  @param pool the ObjectPool to allocate the Message from.
 *  @param flatBytes The flattened Message bytes (as previously produced by Message::Flatten()) to unflatten from
 *  @param numBytes The number of bytes that (flatBytes) points to.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory or unflattening error)
 */
MessageRef GetMessageFromPool(ObjectPool<Message> & pool, const uint8 * flatBytes, uint32 numBytes);

/** Convenience method:  Gets a Message from the message pool, makes it a lightweight copy of (copyMe),
 *  and returns a reference to it.  See Message::MakeLightweightCopyOf() for details.
 *  @param copyMe A Message to make a lightweight copy of.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory).
 */
MessageRef GetLightweightCopyOfMessageFromPool(const Message & copyMe);

/** As above, except that the Message is obtained from the specified pool instead of from the default Message pool.
 *  @param pool the ObjectPool to allocate the Message from.
 *  @param copyMe A Message to make a lightweight copy of.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory).
 */
MessageRef GetLightweightCopyOfMessageFromPool(ObjectPool<Message> & pool, const Message & copyMe);

// this declaration is for internal use only
class AbstractDataArray;

/** This is an iterator that allows you to efficiently iterate over the field names in a Message. 
 *  It is typically allocated using the Message::GetFieldNameIterator() method.
 */
class MessageFieldNameIterator : private HashtableIterator<String, RefCountableRef>
{
public:
   /** Default constructor.   
    *  Creates an "empty" iterator;  Use the assignment operator to turn the iterator into something useful.
    */
   MessageFieldNameIterator() : _typeCode(B_ANY_TYPE) {/* empty */}

   /** This form of the constructor creates an iterator that will iterate over field names in the given Message. 
     * @param msg the Message whose field names we want to iterate over
     * @param type Type of fields you wish to iterate over, or B_ANY_TYPE to iterate over all fields.
     * @param flags Bit-chord of HTIT_FLAG_* flags you want to use to affect the iteration behaviour.
     *              This bit-chord will get passed on to the underlying HashtableIterator.  Defaults
     *              to zero, which provides the default behaviour.
     */
   MessageFieldNameIterator(const Message & msg, uint32 type = B_ANY_TYPE, uint32 flags = 0);

   /** Destructor. */
   ~MessageFieldNameIterator() {/* empty */}

   /** Advances this iterator by one entry in the table.  Equivalent to calling GetNextFieldName() once. */
   void operator++(int) {(void) GetNextFieldNameString();}

   /** Retracts this iterator by one entry in the table. */
   void operator--(int) {bool b = IsBackwards(); SetBackwards(!b); (void) GetNextFieldNameString(); SetBackwards(b);}

   /** Returns true iff there are more field names available to be traversed.  */
   bool HasMoreFieldNames() const {return (PeekNextFieldNameString() != NULL);}

   /** Returns a reference to the current field name.  Note that this will return a NULL
     * reference if there is no current field name, so be sure to call HasMoreFieldNames()
     * first and only call this method if HasMoreFieldNames() returned true!
     */
   const String & GetFieldName() const {return *PeekNextFieldNameString();}

   /**
    *  Places the next field name into (retFieldName) and bumps the iterator position.
    *  @param retFieldName On success, the next field name is written into this object.
    *  @return B_NO_ERROR iff successful, B_ERROR if there were no more names are left to get.
    */
   status_t GetNextFieldName(String & retFieldName);

   /**
    *  Returns the next field name and bumps the iterator position, or returns NULL 
    *  if there are no more field names left to get.  (Note that
    *  the returned string may become invalid when the Message is changed)
    */
   const char * GetNextFieldName() {const String * r = GetNextFieldNameString(); return r ? r->Cstr() : NULL;}

   /** Same as above, only returns a pointer to a String object instead a C-style string. 
    *  (Note that ownership of the String is retained by the Message, and that the
    *  pointer may become invalid if the Message is modified)
    */
   const String * GetNextFieldNameString();

   /** 
    *  Places the next field name into (retFieldName) without modifying the state of the traversal.
    *  @param retFieldName On success, the next field name is written into this object.
    *  @return B_NO_ERROR on success, or B_ERROR if there were no more field names left to traverse.
    */
   status_t PeekNextFieldName(String & retFieldName) const;

   /** 
    *  Returns The next field name on success, or NULL if there were no more field
    *  names left to traverse.  Does not modify the state of the traversal.  (Note that
    *  the returned string may become invalid if the Message is modified)
    */
   const char * PeekNextFieldName() const {const String * r = PeekNextFieldNameString(); return r ? r->Cstr() : NULL;}

   /** Same as above, only returns a pointer to a String object instead a C-style string. 
     *  (Note that ownership of the String object is retained by the Message, and that the
     *  pointer may become invalid if the Message is modified)
     */
   const String * PeekNextFieldNameString() const;

private:
   friend class Message;
   MessageFieldNameIterator(const HashtableIterator<String, RefCountableRef> & iter, uint32 tc) : HashtableIterator<String, RefCountableRef>(iter), _typeCode(tc) {/* empty */}

   uint32 _typeCode;
};

// Version number of the Message serialization protocol.
// Will be the first four bytes of each serialized Message buffer.
// Only buffers beginning with version numbers between these two 
// constants (inclusive) will be Unflattened as Messages.
#define OLDEST_SUPPORTED_PROTOCOL_VERSION 1347235888 // 'PM00'
#define CURRENT_PROTOCOL_VERSION          1347235888 // 'PM00'

/** 
 *  This class is similar to a BMessage, but portable.  
 *  It only acts as a serializable data container; it does not
 *  include any threading capabilities.  Unless otherwise noted, all
 *  methods behave similarly to their BMessage counterparts.  (Exception:
 *  the only error codes returned are B_ERROR and B_NO_ERROR)
 */
class Message : public FlatCountable, public Cloneable
{
public:
   /** 32 bit what code, for quick identification of message types.  Set this however you like. */
   uint32 what;

   /** Default Constructor. */
   Message() : what(0) {/* empty */}

   /** Constructor.
    *  @param what The 'what' member variable will be set to the value you specify here.
    */
   Message(uint32 what) : what(what) {/* empty */}

   /** Copy Constructor. */
   Message(const Message & copyMe) : FlatCountable(), Cloneable() {*this = copyMe;}

   /** Destructor. */
   virtual ~Message() {/* empty */}

   /** Assignment operator. */
   Message & operator=(const Message & msg);

   /** Comparison operator.  Two Message are considered equal iff their what codes are equal,
    *  and their sets of fields are equal.  Field ordering is not considered.  Note that this
    *  operation can be expensive for large Messages!
    */
   bool operator ==(const Message & rhs) const;

   /** Returns the opposite of the == operator. */
   bool operator !=(const Message & rhs) const {return !(*this == rhs);}

   /** Retrieve information about the given field in this message.
    *  @param fieldName Name of the field to look for.
    *  @param type If non-NULL, On sucess the type code of the found field is copied into this value.
    *  @param c If non-NULL, On success, the number of elements in the found field is copied into this value.
    *  @param fixed_size If non-NULL, On success, (*fixed_size) is set to reflect whether the returned field's objects are all the same size, or not.
    *  @return B_NO_ERROR on success, or B_ERROR if the requested field couldn't be found.
    */
   status_t GetInfo(const String & fieldName, uint32 *type, uint32 *c = NULL, bool *fixed_size = NULL) const;

   /** Returns the number of fields of the given type that are present in the Message.
    *  @param type The type of field to count, or B_ANY_TYPE to count all field types.
    *  @return The number of matching fields
    */
   uint32 GetNumNames(uint32 type = B_ANY_TYPE) const;

   /** Returns true if there are any fields of the given type in the Message.
    *  @param type The type of field to check for, or B_ANY_TYPE to check for any field type.
    *  @return True iff any fields of the given type exist.
    */
   bool HasNames(uint32 type = B_ANY_TYPE) const {return (GetNumNames(type) > 0);}

   /** @return true iff there are no fields in this Message. */
   bool IsEmpty() const {return (_entries.IsEmpty());}

   /** Prints debug info describing the contents of this Message to stdout. 
     * @param optFile If non-NULL, the text will be printed to this file.  If left as NULL, stdout will be used as a default.
     * @param recursive if true, we will call PrintToStream() recursively on any held messages also.   Defaults to true.
     * @param indentLevel Number of spaces to indent each printed line.  Used while recursing to format nested messages text nicely
     */
   void PrintToStream(FILE * optFile = NULL, bool recursive = true, int indentLevel = 0) const;

   /** Same as PrintToStream(), only the state of the Message is returned
    *  as a String instead of being printed to stdout.
    */
   String ToString(bool recursive = true, int indentLevel = 0) const;

   /** Same as ToString(), except the text is added to the given String instead
    *  of being returned as a new String.
    */
   void AddToString(String & s, bool recursive = true, int indentLevel = 0) const;

   /** Renames a field.
    *  @param old_entry Field name to rename from.
    *  @param new_entry Field name to rename to.  If a field with this name already exists,
    *                   it will be replaced.
    *  @return B_NO_ERROR on success, or B_ERROR if a field named (old_entry) couldn't be found. 
    */
   status_t Rename(const String & old_entry, const String & new_entry);

   /** Returns false (Messages can be of various sizes, depending on how many fields they have, etc.) */
   virtual bool IsFixedSize() const {return false;}

   /** Returns B_MESSAGE_TYPE */
   virtual uint32 TypeCode() const {return B_MESSAGE_TYPE;}

   /** Returns The number of bytes it would take to flatten this Message into a byte buffer. */
   virtual uint32 FlattenedSize() const;
 
   /**
    *  Converts this Message into a flattened buffer of bytes that can be saved to disk
    *  or sent over a network, and later converted back into an identical Message object.
    *  @param buffer The byte buffer to store the Message into.  It must be FlattenedSize() bytes long.
    */
   virtual void Flatten(uint8 *buffer) const;

   /**
    *  Convert the given byte buffer back into a Message.  Any previous contents of
    *  this Message will be erased, and replaced with the data specified in the byte buffer.
    *  @param buf Pointer to a byte buffer containing a flattened Message to restore.
    *  @param size The number of bytes in the flattened byte buffer.
    *  @return B_NO_ERROR if the buffer was successfully Unflattened, or B_ERROR if there
    *          was an error (usually meaning the buffer was corrupt, or out-of-memory)
    */
   virtual status_t Unflatten(const uint8 *buf, uint32 size);

   /** Adds a new string to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The string to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddString(const String & fieldName, const String & val);

   /** Adds a new int8 to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The int8 to add 
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddInt8(const String & fieldName, int8 val);

   /** Adds a new int16 to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The int16 to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddInt16(const String & fieldName, int16 val);

   /** Adds a new int32 to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The int32 to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddInt32(const String & fieldName, int32 val);

   /** Adds a new int64 to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The int64 to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddInt64(const String & fieldName, int64 val);

   /** Adds a new boolean to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The boolean to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddBool(const String & fieldName, bool val);

   /** Adds a new float to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The float to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddFloat(const String & fieldName, float val);

   /** Adds a new double to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param val The double to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddDouble(const String & fieldName, double val);

   /** Adds a new Message to the Message.
    *  Note that this method is less efficient than the
    *  AddMessage(MessageRef) method, as this method
    *  necessitates copying all the data in (msg).
    *  @param fieldName Name of the field to add (or add to)
    *  @param msg The Message to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddMessage(const String & fieldName, const Message & msg) {return AddMessage(fieldName, GetMessageFromPool(msg));}

   /** Adds a new Message to the Message.
    *  Note that this method is more efficient than the
    *  AddMessage(const Message &) method, as this method
    *  need not make a copy of the referenced Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param msgRef Reference to the Message to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddMessage(const String & fieldName, const MessageRef & msgRef); 

   /** Calls SaveToArchive() on the specified object to save its
    *  state into a Message, then adds the resulting Message into this Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param obj The object to archive.  May be any type with a SaveToArchive() method.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or SaveToArchive() failed.
    */
   template<typename T> inline status_t AddArchiveMessage(const String & fieldName, const T & obj)
   {
      return AddMessage(fieldName, GetArchiveMessageFromPool(obj));
   }

   /** Adds a new pointer value to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param ptr The pointer value to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddPointer(const String & fieldName, const void * ptr);

   /** Adds a new point to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param point The point to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddPoint(const String & fieldName, const Point & point);

   /** Adds a new rect to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param rect The rect to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddRect(const String & fieldName, const Rect & rect);

   /** Flattens a Flattenable object into the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param obj The Flattenable object.  Flatten() is called on this object, and the
    *             resulting bytes are appended to the given field in this Message.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddFlat(const String & fieldName, const Flattenable & obj);

   /** Adds a reference to a FlatCountable object to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param ref The FlatCountable reference to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddFlat(const String & fieldName, const FlatCountableRef & ref);

   /** Adds a reference to a ByteBuffer object to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param ref The ByteBuffer reference to add
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddFlat(const String & fieldName, const ByteBufferRef & ref)
   {
      FlatCountableRef fcRef;
      fcRef.SetFromRefCountableRefUnchecked(ref.GetRefCountableRef());
      return AddFlat(fieldName, fcRef);
   }

   /** Adds a new ephemeral-tag-item to this Message.  Tags are references to arbitrary 
    *  ref-countable C++ objects;  They can be added to a Message as a matter of convenience 
    *  to the local program, but note that they are not persistent--they won't get 
    *  Flatten()'d with the rest of the contents of the Message!
    *  @param fieldName Name of the field to add (or add to)
    *  @param tagRef Reference to the tag object to add.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t AddTag(const String & fieldName, const RefCountableRef & tagRef);

   /** Generic method, capable of adding any type of data to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param type The B_*_TYPE code indicating the type of data that will be added.
    *  @param data A pointer to the bytes of data to add.  The data is copied out of
    *              this byte buffer, and exactly how it is interpreted depends on (type).
    *              (data) may be NULL, in which case default objects (or uninitialized byte space) is added.
    *  Note:  If you are adding B_MESSAGE_TYPE, then (data) must point to a MessageRef
    *         object, and NOT a Message object, or flattened-Message-buffer!
    *         (This is inconsistent with BMessage::AddData, which expects a flattened BMessage buffer.  Sorry!)
    *  @param numBytes The number of bytes that are pointed to by (data).  If (type)
    *                  is a known type such as B_INT32_TYPE or B_RECT_TYPE, (numBytes)
    *                  may be a multiple of the datatype's size, allowing you to add
    *                  multiple entries with a single call.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or (numBytes) isn't a multiple (type)'s 
    *          known size, or a type conflict occurred.
    */
   status_t AddData(const String & fieldName, uint32 type, const void *data, uint32 numBytes) {return AddDataAux(fieldName, data, numBytes, type, false);}

   /** Prepends a new string to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The string to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependString(const String & fieldName, const String & val);

   /** Prepends a new int8 to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The int8 to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependInt8(const String & fieldName, int8 val);

   /** Prepends a new int16 to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The int16 to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependInt16(const String & fieldName, int16 val);

   /** Prepends a new int32 to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The int32 to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependInt32(const String & fieldName, int32 val);

   /** Prepends a new int64 to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The int64 to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependInt64(const String & fieldName, int64 val);

   /** Prepends a new boolean to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The boolean to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependBool(const String & fieldName, bool val);

   /** Prepends a new float to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The float to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependFloat(const String & fieldName, float val);

   /** Prepends a new double to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param val The double to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependDouble(const String & fieldName, double val);

   /** Prepends a new Message to the beginning of a field array in the Message.
    *  Note that this method is less efficient than the AddMessage(MessageRef) method, 
    *  as this method necessitates making a copy of (msg) and all the data it contains.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param msg The Message to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependMessage(const String & fieldName, const Message & msg) {return PrependMessage(fieldName, GetMessageFromPool(msg));}

   /** Prepends a new Message to the beginning of a field array in the Message.
    *  Note that this method is more efficient than the
    *  PrependMessage(const Message &) method, as this method
    *  need not make a copy of the referenced Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param msgRef Reference to the Message to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependMessage(const String & fieldName, const MessageRef & msgRef); 

   /** Calls SaveToArchive() on the specified object to save its
    *  state into a Message, then prepends the resulting Message into this Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param obj The object to archive.  May be any type with a SaveToArchive() method.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or SaveToArchive() failed.
    */
   template<typename T> inline status_t PrependArchiveMessage(const String & fieldName, const T & obj)
   {
      return PrependMessage(fieldName, GetArchiveMessageFromPool(obj));
   }

   /** Prepends a new pointer value to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param ptr The pointer value to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependPointer(const String & fieldName, const void * ptr);

   /** Prepends a new point to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param point The point to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependPoint(const String & fieldName, const Point & point);

   /** Prepends a new rectangle to the beginning of a field array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param rect The rectangle to add or prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependRect(const String & fieldName, const Rect & rect);

   /** Flattens the given object and prepends the resulting bytes into beginning of a field 
    *  array in the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param obj The Flattenable object.  Flatten() is called on this object, and the resulting
    *             bytes are prepended to the beginning of the given field in this Message.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependFlat(const String & fieldName, const Flattenable & obj);

   /** Prepends a reference to a FlatCountable object to the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param flatRef FlatCountable reference to prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependFlat(const String & fieldName, const FlatCountableRef & flatRef);

   /** Prepends a reference to a ByteBuffer object to the Message.
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param ref The ByteBuffer reference to prepend
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependFlat(const String & fieldName, const ByteBufferRef & ref)
   {
      FlatCountableRef fcRef;
      fcRef.SetFromRefCountableRefUnchecked(ref.GetRefCountableRef());
      return PrependFlat(fieldName, fcRef);
   }

   /** Prepends a new ephemeral-tag-item to this Message.  Tags are references to arbitrary 
    *  ref-countable C++ objects;  They can be added to a Message as a matter of convenience 
    *  to the local program, but note that they are not persistent--they won't get 
    *  Flatten()'d with the rest of the contents of the Message!
    *  @param fieldName Name of the field to add (or prepend to)
    *  @param tagRef Reference to the tag object to add or prepend.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t PrependTag(const String & fieldName, const RefCountableRef & tagRef);

   /** Generic method, capable of prepending any type of data to the Message.
    *  @param fieldName Name of the field to add (or add to)
    *  @param type The B_*_TYPE code indicating the type of data that will be added.
    *  @param data A pointer to the bytes of data to add.  The data is copied out of
    *              this byte buffer, and exactly how it is interpreted depends on (type).
    *              (data) may be NULL, in which case default objects (or uninitialized byte space) is added.
    *  Note:  If you are adding B_MESSAGE_TYPE, then (data) must point to a MessageRef
    *         object, and NOT a Message object, or flattened-Message-buffer!
    *         (This is inconsistent with BMessage::AddData, which expects a flattened BMessage buffer.  Sorry!)
    *  @param numBytes The number of bytes that are pointed to by (data).  If (type)
    *                  is a known type such as B_INT32_TYPE or B_RECT_TYPE, (numBytes)
    *                  may be a multiple of the datatype's size, allowing you to add
    *                  multiple entries with a single call.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or (numBytes) isn't a multiple (type)'s 
    *          known size, or a type conflict occurred.
    */
   status_t PrependData(const String & fieldName, uint32 type, const void *data, uint32 numBytes) {return AddDataAux(fieldName, data, numBytes, type, true);}

   /** Removes the (index)'th item from the given field entry.  If the field entry becomes
    *  empty, the field itself is removed also.
    *  @param fieldName Name of the field to remove an item from.
    *  @param index Index of the item to remove.
    *  @return B_NO_ERROR on success, B_ERROR if the field name wasn't found, or the specified index was invalid.
    */
   status_t RemoveData(const String & fieldName, uint32 index = 0);

   /** Removes the given field name and its contents from the Message.
    *  @param fieldName Name of the field to remove.
    *  @return B_NO_ERROR on success, B_ERROR if the field name wasn't found.
    */
   status_t RemoveName(const String & fieldName);

   /** Clears all fields from the Message. 
    *  @param releaseCachedBuffers If set true, any cached buffers we are holding will be immediately freed.
    *                              Otherwise, they will be kept around for future re-use.
    */
   void Clear(bool releaseCachedBuffers = false) {_entries.Clear(releaseCachedBuffers);}

   /** Retrieve a string value from the Message.
    *  @param fieldName The field name to look for the string value under.
    *  @param index The index of the string item in its field entry.
    *  @param setMe On success, this pointer will be pointing to a String containing the result.
    *  @return B_NO_ERROR if the string value was found, or B_ERROR if it wasn't.
    */
   status_t FindString(const String & fieldName, uint32 index, const String ** setMe) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindString(const String & fieldName, const String ** setMe) const {return FindString(fieldName, 0, setMe);}

   /** Retrieve a string value from the Message.
    *  @param fieldName The field name to look for the string value under.
    *  @param index The index of the string item in its field entry.
    *  @param setMe On success, a pointer to the string value is written into this object.
    *  @return B_NO_ERROR if the string value was found, or B_ERROR if it wasn't.
    */
   status_t FindString(const String & fieldName, uint32 index, const char * & setMe) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindString(const String & fieldName, const char * & setMe) const {return FindString(fieldName, 0, setMe);}

   /** Retrieve a string value from the Message.
    *  @param fieldName The field name to look for the string value under.
    *  @param index The index of the string item in its field entry.
    *  @param setMe On success, the value of the string is written into this object.
    *  @return B_NO_ERROR if the string value was found, or B_ERROR if it wasn't.
    */
   status_t FindString(const String & fieldName, uint32 index, String & setMe) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindString(const String & fieldName, String & setMe) const {return FindString(fieldName, 0, setMe);}

   /** Retrieve an int8 value from the Message.
    *  @param fieldName The field name to look for the int8 value under.
    *  @param index The index of the int8 item in its field entry.
    *  @param val On success, the value of the int8 is written into this object.
    *  @return B_NO_ERROR if the int8 value was found, or B_ERROR if it wasn't.
    */
   status_t FindInt8(const String & fieldName, uint32 index, int8 & val) const {return FindDataItemAux(fieldName, index, B_INT8_TYPE, &val, sizeof(int8));}

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindInt8(const String & fieldName, int8 & value) const {return FindInt8(fieldName, 0, value);}

   /** Retrieve an int16 value from the Message.
    *  @param fieldName The field name to look for the int16 value under.
    *  @param index The index of the int16 item in its field entry.
    *  @param val On success, the value of the int16 is written into this object.
    *  @return B_NO_ERROR if the int16 value was found, or B_ERROR if it wasn't.
    */
   status_t FindInt16(const String & fieldName, uint32 index, int16 & val) const {return FindDataItemAux(fieldName, index, B_INT16_TYPE, &val, sizeof(int16));} 

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindInt16(const String & fieldName, int16 & value) const {return FindInt16(fieldName, 0, value);}

   /** Retrieve an int32 value from the Message.
    *  @param fieldName The field name to look for the int32 value under.
    *  @param index The index of the int32 item in its field entry.
    *  @param val On success, the value of the int32 is written into this object.
    *  @return B_NO_ERROR if the int32 value was found, or B_ERROR if it wasn't.
    */
   status_t FindInt32(const String & fieldName, uint32 index, int32 & val) const {return FindDataItemAux(fieldName, index, B_INT32_TYPE, &val, sizeof(int32));} 

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindInt32(const String & fieldName, int32 & value) const {return FindInt32(fieldName, 0, value);}

   /** Retrieve an int64 value from the Message.
    *  @param fieldName The field name to look for the int64 value under.
    *  @param index The index of the int64 item in its field entry.
    *  @param val On success, the value of the int64 is written into this object.
    *  @return B_NO_ERROR if the int64 value was found, or B_ERROR if it wasn't.
    */
   status_t FindInt64(const String & fieldName, uint32 index, int64 & val) const {return FindDataItemAux(fieldName, index, B_INT64_TYPE, &val, sizeof(int64));}  

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindInt64(const String & fieldName, int64 & value) const {return FindInt64(fieldName, 0, value);}

   /** Retrieve a boolean value from the Message.
    *  @param fieldName The field name to look for the boolean value under.
    *  @param index The index of the boolean item in its field entry.
    *  @param val On success, the value of the boolean is written into this object.
    *  @return B_NO_ERROR if the boolean value was found, or B_ERROR if it wasn't.
    */
   status_t FindBool(const String & fieldName, uint32 index, bool & val) const {return FindDataItemAux(fieldName, index, B_BOOL_TYPE, &val, sizeof(bool));} 

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindBool(const String & fieldName, bool & value) const {return FindBool(fieldName, 0, value);}

   /** Retrieve a float value from the Message.
    *  @param fieldName The field name to look for the float value under.
    *  @param index The index of the float item in its field entry.
    *  @param val On success, the value of the float is written into this object.
    *  @return B_NO_ERROR if the float value was found, or B_ERROR if it wasn't.
    */
   status_t FindFloat(const String & fieldName, uint32 index, float & val) const {return FindDataItemAux(fieldName, index, B_FLOAT_TYPE, &val, sizeof(float));}

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindFloat(const String & fieldName, float & f) const {return FindFloat(fieldName, 0, f);}

   /** Retrieve a double value from the Message.
    *  @param fieldName The field name to look for the double value under.
    *  @param index The index of the double item in its field entry.
    *  @param val On success, the value of the double is written into this object.
    *  @return B_NO_ERROR if the double value was found, or B_ERROR if it wasn't.
    */
   status_t FindDouble(const String & fieldName, uint32 index, double & val) const {return FindDataItemAux(fieldName, index, B_DOUBLE_TYPE, &val, sizeof(double));}

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindDouble(const String & fieldName, double & d) const {return FindDouble(fieldName, 0, d);}

   /** Retrieve a Message value from the Message.
    *  Note that this method is less efficient than the FindMessage(MessageRef)
    *  method, because it forces the held Message's data to be copied into (msg)
    *  @param fieldName The field name to look for the Message value under.
    *  @param index The index of the Message item in its field entry.
    *  @param msg On success, the value of the Message is written into this object.
    *  @return B_NO_ERROR if the Message value was found, or B_ERROR if it wasn't.
    */
   status_t FindMessage(const String & fieldName, uint32 index, Message & msg) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindMessage(const String & fieldName, Message & msg) const {return FindMessage(fieldName, 0, msg);}

   /** Retrieve a Message value from the Message.
    *  Note that this method is more efficient than the FindMessage(MessageRef)
    *  method, because it the retrieved Message need not be copied.
    *  @param fieldName The field name to look for the Message value under.
    *  @param index The index of the Message item in its field entry.
    *  @param msgRef On success, the value of the Message is written into this object.
    *  @return B_NO_ERROR if the Message value was found, or B_ERROR if it wasn't.
    */
   status_t FindMessage(const String & fieldName, uint32 index, MessageRef & msgRef) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindMessage(const String & fieldName, MessageRef & msgRef) const {return FindMessage(fieldName, 0, msgRef);}

   /** Convenience method:  Retrieves a Message value from this Message,
    *  and if successful, calls SetFromArchive(msg) on the passed-in object.
    *  @param fieldName The field name to look for the Message value under.
    *  @param index The index of the Message item in its field entry.
    *  @param obj The object to call SetFromArchive(msg) on.  Since this method is templated,
    *             this object may be of any type that has a SetFromArchive(const Message &) method.
    *  @return B_NO_ERROR if the Message value was found and SetFromArchive() returned B_NO_ERROR, or B_ERROR otherwise.
    */
   template<typename T> inline status_t FindArchiveMessage(const String & fieldName, uint32 index, T & obj) const
   {
      MessageRef msg;
      return (FindMessage(fieldName, index, msg) == B_NO_ERROR) ? obj.SetFromArchive(*msg()) : B_ERROR;
   }

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   template<typename T> inline status_t FindArchiveMessage(const String & fieldName, T & obj) const {return FindArchiveMessage(fieldName, 0, obj);}

   /** Convenience method:  Retrieves a Message value from this Message,
    *  and if successful, calls SetFromArchive(foundMsg) on the passed-in object.  
    *  If the specified sub-Message is not found, this method calls SetFromArchive(defaultMsg) on the object instead.
    *  @param fieldName The field name to look for the Message value under.
    *  @param index The index of the Message item in its field entry.
    *  @param obj The object to call SetFromArchive(msg) on.  Since this method is templated,
    *             this object may be of any type that has a SetFromArchive(const Message &) method.
    *  @param defaultMsg The Message to pass to SetFromArchive() if no sub-Message is found.  Defaults to GetEmptyMessage().
    *  @return The value returned by obj.SetFromArchive().
    */
   template<typename T> inline status_t FindArchiveMessageWithDefault(const String & fieldName, uint32 index, T & obj, const Message & defaultMsg = GetEmptyMessage()) const
   {
      MessageRef msg; 
      return (FindMessage(fieldName, index, msg) == B_NO_ERROR) ? obj.SetFromArchive(*msg()) : obj.SetFromArchive(defaultMsg);
   }

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   template<typename T> inline status_t FindArchiveMessageWithDefault(const String & fieldName, T & obj, const Message & defaultMsg = GetEmptyMessage()) const {return FindArchiveMessageWithDefault(fieldName, 0, obj, defaultMsg);}

   /** Retrieve a pointer value from the Message.
    *  @param fieldName The field name to look for the pointer value under.
    *  @param index The index of the pointer item in its field entry.
    *  @param val On success, the value of the pointer is written into this object.
    *  @return B_NO_ERROR if the pointer value was found, or B_ERROR if it wasn't.
    */
   status_t FindPointer(const String & fieldName, uint32 index, void * & val) const {return FindDataItemAux(fieldName, index, B_POINTER_TYPE, &val, sizeof(void *));}

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindPointer(const String & fieldName, void * & ptr) const {return FindPointer(fieldName, 0, ptr);}

   /** Retrieve a point value from the Message.
    *  @param fieldName The field name to look for the point value under.
    *  @param index The index of the point item in its field entry.
    *  @param point On success, the value of the point is written into this object.
    *  @return B_NO_ERROR if the point value was found, or B_ERROR if it wasn't.
    */
   status_t FindPoint(const String & fieldName, uint32 index, Point & point) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindPoint(const String & fieldName, Point & point) const {return FindPoint(fieldName, 0, point);}

   /** Retrieve a rectangle value from the Message.
    *  @param fieldName The field name to look for the rectangle value under.
    *  @param index The index of the rectangle item in its field entry.
    *  @param rect On success, the value of the rectangle is written into this object.
    *  @return B_NO_ERROR if the rectangle value was found, or B_ERROR if it wasn't.
    */
   status_t FindRect(const String & fieldName, uint32 index, Rect & rect) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindRect(const String & fieldName, Rect & rect) const {return FindRect(fieldName, 0, rect);}

   /** Retrieve a flattened object from the Message.
    *  @param fieldName The field name to look for the flattened object value under.
    *  @param index The index of the flattened object item in its field entry.
    *  @param obj On success, the flattened object is copied into this object.
    *  @return B_NO_ERROR if the flattened object was found, or B_ERROR if it wasn't.
    */
   status_t FindFlat(const String & fieldName, uint32 index, Flattenable & obj) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindFlat(const String & fieldName, Flattenable & obj) const {return FindFlat(fieldName, 0, obj);}

   /** Retrieve a FlatCountable reference from the Message.
    *  @param fieldName The field name to look for the FlatCountable reference under.
    *  @param index The index of the flattened object item in its field entry.
    *  @param ref On success, this reference will refer to the FlatCountable object.
    *  @return B_NO_ERROR if the flattened object was found, or B_ERROR if it wasn't.
    */
   status_t FindFlat(const String & fieldName, uint32 index, FlatCountableRef & ref) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindFlat(const String & fieldName, FlatCountableRef & ref) const {return FindFlat(fieldName, 0, ref);}

   /** Convenience method:  As above, only the result is placed into the given ByteBufferRef object.
     * This saves you having to do the necessary FlatCountableRef->ByteBufferRef casting yourself.
     */
   status_t FindFlat(const String & fieldName, uint32 index, ByteBufferRef & ref) const
   {
      FlatCountableRef fcRef;
      return (FindFlat(fieldName, index, fcRef) == B_NO_ERROR) ? ref.SetFromRefCountableRef(fcRef.GetRefCountableRef()) : B_ERROR;
   }

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindFlat(const String & fieldName, ByteBufferRef & ref) const {return FindFlat(fieldName, 0, ref);}

   /** Retrieve an ephemeral-tag-item from the Message.
    *  @param fieldName Name of the field to look for the tag under.
    *  @param index The index of the tag item in its field entry.
    *  @param tagRef On success, this object becomes a reference to the found tag object.
    *  @return B_NO_ERROR on success, B_ERROR if out of memory or a type conflict occurred
    */
   status_t FindTag(const String & fieldName, uint32 index, RefCountableRef & tagRef) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindTag(const String & fieldName, RefCountableRef & tagRef) const {return FindTag(fieldName, 0, tagRef);}

   /** Retrieve a pointer to the raw data bytes of a stored message field of any type.
    *  @param fieldName The field name to retrieve the pointer to
    *  @param type The type code of the field you are interested, or B_ANY_TYPE if any type is acceptable.
    *  @param index Index of the data item to look for (within the field)
    *  @param data On success, a pointer to the data bytes will be written into this object.
    *  Note:  If you are retrieving B_MESSAGE_TYPE, then (data) will be set to point to a MessageRef
    *         object, and NOT a Message object, or flattened-Message-buffer!
    *         (This is inconsistent with BMessage::FindData, which returns a flattened BMessage buffer.  Sorry!)
    *  @param numBytes On success, the number of bytes in the returned data array will be written into this object.  (May be NULL)
    *  @return B_NO_ERROR if the data pointer was retrieved successfully, or B_ERROR if it wasn't.
    */
   status_t FindData(const String & fieldName, uint32 type, uint32 index, const void **data, uint32 *numBytes) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindData(const String & fieldName, uint32 type, const void **data, uint32 *numBytes) const {return FindData(fieldName, type, 0, data, numBytes);}

   /** As above, only this returns a pointer to an array that you can write directly into, for efficiency.  
    *  You should be very careful with this method, though, as you will be writing into the guts of this Message.
    *  The returned pointer is not guaranteed to be valid after the Message is modified by any method!
    *  @see FindData(...)
    */
   status_t FindDataPointer(const String & fieldName, uint32 type, uint32 index, void **data, uint32 *numBytes) const;

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t FindDataPointer(const String & fieldName, uint32 type, void **data, uint32 *numBytes) const {return FindDataPointer(fieldName, type, 0, data, numBytes);}

   /** Replace a string value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param newString The new string value to put overwrite the old string with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceString(bool okayToAdd, const String & fieldName, uint32 index, const String & newString);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceString(bool okayToAdd, const String & fieldName, const String & newString) {return ReplaceString(okayToAdd, fieldName, 0, newString);}

   /** Replace an int8 value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param val The new int8 value to put overwrite the old int8 with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceInt8(bool okayToAdd, const String & fieldName, uint32 index, int8 val);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceInt8(bool okayToAdd, const String & fieldName, int8 val) {return ReplaceInt8(okayToAdd, fieldName, 0, val);}

   /** Replace an int16 value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param val The new int16 value to put overwrite the old int16 with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceInt16(bool okayToAdd, const String & fieldName, uint32 index, int16 val);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceInt16(bool okayToAdd, const String & fieldName, int16 val) {return ReplaceInt16(okayToAdd, fieldName, 0, val);}

   /** Replace an int32 value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param val The new int32 value to put overwrite the old int32 with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceInt32(bool okayToAdd, const String & fieldName, uint32 index, int32 val);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceInt32(bool okayToAdd, const String & fieldName, int32 val) {return ReplaceInt32(okayToAdd, fieldName, 0, val);}

   /** Replace an int64 value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param val The new int64 value to put overwrite the old int8 with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceInt64(bool okayToAdd, const String & fieldName, uint32 index, int64 val);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceInt64(bool okayToAdd, const String & fieldName, int64 val) {return ReplaceInt64(okayToAdd, fieldName, 0, val);}

   /** Replace a boolean value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param val The new boolean value to put overwrite the old boolean with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceBool(bool okayToAdd, const String & fieldName, uint32 index, bool val);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceBool(bool okayToAdd, const String & fieldName, bool val) {return ReplaceBool(okayToAdd, fieldName, 0, val);}

   /** Replace a float value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param val The new float value to put overwrite the old float with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceFloat(bool okayToAdd, const String & fieldName, uint32 index, float val);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceFloat(bool okayToAdd, const String & fieldName, float val) {return ReplaceFloat(okayToAdd, fieldName, 0, val);}

   /** Replace a double value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param val The new double value to put overwrite the old double with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceDouble(bool okayToAdd, const String & fieldName, uint32 index, double val);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceDouble(bool okayToAdd, const String & fieldName, double val) {return ReplaceDouble(okayToAdd, fieldName, 0, val);}

   /** Replace a pointer value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param ptr The new pointer value to put overwrite the old pointer with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplacePointer(bool okayToAdd, const String & fieldName, uint32 index, const void * ptr);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplacePointer(bool okayToAdd, const String & fieldName, const void * ptr) {return ReplacePointer(okayToAdd, fieldName, 0, ptr);}

   /** Replace a point value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param point The new point value to put overwrite the old point with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplacePoint(bool okayToAdd, const String & fieldName, uint32 index, const Point & point);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplacePoint(bool okayToAdd, const String & fieldName, const Point & point) {return ReplacePoint(okayToAdd, fieldName, 0, point);}

   /** Replace a rectangle value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param rect The new rectangle value to put overwrite the old rectangle with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceRect(bool okayToAdd, const String & fieldName, uint32 index, const Rect & rect);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceRect(bool okayToAdd, const String & fieldName, const Rect & rect) {return ReplaceRect(okayToAdd, fieldName, 0, rect);}

   /** Replace a Message value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param msg The new Message value to put overwrite the old Message with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceMessage(bool okayToAdd, const String & fieldName, uint32 index, const Message & msg) {return ReplaceMessage(okayToAdd, fieldName, index, GetMessageFromPool(msg));}

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceMessage(bool okayToAdd, const String & fieldName, const Message & msg) {return ReplaceMessage(okayToAdd, fieldName, 0, msg);}

   /** Replace a Message value in an existing Message field with a new value.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param msgRef The new Message value to put overwrite the old Message with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceMessage(bool okayToAdd, const String & fieldName, uint32 index, const MessageRef & msgRef);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceMessage(bool okayToAdd, const String & fieldName, const MessageRef & msgRef) {return ReplaceMessage(okayToAdd, fieldName, 0, msgRef);}

   /** Replace a Message value in an existing Message field with a new value 
    *  that was generated by calling SaveToArchive() on the passed in object.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param obj The object to call SaveToArchive() on.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, if SaveToArchive() failed, or if (index) wasn't a valid index, or out of memory.
    */
   template<typename T> inline status_t ReplaceArchiveMessage(bool okayToAdd, const String & fieldName, uint32 index, const T & obj)
   {
      return ReplaceMessage(okayToAdd, fieldName, index, GetArchiveMessageFromPool(obj));
   }

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   template<typename T> inline status_t ReplaceArchiveMessage(bool okayToAdd, const String & fieldName, const T & obj) {return ReplaceArchiveMessage(okayToAdd, fieldName, 0, obj);}

   /** Replace a flattened object in an existing Message field with a new object.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param obj The new flattened object value to put overwrite the old flattened object with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceFlat(bool okayToAdd, const String & fieldName, uint32 index, const Flattenable & obj);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceFlat(bool okayToAdd, const String & fieldName, const Flattenable & obj) {return ReplaceFlat(okayToAdd, fieldName, 0, obj);}

   /** Replace a FlatCountable reference in an existing Message field with a new reference.
    *  @param okayToAdd If set true, attempting to replace an reference that doesn't exist will cause the new reference to be added to the end of the field array, instead.  If false, attempting to replace a non-existant reference will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param ref The new FlatCountableRef put overwrite the old reference with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceFlat(bool okayToAdd, const String & fieldName, uint32 index, const FlatCountableRef & ref);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceFlat(bool okayToAdd, const String & fieldName, FlatCountableRef & ref) {return ReplaceFlat(okayToAdd, fieldName, 0, ref);}

   /** As above, only (ref) is specified as a ByteBufferRef, to save you having to do the necessary casting to FlatCountableRef yourself */
   status_t ReplaceFlat(bool okayToAdd, const String & fieldName, uint32 index, const ByteBufferRef & ref) 
   {
      FlatCountableRef fcRef;
      fcRef.SetFromRefCountableRefUnchecked(ref.GetRefCountableRef());
      return ReplaceFlat(okayToAdd, fieldName, index, fcRef);
   }

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceFlat(bool okayToAdd, const String & fieldName, ByteBufferRef & ref) {return ReplaceFlat(okayToAdd, fieldName, 0, ref);}

   /** Replace a tag object in an existing Message field with a new tag object.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name of an existing field to modify
    *  @param index The index of the entry within the field name to modify
    *  @param tag The new tag reference to overwrite the old tag reference with.
    *  @return B_NO_ERROR on success, or B_ERROR if the field wasn't found, or if (index) wasn't a valid index, or out of memory.
    */
   status_t ReplaceTag(bool okayToAdd, const String & fieldName, uint32 index, const RefCountableRef & tag);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceTag(bool okayToAdd, const String & fieldName, const RefCountableRef & tag) {return ReplaceTag(okayToAdd, fieldName, 0, tag);}

   /** Replace one entry in a field of any type.
    *  @param okayToAdd If set true, attempting to replace an item that doesn't exist will cause the new item to be added to the end of the field array, instead.  If false, attempting to replace a non-existant item will cause B_ERROR to be returned with no side effects.
    *  @param fieldName The field name to replace an entry in.
    *  @param type The uint32 of the field you are interested, or B_ANY_TYPE if any type is acceptable.
    *  @param index Index in the field to replace the data at.
    *  @param data The bytes representing the item you wish to replace an old item in the field with
    *  @param numBytes The number of bytes in your data array.
    *  Note:  If you are replacing B_MESSAGE_TYPE, then (data) must point to a MessageRef
    *         object, and NOT a Message object, or flattened-Message-buffer!
    *         (This is inconsistent with BMessage::ReplaceData, which expects a flattened BMessage buffer.  Sorry!)
    *  @return B_NO_ERROR if the data item(s) were replaced successfully, or B_ERROR they weren't.
    */
   status_t ReplaceData(bool okayToAdd, const String & fieldName, uint32 type, uint32 index, const void *data, uint32 numBytes);

   /** As above, only (index) isn't specified.  It is assumed to be zero. */
   status_t ReplaceData(bool okayToAdd, const String & fieldName, uint32 type, const void *data, uint32 numBytes) {return ReplaceData(okayToAdd, fieldName, type, 0, data, numBytes);}

   /** Convenience method:  Returns the first field name of the given type, or NULL if none is found.
     * @param optTypeCode If specified, only field names with the specified type will be considered.
     *                    If left as the default (B_ANY_TYPE), then the first field name of any type will be returned. 
     * @returns a pointer to the returned field's name String on success, or NULL on failure.  Note that this pointer
     *          is not guaranteed to remain valid if the Message is modified.
     */
   const String * GetFirstFieldNameString(uint32 optTypeCode = B_ANY_TYPE) const {return GetExtremeFieldNameStringAux(optTypeCode, false);}

   /** Convenience method:  Returns the last field name of the given type, or NULL if none is found.
     * @param optTypeCode If specified, only field names with the specified type will be considered.
     *                    If left as the default (B_ANY_TYPE), then the last field name of any type will be returned. 
     * @returns a pointer to the returned field's name String on success, or NULL on failure.  Note that this pointer
     *          is not guaranteed to remain valid if the Message is modified.
     */
   const String * GetLastFieldNameString(uint32 optTypeCode = B_ANY_TYPE) const {return GetExtremeFieldNameStringAux(optTypeCode, true);}

   /** Returns true iff there is a field with the given name and type present in the Message.
    *  @param fieldName the field name to look for.
    *  @param type the type to look for, or B_ANY_TYPE if type isn't important to you.
    *  @return true if such a field exists, else false. 
    */
   bool HasName(const String & fieldName, uint32 type = B_ANY_TYPE) const {return (GetArray(fieldName, type) != NULL);}

   /** Returns the number of values in the field with the given name and type in the Message.
    *  @param fieldName the field name to look for.
    *  @param type the type to look for, or B_ANY_TYPE if type isn't important to you.
    *  @return The number of values stored under (fieldName) if a field of the right type exists, 
    *          or zero if the field doesn't exist or isn't of a matching type.
    */
   uint32 GetNumValuesInName(const String & fieldName, uint32 type = B_ANY_TYPE) const;

   /** Takes the field named (fieldName) in this message, and moves the field into (moveTo). 
    *  Any data that was already in (moveTo) under (fieldName) will be replaced.
    *  @param fieldName Name of an existing field (in this Message) to be moved.
    *  @param moveTo A Message to move the specified field into.
    *  @result B_NO_ERROR on success, or B_ERROR if there is an error moving the field.
    */
   status_t MoveName(const String & fieldName, Message & moveTo) {return MoveName(fieldName, moveTo, fieldName);}

   /** Take the data under (oldFieldName) in this message, and moves it into (moveTo), 
    *  where it will appear under the field name (newFieldName).
    *  Any data that was already in (moveTo) under (newFieldName) will be replaced.
    *  @param oldFieldName Name of an existing field (in this Message) to be moved.
    *  @param moveTo A Message to move the field into.
    *  @param newFieldName The name the field should have in the new Message.
    *  @result B_NO_ERROR on success, or B_ERROR if there is an error moving the field.
    */
   status_t MoveName(const String & oldFieldName, Message & moveTo, const String & newFieldName);

   /** Take the data under (fieldName) in this message, and copies it into (copyTo).
    *  Any data that was already in (copyTo) under (fieldName) will be replaced.
    *  @param fieldName Name of an existing field (in this Message) to be copied.
    *  @param copyTo A Message to copy the field into.
    *  @result B_NO_ERROR on success, or B_ERROR if there is an error copying the field.
    */
   status_t CopyName(const String & fieldName, Message & copyTo) const {return CopyName(fieldName, copyTo, fieldName);}

   /** Take the data under (oldFieldName) in this message, and copies it into (copyTo),
    *  where it will appear under the field name (newFieldName).
    *  Any data that was already in (copyTo) under (newFieldName) will be replaced.
    *  @param oldFieldName Name of an existing field (in this Message) to be copied.
    *  @param copyTo A Message to copy the field into.
    *  @param newFieldName The name the field should have in the new Message.
    *  @result B_NO_ERROR on success, or B_ERROR if there is an error copying the field.
    */
   status_t CopyName(const String & oldFieldName, Message & copyTo, const String & newFieldName) const;

   /** Take the data under (fieldName) in this message, and shares it into (shareTo). 
    *  Any data that was under (fieldName) in (shareTo) will be replaced.
    *  This operation is similar to CopyName(), except that no copy of the field
    *  data is made:  instead, the field becomes shared between the two Messages,
    *  and changes to the field in one Message will be seen in the other.
    *  This method is more efficient than CopyName(), but you need to be careful
    *  when using it or you may get unexpected results...
    *  @param fieldName Name of an existing field to be shared.
    *  @param shareTo A Message to share the field into.
    *  @result B_NO_ERROR on success, or B_ERROR if there is an error sharing the field.
    */
   status_t ShareName(const String & fieldName, Message & shareTo) const {return ShareName(fieldName, shareTo, fieldName);}

   /** Take the data under (oldFieldName) in this message, and shares it into (shareTo), 
    *  where it will appear under the field name (newFieldName).
    *  Any data that was under (newFieldName) in (shareTo) will be replaced.
    *  This operation is similar to CopyName(), except that no copy of the field
    *  data is made:  instead, the field becomes shared between the two Messages,
    *  and changes to the field in one Message will be seen in the other.
    *  This method is more efficient than CopyName(), but you need to be careful
    *  when using it or you may get unexpected results...
    *  @param oldFieldName Name of an existing field to be shared.
    *  @param shareTo A Message to share the field into.
    *  @param newFieldName The name the field should have in the new Message.
    *  @result B_NO_ERROR on success, or B_ERROR if there is an error sharing the field.
    */
   status_t ShareName(const String & oldFieldName, Message & shareTo, const String & newFieldName) const;

   /** Examines the specified field to see if it is referenced more than once (e.g. by
     * another Message object).  If it is referenced more than once, makes a copy of the
     * field (including its contents) and replaces the shared field with the copy.
     * If the field is not referenced more than once, this method returns B_NO_ERROR 
     * without doing anything.
     * This method is handy when you are about to modify a field in a Message and you want
     * to first make sure that the field isn't shared by any other Messages.
     * @param fieldName Name of the field to ensure the non-sharedness of.
     * @returns B_NO_ERROR on success (i.e. either the copy was made, or no copy was necessary)
     *          or B_ERROR on failure (field not found, or out-of-memory)
     */
   status_t EnsureFieldIsPrivate(const String & fieldName);

   /** Ensures that the data items held in (field) are stored as a contiguous array in memory,
     * and then returns a pointer to the beginning of the array.
     * Note:  In most cases, the contents of the field will already be stored contiguously, and so this
     * method will be very efficient (O(1)).  However, if the field was previously modified using any of
     * our Prepend*() methods, then the field data may need to be normalized into a contiguous
     * array before a pointer can be returned... in that case, this method will call Queue::Normalize()
     * to do that, which is an O(N) operation with respect to the number of data items stored in the field.
     * @param fieldName The name of the field we wish to have a pointer to the contents of.
     * @param retItemCount If non-NULL, on successful return this will hold the number of items pointed to by our return value.
     * @param type Type of field you are looking for, or B_ANY_TYPE if any type will do.  Defaults to B_ANY_TYPE.
     * @returns A pointer to the field's data on success, or NULL if a field with the specified name and type wasn't found.
     */
   void * GetPointerToNormalizedFieldData(const String & fieldName, uint32 * retItemCount, uint32 type = B_ANY_TYPE);

   /** 
    * Swaps the contents and 'what' code of this Message with the specified Message.
    * This is a very efficient, O(1) operation.
    * @param swapWith Message whose contents should be swapped with this one.
    */
   void SwapContents(Message & swapWith);

   /**
    * Iterates over the contents of this Message to compute a checksum.
    * Note that this method can be CPU-intensive, since it has to scan
    * everything in the Message.  Don't call it often if you want good
    * performance!
    * @param countNonFlattenableFields If true, non-flattenable fields (e.g.
    *                        those added with AddTag() will be included in the
    *                        checksum.  If false (the default), they will be
    *                        ignored.
    */
   uint32 CalculateChecksum(bool countNonFlattenableFields = false) const;

   /**
    * Returns an iterator that iterates over the names of the fields in this
    * message.  If (type) is B_ANY_TYPE, then all field names will be included
    * in the traversal.  Otherwise, only names of the given type will be included.
    * @param type Type of fields you wish to iterate over, or B_ANY_TYPE to iterate over all fields.
    * @param flags Bit-chord of HTIT_FLAG_* flags you want to use to affect the iteration behaviour.
    *              This bit-chord will get passed on to the underlying HashtableIterator.  Defaults
    *              to zero, which provides the default behaviour.
    */
   MessageFieldNameIterator GetFieldNameIterator(uint32 type = B_ANY_TYPE, uint32 flags = 0) const {return MessageFieldNameIterator(_entries.GetIterator(flags), type);}

   /**
    * As above, only starts the iteration at the given field name, instead of at the beginning
    * or end of the list of field names.
    * @param startFieldName the field name to start with.  If (startFieldName) isn't present, the iteration will be empty.
    * @param type Type of fields you wish to iterate over, or B_ANY_TYPE to iterate over all fields.
    * @param flags Bit-chord of HTIT_FLAG_* flags you want to use to affect the iteration behaviour.
    *              This bit-chord will get passed on to the underlying HashtableIterator.  Defaults
    *              to zero, which provides the default behaviour.
    */
   MessageFieldNameIterator GetFieldNameIteratorAt(const String & startFieldName, uint32 type = B_ANY_TYPE, uint32 flags = 0) const {return MessageFieldNameIterator(_entries.GetIteratorAt(startFieldName, flags), type);}

   /** Makes this Message into a "light-weight" copy of (rhs).
     * When this method returns, this object will look like a copy of (rhs), except that
     * it will share the data in (rhs)'s fields such that modifying the data in (rhs)'s
     * fields will modify the data in this Message, and vice versa.  Making a light-weight
     * copy can be significantly cheaper than doing a full copy (e.g. with the assignment operator),
     * but you need to be aware of the potential side effects if you then go on to modify
     * the contents of either Message's fields.  Use with caution!
     * @param rhs The Message to make this Message into a light-weight copy of. 
     */ 
   void BecomeLightweightCopyOf(const Message & rhs) {what = rhs.what; _entries = rhs._entries;}

   /** Convenience method for retrieving a C-string pointer to a string data item inside a Message.
     * @param fn The field name to look for the string under.
     * @param defVal The pointer to return if no data item can be found.  Defaults to NULL.
     * @param idx The index to look under inside the field.  Defaults to zero.
     * @returns a Pointer to the String data item's Nul-terminated C string array on success, or (defVal) if the item could not be found.
     */
   inline const char * GetCstr(const String & fn, const char * defVal = NULL, uint32 idx = 0) const {const char * r; return (FindString( fn, idx, &r) == B_NO_ERROR) ? r : defVal;}

   /** Convenience method for retrieving a pointer to a pointer data item inside a Message.
     * @param fn The field name to look for the pointer under.
     * @param defVal The pointer to return if no data item can be found.  Defaults to NULL.
     * @param idx The index to look under inside the field.  Defaults to zero.
     * @returns The requested pointer on success, or (defVal) on failure.
     */
   inline void * GetPointer(const String & fn, void * defVal = NULL, uint32 idx = 0) const {void * r; return (FindPointer(fn, idx,  r) == B_NO_ERROR) ? r : defVal;}

   /** Convenience method for retrieving a pointer to A String data item inside a Message.
     * @param fn The field name to look for the String under.
     * @param defVal The pointer to return if no data item can be found.  Defaults to NULL.
     * @param idx The index to look under inside the field.  Defaults to zero.
     * @returns a Pointer to the String data item on success, or (defVal) if the item could not be found.
     */
   inline const String * GetStringPointer(const String & fn, const String * defVal=NULL, uint32 idx= 0) const {const String * r;return (FindString(fn, idx, &r) == B_NO_ERROR) ? r : defVal;}

#define DECLARE_MUSCLE_UNSIGNED_INTEGER_FIND_METHODS(bw)                                                                                               \
   inline status_t FindInt##bw (const String & fieldName, uint32 index, uint##bw & val) const {return FindInt##bw (fieldName, index, (int##bw &)val);} \
   inline status_t FindInt##bw (const String & fieldName,               uint##bw & val) const {return FindInt##bw (fieldName,        (int##bw &)val);}
   DECLARE_MUSCLE_UNSIGNED_INTEGER_FIND_METHODS(8);   ///< This macro defined Find methods with unsigned value arguments, so the user doesn't have to do ugly C-style casts to retrieve unsigned values.
   DECLARE_MUSCLE_UNSIGNED_INTEGER_FIND_METHODS(16);  ///< This macro defined Find methods with unsigned value arguments, so the user doesn't have to do ugly C-style casts to retrieve unsigned values.
   DECLARE_MUSCLE_UNSIGNED_INTEGER_FIND_METHODS(32);  ///< This macro defined Find methods with unsigned value arguments, so the user doesn't have to do ugly C-style casts to retrieve unsigned values.
   DECLARE_MUSCLE_UNSIGNED_INTEGER_FIND_METHODS(64);  ///< This macro defined Find methods with unsigned value arguments, so the user doesn't have to do ugly C-style casts to retrieve unsigned values.

#define DECLARE_MUSCLE_POINTER_FIND_METHODS(name, type)                                                                                \
   inline status_t Find##name (const String & fieldName, uint32 index, type * val) const {return Find##name (fieldName, index, *val);} \
   inline status_t Find##name (const String & fieldName,               type * val) const {return Find##name (fieldName,        *val);}
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Bool,    bool);         ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Double,  double);       ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Float,   float);        ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int8,    int8);         ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int16,   int16);        ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int32,   int32);        ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int64,   int64);        ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int8,    uint8);        ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int16,   uint16);       ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int32,   uint32);       ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Int64,   uint64);       ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Point,   Point);        ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(Rect,    Rect);         ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.
   DECLARE_MUSCLE_POINTER_FIND_METHODS(String,  const char *); ///< This macro defines old-style Find methods with pointer value arguments, for backwards compatibility.

#define DECLARE_MUSCLE_CONVENIENCE_METHODS(name, type) \
   inline type Get##name(const String & fieldName, const type & defVal = type(), uint32 idx = 0) const {type r; return (Find##name (fieldName, idx, r) == B_NO_ERROR) ? (const type &)r : defVal;} \
   inline status_t CAdd##name(    const String & fieldName, const type & value, const type & defVal = type())   {return (value == defVal) ? B_NO_ERROR : Add##name     (fieldName, value);}        \
   inline status_t CPrepend##name(const String & fieldName, const type & value, const type & defVal = type())   {return (value == defVal) ? B_NO_ERROR : Prepend##name (fieldName, value);}
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Bool,    bool);            ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Double,  double);          ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Float,   float);           ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Int8,    int8);            ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Int16,   int16);           ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Int32,   int32);           ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Int64,   int64);           ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Point,   Point);           ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Rect,    Rect);            ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(String,  String);          ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Message, MessageRef);      ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Flat,    ByteBufferRef);   ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_MUSCLE_CONVENIENCE_METHODS(Tag,     RefCountableRef); ///< This macro defines Get(), CAdd(), and CPrpend() methods for convience in common use cases.
   DECLARE_STANDARD_CLONE_METHOD(Message);  ///< implements the standard Clone() method to copy a Message object.

protected:
   /** Overridden to copy directly if (copyFrom) is a Message as well. */
   virtual status_t CopyFromImplementation(const Flattenable & copyFrom);

private:
   // Helper functions
   status_t AddDataAux(const String & fieldName, uint32 type, const void *data, uint32 numBytes, bool prepend);

   // Given a known uint32, returns the size of an item of that type.
   // Returns zero if items of the given type are variable length.
   uint32 GetElementSize(uint32 type) const;

   RefCountableRef GetArrayRef(const String & arrayName, uint32 etc) const;
   AbstractDataArray * GetArray(const String & arrayName, uint32 etc) const;
   AbstractDataArray * GetOrCreateArray(const String & arrayName, uint32 tc);

   status_t AddFlatBuffer(const String & fieldName, const Flattenable & flat, uint32 etc, bool prepend);
   status_t AddFlatRef(const String & fieldName, const FlatCountableRef & flat, uint32 etc, bool prepend);
   status_t AddDataAux(const String & fieldName, const void * data, uint32 size, uint32 etc, bool prepend);

   status_t FindFlatAux(const String & fieldName, uint32 index, Flattenable & flat) const;
   status_t FindDataItemAux(const String & fieldName, uint32 index, uint32 tc, void * setValue, uint32 valueSize) const;

   status_t ReplaceFlatItemBuffer(bool okayToAdd, const String & fieldName, uint32 index, const Flattenable & flat, uint32 tc);
   status_t ReplaceDataAux(bool okayToAdd, const String & fieldName, uint32 index, void * dataBuf, uint32 bufSize, uint32 tc);
 
   bool FieldsAreSubsetOf(const Message & rhs, bool compareData) const;

   const String * GetExtremeFieldNameStringAux(uint32 optTypeCode, bool isLast) const
   {
      return (optTypeCode == B_ANY_TYPE) ? (isLast ? _entries.GetLastKey() : _entries.GetFirstKey()) 
                                         : MessageFieldNameIterator(*this, optTypeCode, HTIT_FLAG_NOREGISTER|(isLast?HTIT_FLAG_BACKWARDS:0)).GetNextFieldNameString();
   }

   // Iterator support methods
   friend class MessageFieldNameIterator;
   Hashtable<String, RefCountableRef> _entries;   
};

/** Convenience method:  Gets a Message from the message pool, populates it using the flattened Message
 *  bytes held by (bb), and returns it.
 *  @param bb a ByteBuffer object to unflatten the Message from.
 *  @return Reference to a Message object, or a NULL ref on failure (out of memory or unflattening error)
 */
inline MessageRef GetMessageFromPool(const ByteBuffer & bb) {return GetMessageFromPool(bb.GetBuffer(), bb.GetNumBytes());}

inline MessageFieldNameIterator :: MessageFieldNameIterator(const Message & msg, uint32 type, uint32 flags) : HashtableIterator<String, RefCountableRef>(msg._entries.GetIterator(flags)), _typeCode(type) {/* empty */}

/** Convenience method:  Gets a Message from the message pool, and populates it by calling
  * SaveToArchive(msg) on the passed in object.  Templates so that the passed-in object may be of any type.
  * @param pool The Message pool to get the Message object from.
  * @param objectToArchive The object to call SaveToArchive() on.
  * @returns a non-NULL MessageRef on success, or a NULL MessageRef on failure.
  */
template<typename T> inline MessageRef GetArchiveMessageFromPool(ObjectPool<Message> & pool, const T & objectToArchive)
{
   MessageRef m = GetMessageFromPool(pool);
   if ((m())&&(objectToArchive.SaveToArchive(*m()) != B_NO_ERROR)) m.Reset();
   return m;
}

/** Convenience method:  Gets a Message from the message pool, and populates it by calling
  * SaveToArchive(msg) on the passed in object.  Templates so that the passed-in object may be of any type.
  * @param objectToArchive The object to call SaveToArchive() on.
  * @returns a non-NULL MessageRef on success, or a NULL MessageRef on failure.
  */
template<typename T> inline MessageRef GetArchiveMessageFromPool(const T & objectToArchive)
{
   MessageRef m = GetMessageFromPool();
   if ((m())&&(objectToArchive.SaveToArchive(*m()) != B_NO_ERROR)) m.Reset();
   return m;
}

}; // end namespace muscle

#endif /* _MUSCLEMESSAGE_H */

