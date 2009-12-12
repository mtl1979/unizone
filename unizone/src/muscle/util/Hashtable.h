/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleHashtable_h
#define MuscleHashtable_h

#include "support/MuscleSupport.h"

#ifdef _MSC_VER
# pragma warning(disable: 4786)
#endif

#ifndef MUSCLE_HASHTABLE_DEFAULT_CAPACITY
# define MUSCLE_HASHTABLE_DEFAULT_CAPACITY 8
#endif

namespace muscle {

#ifdef MUSCLE_COLLECT_HASHTABLE_COLLISION_STATISTICS
extern uint32 _totalHashtableCollisions;
#endif

// implementation detail; please ignore this!
static const uint32 MUSCLE_HASHTABLE_INVALID_HASH_CODE = (uint32)-1;

// Forward declarations
template <class KeyType, class ValueType, class HashFunctorType>                     class HashtableIterator;
template <class KeyType, class ValueType, class HashFunctorType>                     class HashtableBase;
template <class KeyType, class ValueType, class HashFunctorType, class SubclassType> class HashtableMid;

/** These flags can be passed to the HashtableIterator constructor (or to the GetIterator()/GetIteratorAt() 
  * functions in the Hashtable class) to modify the iterator's behaviour.
  */
enum {
   HTIT_FLAG_BACKWARDS  = (1<<0), // iterate backwards.  Conveniently equal to ((bool)true), for backwards compatibility with old code
   HTIT_FLAG_NOREGISTER = (1<<1), // don't register with Hashtable object, for thread-safety (at the expense of modification-safety)
   NUM_HTIT_FLAGS = 2             // number of HTIT_FLAG_* constants that have been defined
};

/**
 * This class is an iterator object, used for iterating over the set
 * of keys or values in a Hashtable.  Note that the Hashtable
 * maintains the ordering of keys and values, unlike many hash table
 * implementations.
 *
 * Given a Hashtable object, you can obtain one or more of these
 * iterator objects by calling the Hashtable's GetIterator() method.
 *
 * This iterator actually contains separate state for two iterations:
 * one for iterating over the values in the Hashtable, and one for 
 * iterating over the keys.  These two iterations can be done independently
 * of each other.
 *
 * The most common form for an iteration is this:
 *
 * for (HashtableIterator<String, int> iter(table); iter.HasMoreKeys(); iter++)
 * {
 *    const String & nextKey = iter.GetKey(); 
 *    int nextValue = iter.GetValue(); 
 *    [...]
 * }
 *
 * It is safe to modify or delete a Hashtable during a traversal; the active iterators
 * will be automatically notified so that they do the right thing and won't continue to
 * reference at any now-invalid items.
 */
template <class KeyType, class ValueType, class HashFunctorType = typename AutoChooseHashFunctorHelper<KeyType>::Type > class HashtableIterator
{
public:
   /**
    * Default constructor.  It's here only so that you can include HashtableIterators
    * as member variables, in arrays, etc.  HashtableIterators created with this
    * constructor are "empty", so they won't be useful until you set them equal to a 
    * HashtableIterator that was returned by Hashtable::GetIterator().
    */
   HashtableIterator();

   /** Copy Constructor. */
   HashtableIterator(const HashtableIterator<KeyType, ValueType, HashFunctorType> & rhs);

   /** Convenience Constructor -- makes an iterator equivalent to the value returned by table.GetIterator().  
     * @param table the Hashtable to iterate over.
     * @param flags A bit-chord of HTIT_FLAG_* constants (see above).  Defaults to zero for default behaviour.  
     */
   HashtableIterator(const HashtableBase<KeyType, ValueType, HashFunctorType> & table, uint32 flags = 0);

   /** Convenience Constructor -- makes an iterator equivalent to the value returned by table.GetIteratorAt().  
     * @param table the Hashtable to iterate over.
     * @param startAt the first key that should be returned by the iteration.  If (startAt) is not in the table,
     *                the iterator will not return any results.
     * @param flags A bit-chord of HTIT_FLAG_* constants (see above).  Set to zero to get the default behaviour.
     */
   HashtableIterator(const HashtableBase<KeyType, ValueType, HashFunctorType> & table, const KeyType & startAt, uint32 flags);

   /** Destructor */
   ~HashtableIterator();

   /** Assignment operator. */
   HashtableIterator<KeyType, ValueType, HashFunctorType> & operator=(const HashtableIterator<KeyType, ValueType, HashFunctorType> & rhs);

   /** Advances this iterator by one entry in the table.  Equivalent to calling both GetNextKey() and GetNextValue(). */
   void operator++(int) {(void) GetNextKey(); (void) GetNextValue();}

   /** Retracts this iterator by one entry in the table.  The opposite of the ++ operator. */
   void operator--(int) {bool b = IsBackwards(); SetBackwards(!b); (void) GetNextKey(); (void) GetNextValue(); SetBackwards(b);}

   /** Returns true iff there are more keys left in the key traversal.  */
   bool HasMoreKeys() const {return ((_nextKeyCookie != NULL)||((_owner)&&(_owner->HasItems())&&(_flags&HTIT_INTERNAL_FLAG_RESTARTKEY)));}

   /**
    * Returns a reference to the key this iterator is currently pointing at.  This method does not change the state of the iterator.
    * @note Be careful with this method, if this iterator isn't currently pointing at any key,
    *       it will return a NULL reference and your program will crash when you try to use it.
    *       Typically you would want to call this function only after checking to see that
    *       HasMoreKeys() returns true.
    * @note The returned reference is only guaranteed to remain valid for as long as the Hashtable remains unchanged.
    */
   const KeyType & GetKey() const {return *PeekNextKey();}

   /**
    * Returns a reference to the value this iterator is currently pointing at.
    * @note Be careful with this method, if this iterator isn't currently pointing at any value,
    *       it will return a NULL reference and your program will crash when you try to use it.
    *       Typically you would want to call this function only after checking to see that
    *       HasMoreValues() returns true.
    * @note The returned reference is only guaranteed to remain valid for as long as the Hashtable remains unchanged.
    */
   ValueType & GetValue() const {return *PeekNextValue();}

   /**
    * Iterates to get the next key in the key traversal.
    * @param setNextKey On success, the next key is copied into this object.
    * @return B_NO_ERROR on success, B_ERROR if there are no more keys left.
    */
   status_t GetNextKey(KeyType & setNextKey);

   /**
    * Iterates to get the next key in the key traversal.
    * @param setNextKeyPtr On success, this pointer is set to point to the next key in the traversal.
    * @return B_NO_ERROR on success, B_ERROR if there are no more keys left.
    */
   status_t GetNextKey(const KeyType * & setNextKeyPtr);

   /**
    * Iterates to get a pointer to the next key in the key traversal.
    * Note that the returned pointer is only guaranteed valid as long as the Hashtable remains unchanged.
    * @return A pointer to the next key in the key traversal, or NULL if there are no more keys left.
    */
   const KeyType * GetNextKey();

   /**
    * Peek at the next key without modifying the state of the traversal.
    * @param setKey On success, the next key is copied into this object.
    * @return B_NO_ERROR on success, or B_ERROR if there are no more keys left.
    */
   status_t PeekNextKey(KeyType & setKey) const;

   /**
    * Peek at the next key without modifying the state of the traversal.
    * @param setKey On success, the next key is copied into this object.
    * @return B_NO_ERROR on success, or B_ERROR if there are no more keys left.
    */
   status_t PeekNextKey(const KeyType * & setKey) const;

   /**
    * Peek at the next key without modifying the state of the traversal.
    * Note that the returned pointer is only guaranteed valid as long as the Hashtable remains unchanged.
    * @return a pointer to the next key in the key traversal, or NULL if there are no keys left.
    */
   const KeyType * PeekNextKey() const;

   /** Returns true iff there are more values left in the value traversal.  */
   bool HasMoreValues() const {return ((_nextValueCookie != NULL)||((_owner)&&(_owner->HasItems())&&(_flags & HTIT_INTERNAL_FLAG_RESTARTVALUE)));}

   /**
    * Iterates to get the next value in the values traversal.
    * @param setNextValue On success, the next value in the traversal is copied into this object.
    * @return B_NO_ERROR on success, B_ERROR if there are no more values left in the value traversal.
    */
   status_t GetNextValue(ValueType & setNextValue);

   /**
    * Iterates to get the next value in the values traversal.
    * @param setValuePtr On success, this pointer will be set to the next value in the traversal.
    * @return B_NO_ERROR on success, B_ERROR if there are no more values left in the value traversal.
    */
   status_t GetNextValue(ValueType * & setValuePtr);

   /**
    * Iterates to get the next value in the values traversal.
    * @param setValuePtr On success, this pointer will be set to the next value in the traversal.
    * @return B_NO_ERROR on success, B_ERROR if there are no more values left in the value traversal.
    */
   status_t GetNextValue(const ValueType * & setValuePtr);

   /**
    * Iterates to get the next value in the values traversal.
    * Note that the returned pointer is only guaranteed valid as long as the Hashtable remains unchanged.
    * @return a pointer to the next value in the value traversal, or NULL if there are no values left.
    */
   ValueType * GetNextValue();

   /**
    * Peek at the next value without modifying the state of the traversal.
    * @param setValue On success, the next value is copied into this object.
    * @return B_NO_ERROR on success, or B_ERROR if there are no more values left.
    */
   status_t PeekNextValue(ValueType & setValue) const;

   /**
    * Peek at the next value without modifying the state of the traversal.
    * @param setValue On success, this pointer is set to point to the next value in the traversal.
    * @return B_NO_ERROR on success, or B_ERROR if there are no more values left.
    */
   status_t PeekNextValue(const ValueType * & setValue) const;

   /**
    * Peek at the next value without modifying the state of the traversal.
    * @param setValue On success, this pointer is set to point to the next value in the traversal.
    * @return B_NO_ERROR on success, or B_ERROR if there are no more values left.
    */
   status_t PeekNextValue(ValueType * & setValue) const;

   /**
    * Peek at the next value without modifying the state of the traversal.
    * Note that the returned pointer is only guaranteed valid as long as the Hashtable remains unchanged.
    * @return a pointer to the next value in the value traversal, or NULL if there are no values left.
    */
   ValueType * PeekNextValue() const;

   /** Convenience method -- equivalent to calling both GetNextKey() and GetNextValue(). 
     * @param setKey on success, this parameter will contain the next key in the iteration.
     * @param setValue on success, this parameter will contain the next value in the iteration.
     * @returns B_NO_ERROR if values were written into (setKey) and (setValue), or B_ERROR if the iteration is complete.
     */
   status_t GetNextKeyAndValue(KeyType & setKey, ValueType & setValue);

   /** Convenience method -- equivalent to calling both GetNextKey() and GetNextValue(). 
     * @param setKey on success, this parameter will contain the next key in the iteration.
     * @param setValuePtr on success, this pointer will point to the next value in the iteration.
     * @returns B_NO_ERROR if values were written into (setKey) and (setValue), or B_ERROR if the iteration is complete.
     */
   status_t GetNextKeyAndValue(KeyType & setKey, ValueType * & setValuePtr);

   /** Convenience method -- equivalent to calling both GetNextKey() and GetNextValue(). 
     * @param setKey on success, this parameter will contain the next key in the iteration.
     * @param setValuePtr on success, this pointer will point to the next value in the iteration.
     * @returns B_NO_ERROR if values were written into (setKey) and (setValue), or B_ERROR if the iteration is complete.
     */
   status_t GetNextKeyAndValue(KeyType & setKey, const ValueType * & setValuePtr);

   /** Convenience method -- equivalent to calling both GetNextKey() and GetNextValue(). 
     * @param setKeyPtr on success, this pointer will point to the next key in the iteration.
     * @param setValue on success, this parameter will contain the next value in the iteration.
     * @returns B_NO_ERROR if values were written into (setKey) and (setValue), or B_ERROR if the iteration is complete.
     */
   status_t GetNextKeyAndValue(const KeyType * & setKeyPtr, ValueType & setValue);

   /** Convenience method -- equivalent to calling both GetNextKey() and GetNextValue(). 
     * @param setKeyPtr on success, this pointer will point to the next key in the iteration.
     * @param setValuePtr on success, this pointer will point to the next value in the iteration.
     * @returns B_NO_ERROR if values were written into (setKey) and (setValue), or B_ERROR if the iteration is complete.
     */
   status_t GetNextKeyAndValue(const KeyType * & setKeyPtr, ValueType * & setValuePtr);

   /** Convenience method -- equivalent to calling both GetNextKey() and GetNextValue(). 
     * @param setKeyPtr on success, this pointer will point to the next key in the iteration.
     * @param setValuePtr on success, this pointer will point to the next value in the iteration.
     * @returns B_NO_ERROR if values were written into (setKey) and (setValue), or B_ERROR if the iteration is complete.
     */
   status_t GetNextKeyAndValue(const KeyType * & setKeyPtr, const ValueType * & setValuePtr);

   /** Returns this iterator's HTIT_FLAG_* bit-chord value. */
   uint32 GetFlags() const {return _flags;}

   /** Sets or unsets the HTIT_FLAG_BACWARDS flag on this iterator. 
     * @param backwards If true, this iterator will be set to iterate backwards from wherever it is currently; 
     *                  if false, this iterator will be set to iterate forwards from wherever it is currently.
     */
   void SetBackwards(bool backwards) {if (backwards) _flags |= HTIT_FLAG_BACKWARDS; else _flags &= ~HTIT_FLAG_BACKWARDS;}

   /** Returns true iff this iterator is set to iterate in reverse order -- i.e. if HTIT_FLAG_BACKWARDS
     * was passed in to the constructor, or if SetBackwards(true) was called.
     */
   bool IsBackwards() const {return ((_flags & HTIT_FLAG_BACKWARDS) != 0);}

private:
   enum {
      HTIT_INTERNAL_FLAG_RESTARTKEY   = (1<<31), // tells GetNextKey() not to iterate on the next iteration
      HTIT_INTERNAL_FLAG_RESTARTVALUE = (1<<30)  // tells GetNextValue() not to iterate on the next iteration
   };

   friend class HashtableBase<KeyType, ValueType, HashFunctorType>;

   void * _scratchSpace[2];   // ignore this; it's temp scratch space used by EnsureSize().

   void * _nextKeyCookie;
   void * _nextValueCookie;
   uint32 _flags;

   HashtableIterator<KeyType,   ValueType, HashFunctorType> * _prevIter; // for the doubly linked list so that the table can notify us if it is modified
   HashtableIterator<KeyType,   ValueType, HashFunctorType> * _nextIter; // for the doubly linked list so that the table can notify us if it is modified
   const HashtableBase<KeyType, ValueType, HashFunctorType> * _owner;    // table that we are associated with
};

// Monni says VC++6.0 can't handle empty initializer arguments, but I want to avoid a potential unnecessary item copy in newer compilers --jaf
#ifdef MUSCLE_USING_OLD_MICROSOFT_COMPILER
# define DEFAULT_MUSCLE_HASHTABLE_KEY_INITIALIZER   KeyType()
# define DEFAULT_MUSCLE_HASHTABLE_VALUE_INITIALIZER ValueType()
#else
# define DEFAULT_MUSCLE_HASHTABLE_KEY_INITIALIZER
# define DEFAULT_MUSCLE_HASHTABLE_VALUE_INITIALIZER
#endif

/** This internal superclass is an implementation detail and should not be instantiated directly.  Instantiate a Hashtable, OrderedKeysHashtable, or OrderedValuesHashtable instead. */
template<class KeyType, class ValueType, class HashFunctorType=typename DEFAULT_HASH_FUNCTOR(KeyType) > class HashtableBase
{
public:
   /** Returns the number of items stored in the table. */
   uint32 GetNumItems() const {return _count;}

   /** Convenience method;  Returns true iff the table is empty (i.e. if GetNumItems() is zero). */
   bool IsEmpty() const {return (_count == 0);}

   /** Convenience method;  Returns true iff the table is non-empty (i.e. if GetNumItems() is non-zero). */
   bool HasItems() const {return (_count > 0);}

   /** Returns true iff the table contains a mapping with the given key.  (O(1) search time) */
   bool ContainsKey(const KeyType & key) const {return (GetEntry(ComputeHash(key), key) != NULL);}

   /** Returns true iff the table contains a mapping with the given value.  (O(n) search time) */
   bool ContainsValue(const ValueType & value) const;

   /** Returns true iff this Hashtable has the same contents as (rhs).
     * @param rhs The table to compare this table against
     * @param considerOrdering Whether the order of the key/value pairs within the tables should be considered as part of "equality".  Defaults to false. 
     * @returns true iff (rhs) is equal to to (this), false if they differ.
     */
   bool IsEqualTo(const HashtableBase<KeyType, ValueType, HashFunctorType> & rhs, bool considerOrdering = false) const;

   /** Returns the given key's position in the hashtable's linked list, or -1 if the key wasn't found.  O(n) count time (if the key exists, O(1) if it doesn't) */
   int32 IndexOfKey(const KeyType & key) const;

   /** Returns the index of the first (or last) occurrance of (value), or -1 if (value) does
     * not exist in this Hashtable.  Note that the search is O(N).
     * @param value The value to search for.
     * @param searchBackwards If set to true, the table will be searched in reverse order.
     *                        and the index returned (if valid) will be the index of the
     *                        last instance of (value) in the table, rather than the first.
     * @returns The index into the table, or -1 if (value) doesn't exist in the table's set of values.
     */
   int32 IndexOfValue(const ValueType & value, bool searchBackwards = false) const;

   /** Attempts to retrieve the associated value from the table for a given key.  (O(1) lookup time)
    *  @param key The key to use to look up a value.
    *  @param setValue On success, the associated value is copied into this object.
    *  @return B_NO_ERROR on success, B_ERROR if their was no value found for the given key.
    */
   status_t GetValue(const KeyType & key, ValueType & setValue) const; 

   /** Retrieve a pointer to the associated value object for the given key.  (O(1) lookup time)
    *  @param key The key to use to look up a value.
    *  @return A pointer to the internally held value object for the given key,
    *          or NULL of no object was found.  Note that this object is only
    *          guaranteed to remain valid as long as the Hashtable remains unchanged.
    */
   ValueType * GetValue(const KeyType & key) const;

   /** Given a lookup key, returns the a copy of the actual key as held by the table.
    *  This method is only useful in rare situations where the hashing or comparison
    *  functions are such that lookupKeys and held keys are not guaranteed equivalent.
    *  @param lookupKey The key used to look up the held key object.
    *  @param setKey On success, the actual key held in the table is placed here.
    *  @return B_NO_ERROR on success, or B_ERROR on failure.
    */
   status_t GetKey(const KeyType & lookupKey, KeyType & setKey) const;

   /** Given a key, returns a pointer to the actual key object in this table that matches
    *  that key, or NULL if there is no matching key.  This method is only useful in rare
    *  situations where the hashing or comparison functions are such that lookup keys and
    *  held keys are not guaranteed equivalent.
    *  @param lookupKey The key used to look up the key object
    *  @return A pointer to an internally held key object on success, or NULL on failure.
    */
   const KeyType * GetKey(const KeyType & lookupKey) const;

   /** The iterator type that goes with this HashtableBase type */
   typedef HashtableIterator<KeyType,ValueType,HashFunctorType> IteratorType;

   /** Get an iterator for use with this table.
     * @param flags A bit-chord of HTIT_FLAG_* constants (see above).  Defaults to zero, for default behaviour.
     * @return an iterator object that can be used to examine the items in the hash table, starting at
     *         the specified key.  If the specified key is not in this table, an empty iterator will be returned.
     */
   IteratorType GetIterator(uint32 flags = 0) const {return IteratorType(*this, flags);}

   /** Get an iterator for use with this table, starting at the given entry.
     * @param startAt The key in this table to start the iteration at.
     * @param flags A bit-chord of HTIT_FLAG_* constants (see above).  Defaults to zero, for default behaviour.
     * @return an iterator object that can be used to examine the items in the hash table, starting at
     *         the specified key.  If the specified key is not in this table, an empty iterator will be returned.
     */
   IteratorType GetIteratorAt(const KeyType & startAt, uint32 flags = 0) const 
   {
      return HashtableIterator<KeyType,ValueType,HashFunctorType>(*this, startAt, flags);
   }

   /** Returns a pointer to the (index)'th key in this Hashtable.
    *  (This Hashtable class keeps its entries in a well-defined order)
    *  Note that this method is an O(N) operation, so for iteration, always use GetIterator() instead.
    *  @param index Index of the key to return a pointer to.  Should be in the range [0, GetNumItems()-1].
    *  @returns Pointer to the key at position (index) on success, or NULL on failure (bad index?)
    */
   const KeyType * GetKeyAt(uint32 index) const;

   /** Returns the (index)'th key in this Hashtable.
    *  (This Hashtable class keeps its entries in a well-defined order)
    *  Note that this method is an O(N) operation, so for iteration, always use GetIterator() instead.
    *  @param index Index of the key to return a pointer to.  Should be in the range [0, GetNumItems()-1].
    *  @param retKey On success, the contents of the (index)'th key will be written into this object.
    *  @return B_NO_ERROR on success, or B_ERROR on failure.
    */
   status_t GetKeyAt(uint32 index, KeyType & retKey) const;

   /** Removes a mapping from the table.  (O(1) removal time)
    *  @param key The key of the key-value mapping to remove.
    *  @return B_NO_ERROR if a key was found and the mapping removed, or B_ERROR if the key wasn't found.
    */
   status_t Remove(const KeyType & key) {return RemoveAux(key, NULL);}

   /** Removes the mapping with the given (key) and places its value into (setRemovedValue).  (O(1) removal time)
    *  @param key The key of the key-value mapping to remove.
    *  @param setRemovedValue On success, the removed value is copied into this object.
    *  @return B_NO_ERROR if a key was found and the mapping removed, or B_ERROR if the key wasn't found.
    */
   status_t Remove(const KeyType & key, ValueType & setRemovedValue) {return RemoveAux(key, &setRemovedValue);}

   /** Convenience method:  For each key in the passed-in-table, removes that key (and its associated value) from this table. 
     * @param pairs A table containing keys that should be removed from this table.
     * @returns the number of items actually removed from this table.
     */
   uint32 Remove(const HashtableBase & pairs);

   /** Convenience method:  Removes from this Hashtable all key/value pairs for which the same key is not present in (pairs)
     * @returns the number of items actually removed from this table.
     */
   uint32 Intersect(const HashtableBase & pairs);

   /** Convenience method:  Removes the first key/value mapping in the table.  (O(1) removal time)
    *  @return B_NO_ERROR if the first mapping was removed, or B_ERROR if this table was empty.
    */
   status_t RemoveFirst() {return _iterHead ? RemoveEntry(_iterHead, NULL) : B_ERROR;}

   /** Convenience method:  Removes the first key/value mapping in the table and places the removed key
    *  into (setRemovedKey).  (O(1) removal time)
    *  @param setRemovedKey On success, the removed key is copied into this object.
    *  @return B_NO_ERROR if the first mapping was removed and the key placed into (setRemovedKey), or B_ERROR if the table was empty.
    */
   status_t RemoveFirst(KeyType & setRemovedKey) 
   {
      if (_iterHead == NULL) return B_ERROR;
      setRemovedKey = _iterHead->_key;
      return RemoveEntry(_iterHead, NULL);
   }

   /** Convenience method:  Removes the first key/value mapping in the table and places its 
    *  key into (setRemovedKey) and its value into (setRemovedValue).  (O(1) removal time)
    *  @param setRemovedKey On success, the removed key is copied into this object.
    *  @param setRemovedValue On success, the removed value is copied into this object.
    *  @return B_NO_ERROR if the first mapping was removed and the key and value placed into the arguments, or B_ERROR if the table was empty.
    */
   status_t RemoveFirst(KeyType & setRemovedKey, ValueType & setRemovedValue) 
   {
      if (_iterHead == NULL) return B_ERROR;
      setRemovedKey = _iterHead->_key;
      return RemoveEntry(_iterHead, &setRemovedValue);
   }

   /** Convenience method:  Removes the last key/value mapping in the table.  (O(1) removal time)
    *  @return B_NO_ERROR if the last mapping was removed, or B_ERROR if the table was empty.
    */
   status_t RemoveLast() {return _iterTail ? RemoveEntry(_iterTail, NULL) : B_ERROR;}

   /** Convenience method:  Removes the last key/value mapping in the table and places the removed key
    *  into (setRemovedKey).  (O(1) removal time)
    *  @param setRemovedKey On success, the removed key is copied into this object.
    *  @return B_NO_ERROR if the last mapping was removed and the key placed into (setRemovedKey), or B_ERROR if the table was empty.
    */
   status_t RemoveLast(KeyType & setRemovedKey) 
   {
      if (_iterTail == NULL) return B_ERROR;
      setRemovedKey = _iterTail->_key;
      return RemoveEntry(_iterTail, NULL);
   }

   /** Convenience method:  Removes the last key/value mapping in the table and places its 
    *  key into (setRemovedKey) and its value into (setRemovedValue).  (O(1) removal time)
    *  @param setRemovedKey On success, the removed key is copied into this object.
    *  @param setRemovedValue On success, the removed value is copied into this object.
    *  @return B_NO_ERROR if the last mapping was removed and the key and value placed into the arguments, or B_ERROR if the table was empty.
    */
   status_t RemoveLast(KeyType & setRemovedKey, ValueType & setRemovedValue) 
   {
      if (_iterTail == NULL) return B_ERROR;
      setRemovedKey = _iterTail->_key;
      return RemoveEntry(_iterTail, &setRemovedValue);
   }

   /** Removes all mappings from the hash table.  (O(N) clear time)
    *  @param releaseCachedData If set true, we will immediately free any buffers we may contain.  
    *                           Otherwise, we'll keep them around in case they can be re-used later on.
    */
   void Clear(bool releaseCachedData = false);

   /** Sorts the iteration-traversal list of this table by key, using the given key-comparison functor object.
     * This method uses a very efficient O(log(N)) MergeSort algorithm.
     * @param func The key-comparison functor to use.
     * @param optCompareCookie Optional cookie value to pass to func.Compare().  Its meaning is specific to the functor type.
     */
   template<class KeyCompareFunctorType> void SortByKey(const KeyCompareFunctorType & func, void * optCompareCookie = NULL);

   /** As above, except that the comparison functor is not specified.  The default comparison functor for our key type will be used.
     * @param optCompareCookie Optional cookie value to pass to func.Compare().  Its meaning is specific to the functor type.
     */
   void SortByKey(void * optCompareCookie = NULL) {SortByKey(CompareFunctor<KeyType>(), optCompareCookie);}

   /** Sorts the iteration-traversal list of this table by value, using the given value-comparison functor object.
     * This method uses a very efficient O(log(N)) MergeSort algorithm.
     * @param func The value-comparison functor to use.
     * @param optCompareCookie Optional cookie value to pass to func.Compare().  Its meaning is specific to the functor type.
     */
   template<class ValueCompareFunctorType> void SortByValue(const ValueCompareFunctorType & func, void * optCompareCookie = NULL);

   /** As above, except that the comparison functor is not specified.  The default comparison functor for our value type will be used.
     * @param optCompareCookie Optional cookie value to pass to func.Compare().  Its meaning is specific to the functor type.
     */
   void SortByValue(void * optCompareCookie = NULL) {SortByValue(CompareFunctor<ValueType>(), optCompareCookie);}

   /** Returns true iff auto-sort is currently enabled on this Hashtable.  Note that auto-sort only has an effect on
     * OrderedKeysHashtable and OrderedValuesHashtable objects;  for plain Hashtable objects it has no effect.
     */
   bool GetAutoSortEnabled() const {return _autoSortEnabled;}

   /** This method can be used to set the "cookie value" that will be passed in to the comparison functor
     * objects that this class is templated on.  The functor objects can then use this value to access
     * any context information that might be necessary to do their comparison.
     * This value will be passed along during whenever a user-defined compare functor is called implicitly.
     */
   void SetCompareCookie(void * cookie) {_compareCookie = cookie;}

   /** Returns the current comparison cookie, as previously set by SetCompareCookie().  Default value is NULL. */
   void * GetCompareCookie() const {return _compareCookie;}

   /** This method doesn an efficient zero-copy swap of this hash table's contents with those of (swapMe).  
    *  That is to say, when this method returns, (swapMe) will be identical to the old state of this 
    *  Hashtable, and this Hashtable will be identical to the old state of (swapMe). 
    *  Any active iterators present for either table will swap owners also, becoming associated with the other table.
    *  @param swapMe The table whose contents and iterators are to be swapped with this table's.
    */
   void SwapContents(HashtableBase & swapMe);

   /** Moves the given entry to the head of the HashtableIterator traversal sequence.
     * Note that calling this method is generally a bad idea of the table is in auto-sort mode,
     * as it is likely to unsort the traversal ordering and thus break auto-sorting.  However,
     * calling Sort() will restore the sort-order and make auto-sorting work again)
     * @param moveMe Key of the item to be moved to the front of the sequence.
     * @return B_NO_ERROR on success, or B_ERROR if (moveMe) was not found in the table.
     */
   status_t MoveToFront(const KeyType & moveMe);

   /** Moves the given entry to the tail of the HashtableIterator traversal sequence.
     * Note that calling this method is generally a bad idea of the table is in auto-sort mode,
     * as it is likely to unsort the traversal ordering and thus break auto-sorting.  However,
     * calling Sort() will restore the sort-order and make auto-sorting work again)
     * @param moveMe Key of the item to be moved to the end of the sequence.
     * @return B_NO_ERROR on success, or B_ERROR if (moveMe) was not found in the table.
     */
   status_t MoveToBack(const KeyType & moveMe);

   /** Moves the given entry to the spot just in front of the other specified entry in the 
     * HashtableIterator traversal sequence.
     * Note that calling this method is generally a bad idea of the table is in auto-sort mode,
     * as it is likely to unsort the traversal ordering and thus break auto-sorting.  However,
     * calling Sort() will restore the sort-order and make auto-sorting work again)
     * @param moveMe Key of the item to be moved.
     * @param toBeforeMe Key of the item that (moveMe) should be placed in front of.
     * @return B_NO_ERROR on success, or B_ERROR if (moveMe) was not found in the table, 
     *         or was the same item as (toBeforeMe).
     */
   status_t MoveToBefore(const KeyType & moveMe, const KeyType & toBeforeMe);

   /** Moves the given entry to the spot just behind the other specified entry in the 
     * HashtableIterator traversal sequence.
     * Note that calling this method is generally a bad idea of the table is in auto-sort mode,
     * as it is likely to unsort the traversal ordering and thus break auto-sorting.  However,
     * calling Sort() will restore the sort-order and make auto-sorting work again)
     * @param moveMe Key of the item to be moved.
     * @param toBehindMe Key of the item that (moveMe) should be placed behind.
     * @return B_NO_ERROR on success, or B_ERROR if (moveMe) was not found in the table, 
     *         or was the same item as (toBehindMe).
     */
   status_t MoveToBehind(const KeyType & moveMe, const KeyType & toBehindMe);

   /** Convenience synonym for GetValue() */
   status_t Get(const KeyType & key, ValueType & setValue) const {return GetValue(key, setValue);}

   /** Convenience synonym for GetValue() */
   ValueType * Get(const KeyType & key) const {return GetValue(key);}

   /** Similar to Get(), except that if the specified key is not found,
     * the ValueType's default value is returned.
     * @param key The key whose value should be returned.
     * @returns (key)'s associated value, or the default ValueType value.
     */
   const ValueType & GetWithDefault(const KeyType & key) const
   {
      const ValueType * v = Get(key);
      return v ? *v : GetDefaultValue();
   }

   /** Similar to Get(), except that if the specified key is not found,
     * the specified default value is returned.  Note that this method
     * returns its value by value, not by reference, to avoid any
     * dangling-reference issues that might occur if (defaultValue)
     * was a temporary.
     * @param key The key whose value should be returned.
     * @param defaultValue The value to return is (key) is not in the table.
     *                     Defaults to a default-constructed item of the value type.
     * @returns (key)'s associated value, or (defaultValue).
     */
   ValueType GetWithDefault(const KeyType & key, const ValueType & defaultValue) const
   {
      const ValueType * v = Get(key);
      return v ? *v : defaultValue;
   }

   /** Convenience method.  Returns a pointer to the first key in our iteration list, or NULL if the table is empty. */
   const KeyType * GetFirstKey() const {return _iterHead ? &_iterHead->_key : NULL;}

   /** Convenience method.  Returns a pointer to the last key in our iteration list, or NULL if the table is empty. */
   const KeyType * GetLastKey() const {return _iterTail ? &_iterTail->_key : NULL;}

   /** Convenience method.  Returns a pointer to the last value in our iteration list, or NULL if the table is empty. */
   ValueType * GetFirstValue() const {return _iterHead ? &_iterHead->_value : NULL;}

   /** Convenience method.  Returns a pointer to the last value in our iteration list, or NULL if the table is empty. */
   ValueType * GetLastValue() const {return _iterTail ? &_iterTail->_value : NULL;}

   /** Returns the number of table-slots that we currently have allocated.  Since we often
    *  pre-allocate slots to avoid unnecessary reallocations, this number will usually be
    *  greater than the value returned by GetNumItems().  It will never be less than that value.
    */
   uint32 GetNumAllocatedSlots() const {return _tableSize;}

   /** Returns a reference to a default-constructed Key item.  The reference will remain valid for as long as this Hashtable is valid. */
   const KeyType & GetDefaultKey() const {return _defaultKey;}

   /** Returns a reference to a default-constructed Value item.  The reference will remain valid for as long as this Hashtable is valid. */
   const ValueType & GetDefaultValue() const {return _defaultValue;}

protected:
   HashtableBase(uint32 tableSize, bool autoSortEnabled, void * cookie) : _count(0), _tableSize(tableSize), _table(NULL), _iterHead(NULL), _iterTail(NULL), _freeHead(NULL), _autoSortEnabled(autoSortEnabled), _compareCookie(cookie), _iterList(NULL), _defaultKey(DEFAULT_MUSCLE_HASHTABLE_KEY_INITIALIZER), _defaultValue(DEFAULT_MUSCLE_HASHTABLE_VALUE_INITIALIZER) {/* empty */}
   ~HashtableBase() {Clear(true);}

   void CopyFromAux(const HashtableBase<KeyType, ValueType, HashFunctorType> & rhs)
   {
      bool wasEmpty = IsEmpty();
      const HashtableEntry * e = rhs._iterHead;  // start of linked list to iterate through
      while(e)
      {
         HashtableEntry * myEntry = wasEmpty ? NULL : GetEntry(e->_hash, e->_key);
         if (myEntry) myEntry->_value = e->_value;
         else
         { 
            InsertIterationEntry(PutAuxAux(e->_hash, e->_key, e->_value), _iterTail);
            this->_count++;
         }
         e = e->_iterNext;
      }
   }

   friend class HashtableIterator<KeyType, ValueType, HashFunctorType>;

   void InitializeIterator(IteratorType & iter) const
   {
      RegisterIterator(&iter);
      iter._nextKeyCookie = iter._nextValueCookie = (iter._flags & HTIT_FLAG_BACKWARDS) ? _iterTail : _iterHead;
   }

   void InitializeIteratorAt(IteratorType & iter, const KeyType & startAt) const
   {
      RegisterIterator(&iter);
      iter._nextKeyCookie = iter._nextValueCookie = GetEntry(ComputeHash(startAt), startAt);
   }

   // These ugly methods are here to expose the HashtableIterator's privates to the Hashtable class without exposing them to the rest of the world
   void * GetIteratorScratchSpace(const IteratorType & iter, int i) const {return iter._scratchSpace[i];}
   void SetIteratorScratchSpace(IteratorType & iter, int i, void * val) const {iter._scratchSpace[i] = val;}
   void * GetIteratorNextKeyCookie(const IteratorType & iter) const {return iter._nextKeyCookie;}
   void SetIteratorNextKeyCookie(IteratorType & iter, void * val) const {iter._nextKeyCookie = val;}
   void * GetIteratorNextValueCookie(const IteratorType & iter) const {return iter._nextValueCookie;}
   void SetIteratorNextValueCookie(IteratorType & iter, void * val) const {iter._nextValueCookie = val;}
   IteratorType * GetIteratorNextIterator(const IteratorType & iter) const {return iter._nextIter;}

   /** This class is an implementation detail, please ignore it.  Do not access it directly. */
   class HashtableEntry
   {
   public:
      // Note:  _bucketPrev and _bucketNext are intentionally not set here
      HashtableEntry() : _hash(MUSCLE_HASHTABLE_INVALID_HASH_CODE), _iterPrev(NULL), _iterNext(NULL)
#ifndef _MSC_VER
                         , _mapTo(this), _mappedFrom(this)  // this is slightly more efficient...
#endif
      {
#ifdef _MSC_VER
         _mapTo      = this;  // ... but VC++ complains if (this) is in the init list, so for VC++ we do it here
         _mappedFrom = this;
#endif
      }

      ~HashtableEntry() {/* empty */}

      /** Returns this entry to the free-list, and resets its key and value to their default values. */
      void PushToFreeList(const KeyType & defaultKey, const ValueType & defaultValue, HashtableEntry * & getRetFreeHead)
      {
         _iterPrev = _iterNext = _bucketPrev = NULL;
         _bucketNext = getRetFreeHead; 
         if (_bucketNext) _bucketNext->_bucketPrev = this;
         getRetFreeHead = this;

         _hash  = MUSCLE_HASHTABLE_INVALID_HASH_CODE;
         _key   = defaultKey;    // NOTE:  These lines could have side-effects due to code in the templatized
         _value = defaultValue;  //        classes!  So it's important that the Hashtable be in a consistent state here
      }

      /** Removes this entry from the free list, so that we are ready for use.
        * @param freeHead current head of the free list
        * @returns the new head of the free list
        */
      HashtableEntry * PopFromFreeList(HashtableEntry * freeHead) 
      {
         if (_bucketNext) _bucketNext->_bucketPrev = _bucketPrev;
         if (_bucketPrev) _bucketPrev->_bucketNext = _bucketNext;
         HashtableEntry * ret = (freeHead == this) ? _bucketNext : freeHead;
         _bucketPrev = _bucketNext = NULL;
         return ret;
      }

      void SwapMaps(HashtableEntry * e)
      {
         muscleSwap(e->_mapTo, _mapTo);
         _mapTo->_mappedFrom = this;
         e->_mapTo->_mappedFrom = e;
      }

      inline HashtableEntry * GetMapTo() const {return _mapTo;}
      inline HashtableEntry * GetMappedFrom() const {return _mappedFrom;}

      static HashtableEntry * CreateEntriesArray(uint32 size)
      {
         HashtableEntry * ret = newnothrow_array(HashtableEntry,size);
         if (ret)
         {
            HashtableEntry * prev = NULL;
            for (uint32 i=0; i<size; i++) 
            {
               HashtableEntry * cur = &ret[i];
               cur->_bucketPrev = prev;
               if (prev) prev->_bucketNext = cur;
               prev = cur;
            }
            ret[size-1]._bucketNext = NULL;
         }
         else WARN_OUT_OF_MEMORY;
         return ret;
      }
  
      uint32 _hash;                // precalculated for efficiency
      KeyType _key;                // used for '==' checking
      ValueType _value;            // payload
      HashtableEntry* _bucketPrev; // for making linked lists in our bucket
      HashtableEntry* _bucketNext; // for making linked lists in our bucket
      HashtableEntry* _iterPrev;   // for user's table iteration
      HashtableEntry* _iterNext;   // for user's table iteration

   private:
      HashtableEntry* _mapTo;
      HashtableEntry* _mappedFrom;
   };

   HashtableEntry * PutAuxAux(uint32 hash, const KeyType & key, const ValueType & value);
   status_t RemoveAux(uint32 hash, const KeyType & key, ValueType * optSetValue)
   {
      HashtableEntry * e = GetEntry(hash, key);
      return e ? RemoveEntry(e, optSetValue) : B_ERROR;
   }
   status_t RemoveAux(const KeyType & key, ValueType * optSetValue)
   {
      HashtableEntry * e = GetEntry(ComputeHash(key), key);
      return e ? RemoveEntry(e, optSetValue) : B_ERROR;
   }
   status_t RemoveEntry(HashtableEntry * e, ValueType * optSetValue);

   inline uint32 ComputeHash(const KeyType & key) const
   {
      uint32 ret = _hashFunctor(key);
      if (ret == MUSCLE_HASHTABLE_INVALID_HASH_CODE) ret++;  // avoid using the guard value as a hash code (unlikely but possible)
      return ret;
   }

   inline bool AreKeysEqual(const KeyType & k1, const KeyType & k2) const
   {
      return _hashFunctor.AreKeysEqual(k1, k2);
   }

   void InsertIterationEntry(HashtableEntry * e, HashtableEntry * optBehindThisOne);
   void RemoveIterationEntry(HashtableEntry * e);
   HashtableEntry * GetEntry(uint32 hash, const KeyType & key) const;
   HashtableEntry * GetEntryAt(uint32 idx) const;
   bool IsBucketHead(const HashtableEntry * e) const {return ((e->_hash != MUSCLE_HASHTABLE_INVALID_HASH_CODE)&&(_table[e->_hash%_tableSize].GetMapTo() == e));}

   // HashtableIterator's private API
   void RegisterIterator(IteratorType * iter) const
   {
      if (iter->_flags & HTIT_FLAG_NOREGISTER) iter->_prevIter = iter->_nextIter = NULL;
      else
      {
         // add him to the head of our linked list of iterators
         iter->_prevIter = NULL;
         iter->_nextIter = _iterList;  
         if (_iterList) _iterList->_prevIter = iter;
         _iterList = iter;
      }
   }

   void UnregisterIterator(IteratorType * iter) const
   {
      if (iter->_flags & HTIT_FLAG_NOREGISTER) iter->_prevIter = iter->_nextIter = NULL;
      else
      {
         if (iter->_prevIter) iter->_prevIter->_nextIter = iter->_nextIter;
         if (iter->_nextIter) iter->_nextIter->_prevIter = iter->_prevIter;
         if (iter == _iterList) _iterList = iter->_nextIter;
         iter->_prevIter = iter->_nextIter = NULL;
      }
   }

   KeyType * GetKeyFromCookie(void * c) const {return c ? &(((HashtableEntry *)c)->_key) : NULL;}
   ValueType * GetValueFromCookie(void * c) const  {return c ? &(((HashtableEntry *)c)->_value) : NULL;}

   HashtableEntry * GetInitialEntry(uint32 flags) const {return (flags & HTIT_FLAG_BACKWARDS) ? _iterTail : _iterHead;}
   HashtableEntry * GetSubsequentEntry(void * entryPtr, uint32 flags) const 
   {
      HashtableEntry * ep = static_cast<HashtableEntry *>(entryPtr);
      return ep ? ((flags & HTIT_FLAG_BACKWARDS) ? ep->_iterPrev : ep->_iterNext) : NULL;
   }

   void MoveToBackAux(HashtableEntry * moveMe)
   {
      if (moveMe->_iterNext)
      {
         RemoveIterationEntry(moveMe);
         InsertIterationEntry(moveMe, _iterTail);
      }
   }

   void MoveToFrontAux(HashtableEntry * moveMe)
   {
      if (moveMe->_iterPrev)
      {
         RemoveIterationEntry(moveMe);
         InsertIterationEntry(moveMe, NULL);
      }
   }

   void MoveToBeforeAux(HashtableEntry * moveMe, HashtableEntry * toBeforeMe)
   {
      if (moveMe->_iterNext != toBeforeMe)
      {
         RemoveIterationEntry(moveMe);
         InsertIterationEntry(moveMe, toBeforeMe->_iterPrev);
      }
   }

   void MoveToBehindAux(HashtableEntry * moveMe, HashtableEntry * toBehindMe)
   {
      if (moveMe->_iterPrev != toBehindMe)
      {
         RemoveIterationEntry(moveMe);
         InsertIterationEntry(moveMe, toBehindMe);
      }
   }

   HashFunctorType _hashFunctor;  // used to compute hash codes for key objects

   uint32 _count;       // the number of valid elements in the hashtable
   uint32 _tableSize;   // the number of entries in _table (or the number to allocate if _table is NULL)

   HashtableEntry * _table;       // our array of table entries
   HashtableEntry * _iterHead;    // start of linked list to iterate through
   HashtableEntry * _iterTail;    // end of linked list to iterate through
   HashtableEntry * _freeHead;    // head of the list of unused HashtableEntries in our _table array

   bool _autoSortEnabled;  // only used in the ordered Hashtable subclasses, but kept here so that its state can be swapped by SwapContents(), etc.
   void * _compareCookie;

   mutable IteratorType * _iterList;  // list of existing iterators for this table

   // These always remain in their default state, and are kept around so that we can
   // reset other objects with them, without having to construct or destroy any temporaries
   const KeyType _defaultKey;
   const ValueType _defaultValue;
};

/** This internal superclass is an implementation detail and should not be instantiated directly.  Instantiate a Hashtable, OrderedKeysHashtable, or OrderedValuesHashtable instead. */
template <class KeyType, class ValueType, class HashFunctorType, class SubclassType> class HashtableMid : public HashtableBase<KeyType, ValueType, HashFunctorType>
{
public:
   /** The iterator type that goes with this HashtableMid type */
   typedef HashtableIterator<KeyType,ValueType,HashFunctorType> IteratorType;

   /** Equality operator.  Returns true iff both hash tables contains the same set of keys and values.
     * Note that the ordering of the keys is NOT taken into account!
     */
   bool operator== (const HashtableMid & rhs) const {return IsEqualTo(rhs, false);}

   /** Templated psuedo-equality operator.  Returns true iff both hash tables contains the same set of keys and values.
     * Note that the ordering of the keys is NOT taken into account!
     */
   template<class RHSHashFunctorType,class RHSSubclassType> bool operator== (const HashtableMid<KeyType,ValueType,RHSHashFunctorType,RHSSubclassType> & rhs) const {return IsEqualTo(rhs, false);}

   /** Returns true iff the contents of this table differ from the contents of (rhs).
     * Note that the ordering of the keys is NOT taken into account!
     */
   bool operator!= (const HashtableMid & rhs) const {return !IsEqualTo(rhs, false);}

   /** Templated psuedo-inequality operator.  Returns false iff both hash tables contains the same set of keys and values.
     * Note that the ordering of the keys is NOT taken into account!
     */
   template<class RHSHashFunctorType,class RHSSubclassType> bool operator!= (const HashtableMid<KeyType,ValueType,RHSHashFunctorType,RHSSubclassType> & rhs) const {return !IsEqualTo(rhs, false);}

   /** Makes this table into a copy of a table passed in as an argument.
     * @param rhs The HashtableMid to make this HashtableMid a copy of.  Note that only (rhs)'s items are
     *            copied in; other settings such as sort mode and key/value cookies are not copied in.
     * @param clearFirst If true, this HashtableMid will be cleared before the contents of (rhs) are copied
     *                   into it.  Otherwise, the contents of (rhs) will be copied in on top of the current contents.
     * @returns B_NO_ERROR on success, or B_ERROR on failure.
     */
   template<class RHSHashFunctorType> status_t CopyFrom(const HashtableBase<KeyType,ValueType,RHSHashFunctorType> & rhs, bool clearFirst = true);

   /** Places the given (key, value) mapping into the table.  Any previous entry with a key of (key) will be replaced.  
    *  (average O(1) insertion time, unless auto-sorting is enabled, in which case it becomes O(N) insertion time for
    *  keys that are not already in the table)
    *  @param key The key that the new value is to be associated with.
    *  @param value The value to associate with the new key.
    *  @param setPreviousValue If there was a previously existing value associated with (key), it will be copied into this object.
    *  @param optSetReplaced If set non-NULL, this boolean will be set to true if (setPreviousValue) was written into, false otherwise.
    *  @return B_NO_ERROR If the operation succeeded, B_ERROR if it failed (out of memory?)
    */
   status_t Put(const KeyType & key, const ValueType & value, ValueType & setPreviousValue, bool * optSetReplaced = NULL) {return (PutAux(ComputeHash(key), key, value, &setPreviousValue, optSetReplaced) != NULL) ? B_NO_ERROR : B_ERROR;}

   /** Places the given (key, value) mapping into the table.  Any previous entry with a key of (key) will be replaced. 
    *  (average O(1) insertion time, unless auto-sorting is enabled, in which case it becomes O(N) insertion time for
    *  keys that are not already in the table)
    *  @param key The key that the new value is to be associated with.
    *  @param value The value to associate with the new key.
    *  @return B_NO_ERROR If the operation succeeded, B_ERROR if it failed (out of memory?)
    */
   status_t Put(const KeyType & key, const ValueType & value) {return (PutAux(ComputeHash(key), key, value, NULL, NULL) != NULL) ? B_NO_ERROR : B_ERROR;}

   /** Convenience method:  For each key/value pair in the passed-in-table, Put()'s that
     * key/value pair into this table.  Any existing items in this table with the same
     * key as any in (pairs) will be overwritten.
     * @param pairs A table full of items to Put() into this table.
     * @returns B_NO_ERROR on success, or B_ERROR on failue (out of memory?)
     */
   template<class RHSHashFunctorType, class RHSSubclassType> status_t Put(const HashtableMid<KeyType, ValueType, RHSHashFunctorType, RHSSubclassType> & pairs) {return CopyFrom(pairs, false);}

   /** Convenience method -- returns a pointer to the value specified by (key),
    *  or if no such value exists, it will Put() a (key,value) pair in the HashtableMid,
    *  and then return a pointer to the newly-placed value.  Returns NULL on
    *  error only (out of memory?)
    *  @param key The key to look for a value with
    *  @param defValue The value to auto-place in the HashtableMid if (key) isn't found.
    *  @returns a Pointer to the retrieved or placed value.
    */
   ValueType * GetOrPut(const KeyType & key, const ValueType & defValue)
   {
      uint32 hash = ComputeHash(key);
      typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = GetEntry(hash, key);
      if (e == NULL) e = PutAux(hash, key, defValue, NULL, NULL);
      return e ? &e->_value : NULL;
   }

   /** Convenience method -- returns a pointer to the value specified by (key),
    *  or if no such value exists, it will Put() a (key,value) pair in the HashtableMid,
    *  using a default value (as specified by ValueType's default constructor)
    * and then return a pointer to the newly-placed value.  Returns NULL on
    *  error only (out of memory?)
    *  @param key The key to look for a value with
    *  @returns a Pointer to the retrieved or placed value.
    */
   ValueType * GetOrPut(const KeyType & key)
   {
      uint32 hash = ComputeHash(key);
      typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = GetEntry(hash, key);
      if (e == NULL) e = PutAux(hash, key, this->_defaultValue, NULL, NULL);
      return e ? &e->_value : NULL;
   }

   /** Places the given (key, value) mapping into the table.  Any previous entry with a key of (key) will be replaced. 
    *  (average O(1) insertion time, unless auto-sorting is enabled, in which case it becomes O(N) insertion time for
    *  keys that are not already in the table)
    *  @param key The key that the new value is to be associated with.
    *  @param value The value to associate with the new key.  If not specified, a value object created using
    *               the default constructor will be placed by default.
    *  @return A pointer to the value object in the table on success, or NULL on failure (out of memory?)
    */
   ValueType * PutAndGet(const KeyType & key, const ValueType & value) 
   { 
       typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = PutAux(ComputeHash(key), key, value, NULL, NULL);
       return e ? &e->_value : NULL;
   }

   /** As above, except that a default value is placed into the table and returned. 
     * @param key The key that the new value is to be associated with.
     * @return A pointer to the value object in the table on success, or NULL on failure (out of memory?)
     */
   ValueType * PutAndGet(const KeyType & key) {return PutAndGet(key, this->_defaultValue);}

   /** Convenience method:  If (value) is the different from (defaultValue), then (key/value) is placed into the table and a pointer
     *                      to the placed value object is returned.
     *                      If (value) is equal to (defaultValue), on the other hand, (key) will be removed from the table, and NULL will be returned.
     * @param key The key value to affect.
     * @param value The value to possibly place into the table.
     * @param defaultValue The value to compare (value) with to decide whether to Put() or Remove() the key.
     * @returns A pointer to the placed value if a value was placed, or a NULL pointer if the value was removed (or on error).
     */
   ValueType * PutOrRemove(const KeyType & key, const ValueType & value, const ValueType & defaultValue)
   {
      if (value == defaultValue)
      {
         (void) Remove(key);
         return NULL;
      }
      else return PutAndGet(key, value);
   }

   /** As above, except no (defaultValue) is specified.  The default-constructed ValueType is assumed.
     * @param key The key value to affect.
     * @param value The value to possibly place into the table.
     * @returns A pointer to the placed value if a value was placed, or a NULL pointer if the value was removed (or on out of memory)
     */
   ValueType * PutOrRemove(const KeyType & key, const ValueType & value)
   {
      if (value == this->_defaultValue)
      {
         (void) Remove(key);
         return NULL;
      }
      else return PutAndGet(key, value);
   }

   /** If (optValue) is non-NULL, this call places the specified key/value pair into the table.
     * If (optValue) is NULL, this call removes the key/value pair from the table.
     * @param key The key value to affect.
     * @param optValue A pointer to the value to place into the table, or a NULL pointer to remove the key/value pair of the specified key.
     * @returns A pointer to the placed value if a value was placed, or a NULL pointer if the value was removed (or on out of memory)
     */
   ValueType * PutOrRemove(const KeyType & key, const ValueType * optValue)
   {
      if (optValue) return PutAndGet(key, *optValue);
      else
      {
         (void) Remove(key);
         return NULL;
      }
   }

   /** Convenience method.  If the given key already exists in the Hashtable, this method returns NULL.
    *  Otherwise, this method puts a copy of specified value into the table and returns a pointer to the 
    *  just-placed value.
    *  (average O(1) insertion time, unless auto-sorting is enabled, in which case it becomes O(N) insertion time)
    *  @param key The key that the new value is to be associated with.
    *  @param value The value to associate with the new key.
    *  @return A pointer to the value object in the table on success, or NULL on failure (key already exists, out of memory)
    */
   ValueType * PutIfNotAlreadyPresent(const KeyType & key, const ValueType & value) 
   {
      uint32 hash = ComputeHash(key);
      typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = GetEntry(hash, key);
      if (e) return NULL;
      else
      {
         e = PutAux(hash, key, value, NULL, NULL);
         return e ? &e->_value : NULL;
      }
   }

   /** Convenience method.  If the given key already exists in the Hashtable, this method returns NULL.
    *  Otherwise, this method puts a copy of specified value into the table and returns a pointer to the 
    *  just-placed value.
    *  (average O(1) insertion time, unless auto-sorting is enabled, in which case it becomes O(N) insertion time)
    *  @param key The key that the new value is to be associated with.
    *  @return A pointer to the value object in the table on success, or NULL on failure (key already exists, out of memory)
    */
   ValueType * PutIfNotAlreadyPresent(const KeyType & key) 
   {
      uint32 hash = ComputeHash(key);
      typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = GetEntry(hash, key);
      if (e) return NULL;
      else
      {
         e = PutAux(hash, key, this->_defaultValue, NULL, NULL);
         return e ? &e->_value : NULL;
      }
   }

   /** Convenience method:  Moves an item from this table to another table.
     * @param moveMe The key in this table of the item that should be moved.
     * @param toTable The table that the item should be in when this operation completes.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory, or (moveMe)
     *          was not found in this table.  Note that trying to move an item into its
     *          own table will simply return B_NO_ERROR with no side effects.
     */
   template<class RHSHashFunctorType, class RHSSubclassType> status_t MoveToTable(const KeyType & moveMe, HashtableMid<KeyType, ValueType, RHSHashFunctorType, RHSSubclassType> & toTable);

   /** Convenience method:  Copies an item from this table to another table.
     * @param copyMe The key in this table of the item that should be copied.
     * @param toTable The table that the item should be in when this operation completes.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory, or (copyMe)
     *          was not found in this table.  Note that trying to copy an item into its
     *          own table will simply return B_NO_ERROR with no side effects.
     */
   template<class RHSHashFunctorType, class RHSSubclassType> status_t CopyToTable(const KeyType & copyMe, HashtableMid<KeyType, ValueType, RHSHashFunctorType, RHSSubclassType> & toTable) const;

   /** This method resizes the Hashtable larger if necessary, so that it has at least (newTableSize)
    *  entries in it.  It is not necessary to call this method, but if you know in advance how many
    *  items you will be adding to the table, you can make the population of the table more efficient
    *  by calling this method before adding the items.  This method will only grow the table, it will
    *  never shrink it.
    *  @param newTableSize Number of slots that you with to have in the table (note that this is different
    *                      from the number of actual items in the table)
    *  @return B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
    */
   status_t EnsureSize(uint32 newTableSize);

   /** Sorts this Hashtable's contents according to its built-in sort ordering. 
     * Specifically, if this object is an OrderedKeysHashtable, the contents will be sorted by key;
     * if this object is an OrderedValuesHashtable, the contents will be sorted by value; and if this
     * class is a plain Hashtable, this method will be a no-op (since a plain Hashtable doesn't have
     * a built-in sort ordering).  If you want to sort a plain Hashtable, have a look at the SortByKey()
     * and SortByValue() methods instead; those allow you to provide a sort ordering on the fly.
     */
   void Sort() {static_cast<SubclassType *>(this)->SortAux();}

   /** This method can be used to activate or deactivate auto-sorting on this Hashtable.
     *
     * If active, auto-sorting ensures that whenever Put() is called, the new/updated item is
     * automatically moved to the correct place in the iterator traversal list.  Note that when
     * auto-sort is enabled, Put() becomes an O(N) operation instead of O(1).
     *
     * Note that auto-sorting only has an effect on OrderedKeysHashtables and OrderedValuesHashtables,
     * in which it is enabled by default.  For plain Hashtables, auto-sort mode has no effect and is
     * disabled by default.
     *
     * @param enabled True if autosort should be enabled; false if it shouldn't be.
     * @param sortNow If true, Sort() will be called when entering a new auto-sort mode, to ensure that
     *                the table is in a sorted state.  You can avoid an immediate sort by specifying this
     *                parameter as false, but be aware that when auto-sorting is enabled, Put() expects
     *                the table's contents to already be sorted according to the current auto-sort state,
     *                and if they aren't, it won't insert its new item at the correct location.  Defaults to true.
     */
   void SetAutoSortEnabled(bool enabled, bool sortNow = true)
   {
      if (enabled != this->_autoSortEnabled)
      {
         this->_autoSortEnabled = enabled;
         if ((sortNow)&&(enabled)) static_cast<SubclassType *>(this)->Sort();
      }
   }

protected:
   /** Default constructor. */
   HashtableMid(uint32 tableSize, bool autoSortEnabled, void * cookie) : HashtableBase<KeyType,ValueType,HashFunctorType>(tableSize, autoSortEnabled, cookie) {/* empty */}

private:
   typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * PutAux(uint32 hash, const KeyType & key, const ValueType & value, ValueType * optSetPreviousValue, bool * optReplacedFlag);
   status_t EnsureTableAllocated();
};

/**
 *  This is a handy templated Hashtable class, rather similar to Java's java.util.Hashtable,
 *  but with a number of enhancements.  Enhancements include:  Iterator objects that are
 *  safe to use even if the table is modified or deleted during a traversal (not thread-safe though),
 *  traversal ordering is maintained at all times and may optionally be sorted or auto-sorted,
 *  and the traversal order may be manually changed as well.
 */
template <class KeyType, class ValueType, class HashFunctorType=typename DEFAULT_HASH_FUNCTOR(KeyType) > class Hashtable : public HashtableMid<KeyType, ValueType, HashFunctorType, Hashtable<KeyType, ValueType, HashFunctorType> >
{
public:
   /** Default constructor. */
   Hashtable() : HashtableMid<KeyType,ValueType,HashFunctorType,Hashtable<KeyType,ValueType,HashFunctorType> >(MUSCLE_HASHTABLE_DEFAULT_CAPACITY, false, NULL) {/* empty */}

   /** Copy Constructor.  */
   Hashtable(const Hashtable & rhs) : HashtableMid<KeyType,ValueType,HashFunctorType,Hashtable<KeyType,ValueType,HashFunctorType> >(rhs.GetNumAllocatedSlots(), false, NULL) {(void) CopyFrom(rhs);}

   /** Templated pseudo-copy-constructor:  Allows us to be instantiated as a copy of a similar table with different functor types. */
   template<class RHSHashFunctorType, class RHSSubclassType> Hashtable(const HashtableMid<KeyType,ValueType,RHSHashFunctorType,RHSSubclassType> & rhs) : HashtableMid<KeyType,ValueType,HashFunctorType,Hashtable<KeyType,ValueType,HashFunctorType> >(rhs.GetNumAllocatedSlots(), false, NULL) {(void) CopyFrom(rhs);}

   /** The iterator type that goes with this HashtableMid type */
   typedef HashtableIterator<KeyType,ValueType,HashFunctorType> IteratorType;

   /** Assignment operator */
   Hashtable & operator=(const Hashtable & rhs) {(void) CopyFrom(rhs); return *this;}

   /** Templated pseudo-assignment-operator:  Allows us to be set from similar table with different functor types. */
   template<class RHSHashFunctorType, class RHSSubclassType> Hashtable & operator=(const HashtableMid<KeyType,ValueType,RHSHashFunctorType,RHSSubclassType> & rhs) {(void) CopyFrom(rhs); return *this;}

private:
   // These are the "fake virtual functions" that can be called by HashtableMid as part of the
   // Curiously Recurring Template Pattern that Hashtable uses to implement polymorphic behavior at compile time.
   friend class HashtableMid<KeyType,ValueType,HashFunctorType,Hashtable<KeyType,ValueType,HashFunctorType> >;
   void InsertIterationEntryAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e) {InsertIterationEntry(e, this->_iterTail);}
   void RepositionAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry *) {/* empty -- reposition is a no-op for this class */}
   void SortAux() {/* empty -- this class doesn't have a well-defined sort ordering */}
};

