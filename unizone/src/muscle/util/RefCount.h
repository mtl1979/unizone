/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */ 

#ifndef MuscleRefCount_h 
#define MuscleRefCount_h 

#include "util/Cloneable.h" 
#include "util/ObjectPool.h" 
#include "system/AtomicCounter.h"

namespace muscle {

class RefCountable;

// This macro lets you easily declare Reference classes fora give RefCountable type in the standard way,
// which is that for a RefCountable class Named XXX you would have XXXRef and ConstXXXRef as simpler ways
// to express Ref<XXX> and ConstRef<XXX>, respectively.
#define DECLARE_REFTYPES(RefCountableClassName)                               \
   typedef ConstRef<RefCountableClassName> Const##RefCountableClassName##Ref; \
   typedef Ref<RefCountableClassName>      RefCountableClassName##Ref

/** This class represents objects that can be reference-counted using the Ref class. 
  * Note that any object that can be reference counted can also be pooled and recycled.
  */
class RefCountable
{
public:
   /** Default constructor.  Refcount begins at zero. */
   RefCountable() : _manager(NULL) {/* empty */}

   /** Copy constructor -- ref count and manager settings are deliberately not copied over! */
   RefCountable(const RefCountable &) : _manager(NULL) {/* empty */}

   /** Virtual destructor, to keep C++ honest.  Don't remove this unless you like crashing */
   virtual ~RefCountable() {/* empty */}

   /** Assigment operator.  deliberately implemented as a no-op! */
   inline RefCountable &operator=(const RefCountable &) {return *this;}

   /** Increments the counter and returns true iff the new value is zero.  Thread safe. */
   inline void IncrementRefCount() const {_refCount.AtomicIncrement();}

   /** Decrements the counter and returns true iff the new value is zero.  Thread safe. */
   inline bool DecrementRefCount() const {return _refCount.AtomicDecrement();}

   /** Sets the recycle-pointer for this object.  If set to non-NULL, this pointer
     * is used by the ObjectPool class to recycle this object when it is no longer
     * in use, so as to avoid the overhead of having to delete it and re-create it
     * later on.  The RefCountable class itself does nothing with this pointer.
     * Default value is NULL. 
     * @param manager Pointer to the new manager object to use, or NULL to use no manager.
     */
   void SetManager(AbstractObjectManager * manager) {_manager = manager;}

   /** Returns this object's current recyler pointer. */
   AbstractObjectManager * GetManager() const {return _manager;}
   
