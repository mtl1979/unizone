/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */ 

#ifndef MuscleRefCount_h 
#define MuscleRefCount_h 

#include "util/ObjectPool.h" 
#include "system/AtomicCounter.h"

BEGIN_NAMESPACE(muscle);

class RefCountable;

/** This class represents objects that can be reference-counted using the Ref class. 
  * Note that any object that can be reference counted can also be pooled and recycled.
  */
class RefCountable
{
public:
   /** Default constructor.  Refcount begins at zero. */
   RefCountable() {/* empty */}

   /** Copy constructor -- does NOT copy the refcount! */
   RefCountable(const RefCountable &) {/* empty */}

   /** Virtual destructor, to keep C++ honest.  Don't remove this unless you like crashing */
   virtual ~RefCountable() {/* empty */}

   /** Assigment operator.  Implemented as a no-op; does NOT copy the refcount! */
   inline RefCountable &operator=(const RefCountable &) {return *this;}

   /** Increments the counter and returns true iff the new value is zero. */
   inline void IncrementRefCount() const {_refCount.AtomicIncrement();}

   /** Decrements the counter and returns true iff the new value is zero. */
   inline bool DecrementRefCount() const {return _refCount.AtomicDecrement();}

private:
   mutable AtomicCounter _refCount;
};

template <class Item> class Ref;  // forward reference

/** A GenericRef is a reference to any kind of RefCountable object. */
typedef Ref<RefCountable> GenericRef;

/**
 *  This is a ref-count token object that works with any instantiation
 *  type that is a subclass of RefCountable. 
 */