/** This is a Hashtable that keeps its iteration entries sorted by key at all times (unless you specifically call SetAutoSortEnabled(false)) */
template <class KeyType, class ValueType, class KeyCompareFunctorType=CompareFunctor<KeyType>, class HashFunctorType=typename DEFAULT_HASH_FUNCTOR(KeyType)> class OrderedKeysHashtable : public HashtableMid<KeyType, ValueType, HashFunctorType, OrderedKeysHashtable<KeyType, ValueType, KeyCompareFunctorType, HashFunctorType> >
{
public:
   /** Default constructor.
     * @param optCompareCookie the value that will be passed to our compare functor.  Defaults to NULL.
     */
   OrderedKeysHashtable(void * optCompareCookie = NULL) : HashtableMid<KeyType,ValueType,HashFunctorType,OrderedKeysHashtable<KeyType,ValueType,KeyCompareFunctorType,HashFunctorType> >(MUSCLE_HASHTABLE_DEFAULT_CAPACITY, true, optCompareCookie) {/* empty */}

   /** Copy Constructor.  */
   OrderedKeysHashtable(const OrderedKeysHashtable & rhs) : HashtableMid<KeyType,ValueType,HashFunctorType,OrderedKeysHashtable<KeyType,ValueType,KeyCompareFunctorType,HashFunctorType> >(rhs.GetNumAllocatedSlots(), true, NULL) {(void) CopyFrom(rhs);}

   /** Templated pseudo-copy-constructor:  Allows us to be instantiated as a copy of a similar table with different functor types. */
   template<class RHSKeyCompareFunctorType, class RHSHashFunctorType> OrderedKeysHashtable(const HashtableMid<KeyType,ValueType,RHSKeyCompareFunctorType,RHSHashFunctorType> & rhs) : HashtableMid<KeyType,ValueType,HashFunctorType,OrderedKeysHashtable<KeyType,ValueType,KeyCompareFunctorType,HashFunctorType> >(rhs.GetNumAllocatedSlots(), NULL) {(void) CopyFrom(rhs);}

   /** The iterator type that goes with this HashtableMid type */
   typedef HashtableIterator<KeyType,ValueType,HashFunctorType> IteratorType;

   /** Assignment operator */
   OrderedKeysHashtable & operator=(const OrderedKeysHashtable & rhs) {(void) CopyFrom(rhs); return *this;}

   /** Templated pseudo-assignment-operator:  Allows us to be set from similar table with different functor types. */
   template<class RHSHashFunctorType, class RHSSubclassType> OrderedKeysHashtable & operator=(const HashtableMid<KeyType,ValueType,RHSHashFunctorType,RHSSubclassType> & rhs) {(void) CopyFrom(rhs); return *this;}

private:
   // These are the "fake virtual functions" that can be called by HashtableMid as part of the
   // Curiously Recurring Template Pattern that Hashtable uses to implement polymorphic behavior at compile time.
   friend class HashtableMid<KeyType,ValueType,HashFunctorType,OrderedKeysHashtable<KeyType,ValueType,KeyCompareFunctorType,HashFunctorType> >;
   void InsertIterationEntryAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e);
   void RepositionAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry *) {/* empty -- reposition is a no-op for this class */}
   void SortAux() {SortByKey(_compareFunctor, this->GetCompareCookie());}
   KeyCompareFunctorType _compareFunctor;
};