   /** Returns this object's current reference count.  Note that
     * the value returned by this method is volatile in multithreaded
     * environments, so it may already be wrong by the time it is returned.
     * Be careful!
     */
   uint32 GetRefCount() const {return _refCount.GetCount();}

private:
   mutable AtomicCounter _refCount;
   AbstractObjectManager * _manager;
};
template <class Item> class Ref;       // forward reference
template <class Item> class ConstRef;  // forward reference
DECLARE_REFTYPES(RefCountable);

/**
 *  This is a ref-count token object that works with any instantiation
 *  type that is a subclass of RefCountable.  The RefCountable object
 *  that this ConstRef item holds is considered read-only; if you want
 *  it to be modfiable, you should use the Ref class instead.
 */
template <class Item> class ConstRef
{
public:
   typedef ObjectPool<Item> ItemPool;        /**< type of an ObjectPool of user data structures */

   /** 
    *  Default constructor.
    *  Creates a NULL reference (suitable for later initialization with SetRef(), or the assignment operator)
    */
   ConstRef() : _item(NULL), _doRefCount(true) {/* empty */}

   /** 
     * Creates a new reference-count for the given item.
     * Once referenced, (item) will be automatically deleted (or recycled) when the last ConstRef that references it goes away.
     * @param item A dynamically allocated object that the ConstRef class will assume responsibility for deleting.  May be NULL.
     * @param doRefCount If set false, then this ConstRef will not do any ref-counting on (item); rather it
     *                   just acts as a fancy version of a normal C++ pointer.  Specifically, it will not 
     *                   modify the object's reference count, nor will it ever delete the object.  
     *                   Setting this argument to false can be useful if you want to supply a  
     *                   ConstRef to an item that wasn't dynamically allocated from the heap.
     *                   But if you do that, it allows the possibility of the object going away while
     *                   other Refs are still using it, so be careful!
     */
   ConstRef(const Item * item, bool doRefCount = true) : _item(item), _doRefCount(doRefCount) {RefItem();} 

   /** Copy constructor.  Creates an additional reference to the object referenced by (copyMe).
    *  The referenced object won't be deleted until ALL Refs that reference it are gone.
    */
   ConstRef(const ConstRef & copyMe) : _item(NULL), _doRefCount(true) {*this = copyMe;}

   /** Attempts to set this reference by downcasting the reference in the provided ConstRefCountableRef.
     * If the downcast cannot be done (via dynamic_cast) then we become a NULL reference.
     * @param ref The read-only RefCountable reference to set ourselves from.
     * @param junk This parameter is ignored; it is just here to disambiguate constructors.
     */
   ConstRef(const ConstRefCountableRef & ref, bool junk) : _item(NULL), _doRefCount(true) {(void) junk; (void) SetFromRefCountableRef(ref);}

   /** Unreferences the held data item.  If this is the last ConstRef that
    *  references the held data item, the data item will be deleted or recycled at this time.
    */
   ~ConstRef() {UnrefItem();}

   /** Convenience method; unreferences any item this ConstRef is currently referencing; and
     * causes this ConstRef to reference the given Item instead.  Very similar to the assignment operator.
     * (in fact the assignment operator just calls this method)
     * @param item A dynamically allocated object that this ConstRef object will assume responsibility for deleting.
     *             May be NULL, in which case the effect is just to unreference the current item.
     * @param doRefCount If set false, then this ConstRef will not do any ref-counting on (item); rather it
     *                   just acts as a fancy version of a normal C++ pointer.  Specifically, it will not 
     *                   modify the object's reference count, nor will it ever delete the object.  
     *                   Setting this argument to false can be useful if you want to supply a  
     *                   ConstRef to an item that wasn't dynamically allocated from the heap.
     *                   But if you do that, it allows the possibility of the object going away while
     *                   other Refs are still using it, so be careful!
     */
   void SetRef(const Item * item, bool doRefCount = true)
   {
      if (item == _item)
      {
         if (doRefCount != _doRefCount)
         {
            if (doRefCount)
            {
               // We weren't ref-counting before, but the user wants us to start
               _doRefCount = true;
               RefItem();
            }
            else 
            {
               // We were ref-counting before, but the user wants us to drop it
               UnrefItem();
               _doRefCount = false;
            } 
         }
      }
      else
      {
         // switch items
         UnrefItem();
          _item       = item;
          _doRefCount = doRefCount;
         RefItem();
      }
   }

   /** Assigment operator.
    *  Unreferences the previous held data item, and adds a reference to the the data item of (rhs).
    */
   inline ConstRef &operator=(const ConstRef & rhs) {if (this != &rhs) SetRef(rhs._item, rhs._doRefCount); return *this;}

   /** Similar to the == operator, except that this version will also call the comparison operator
     * on the objects themselves if necessary, to determine exact equality.  (Compare with the ==
     * operator which only compares the pointers, not the objects themselves) 
     */
   bool IsDeeplyEqualTo(const ConstRef & rhs) const
   {
      if (_item == rhs._item) return true;
      if ((_item != NULL) != (rhs._item != NULL)) return false;
      return ((_item == NULL)||(*_item == *rhs._item));
   }

   /** Returns true iff both Refs are referencing the same data. */
   bool operator ==(const ConstRef &rhs) const {return _item == rhs._item;}
 
   /** Returns true iff both Refs are not referencing the same data. */
   bool operator !=(const ConstRef &rhs) const {return _item != rhs._item;}
 
   /** Returns the ref-counted data item.  The returned data item
    *  is only guaranteed valid for as long as this RefCount object exists.
    */
   const Item * GetItemPointer() const {return _item;}

   /** This is the same as GetItemPointer(), except that it can be safely
    *  called on a NULL ConstRef object, because it checks the (this) pointer
    *  and returns NULL if (this) was NULL.
    *  @note This function is unusual in that it safe to call this function on
    *        a NULL ConstRef pointer!
    */
   const Item * CheckedGetItemPointer() const {return this ? _item : NULL;}

   /** Convenience synonym for GetItemPointerConst(). */
   const Item * operator()() const {return _item;}

   /** Unreferences our held data item (if any), and turns this object back into a NULL reference.  
    *  (equivalent to *this = ConstRef();)
    */
   void Reset() {UnrefItem();}

   /** Equivalent to Reset(), except that this method will not delete or recycle 
     * the held object under any circumstances.  Use with caution.
     */
   void Neutralize() {if ((_doRefCount)&&(_item)) (void) _item->DecrementRefCount(); _item = NULL;}

   /** Swaps this ConstRef's contents with those of the specified ConstRef.
     * @param swapWith ConstRef to swap state with.
     */
   void SwapContents(ConstRef & swapWith)
   {
      muscleSwap(_item,       swapWith._item); 
      muscleSwap(_doRefCount, swapWith._doRefCount);
   }

   /** Returns true iff we are refcounting our held object, or false
     * if we are merely pointing to it (see constructor documentation for details)
     */
   bool IsRefCounting() const {return _doRefCount;}

   /** Convenience method:  Returns a ConstRefCountableRef object referencing the same RefCountable as this typed ref. */
   ConstRefCountableRef GetRefCountableRef() const {return ConstRefCountableRef(_item, _doRefCount);}

   /** Convenience method; attempts to set this typed ConstRef to be referencing the same item as the given ConstRefCountableRef.  
     * If the conversion cannot be done, our state will remain unchanged.
     * @param refCountableRef The ConstRefCountableRef to set ourselves from.
     * @returns B_NO_ERROR if the conversion was successful, or B_ERROR if the ConstRefCountableRef's item
     *          type is incompatible with our own item type (as dictated by dynamic_cast)
     */
   status_t SetFromRefCountableRef(const ConstRefCountableRef & refCountableRef)
   {
      const RefCountable * refCountableItem = refCountableRef();
      if (refCountableItem)
      {
         const Item * typedItem = dynamic_cast<const Item *>(refCountableItem);
         if (typedItem == NULL) return B_ERROR;
         SetRef(typedItem, refCountableRef.IsRefCounting());
      }
      else Reset();

      return B_NO_ERROR;
   }

   /** Same as SetFromRefCountableRef(), but uses a static_cast instead of a dynamic_cast to
     * do the type conversion.  This is faster that SetFromRefCountableRef(), but it is up to
     * the user's code to guarantee that the conversion is valid.  If (refCountableRef)
     * references an object of the wrong type, undefined behaviour (read: crashing)
     * will likely occur.
     */
   inline void SetFromRefCountableRefUnchecked(const ConstRefCountableRef & refCountableRef)
   {
      SetRef(static_cast<const Item *>(refCountableRef()), refCountableRef.IsRefCounting());
   }

   /** Returns true only if we are certain that no other Refs are pointing
     * at the same RefCountable object that we are.  If this Ref's do-reference-counting
     * flag is false, then this method will always return false, since we can't
     * be sure about sharing unless we are reference counting.  If this ConstRef is
     * a NULL Ref, then this method will return true.
     */
   bool IsRefPrivate() const
   {
      return ((_item == NULL)||((_doRefCount)&&(_item->GetRefCount() == 1)));
   }

   /** This method will check our referenced object to see if there is any
     * chance that it is shared by other ConstRef objects.  If it is, it will
     * make a copy of the referenced object and set this ConstRef to reference 
     * the copy instead of the original.  The upshot of this is that once
     * this method returns B_NO_ERROR, you can safely modify the referenced
     * object without having to worry about race conditions caused by sharing
     * data with other threads.  This method is thread safe -- it may occasionally
     * make a copy that wasn't strictly necessary, but it will never fail to
     * make a copy when making a copy is necessary.
     * @returns B_NO_ERROR on success (i.e. the object was successfully copied,
     *                     or a copy turned out to be unnecessary), or B_ERROR
     *                     on failure (i.e. out of memory)
     */
   status_t EnsureRefIsPrivate()
   {
      if (IsRefPrivate() == false)
      {
         Ref<Item> copyRef = this->Clone();
         if (copyRef() == NULL) return B_ERROR;
         *this = copyRef;
      }
      return B_NO_ERROR;
   }

   /** Makes a copy of our held Item and returns it in a non-const Ref object,
     * or returns a NULL Ref on out-of-memory (or if we aren't holding an Item).
     */
   Ref<Item> Clone()
   {
      if (_item)
      {
         AbstractObjectManager * m = _item->GetManager();
         Item * newItem = m ? static_cast<Item *>(m->ObtainObjectGeneric()) : static_cast<Item *>(CloneObject(*_item));
         if (newItem) return Ref<Item>(newItem);
                 else WARN_OUT_OF_MEMORY;
      }
      return Ref<Item>();
   }

   /** This method allows Refs to be keys in Hashtables. */
   uint32 HashCode() const {return (uint32)((unsigned long)_item);}  // double-cast for AMD64

private:
   void RefItem() {if ((_doRefCount)&&(_item)) _item->IncrementRefCount();}
   void UnrefItem()
   {
      if (_item)
      {
         if ((_doRefCount)&&(_item->DecrementRefCount()))
         {
            AbstractObjectManager * m = _item->GetManager();
            if (m) m->RecycleObject(const_cast<Item *>(_item));
              else delete _item;
         }
         _item = NULL;
      }
   }
   
   const Item * _item; 
   bool _doRefCount;
};

/**
 *  This is a ref-count token object that works with any instantiation
 *  type that is a subclass of RefCountable.  The RefCountable object
 *  that this ConstRef item holds is considered modifiable; if you want
 *  it to be read-only, you should use the ConstRef class instead.
 */
template <class Item> class Ref : public ConstRef<Item>
{
public:
   /** 
    *  Default constructor.
    *  Creates a NULL reference (suitable for later initialization with SetRef(), or the assignment operator)
    */
   Ref() : ConstRef<Item>() {/* empty */}

   /** 
     * Creates a new reference-count for the given item.
     * Once referenced, (item) will be automatically deleted (or recycled) when the last Ref that references it goes away.
     * @param item A dynamically allocated object that the Ref class will assume responsibility for deleting.  May be NULL.
     * @param doRefCount If set false, then this Ref will not do any ref-counting on (item); rather it
     *                   just acts as a fancy version of a normal C++ pointer.  Specifically, it will not 
     *                   modify the object's reference count, nor will it ever delete the object.  
     *                   Setting this argument to false can be useful if you want to supply a  
     *                   Ref to an item that wasn't dynamically allocated from the heap.
     *                   But if you do that, it allows the possibility of the object going away while
     *                   other Refs are still using it, so be careful!
     */
   Ref(Item * item, bool doRefCount = true) : ConstRef<Item>(item, doRefCount) {/* empty */}

   /** Copy constructor.  Creates an additional reference to the object referenced by (copyMe).
    *  The referenced object won't be deleted until ALL Refs that reference it are gone.
    */
   Ref(const Ref & copyMe) : ConstRef<Item>(copyMe) {/* empty */}

   /** Attempts to set this reference by downcasting the reference in the provided RefCountableRef.
     * If the downcast cannot be done (via dynamic_cast) then we become a NULL reference.
     * @param ref The RefCountable reference to set ourselves from.
     * @param junk This parameter is ignored; it is just here to disambiguate constructors.
     */
   Ref(const RefCountableRef & ref, bool junk) : ConstRef<Item>(ref, junk) {/* empty */}

   /** Returns the ref-counted data item.  The returned data item
    *  is only guaranteed valid for as long as this RefCount object exists.
    */
   Item * GetItemPointer() const {return const_cast<Item *>((static_cast<const ConstRef<Item> *>(this))->GetItemPointer());}

   /** This is the same as GetItemPointer(), except that it can be safely
    *  called on a NULL Ref object, because it checks the (this) pointer
    *  and returns NULL if (this) was NULL.
    *  @note This function is unusual in that it safe to call this function on
    *        a NULL Ref pointer!
    */
   Item * CheckedGetItemPointer() const {return this ? GetItemPointer() : NULL;}

   /** Read-write implementation of the convenience synonym for GetItemPointer(). */
   Item * operator()() const {return GetItemPointer();}

   /** Convenience method:  Returns a read/write RefCountableRef object referencing the same RefCountable as this typed ref. */
   RefCountableRef GetRefCountableRef() const {return RefCountableRef(GetItemPointer(), this->IsRefCounting());}
};

/** Convenience method for converting a ConstRef into a non-const Ref.  Only call this method if you are sure you know what you are doing,
  * because usually the original ConstRef was declared as a ConstRef for a good reason!
  */
template <class Item> inline Ref<Item> CastAwayConstFromRef(const ConstRef<Item> & constItem) {return Ref<Item>(const_cast<Item *>(constItem()), constItem.IsRefCounting());}

// VC++6 and earlier can't handle partial template specialization, so don't let it see this
// To compile using them, you'll have to write an explicit HashFunctor.  Sucks, eh?
// At least you can use the DECLARE_HASHTABLE_KEY_CLASS macro to make it quicker for you.
#ifndef MUSCLE_USING_OLD_MICROSOFT_COMPILER
template <class T> class HashFunctor;

/** This HashFunctor lets you use ConstRef objects as keys in a Hashtable.  They will be hashed based on the pointer of the object they hold. */
template <class Item> class HashFunctor<ConstRef<Item> >
{
public:
   /** Returns a hash code corresponding to the specified ConstRef object.
     * @param ref The ConstRef object that is being used as a key in the Hashtable.
     */
   uint32 operator()(const ConstRef<Item> & ref) const {return ref.HashCode();}
};

/** This HashFunctor lets you use ConstRef pointers as keys in a Hashtable.  They will be hashed based on the pointer of the object they hold. */
template <class Item> class HashFunctor<const ConstRef<Item> * >
{
public:
   /** Returns a hash code corresponding to the specified ConstRef pointer.
     * @param ref The ConstRef pointer that is being used as a key in the Hashtable.
     */
   uint32 operator()(const ConstRef<Item> * ref) const {return ref->HashCode();}
};

/** This HashFunctor lets you use Ref objects as keys in a Hashtable.  They will be hashed based on the pointer of the object they hold. */
template <class Item> class HashFunctor<Ref<Item> >
{
public:
   /** Returns a hash code corresponding to the specified Ref object.
     * @param ref The Ref object that is being used as a key in the Hashtable.
     */
   uint32 operator()(const Ref<Item> & ref) const {return ref.HashCode();}
};

/** This HashFunctor lets you use Ref pointers as keys in a Hashtable.  They will be hashed based on the pointer of the object they hold. */
template <class Item> class HashFunctor<const Ref<Item> * >
{
public:
   /** Returns a hash code corresponding to the specified Ref pointer.
     * @param ref The Ref pointer that is being used as a key in the Hashtable.
     */
   uint32 operator()(const Ref<Item> * ref) const {return ref->HashCode();}
};
#endif

}; // end namespace muscle

#endif