template <class Item> class Ref
{
public:
   typedef ObjectPool<Item> ItemPool;        /**< type of an ObjectPool of user data structures */

   /** 
    *  Default constructor.
    *  Creates a NULL reference (suitable for later initialization with SetRef(), or the assignment operator)
    */
   Ref() : _item(NULL), _recycler(NULL), _doRefCount(true) {/* empty */}

   /** 
     * Creates a new reference-count for the given item.
     * Once referenced, (item) will be automatically deleted (or recycled) when the last Ref that references it goes away.
     * @param item A dynamically allocated object that the Ref class will assume responsibility for deleting.  May be NULL.
     * @param recycler An optional AbstractObjectRecycler to use to recycle the object when we're done with it.  May be NULL.
     * @param doRefCount If set false, then this Ref will not do any ref-counting on (item); rather it
     *                   just acts as a fancy version of a normal C++ pointer.  Specifically, it will not 
     *                   modify the object's reference count, nor will it ever delete the object.  
     *                   Setting this argument to false can be useful if you want to supply a  
     *                   Ref to an item that wasn't dynamically allocated from the heap.
     *                   But if you do that, it allows the possibility of the object going away while
     *                   other Refs are still using it, so be careful!
     */
   Ref(Item * item, AbstractObjectRecycler * recycler, bool doRefCount = true) : _item(item), _recycler(recycler), _doRefCount(doRefCount) {RefItem();} 

   /** Copy constructor.  Creates an additional reference to the object referenced by (copyMe).
    *  The referenced object won't be deleted until ALL Refs that reference it are gone.
    */
   Ref(const Ref & copyMe) : _item(NULL), _recycler(NULL), _doRefCount(true) {*this = copyMe;}

   /** Attempts to set this reference by downcasting the reference in the provided GenericRef.
     * If the downcast cannot be done (via dynamic_cast) then we become a NULL reference.
     * @param ref The generic reference to set ourselves from.
     * @param junk This parameter is ignored (it's only here to disambiguate constructors)
     */
   Ref(const GenericRef & ref, bool /*junk*/) : _item(NULL), _recycler(NULL), _doRefCount(true) {(void) SetFromGeneric(ref);}

   /** Unreferences the held data item.  If this is the last Ref that
    *  references the held data item, the data item will be deleted or recycled at this time.
    */
   ~Ref() {UnrefItem();}

   /** Convenience method; unreferences any item this Ref is currently referencing; and
     * causes this Ref to reference the given Item instead.  Very similar to the assignment operator.
     * (in fact the assignment operator just calls this method)
     * @param item A dynamically allocated object that this Ref object will assume responsibility for deleting.
     *             May be NULL, in which case the effect is just to unreference the current item.
     * @param recycler An optional ObjectPool to recycle the object into.  May be NULL.
     * @param doRefCount If set false, then this Ref will not do any ref-counting on (item); rather it
     *                   just acts as a fancy version of a normal C++ pointer.  Specifically, it will not 
     *                   modify the object's reference count, nor will it ever delete the object.  
     *                   Setting this argument to false can be useful if you want to supply a  
     *                   Ref to an item that wasn't dynamically allocated from the heap.
     *                   But if you do that, it allows the possibility of the object going away while
     *                   other Refs are still using it, so be careful!
     */
   void SetRef(Item * item, AbstractObjectRecycler * recycler, bool doRefCount = true)
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
         _recycler = recycler;
      }
      else
      {
         // switch items
         UnrefItem();
          _item       = item;
          _recycler   = recycler;
          _doRefCount = doRefCount;
         RefItem();
      }
   }

   /** Assigment operator.
    *  Unreferences the previous held data item, and adds a reference to the the data item of (r).
    */
   inline Ref &operator=(const Ref &r) {if (this != &r) SetRef(r._item, r._recycler, r._doRefCount); return *this;}

   /** Returns true iff both Refs are referencing the same data. */
   bool operator ==(const Ref &rhs) const {return _item == rhs._item;}
 
   /** Returns true iff both Refs are not referencing the same data. */
   bool operator !=(const Ref &rhs) const {return _item != rhs._item;}
 
   /** Returns the ref-counted data item.  The returned data item
    *  is only guaranteed valid for as long as this RefCount object exists.
    */
   Item * GetItemPointer() const {return _item;}

   /** Convenience synonym for GetItemPointer(). */
   Item * operator()() const {return _item;}

   /** Unreferences our held data item (if any), and turns this object back into a NULL reference.  
    *  (equivalent to *this = Ref();)
    */
   void Reset() {UnrefItem();}

   /** Equivalent to Reset(), except that this method will not delete or recycle 
     * the held object under any circumstances.  Use with caution.
     */
   void Neutralize() {if ((_doRefCount)&&(_item)) (void) _item->DecrementRefCount(); _item = NULL;}

   /** Returns this Ref's recycler object (may be NULL). */
   AbstractObjectRecycler * GetItemRecycler() const {return _recycler;}

   /** Returns true iff we are refcounting our held object, or false
     * if we are merely pointing to it (see constructor documentation for details)
     */
   bool IsRefCounting() const {return _doRefCount;}

   /** Convenience method:  Returns a GenericRef object referencing the same RefCountable as this typed ref. */
   GenericRef GetGeneric() const {return GenericRef(_item, _recycler, _doRefCount);}

   /** Convenience method; attempts to set this typed Ref to be referencing the same item as the given GenericRef.  
     * If the conversion cannot be done, our state will remain unchanged.
     * @param genericRef The GenericRef to set ourselves from.
     * @returns B_NO_ERROR if the conversion was successful, or B_ERROR if the GenericRef's item
     *          type is incompatible with our own item type (as dictated by dynamic_cast)
     */
   status_t SetFromGeneric(const GenericRef & genericRef)
   {
      RefCountable * genericItem = genericRef();
      if (genericItem)
      {
         Item * typedItem = dynamic_cast<Item *>(genericItem);
         if (typedItem == NULL) return B_ERROR;
         SetRef(typedItem, genericRef.GetItemRecycler(), genericRef.IsRefCounting());
      }
      return B_NO_ERROR;
   }

private:
   void RefItem() {if ((_doRefCount)&&(_item)) _item->IncrementRefCount();}
   void UnrefItem()
   {
      if (_item)
      {
         if ((_doRefCount)&&(_item->DecrementRefCount()))
         {
            if (_recycler) _recycler->RecycleObject(_item);
                      else delete _item;
         }
         _item = NULL;
      }
   }
   
   Item * _item; 
   AbstractObjectRecycler * _recycler;
   bool _doRefCount;
};

template <class T> class HashFunctor;

// VC++ can't handle partial template specialization, so don't let it see this
// For VC++, you'll have to write an explicit HashFunctor.  Sucks, eh?
#ifndef _MSC_VER
template <class Item>
class HashFunctor<Ref<Item> >
{
public:
   uint32 operator () (const Ref<Item> & ref) const {return (uint32) ref();}
};
#endif

END_NAMESPACE(muscle);

#endif