/** This is a Hashtable that keeps its iteration entries sorted by value at all times (unless you specifically call SetAutoSortEnabled(false)) */
template <class KeyType, class ValueType, class ValueCompareFunctorType=CompareFunctor<ValueType>, class HashFunctorType=typename DEFAULT_HASH_FUNCTOR(KeyType)> class OrderedValuesHashtable : public HashtableMid<KeyType, ValueType, HashFunctorType, OrderedValuesHashtable<KeyType, ValueType, ValueCompareFunctorType, HashFunctorType> >
{
public:
   /** Default constructor.
     * @param optCompareCookie the value that will be passed to our compare functor.  Defaults to NULL.
     */
   OrderedValuesHashtable(void * optCompareCookie = NULL) : HashtableMid<KeyType,ValueType,HashFunctorType,OrderedValuesHashtable<KeyType,ValueType,ValueCompareFunctorType,HashFunctorType> >(MUSCLE_HASHTABLE_DEFAULT_CAPACITY, true, optCompareCookie) {/* empty */}

   /** Copy Constructor. */
   OrderedValuesHashtable(const OrderedValuesHashtable & rhs) : HashtableMid<KeyType,ValueType,HashFunctorType,OrderedValuesHashtable<KeyType,ValueType,ValueCompareFunctorType,HashFunctorType> >(rhs.GetNumAllocatedSlots(), true, NULL) {(void) CopyFrom(rhs);}

   /** The iterator type that goes with this HashtableMid type */
   typedef HashtableIterator<KeyType,ValueType,HashFunctorType> IteratorType;

   /** Templated pseudo-copy-constructor:  Allows us to be instantiated as a copy of a similar table with different functor types. */
   template<class RHSValueCompareFunctorType, class RHSHashFunctorType> OrderedValuesHashtable(const HashtableMid<KeyType,ValueType,RHSValueCompareFunctorType,RHSHashFunctorType> & rhs) : HashtableMid<KeyType,ValueType,HashFunctorType,OrderedValuesHashtable<KeyType,ValueType,ValueCompareFunctorType,HashFunctorType> >(rhs.GetNumAllocatedSlots(), true, NULL) {(void) CopyFrom(rhs);}

   /** Assignment operator */
   OrderedValuesHashtable & operator=(const OrderedValuesHashtable & rhs) {(void) CopyFrom(rhs); return *this;}

   /** Templated pseudo-assignment-operator:  Allows us to be set from similar table with different functor types. */
   template<class RHSHashFunctorType, class RHSSubclassType> OrderedValuesHashtable & operator=(const HashtableMid<KeyType,ValueType,RHSHashFunctorType,RHSSubclassType> & rhs) {(void) CopyFrom(rhs); return *this;}

   /** Moves the specified key/value pair so that it is in the correct position based on the
     * current sort-by-value ordering.   The only time you would need to call this is if the
     * Hashtable is in automatic-sort-by-value mode (see SetAutoSortEnabled()) and you
     * have done an in-place modification of this key's value that might have affected the key's 
     * correct position in the sort ordering.  If you are not using sort-by-value mode,
     * or if you are only doing Put()'s and Get()'s, and never modifying ValueType objects
     * in-place within the table, then calling this method is not necessary and will have no effect.
     * @param key The key object of the key/value pair that may need to be repositioned.
     * @returns B_NO_ERROR on success, or B_ERROR if (key) was not found in the table.
     */
   status_t Reposition(const KeyType & key);

private:
   // These are the "fake virtual functions" that can be called by HashtableMid as part of the
   // Curiously Recurring Template Pattern that Hashtable uses to implement polymorphic behavior at compile time.
   friend class HashtableMid<KeyType,ValueType,HashFunctorType,OrderedValuesHashtable<KeyType,ValueType,ValueCompareFunctorType,HashFunctorType> >;
   void InsertIterationEntryAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e);
   void RepositionAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e);
   void SortAux() {SortByValue(_compareFunctor, this->GetCompareCookie());}

   ValueCompareFunctorType _compareFunctor;
};

//===============================================================
// Implementation of HashtableBase
//===============================================================

// Similar to the equality operator
template <class KeyType, class ValueType, class HashFunctorType>
bool
HashtableBase<KeyType, ValueType, HashFunctorType> ::
IsEqualTo(const HashtableBase<KeyType, ValueType, HashFunctorType> & rhs, bool considerOrdering) const
{
   if (this == &rhs) return true;
   if (GetNumItems() != rhs.GetNumItems()) return false;

   const HashtableEntry * e = _iterHead;
   if (considerOrdering)
   {
      const HashtableEntry * hisE = rhs._iterHead;
      while(e)
      {
         if (hisE->_value != e->_value) return false;
         e = e->_iterNext;
         hisE = hisE->_iterNext;
      }
   }
   else
   {
      while(e)
      {
         const HashtableEntry * hisE = rhs.GetEntry(e->_hash, e->_key);
         if ((hisE == NULL)||(hisE->_value != e->_value)) return false;
         e = e->_iterNext;
      }
   }
   return true;
}

template <class KeyType, class ValueType, class HashFunctorType>
bool
HashtableBase<KeyType,ValueType,HashFunctorType>::ContainsValue(const ValueType & value) const
{
   const HashtableEntry * e = _iterHead;
   while(e)
   {
      if (e->_value == value) return true;
      e = e->_iterNext;
   }
   return false;
}

template <class KeyType, class ValueType, class HashFunctorType>
int32
HashtableBase<KeyType,ValueType,HashFunctorType>::IndexOfKey(const KeyType & key) const
{
   const HashtableEntry * entry = GetEntry(ComputeHash(key), key);
   int32 count = -1;
   if (entry)
   {
      if (entry == _iterTail) count = GetNumItems()-1;
      else
      {
         while(entry)
         {
            entry = entry->_iterPrev; 
            count++;
         }
      }
   }
   return count;
}

template <class KeyType, class ValueType, class HashFunctorType>
int32
HashtableBase<KeyType,ValueType,HashFunctorType>::IndexOfValue(const ValueType & value, bool searchBackwards) const
{
   if (searchBackwards)
   {
      int32 idx = GetNumItems();
      const HashtableEntry * entry = _iterTail;
      while(entry)
      {
         --idx;
         if (entry->_value == value) return idx;
         entry = entry->_iterPrev;
      }
   }
   else
   {
      int32 idx = 0;
      const HashtableEntry * entry = _iterHead;
      while(entry)
      {
         if (entry->_value == value) return idx;
         entry = entry->_iterNext;
         idx++;
      }
   }
   return -1;
}

template <class KeyType, class ValueType, class HashFunctorType>
const KeyType * 
HashtableBase<KeyType,ValueType,HashFunctorType>::GetKeyAt(uint32 index) const
{
   HashtableEntry * e = GetEntryAt(index);
   return e ? &e->_key : NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableBase<KeyType,ValueType,HashFunctorType>::GetKeyAt(uint32 index, KeyType & retKey) const
{
   HashtableEntry * e = GetEntryAt(index);
   if (e)
   {
      retKey = e->_key;
      return B_NO_ERROR;
   }
   return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableBase<KeyType,ValueType,HashFunctorType>::GetValue(const KeyType & key, ValueType & setValue) const
{
   ValueType * ptr = GetValue(key);
   if (ptr)
   {
      setValue = *ptr;
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
ValueType *
HashtableBase<KeyType,ValueType,HashFunctorType>::GetValue(const KeyType & key) const
{
   HashtableEntry * e = GetEntry(ComputeHash(key), key);
   return e ? &e->_value : NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
const KeyType * 
HashtableBase<KeyType,ValueType,HashFunctorType>::GetKey(const KeyType & lookupKey) const
{
   HashtableEntry * e = GetEntry(ComputeHash(lookupKey), lookupKey);
   return e ? &e->_key : NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableBase<KeyType,ValueType,HashFunctorType>::GetKey(const KeyType & lookupKey, KeyType & setKey) const
{
   const KeyType * ptr = GetKey(lookupKey);
   if (ptr)
   {
      setKey = *ptr;
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry *
HashtableBase<KeyType,ValueType,HashFunctorType>::GetEntry(uint32 hash, const KeyType & key) const
{
   if (HasItems()) 
   {
      HashtableEntry * e = _table[hash%_tableSize].GetMapTo();
      if (IsBucketHead(e))  // if the e isn't the start of a bucket, then we know our entry doesn't exist
      {
         while(e)
         {
            if ((e->_hash == hash)&&(AreKeysEqual(e->_key, key))) return e;
            e = e->_bucketNext; 
         }
      }
   }
   return NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry *
HashtableBase<KeyType,ValueType,HashFunctorType>::GetEntryAt(uint32 idx) const
{
   HashtableEntry * e = NULL;
   if (idx < _count)
   {
      if (idx < _count/2)
      {
         e = _iterHead;
         while((e)&&(idx--)) e = e->_iterNext;
      }
      else
      {
         idx = _count-(idx+1);
         e = _iterTail;
         while((e)&&(idx--)) e = e->_iterPrev;
      }
   }
   return e;
}

// Linked-list MergeSort adapted from Simon Tatham's C code at http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.c
template <class KeyType, class ValueType, class HashFunctorType>
template <class KeyCompareFunctorType>
void 
HashtableBase<KeyType,ValueType,HashFunctorType>::SortByKey(const KeyCompareFunctorType & keyFunctor, void * cookie)
{
   if (this->_iterHead == NULL) return;

   for (uint32 mergeSize = 1; /* empty */; mergeSize *= 2)
   {
      typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * p = this->_iterHead;
      this->_iterHead = this->_iterTail = NULL;

      uint32 numMerges = 0;  /* count number of merges we do in this pass */
      while(p) 
      {
         numMerges++;  /* there exists a merge to be done */

         /* step `mergeSize' places along from p */
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * q = p;
         uint32 psize = 0;
         for (uint32 i=0; i<mergeSize; i++) 
         {
             psize++;
             q = q->_iterNext;
             if (!q) break;
         }

         /* now we have two lists; merge them */
         for (uint32 qsize=mergeSize; ((psize > 0)||((qsize > 0)&&(q))); /* empty */) 
         {
            typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e;

            /* decide whether next element of the merge comes from p or q */
                 if (psize == 0)                                      {e = q; q = q->_iterNext; qsize--;}
            else if ((qsize == 0)||(q == NULL))                       {e = p; p = p->_iterNext; psize--;}
            else if (keyFunctor.Compare(p->_key,q->_key,cookie) <= 0) {e = p; p = p->_iterNext; psize--;}
            else                                                      {e = q; q = q->_iterNext; qsize--;}

            /* append to our new more-sorted list */
            if (this->_iterTail) this->_iterTail->_iterNext = e;
                            else this->_iterHead            = e;
            e->_iterPrev = this->_iterTail;
            this->_iterTail = e;
         }

         p = q; /* now p has stepped `mergeSize' places along, and q has too */
      }
      this->_iterTail->_iterNext = NULL;
      if (numMerges <= 1) return;
   }
}

// Linked-list MergeSort adapted from Simon Tatham's C code at http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.c
template <class KeyType, class ValueType, class HashFunctorType>
template <class ValueCompareFunctorType>
void 
HashtableBase<KeyType,ValueType,HashFunctorType>::SortByValue(const ValueCompareFunctorType & valFunctor, void * cookie)
{
   if (this->_iterHead == NULL) return;

   for (uint32 mergeSize = 1; /* empty */; mergeSize *= 2)
   {
      typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * p = this->_iterHead;
      this->_iterHead = this->_iterTail = NULL;

      uint32 numMerges = 0;  /* count number of merges we do in this pass */
      while(p) 
      {
         numMerges++;  /* there exists a merge to be done */

         /* step `mergeSize' places along from p */
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * q = p;
         uint32 psize = 0;
         for (uint32 i=0; i<mergeSize; i++) 
         {
             psize++;
             q = q->_iterNext;
             if (!q) break;
         }

         /* now we have two lists; merge them */
         for (uint32 qsize=mergeSize; ((psize > 0)||((qsize > 0)&&(q))); /* empty */) 
         {
            typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e;

            /* decide whether next element of the merge comes from p or q */
                 if (psize == 0)                                          {e = q; q = q->_iterNext; qsize--;}
            else if ((qsize == 0)||(q == NULL))                           {e = p; p = p->_iterNext; psize--;}
            else if (valFunctor.Compare(p->_value,q->_value,cookie) <= 0) {e = p; p = p->_iterNext; psize--;}
            else                                                          {e = q; q = q->_iterNext; qsize--;}

            /* append to our new more-sorted list */
            if (this->_iterTail) this->_iterTail->_iterNext = e;
                            else this->_iterHead            = e;
            e->_iterPrev = this->_iterTail;
            this->_iterTail = e;
         }

         p = q; /* now p has stepped `mergeSize' places along, and q has too */
      }
      this->_iterTail->_iterNext = NULL;
      if (numMerges <= 1) return;
   }
}

template <class KeyType, class ValueType, class HashFunctorType>
void
HashtableBase<KeyType,ValueType,HashFunctorType>::SwapContents(HashtableBase<KeyType,ValueType,HashFunctorType> & swapMe)
{
   muscleSwap(_count,           swapMe._count);
   muscleSwap(_tableSize,       swapMe._tableSize);
   muscleSwap(_table,           swapMe._table);
   muscleSwap(_iterHead,        swapMe._iterHead);
   muscleSwap(_iterTail,        swapMe._iterTail);
   muscleSwap(_freeHead,        swapMe._freeHead);
   muscleSwap(_autoSortEnabled, swapMe._autoSortEnabled);
   muscleSwap(_compareCookie,   swapMe._compareCookie);
   muscleSwap(_iterList,        swapMe._iterList);

   // Lastly, swap the owners of all iterators, so that they will unregister from the correct table when they die
   {
      IteratorType * next = _iterList;
      while(next)
      {
         next->_owner = &swapMe;
         next = next->_nextIter;
      }
   }
   {
      IteratorType * next = swapMe._iterList;
      while(next)
      {
         next->_owner = this;
         next = next->_nextIter;
      }
   }
}

// This is the part of the insertion that is CompareFunctor-neutral.
template <class KeyType, class ValueType, class HashFunctorType>
typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry *
HashtableBase<KeyType,ValueType,HashFunctorType>::PutAuxAux(uint32 hash, const KeyType & key, const ValueType & value)
{
   HashtableEntry * tableSlot = _table[hash%_tableSize].GetMapTo();
   if (IsBucketHead(tableSlot))
   {
#ifdef MUSCLE_COLLECT_HASHTABLE_COLLISION_STATISTICS
      _totalHashtableCollisions++;
#endif
      // This slot's chain is already present -- so just create a new entry in the chain's linked list to hold our item
      HashtableEntry * e = _freeHead;
      _freeHead = e->PopFromFreeList(_freeHead);
      e->_hash  = hash;
      e->_key   = key;
      e->_value = value;

      // insert e into the list immediately after (tableSlot)
      e->_bucketPrev = tableSlot;
      e->_bucketNext = tableSlot->_bucketNext;
      if (e->_bucketNext) e->_bucketNext->_bucketPrev = e;
      tableSlot->_bucketNext = e;
      return e;
   }
   else 
   {
      if (tableSlot->_hash != MUSCLE_HASHTABLE_INVALID_HASH_CODE)
      {
         // Hey, some other bucket is using my starter-slot!
         // To get around this, we'll swap my starter-slot for an empty one and use that instead.
         tableSlot->GetMappedFrom()->SwapMaps(_freeHead->GetMappedFrom());
         tableSlot = _freeHead;
      }
      _freeHead = tableSlot->PopFromFreeList(_freeHead);

      // First entry in tableSlot; just copy data over
      tableSlot->_hash  = hash;
      tableSlot->_key   = key;
      tableSlot->_value = value;  
 
      tableSlot->_bucketPrev = tableSlot->_bucketNext = NULL;
      return tableSlot;
   }
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableBase<KeyType,ValueType,HashFunctorType>::RemoveEntry(HashtableEntry * e, ValueType * optSetValue)
{
   RemoveIterationEntry(e);
   if (optSetValue) *optSetValue = e->_value;

   HashtableEntry * prev = e->_bucketPrev;
   HashtableEntry * next = e->_bucketNext;
   if (prev)
   {
      prev->_bucketNext = next;
      if (next) next->_bucketPrev = prev;
   }
   else if (next) 
   {
      next->_bucketPrev = NULL;
      e->GetMappedFrom()->SwapMaps(next->GetMappedFrom());
   }

   _count--;
   e->PushToFreeList(_defaultKey, GetDefaultValue(), _freeHead);

   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
void
HashtableBase<KeyType,ValueType,HashFunctorType>::Clear(bool releaseCachedBuffers)
{
   // First go through our list of active iterators, and let them all know they are now invalid
   while(_iterList)
   {
      IteratorType * next = _iterList->_nextIter;
      _iterList->_owner = NULL;
      _iterList->_nextKeyCookie = _iterList-> _nextValueCookie = NULL;
      _iterList->_prevIter      = _iterList->_nextIter         = NULL;
      _iterList = next;
   }

   // It's important to set each in-use HashtableEntry to its default state so
   // that any held memory (e.g. RefCountables) will be freed, etc.
   // Calling RemoveEntry() on each item is necessary to ensure correct behavior
   // even when the templatized classes' assignment operations cause re-entrancies, etc.
   while(_iterHead) (void) RemoveEntry(_iterHead, NULL);

   if (releaseCachedBuffers)
   {
      HashtableEntry * oldTable = _table;
      _table     = NULL;
      _freeHead  = NULL;
      _tableSize = MUSCLE_HASHTABLE_DEFAULT_CAPACITY;
      delete [] oldTable;  // done after state is updated, in case of re-entrancies in the dtors
   }
}

template <class KeyType, class ValueType, class HashFunctorType>
uint32 
HashtableBase<KeyType,ValueType,HashFunctorType>::Remove(const HashtableBase & pairs)
{
   uint32 removeCount = 0;
   if (&pairs == this)
   {
      removeCount = GetNumItems();
      Clear();
   }
   else
   {
      HashtableEntry * e = pairs._iterHead;
      while(e)
      {
         if (RemoveAux(e->_hash, e->_key, NULL) == B_NO_ERROR) removeCount++;
         e = e->_iterNext;
      }
   }
   return removeCount;
}

template <class KeyType, class ValueType, class HashFunctorType>
uint32 
HashtableBase<KeyType,ValueType,HashFunctorType>::Intersect(const HashtableBase & pairs)
{
   uint32 removeCount = 0;
   if (&pairs != this)
   {
      HashtableEntry * e = _iterHead;
      while(e)
      {
         HashtableEntry * next = e->_iterNext;  // save this first, since we might be erasing (e)
         if ((pairs.GetEntry(e->_hash, e->_key) == NULL)&&(RemoveAux(e->_hash, e->_key, NULL) == B_NO_ERROR)) removeCount++;
         e = next;
      }
   }
   return removeCount;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableBase<KeyType,ValueType,HashFunctorType>::MoveToFront(const KeyType & moveMe)
{
   HashtableEntry * e = GetEntry(ComputeHash(moveMe), moveMe);
   if (e == NULL) return B_ERROR;
   MoveToFrontAux(e);
   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableBase<KeyType,ValueType,HashFunctorType>::MoveToBack(const KeyType & moveMe)
{
   HashtableEntry * e = GetEntry(ComputeHash(moveMe), moveMe);
   if (e == NULL) return B_ERROR;
   MoveToBackAux(e);
   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableBase<KeyType,ValueType,HashFunctorType>::MoveToBefore(const KeyType & moveMe, const KeyType & toBeforeMe)
{
   if (HasItems())
   {
      HashtableEntry * e = GetEntry(ComputeHash(moveMe),     moveMe);
      HashtableEntry * f = GetEntry(ComputeHash(toBeforeMe), toBeforeMe);
      if ((e == NULL)||(f == NULL)||(e == f)) return B_ERROR;
      MoveToBeforeAux(e, f);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableBase<KeyType,ValueType,HashFunctorType>::MoveToBehind(const KeyType & moveMe, const KeyType & toBehindMe)
{
   if (HasItems())
   {
      HashtableEntry * d = GetEntry(ComputeHash(toBehindMe), toBehindMe);
      HashtableEntry * e = GetEntry(ComputeHash(moveMe),     moveMe);
      if ((d == NULL)||(e == NULL)||(d == e)) return B_ERROR;
      MoveToBehindAux(e, d);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

// Adds (e) to the our iteration linked list, behind (optBehindThis), or at the head if (optBehindThis) is NULL.
template <class KeyType, class ValueType, class HashFunctorType>
void
HashtableBase<KeyType,ValueType,HashFunctorType>::InsertIterationEntry(HashtableEntry * e, HashtableEntry * optBehindThis)
{
   e->_iterPrev = optBehindThis;
   e->_iterNext = optBehindThis ? optBehindThis->_iterNext : this->_iterHead;
   if (e->_iterPrev) e->_iterPrev->_iterNext = e;
                else this->_iterHead = e;
   if (e->_iterNext) e->_iterNext->_iterPrev = e;
                else this->_iterTail = e;
}

// Remove (e) from our iteration linked list
template <class KeyType, class ValueType, class HashFunctorType>
void 
HashtableBase<KeyType,ValueType,HashFunctorType>::RemoveIterationEntry(HashtableEntry * e)
{
   // Update any iterators that were pointing at (e), so that they now point to the entry after e.
   IteratorType * next = this->_iterList;
   while(next)
   {
      if (next->_nextKeyCookie == e) 
      {
         bool isBack = next->IsBackwards();
         if (e == (isBack?this->_iterTail:this->_iterHead)) 
         {
            next->_flags |= IteratorType::HTIT_INTERNAL_FLAG_RESTARTKEY;
            next->_nextKeyCookie = NULL;
         }
         else next->_nextKeyCookie = GetSubsequentEntry(next->_nextKeyCookie, isBack?0:HTIT_FLAG_BACKWARDS);
      }
      if (next->_nextValueCookie == e) 
      {
         bool isBack = next->IsBackwards();
         if (e == (isBack?this->_iterTail:this->_iterHead)) 
         {
            next->_flags |= IteratorType::HTIT_INTERNAL_FLAG_RESTARTVALUE;
            next->_nextValueCookie = NULL;
         }
         else next->_nextValueCookie = GetSubsequentEntry(next->_nextValueCookie, isBack?0:HTIT_FLAG_BACKWARDS);
      }
      next = next->_nextIter;
   }

   if (this->_iterHead == e) this->_iterHead = e->_iterNext;
   if (this->_iterTail == e) this->_iterTail = e->_iterPrev;
   if (e->_iterPrev) e->_iterPrev->_iterNext = e->_iterNext;
   if (e->_iterNext) e->_iterNext->_iterPrev = e->_iterPrev;
   e->_iterPrev = e->_iterNext = NULL; 
}

//===============================================================
// Implementation of HashtableMid
//===============================================================

// CopyFrom() method is similar to the assignment operator, but gives a return value.
template <class KeyType, class ValueType, class HashFunctorType, class SubclassType>
template<class RHSHashFunctorType>
status_t
HashtableMid<KeyType, ValueType, HashFunctorType, SubclassType> ::
CopyFrom(const HashtableBase<KeyType, ValueType, RHSHashFunctorType> & rhs, bool clearFirst)
{
   if (((const void *)(this)) == ((const void *)(&rhs))) return B_NO_ERROR;

   if (clearFirst) this->Clear();
   if (rhs.HasItems())
   {
      if ((EnsureSize(this->GetNumItems()+rhs.GetNumItems()) != B_NO_ERROR)||(EnsureTableAllocated() != B_NO_ERROR)) return B_ERROR;
      CopyFromAux(rhs);
      static_cast<SubclassType *>(this)->SortAux();  // We do the sort (if any) at the end, since that is more efficient than traversing the list after every insert
   }
   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType, class SubclassType>
status_t
HashtableMid<KeyType,ValueType,HashFunctorType,SubclassType>::EnsureSize(uint32 requestedSize)
{
   if (requestedSize <= this->_tableSize) return B_NO_ERROR;  // no need to do anything if we're already big enough!

   // 1. Initialize the scratch space for our active iterators.
   {
      IteratorType * nextIter = this->_iterList;
      while(nextIter)
      {
         SetIteratorScratchSpace(*nextIter, 0, NULL);
         SetIteratorScratchSpace(*nextIter, 1, NULL); // these will hold our switch-to-on-success values
         nextIter = GetIteratorNextIterator(*nextIter);
      }
   }
    
   // 2. Create a new, bigger table, to hold a copy of our data.
   SubclassType biggerTable;
   biggerTable._tableSize = muscleMax(this->_count, requestedSize);
   biggerTable.SetAutoSortEnabled(false);  // make sure he doesn't do any sorting during the initial population phase

   // 3. Place all of our data into (biggerTable)
   {
      typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * next = this->_iterHead;
      while(next)
      {
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * hisClone = biggerTable.PutAux(next->_hash, next->_key, next->_value, NULL, NULL);
         if (hisClone)
         {
            // Mark any iterators that will need to be redirected to point to the new nodes.
            IteratorType * nextIter = this->_iterList;
            while(nextIter)
            {
               if (GetIteratorNextKeyCookie(*nextIter)   == next) SetIteratorScratchSpace(*nextIter, 0, hisClone);
               if (GetIteratorNextValueCookie(*nextIter) == next) SetIteratorScratchSpace(*nextIter, 1, hisClone);
               nextIter = GetIteratorNextIterator(*nextIter);
            }
         }
         else return B_ERROR;  // oops, out of mem, too bad.  

         next = next->_iterNext;
      }
   }

   // 4. Only now do we set biggerTable's auto-sort params; that way he isn't trying to sort the data we gave him in step (3)
   //    (which is unecessary since if auto-sort is activated, we already have the data sorted in the correct order)
   biggerTable.SetCompareCookie(this->_compareCookie);
   biggerTable.SetAutoSortEnabled(this->_autoSortEnabled, false);  // no need to sort now, he is already sorted

   // 5. Swap contents with the bigger table, but don't swap iterator lists (we want to keep ours!)
   {
      IteratorType * temp = this->_iterList;
      this->_iterList = NULL;
      SwapContents(biggerTable);
      this->_iterList = temp;
   }

   // 6. Lastly, fix up our iterators to point to their new entries.
   {
      IteratorType * nextIter = this->_iterList;
      while(nextIter)
      {
         SetIteratorNextKeyCookie(  *nextIter, GetIteratorScratchSpace(*nextIter, 0));
         SetIteratorNextValueCookie(*nextIter, GetIteratorScratchSpace(*nextIter, 1));
         nextIter = GetIteratorNextIterator(*nextIter);
      }
   }

   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType, class SubclassType>
template <class RHSHashFunctorType, class RHSSubclassType>
status_t 
HashtableMid<KeyType,ValueType,HashFunctorType,SubclassType>::CopyToTable(const KeyType & copyMe, HashtableMid<KeyType, ValueType, RHSHashFunctorType, RHSSubclassType> & toTable) const
{
   uint32 hash = ComputeHash(copyMe);
   const typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = GetEntry(hash, copyMe);
   if (e)
   { 
      if (this == &toTable) return B_NO_ERROR;  // it's already here!
      if (toTable.PutAux(hash, copyMe, e->_value, NULL, NULL) != NULL) return B_NO_ERROR;
   }
   return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType, class SubclassType>
template <class RHSHashFunctorType, class RHSSubclassType>
status_t 
HashtableMid<KeyType,ValueType,HashFunctorType,SubclassType>::MoveToTable(const KeyType & moveMe, HashtableMid<KeyType, ValueType, RHSHashFunctorType, RHSSubclassType> & toTable)
{
   uint32 hash = ComputeHash(moveMe);
   const typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = GetEntry(hash, moveMe);
   if (e)
   {
      if (this == &toTable) return B_NO_ERROR;  // it's already here!
      if (toTable.PutAux(hash, moveMe, e->_value, NULL, NULL) != NULL) return RemoveAux(e->_hash, moveMe, NULL);
   }
   return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType, class SubclassType>
status_t
HashtableMid<KeyType,ValueType,HashFunctorType,SubclassType>::EnsureTableAllocated()
{
   if (this->_table == NULL) this->_freeHead = this->_table = HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry::CreateEntriesArray(this->_tableSize);
   return this->_table ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType, class SubclassType>
typename HashtableBase<KeyType,ValueType, HashFunctorType>::HashtableEntry *
HashtableMid<KeyType,ValueType,HashFunctorType,SubclassType>::PutAux(uint32 hash, const KeyType & key, const ValueType & value, ValueType * optSetPreviousValue, bool * optReplacedFlag)
{
   if (optReplacedFlag) *optReplacedFlag = false;
   if (EnsureTableAllocated() != B_NO_ERROR) return NULL;

   // If we already have an entry for this key in the table, we can just replace its contents
   typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = GetEntry(hash, key);
   if (e)
   {
      if (optSetPreviousValue) *optSetPreviousValue = e->_value;
      if (optReplacedFlag)     *optReplacedFlag     = true;
      e->_value = value;
      static_cast<SubclassType*>(this)->RepositionAux(e);
      return e;
   }

   // Rehash the table if the threshold is exceeded
   if (this->_count == this->_tableSize) return (EnsureSize(this->_tableSize*2) == B_NO_ERROR) ? PutAux(hash, key, value, optSetPreviousValue, optReplacedFlag) : NULL;

   e = PutAuxAux(hash, key, value);
   static_cast<SubclassType*>(this)->InsertIterationEntryAux(e);

   this->_count++;
   return e; 
}

//===============================================================
// Implementation of OrderedKeysHashtable
//===============================================================

template <class KeyType, class ValueType, class CompareFunctorType, class HashFunctorType>
void
OrderedKeysHashtable<KeyType,ValueType,CompareFunctorType,HashFunctorType>::InsertIterationEntryAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e)
{
   typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * insertAfter = this->_iterTail;  // default to appending to the end of the list
   if ((this->GetAutoSortEnabled())&&(this->_iterHead))
   {
      // We're in sorted mode, so we'll try to place this guy in the correct position.
           if (_compareFunctor.Compare(e->_key, this->_iterHead->_key, this->_compareCookie) < 0) insertAfter = NULL;  // easy; append to the head of the list
      else if (_compareFunctor.Compare(e->_key, this->_iterTail->_key, this->_compareCookie) < 0)  // only iterate through if we're before the tail, otherwise the tail is fine
      {
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * prev = this->_iterHead;
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * next = this->_iterHead->_iterNext;  // more difficult;  find where to insert into the middle
         while(next)
         {
            if (_compareFunctor.Compare(e->_key, next->_key, this->_compareCookie) < 0)
            {
               insertAfter = prev;
               break;
            }
            else 
            {
               prev = next;
               next = next->_iterNext;
            }
         }   
      }
   }
   InsertIterationEntry(e, insertAfter);
}

//===============================================================
// Implementation of OrderedValuesHashtable
//===============================================================

template <class KeyType, class ValueType, class CompareFunctorType, class HashFunctorType>
status_t 
OrderedValuesHashtable<KeyType,ValueType,CompareFunctorType,HashFunctorType>::Reposition(const KeyType & key)
{
   typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e = this->GetAutoSortEnabled() ? GetEntry(ComputeHash(key), key) : NULL;
   if (e)
   {
      RepositionAux(e);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class CompareFunctorType, class HashFunctorType>
void
OrderedValuesHashtable<KeyType,ValueType,CompareFunctorType,HashFunctorType>::RepositionAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e)
{
   if (this->GetAutoSortEnabled() == false) return;

   // If our new value has changed our position in the sort-order, then adjust the traversal list
   if ((e->_iterPrev)&&(_compareFunctor.Compare(e->_value, e->_iterPrev->_value, this->_compareCookie) < 0))
   {
      if (_compareFunctor.Compare(e->_value, this->_iterHead->_value, this->_compareCookie) < 0) MoveToFrontAux(e);
      else
      {
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * moveToBefore = e->_iterPrev;
         while((moveToBefore->_iterPrev)&&(_compareFunctor.Compare(e->_value, moveToBefore->_iterPrev->_value, this->_compareCookie) < 0)) moveToBefore = moveToBefore->_iterPrev;
         MoveToBeforeAux(e, moveToBefore);
      }
   }
   else if ((e->_iterNext)&&(_compareFunctor.Compare(e->_value, e->_iterNext->_value, this->_compareCookie) > 0))
   {
      if (_compareFunctor.Compare(e->_value, this->_iterTail->_value, this->_compareCookie) > 0) MoveToBackAux(e);
      else
      {
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * moveToBehind = e->_iterNext;
         while((moveToBehind->_iterNext)&&(_compareFunctor.Compare(e->_value, moveToBehind->_iterNext->_value, this->_compareCookie) > 0)) moveToBehind = moveToBehind->_iterNext;
         MoveToBehindAux(e, moveToBehind);
      }
   }
}

template <class KeyType, class ValueType, class CompareFunctorType, class HashFunctorType>
void
OrderedValuesHashtable<KeyType,ValueType,CompareFunctorType,HashFunctorType>::InsertIterationEntryAux(typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * e)
{
   typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * insertAfter = this->_iterTail;  // default to appending to the end of the list
   if ((this->GetAutoSortEnabled())&&(this->_iterHead))
   {
      // We're in sorted mode, so we'll try to place this guy in the correct position.
           if (_compareFunctor.Compare(e->_value, this->_iterHead->_value, this->_compareCookie) < 0) insertAfter = NULL;  // easy; append to the head of the list
      else if (_compareFunctor.Compare(e->_value, this->_iterTail->_value, this->_compareCookie) < 0)  // only iterate through if we're before the tail, otherwise the tail is fine
      {
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * prev = this->_iterHead;
         typename HashtableBase<KeyType,ValueType,HashFunctorType>::HashtableEntry * next = this->_iterHead->_iterNext;  // more difficult;  find where to insert into the middle
         while(next)
         {
            if (_compareFunctor.Compare(e->_value, next->_value, this->_compareCookie) < 0)
            {
               insertAfter = prev;
               break;
            }
            else 
            {
               prev = next;
               next = next->_iterNext;
            }
         }   
      }
   }
   InsertIterationEntry(e, insertAfter);
}

//===============================================================
// Implementation of HashtableIterator
//===============================================================

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::HashtableIterator() : _nextKeyCookie(NULL), _nextValueCookie(NULL), _flags(0), _owner(NULL)
{
   // empty
}

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::HashtableIterator(const HashtableIterator<KeyType, ValueType, HashFunctorType> & rhs) : _flags(0), _owner(NULL)
{
   *this = rhs;
}

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::HashtableIterator(const HashtableBase<KeyType, ValueType, HashFunctorType> & table, uint32 flags) : _flags(flags), _owner(&table)
{
   table.InitializeIterator(*this);
}

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::HashtableIterator(const HashtableBase<KeyType, ValueType, HashFunctorType> & table, const KeyType & startAt, uint32 flags) : _flags(flags), _owner(&table)
{
   table.InitializeIteratorAt(*this, startAt);
}

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::~HashtableIterator()
{
   if (_owner) _owner->UnregisterIterator(this);
}

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType,ValueType,HashFunctorType> &
HashtableIterator<KeyType,ValueType,HashFunctorType>:: operator=(const HashtableIterator<KeyType,ValueType,HashFunctorType> & rhs)
{
   if (this != &rhs)
   {
      if (_owner) _owner->UnregisterIterator(this);
      _flags = rhs._flags;   // must be done while unregistered, in case NOREGISTER flag changes state
      _owner = rhs._owner;
      if (_owner) _owner->RegisterIterator(this);

      _nextKeyCookie   = rhs._nextKeyCookie;
      _nextValueCookie = rhs._nextValueCookie;
   }
   return *this;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKey(KeyType & key) 
{
   const KeyType * ret = GetNextKey();
   if (ret) 
   {
      key = *ret;
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKey(const KeyType * & setKeyPtr)
{
   return ((setKeyPtr = GetNextKey()) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextValue(ValueType & val)
{
   ValueType * ret = GetNextValue();
   if (ret)
   {
      val = *ret;
      return B_NO_ERROR;
   } 
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextValue(ValueType * & setValuePtr)
{
   return ((setValuePtr = GetNextValue()) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextValue(const ValueType * & setValuePtr)
{
   return ((setValuePtr = GetNextValue()) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextKey(KeyType & key) const
{
   const KeyType * ret = PeekNextKey();
   if (ret)
   {
      key = *ret;
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextKey(const KeyType * & setKeyPtr) const
{
   return ((setKeyPtr = PeekNextKey()) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextValue(ValueType & val) const
{
   ValueType * ret = PeekNextValue();
   if (ret)
   {
      val = *ret;
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextValue(const ValueType * & setValuePtr) const
{
   return ((setValuePtr = PeekNextValue()) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextValue(ValueType * & setValuePtr) const
{
   return ((setValuePtr = PeekNextValue()) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
ValueType *
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextValue()
{
   ValueType * val = PeekNextValue();

        if (val) _nextValueCookie = _owner->GetSubsequentEntry(_nextValueCookie, _flags);
   else if ((_owner)&&(_flags & HTIT_INTERNAL_FLAG_RESTARTVALUE))
   {
      _nextValueCookie = _owner->GetInitialEntry(_flags);
      val = PeekNextValue();
      _flags &= ~HTIT_INTERNAL_FLAG_RESTARTVALUE;
   }
   return val;
}

template <class KeyType, class ValueType, class HashFunctorType>
const KeyType *
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKey() 
{
   const KeyType * key = PeekNextKey();

        if (key) _nextKeyCookie = _owner->GetSubsequentEntry(_nextKeyCookie, _flags);
   else if ((_owner)&&(_flags & HTIT_INTERNAL_FLAG_RESTARTKEY))
   {
      _nextKeyCookie = _owner->GetInitialEntry(_flags);
      key = PeekNextKey();
      _flags &= ~HTIT_INTERNAL_FLAG_RESTARTKEY;
   }
   return key;
}

template <class KeyType, class ValueType, class HashFunctorType>
ValueType *
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextValue() const
{
   return (_owner) ? _owner->GetValueFromCookie(_nextValueCookie) : NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
const KeyType *
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextKey() const
{
   return (_owner) ? _owner->GetKeyFromCookie(_nextKeyCookie) : NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKeyAndValue(KeyType & setKey, ValueType & setValue)
{
   return (GetNextKey(setKey) == B_NO_ERROR) ? GetNextValue(setValue) : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKeyAndValue(KeyType & setKey, ValueType * & setValuePtr)
{
   return ((setValuePtr = GetNextValue()) != NULL) ? GetNextKey(setKey) : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKeyAndValue(KeyType & setKey, const ValueType * & setValuePtr)
{
   return ((setValuePtr = GetNextValue()) != NULL) ? GetNextKey(setKey) : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKeyAndValue(const KeyType * & setKeyPtr, ValueType & setValue)
{
   return ((setKeyPtr = GetNextKey()) != NULL) ? GetNextValue(setValue) : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKeyAndValue(const KeyType * & setKeyPtr, ValueType * & setValuePtr)
{
   return (((setKeyPtr = GetNextKey()) != NULL)&&((setValuePtr = GetNextValue()) != NULL)) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKeyAndValue(const KeyType * & setKeyPtr, const ValueType * & setValuePtr)
{
   return (((setKeyPtr = GetNextKey()) != NULL)&&((setValuePtr = GetNextValue()) != NULL)) ? B_NO_ERROR : B_ERROR;
}

}; // end namespace muscle

#endif
