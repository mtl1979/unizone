/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

/* Note:  This file was originally written by Michael Olivero, but has been modified
 *        extensively.  Nonetheless, the original license is listed below.  --jaf
 */

/*
   This library was downloaded from: http://www.mike95.com

   This library is copyright.  It may freely be used for personal purposes 
   if the restriction listed below is adhered to.
       Author: Michael Olivero
       Email:  mike95@mike95.com

   //===============================
   //Start of Restriction Definition
   //===============================
   Anyone can have full use of the library provided they keep this complete comment
   with the source.  Also I would like to ask if any changes are made to the
   code for efficiency reasons, please let me know so I may look into your change and
   likewise incorporate it into library.  If a suggestion makes it into the library,
   your credits will be added to this information.

   Authors of Computer related books are welcome to include this source code as part
   of their publishing, provided credit the Author and make note of where the source
   code was obtained from: http://www.mike95.com
   //=============================
   //End of Restriction Definition
   //=============================

   Description:
   Visit http://www.mike95.com/c_plusplus/classes/JHashtable/

   This class is a based on the Java Hashtable class and as such contains all the public
   member functions of its Java equivalent.  Unlike Java, typecasts are not necessary
   since C++ allows template instatiation of types at compile time.

   Note:  Since java has hashCode() as a member of the base Object class, all 
   Java classes are inherently hashable.  Since the template parameter types do
   not necessarily have to have a built in hashing function, the user of the class
   must pass a hash function callback as part of the Hashtable constructor.

   The has function must be declared as the following:

       uint32 function(const KeyType&);

   Where:
   function = any name you choose to use for the function name
   KeyType  = the type used for the key in the construction of the Hashtable object.

   Example:
       uint32 myHash( const int&)
       {
           //your hashing code here for a key of type int.
       }
*/

#ifndef MuscleHashtable_h
#define MuscleHashtable_h

#include "support/MuscleSupport.h"

#ifdef _MSC_VER
# pragma warning(disable: 4786)
#endif

namespace muscle {

/** This hashing functor type handles the "easy" cases, where the KeyType is
 *  something that can be static_cast into a uint32 by the compiler.
 *  For more complicated key types, you'll need to define your own 
 *  specialization of this functor template.  (See util/String.h for an example of this)
 */
template <class T> class HashFunctor
{
public:
   uint32 operator () (const T x) const {return (uint32) x;}
};

template <class KeyType, class ValueType, class HashFunctorType> class Hashtable;  // forward declaration

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
 * It is safe to modify or delete a hashtable during a traversal; the active iterators
 * will be automatically notified so that they do the right thing.
 */
template <class KeyType, class ValueType, class HashFunctorType = HashFunctor<KeyType> > class HashtableIterator
{
public:
   /**
    * Default constructor.  It's here only so that you can include HashtableIterators
    * as member variables, in arrays, etc.  HashtableIterators created with this
    * constructor are "empty", so they won't be useful until you set them equal to a 
    * HashtableIterator that was returned by Hashtable::GetIterator().
    */
   HashtableIterator();

   /** Copy Constructor.  */
   HashtableIterator(const HashtableIterator<KeyType, ValueType, HashFunctorType> & rhs);

   /** Destructor */
   ~HashtableIterator();

   /** Assignment operator. */
   HashtableIterator<KeyType,ValueType, HashFunctorType> & operator=(const HashtableIterator<KeyType,ValueType,HashFunctorType> & rhs);

   /** Returns true iff there are more keys left in the key traversal.  */
   bool HasMoreKeys() const;

   /**
    * Gets the next key in the key traversal.
    * @param setNextKey On success, the next key is copied into this object.
    * @return B_NO_ERROR on success, B_ERROR if there are no more keys left.
    */
   status_t GetNextKey(KeyType & setNextKey);

   /**
    * Gets a pointer to the next key in the key traversal.
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
    * Note that the returned pointer is only guaranteed valid as long as the Hashtable remains unchanged.
    * @return a pointer to the next key in the key traversal, or NULL if there are no keys left.
    */
   const KeyType * PeekNextKey() const;

   /** Returns true iff there are more values left in the value traversal.  */
   bool HasMoreValues() const;

   /**
    * Get the next value in the values traversal.
    * @param setNextValue On success, the next value in the traversal is copied into this object.
    * @return B_NO_ERROR on success, B_ERROR if there are no more values left in the value traversal.
    */
   status_t GetNextValue(ValueType & setNextValue);

   /**
    * Get the next value in the values traversal.
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
    * Note that the returned pointer is only guaranteed valid as long as the Hashtable remains unchanged.
    * @return a pointer to the next value in the value traversal, or NULL if there are no values left.
    */
   ValueType * PeekNextValue() const;

private:
   friend class Hashtable<KeyType, ValueType, HashFunctorType>;

   HashtableIterator(const Hashtable<KeyType, ValueType, HashFunctorType> & owner, void * startCookie, bool backwards);

   void * _scratchSpace[2];   // ignore this; it's temp scratch space used by GrowTable().

   void * _nextKeyCookie;
   void * _nextValueCookie;
   bool _backwards;

   HashtableIterator<KeyType, ValueType, HashFunctorType> * _prevIter;  // for the doubly linked list so that the table can notify us if it is modified
   HashtableIterator<KeyType, ValueType, HashFunctorType> * _nextIter;  // for the doubly linked list so that the table can notify us if it is modified

   const Hashtable<KeyType, ValueType, HashFunctorType>   * _owner;     // table that we are associated with
};

/**
 *  This is a handy templated Hashtable class, rather similar to Java's java.util.Hashtable,
 *  but with a number of enhancements.  Enhancements include:  Iterator objects that are
 *  safe to use even if the table is modified or deleted during a traversal (not thread-safe though),
 *  traversal ordering is maintained at all times and may optionally be sorted or auto-sorted,
 *  and the traversal order may be manually changed as well.
 * 
 *  The Hashtable class uses a HashFunctor template argument to specify how the hash code for
 *  a given KeyType object can be computed.  For built-in numeric types (e.g. int, long, etc)
 *  the default HashFunctor can be used.  For more complex types (e.g. class objects), you will
 *  need to either create your own custom HashFunctor, or add a () operator to your class that
 *  returns a uint32 HashCode.  The best way to do a custom HashFunctor is to specialize the
 *  default HashFunctor type for your class;  check the example in util/String.h for how to do this.
 */
template <class KeyType, class ValueType, class HashFunctorType = HashFunctor<KeyType> > class Hashtable
{
public:
   /** This is the signature of the type of callback function that you can pass into the
     * SetKeyCompareFunction() and SortByKey() methods.  The function should work like strcmp(),
     * return a negative value if (key1) is less than (key2), 0 if the two keys are equal,
     * or a positive value if (key1) is greater than (key2).
     * @param key1 The first key.
     * @param key2 The second key.
     * @param cookie A user-defined value that was passed in to the Sort() or SetCompareCookie() method.
     * @return An integer indicating which key is "larger", as defined above.
     */
   typedef int (*KeyCompareFunc)(const KeyType& key1, const KeyType& key2, void * cookie);

   /** This is the signature of the type of callback function that you can pass into the
     * SetValueCompareFunction() and SortByValue() methods.  The function should work like strcmp(),
     * return a negative value if (value1) is less than (value2), 0 if the two values are equal,
     * or a positive value if (value1) is greater than (value2).
     * @param value1 The first value.
     * @param value2 The second value.
     * @param cookie A user-defined value that was passed in to the Sort() or SetCompareCookie() method.
     * @return An integer indicating which value is "larger", as defined above.
     */
   typedef int (*ValueCompareFunc)(const ValueType& key1, const ValueType& key2, void * cookie);

   /** Tokens for the various auto-sort modes we know how to do. */
   enum {
      AUTOSORT_DISABLED = 0,
      AUTOSORT_BY_KEY,
      AUTOSORT_BY_VALUE
   };

   /** This constructor creates a standard, non-sorting Hashtable.
    *  @param initialCapacity Specifies the number of table slots to initially pre-allocate.   Defaults to 11.
    *  @param loadFactor Specifies how densely populated the table must be before it is re-hashed.  Defaults to 0.75 (i.e. 75%)
    *                    Must be greater than 0.0f and less than 1.0f.
    */
   Hashtable(uint32 initialCapacity = 11, float loadFactor = 0.75f);

   /** Copy Constructor.  */
   Hashtable(const Hashtable<KeyType,ValueType,HashFunctorType> & rhs);

   /** Assignment operator. */
   Hashtable<KeyType,ValueType,HashFunctorType> & operator=(const Hashtable<KeyType,ValueType,HashFunctorType> & rhs);

   /** Destructor. */
   ~Hashtable();

   /** Returns the number of items stored in the table. */
   uint32 GetNumItems() const {return _count;}

   /** Convenience method;  Returns true iff the table is empty (i.e. if GetNumItems() is zero). */
   bool IsEmpty() const {return _count == 0;}

   /** Returns true iff the table contains a mapping with the given key.  (O(1) search time) */
   bool ContainsKey(const KeyType& key) const;

   /** Returns true iff the table contains a mapping with the given value.  (Note: O(n) search time) */
   bool ContainsValue(const ValueType& value) const;

   /** Returns the given key's position in the hashtable's linked list, or -1 if the key wasn't found.  O(n) count time (if the key exists, O(1) if it doesn't) */
   int32 IndexOfKey(const KeyType& key) const;

   /** Attempts to retrieve the associated value from the table for a given key.  (O(1) lookup time)
    *  @param key The key to use to look up a value.
    *  @param setValue On success, the associated value is copied into this object.
    *  @return B_NO_ERROR on success, B_ERROR if their was no value found for the given key.
    */
   status_t Get(const KeyType& key, ValueType& setValue) const; 

   /** Retrieve a pointer to the associated value object for the given key.  (O(1) lookup time)
    *  @param key The key to use to look up a value.
    *  @return A pointer to the internally held value object for the given key,
    *          or NULL of no object was found.  Note that this object is only
    *          guaranteed to remain valid as long as the Hashtable remains unchanged.
    */
   ValueType * Get(const KeyType & key) const;

   /** Get an iterator for use with this table.
     * @param backwards Set this to true if you want to iterate through the item list backwards.  Defaults to false.
     * @return an iterator object that can be used to examine the items in the hash table. 
     */
   HashtableIterator<KeyType,ValueType,HashFunctorType> GetIterator(bool backwards = false) const {return HashtableIterator<KeyType,ValueType,HashFunctorType>(*this, backwards ? _iterTail : _iterHead, backwards);}

   /** Get an iterator for use with this table, starting at the given entry.
     * @param startAt The key in this table to start the iteration at.
     * @param backwards Set this to true if you want to iterate through the item list backwards.  Defaults to false.
     * @return an iterator object that can be used to examine the items in the hash table, starting at
     *         the specified key.  If the specified key is not in this table, an empty iterator will be returned.
     */
   HashtableIterator<KeyType,ValueType,HashFunctorType> GetIteratorAt(const KeyType & startAt, bool backwards = false) const {return HashtableIterator<KeyType,ValueType,HashFunctorType>(*this, GetEntry(startAt, NULL), backwards);}

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
    *  @retKey On success, the contents of the (index)'th key will be written into this object.
    *  @return B_NO_ERROR on success, or B_ERROR on failure.
    */
   status_t GetKeyAt(uint32 index, KeyType & retKey) const;

   /** Places the given (key, value) mapping into the table.  Any previous entry with a key of (key) will be replaced.  (average O(1) insertion time)
    *  @param key The key that the new value is to be associated with.
    *  @param value The value to associate with the new key.
    *  @param setPreviousValue If there was a previously existing value associated with (key), it will be copied into this object.
    *  @param optSetReplaced If set non-NULL, this boolean will be set to true if (setPreviousValue) was written into, false otherwise.
    *  @return B_NO_ERROR If the operation succeeded, B_ERROR if it failed (out of memory?)
    */
   status_t Put(const KeyType& key, const ValueType& value, ValueType & setPreviousValue, bool * optSetReplaced = NULL);

   /** Places the given (key, value) mapping into the table.  Any previous entry with a key of (key) will be replaced.  (average O(1) insertion time)
    *  @param key The key that the new value is to be associated with.
    *  @param value The value to associate with the new key.
    *  @return B_NO_ERROR If the operation succeeded, B_ERROR if it failed (out of memory?)
    */
   status_t Put(const KeyType& key, const ValueType& value);

   /** Removes a mapping from the table.  (O(1) removal time)
    *  @param key The key of the key-value mapping to remove.
    *  @return B_NO_ERROR if a key was found and the mapping removed, or B_ERROR if the key wasn't found.
    */
   status_t Remove(const KeyType& key);

   /** Removes the mapping with the given (key) and places its value into (setRemovedValue).  (O(1) removal time)
    *  @param key The key of the key-value mapping to remove.
    *  @param setRemovedValue On success, the removed value is copied into this object.
    *  @return B_NO_ERROR if a key was found and the mapping removed, or B_ERROR if the key wasn't found.
    */
   status_t Remove(const KeyType& key, ValueType & setRemovedValue);

   /** Removes all mappings from the hash table.  (O(n) clear time) */
   void Clear();

   /** This method can be used to activate or deactivate auto-sorting on this Hashtable.
     * If active, auto-sorting ensures that whenever Put() is called, the new/updated item is
     * moved to the correct place in the iterator traversal list.
     * Default mode is AUTOSORT_DISABLED.
     * @param mode Either AUTOSORT_DISABLED, AUTOSORT_BY_KEY, or AUTOSORT_BY_VALUE.
     * @param sortNow If true, Sort() will be called to ensure that the table is in a sorted state.
     *                You can avoid an immediate sort by specifying this parameter as false, but be aware 
     *                that auto-sorting won't sort properly if the contents of the table aren't already in
     *                in a correctly sorted state--so you may then need to call Sort() before auto-sorting 
     *                will sort properly.
     */
   void SetAutoSortMode(int mode, bool sortNow = true) {_autoSortMode = mode; if (sortNow) (void) Sort();}

   /** Returns this hash table's current auto-sort mode. */
   int GetAutoSortMode() const {return _autoSortMode;}

   /** This method can be used to set the "cookie value" that will be passed in to the
     * comparison functions specified by SetKeyCompareFunction() and SetValueCompareFunction().
     * This value will be passed along during whenever a user-defined compare function is called implicitely.
     * 
     */
   void SetCompareCookie(void * cookie) {_compareCookie = cookie;}

   /** Returns the current comparison cookie, as previously set by SetCompareCookie().  Default value is NULL. */
   void * GetCompareCookie() const {return _compareCookie;}

   /** You can call this to set a custom function to use to compare keys.  By default, the Key class's '==' operator 
    *  is used to compare keys, and no key sorting is done.  But for some key types (e.g. const char *'s) '==' is not 
    *  terribly useful, and in any case it doesn't handle sorting properly, so if you like you you can supply your 
    *  own key-comparison-function here.
    *  @see KeyCompareFunc for a description of the callback function's required semantics.
    *  @param func The new comparison function to use for this table, or NULL to revert to no key-sorting-function.
    */
   void SetKeyCompareFunction(KeyCompareFunc func) {_userKeyCompareFunc = func;}

   /** Returns the current comparison function used for comparing keys.  Default value is NULL. */
   KeyCompareFunc GetKeyCompareFunction() const {return _userKeyCompareFunc;}

   /** You can call this to set a custom function to use to compare values.  By default, no value sorting is done.
    *  If you wish to enable auto-sort-by-value, you will need to a value-comparison-function here.  
    *  @see ValueCompareFunc for a description of the callback function's required semantics.
    *  @param func The new comparison function to use for this table, or NULL to revert to no value-sorting-function.
    */
   void SetValueCompareFunction(ValueCompareFunc func) {_userValueCompareFunc = func;}

   /** Returns the current comparison function used for comparing values.  Default value is NULL. */
   ValueCompareFunc GetValueCompareFunction() const {return _userValueCompareFunc;}

   /** This method doesn anhefficient zero-copy swap of this hash table's contents with those of (swapMe).  
    *  That is to say, when this method returns, (swapMe) will be identical to the old state of this 
    *  Hashtable, and this Hashtable will be identical to the old state of (swapMe). 
    *  Any active iterators present for either table will swap owners also, becoming associated with the other table.
    *  @param swapMe The table whose contents and iterators are to be swapped with this table's.
    */
   void SwapContents(Hashtable<KeyType,ValueType,HashFunctorType> & swapMe);

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

   /** Forcefully sorts the iteration traversal list of this table using the preferred sorting mode.
     * The preferred sorting mode will be determined as follows:
     * <ol>
     *  <li>If the auto-sort mode is set, the auto-sort mode will be used.</li>
     *  <li>Else if the sort-by-value callback function is set, sort-by-value will be used.</li>
     *  <li>Else if the sort-by-key callback function is set, sort-by-key will be used.</li>
     *  <li>Else we return B_ERROR.</li>
     * </ol>
     * Note that this sort algorithm is O(n^2), so it's not efficient for very large tables.
     * @return B_NO_ERROR on success, or B_ERROR if no sort method could be determined (as described above)
     */
   status_t Sort() {return SortByAux((_autoSortMode != AUTOSORT_BY_VALUE) ? _userKeyCompareFunc : NULL, (_autoSortMode != AUTOSORT_BY_KEY) ? _userValueCompareFunc : NULL, _compareCookie);}

   /** Forcefully sorts the iteration traversal list of this table using the given key comparison function.
     * Note that the sort algorithm is O(n^2), so it's not efficient for very large tables.
     * @param func The key-comparison function to use.  (If NULL, this call becomes a no-op)
     * @param optCompareCookie Optional cookie to pass to func().
     */
   void SortByKey(KeyCompareFunc func, void * optCompareCookie = NULL) {(void) SortByAux(func, NULL, optCompareCookie);}

   /** Forcefully sorts the iteration traversal list of this table using the given value comparison function.
     * Note that the sort algorithm is O(n^2), so it's not efficient for very large tables.
     * @param func The value-comparison function to use.  (If NULL, this call becomes a no-op)
     * @param optCompareCookie Optional cookie to func().
     */
   void SortByValue(ValueCompareFunc func, void * optCompareCookie = NULL) {(void) SortByAux(NULL, func, optCompareCookie);}

private:
   friend class HashtableIterator<KeyType, ValueType, HashFunctorType>;

   class HashtableEntry
   {
   public:
      HashtableEntry() : _next(NULL), _iterPrev(NULL), _iterNext(NULL), _valid(false)
      {
         // empty
      }
 
      HashtableEntry(const HashtableEntry & rhs) : _iterPrev(NULL), _iterNext(NULL)
      {
         *this = rhs;
      }

      ~HashtableEntry()
      {
         // empty
      }

      HashtableEntry & operator=(const HashtableEntry & rhs)
      {
         _hash  = rhs._hash;
         _key   = rhs._key;
         _value = rhs._value;
         _next  = rhs._next;
         _valid = rhs._valid;
         // DO NOT copy _iterPrev and _iterNext from rhs!  They must remain the same as before.
         return * this;
      }
 
      // Returns the entry to its just-created state
      void Invalidate(const KeyType & defaultKey, const ValueType & defaultValue)
      {
         // Gotta reset, so that something like a RefCount object
         // as a KeyType or ValueType, doesn't sit around without
         // releasing its held object/memory for a loooonnng time...
         _key   = defaultKey;
         _value = defaultValue;
         _valid = false;
         _next = _iterPrev = _iterNext = NULL;
         // hash is undefined, so no point in resetting it here
      }

      uint32 _hash;               // precalculated for efficiency
      KeyType _key;               // used for '==' checking
      ValueType _value;           // payload
      HashtableEntry* _next;      // for making linked lists in our bucket
      HashtableEntry* _iterPrev;  // for table iteration
      HashtableEntry* _iterNext;  // for table iteration
      bool _valid;                // only used for entries in our _table array, (which are by value and so can't be NULL)
   };

   // Auxilliary methods
   HashtableEntry * PutAux(const KeyType& key, const ValueType& value, ValueType * optSetPreviousValue, bool * optReplacedFlag);
   status_t RemoveAux(const KeyType& key, ValueType * setRemovedValue);
   status_t SortByAux(KeyCompareFunc optKeyFunc, ValueCompareFunc optValueFunc, void * cookie);

   // Linked list maintainence
   void InsertSortedIterationEntry(HashtableEntry * e, KeyCompareFunc optKeyFunc, ValueCompareFunc optValueCompareFunc, void * cookie);
   void InsertIterationEntry(HashtableEntry * e, HashtableEntry * optBehindThisOne);
   void RemoveIterationEntry(HashtableEntry * e);
   HashtableEntry * GetEntry(const KeyType& key, HashtableEntry ** optRetPrev) const;
   HashtableEntry * GetEntryAt(uint32 idx) const;
   int CompareEntries(const HashtableEntry & left, const HashtableEntry & right, KeyCompareFunc optKeyFunc, ValueCompareFunc optValueFunc, void * cookie) const;

   // HashtableIterator's private API
   void RegisterIterator(HashtableIterator<KeyType,ValueType,HashFunctorType> * iter) const
   {
      // add him to the head of our linked list of iterators
      iter->_prevIter = NULL;
      iter->_nextIter = _iterList;  
      if (_iterList) _iterList->_prevIter = iter;
      _iterList = iter;
   }

   void UnregisterIterator(HashtableIterator<KeyType,ValueType,HashFunctorType> * iter) const
   {
      if (iter->_prevIter) iter->_prevIter->_nextIter = iter->_nextIter;
      if (iter->_nextIter) iter->_nextIter->_prevIter = iter->_prevIter;
      if (iter == _iterList) _iterList = iter->_nextIter;
      iter->_prevIter = iter->_nextIter = NULL;
   }

   KeyType * GetKeyFromCookie(void * c) const {return c ? &(((HashtableEntry *)c)->_key) : NULL;}
   ValueType * GetValueFromCookie(void * c) const  {return c ? &(((HashtableEntry *)c)->_value) : NULL;}

   void IterateCookie(void ** c, bool backwards) const 
   {
      HashtableEntry * entry = *((HashtableEntry **)c);
      *c = entry ? (backwards ? entry->_iterPrev : entry->_iterNext) : NULL;
   }

   uint32 NextPrime(uint32 start) const;
   status_t GrowTable();

   uint32 _count;       //the size of the elements in the hashtable
   uint32 _tableSize;   //the size of the table.
   uint32 _threshold;
   float _loadFactor;

   HashtableEntry * _table;
   HashtableEntry * _iterHead;    // start of linked list to iterate through
   HashtableEntry * _iterTail;    // end of linked list to iterate through

   KeyCompareFunc _userKeyCompareFunc;       // (optional) used to compare two Keys
   ValueCompareFunc _userValueCompareFunc;   // (optional) used to compare two Values
   int _autoSortMode;                        // one of the AUTOSORT_BY_* tokens
   void * _compareCookie;                    // (optional) passed to the CompareFuncs

   HashFunctorType _functor;  // used to compute hash codes for key objects

   mutable HashtableIterator<KeyType, ValueType, HashFunctorType> * _iterList;  // list of existing iterators for this table
};

//===============================================================
// Implementation of Hashtable
// Necessary location for appropriate template instantiation.
//===============================================================
template <class KeyType, class ValueType, class HashFunctorType>
Hashtable<KeyType,ValueType,HashFunctorType>::Hashtable(uint32 initialCapacity, float loadFactor)
   : _count(0), _tableSize(initialCapacity), _threshold((uint32)(initialCapacity*loadFactor)), _loadFactor(loadFactor), _table(NULL), _iterHead(NULL), _iterTail(NULL), _userKeyCompareFunc(NULL), _userValueCompareFunc(NULL), _autoSortMode(AUTOSORT_DISABLED), _compareCookie(NULL), _iterList(NULL)
{
   // empty
}

template <class KeyType, class ValueType, class HashFunctorType>
Hashtable<KeyType,ValueType,HashFunctorType>::
Hashtable(const Hashtable<KeyType,ValueType,HashFunctorType> & rhs)
   : _count(0), _tableSize(rhs._tableSize), _threshold(rhs._threshold), _loadFactor(rhs._loadFactor), _table(NULL), _iterHead(NULL), _iterTail(NULL), _userKeyCompareFunc(rhs._userKeyCompareFunc), _userValueCompareFunc(rhs._userValueCompareFunc), _autoSortMode(rhs._autoSortMode), _compareCookie(rhs._compareCookie), _iterList(NULL)
{
   *this = rhs;
}

template <class KeyType, class ValueType, class HashFunctorType>
Hashtable<KeyType, ValueType, HashFunctorType> &
Hashtable<KeyType, ValueType, HashFunctorType> ::
operator=(const Hashtable<KeyType, ValueType, HashFunctorType> & rhs)
{
   if (this != &rhs)
   {
      Clear();
      HashtableIterator<KeyType, ValueType, HashFunctorType> iter = rhs.GetIterator();
      const KeyType * nextKey;
      while((nextKey = iter.GetNextKey()) != NULL) (void) Put(*nextKey, *iter.GetNextValue());  // no good way to handle out-of-mem here?  
   }
   return *this;
}

template <class KeyType, class ValueType, class HashFunctorType>
Hashtable<KeyType,ValueType,HashFunctorType>::~Hashtable()
{
   Clear();
   delete [] _table;
}

template <class KeyType, class ValueType, class HashFunctorType>
bool
Hashtable<KeyType,ValueType,HashFunctorType>::ContainsValue(const ValueType& value) const
{
   HashtableIterator<KeyType, ValueType, HashFunctorType> iter = GetIterator();
   ValueType * v;
   while((v = iter.GetNextValue()) != NULL) if (*v == value) return true;
   return false;
}

template <class KeyType, class ValueType, class HashFunctorType>
bool
Hashtable<KeyType,ValueType,HashFunctorType>::ContainsKey(const KeyType& key) const
{
   return (GetEntry(key, NULL) != NULL);
}

template <class KeyType, class ValueType, class HashFunctorType>
int32
Hashtable<KeyType,ValueType,HashFunctorType>::IndexOfKey(const KeyType& key) const
{
   const HashtableEntry * entry = GetEntry(key, NULL);
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
const KeyType * 
Hashtable<KeyType,ValueType,HashFunctorType>::GetKeyAt(uint32 index) const
{
   HashtableEntry * e = GetEntryAt(index);
   return e ? &e->_key : NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
Hashtable<KeyType,ValueType,HashFunctorType>::GetKeyAt(uint32 index, KeyType & retKey) const
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
Hashtable<KeyType,ValueType,HashFunctorType>::Get(const KeyType& key, ValueType & setValue) const
{
   ValueType * ptr = Get(key);
   if (ptr)
   {
      setValue = *ptr;
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
ValueType *
Hashtable<KeyType,ValueType,HashFunctorType>::Get(const KeyType& key) const
{
   HashtableEntry * e = GetEntry(key, NULL);
   return e ? &e->_value : NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
typename Hashtable<KeyType,ValueType,HashFunctorType>::HashtableEntry *
Hashtable<KeyType,ValueType,HashFunctorType>::GetEntry(const KeyType& key, HashtableEntry ** optRetPrev) const
{
   if (_table)
   {
		uint32 hash = _functor(key);
		HashtableEntry * e = &_table[hash % _tableSize];
		if (e->_valid)
		{
			HashtableEntry * prev = NULL;
			while(e)
			{
				if ((e->_hash == hash)&&((_userKeyCompareFunc) ? (_userKeyCompareFunc(e->_key, key, _compareCookie) == 0) : (e->_key == key))) 
				{
					if (optRetPrev) *optRetPrev = prev;
					return e;
				}
				prev = e;
				e = e->_next; 
			}
		}
      }
   return NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
typename Hashtable<KeyType,ValueType,HashFunctorType>::HashtableEntry *
Hashtable<KeyType,ValueType,HashFunctorType>::GetEntryAt(uint32 idx) const
{
   if (idx >= GetNumItems()) return NULL;

   HashtableEntry * e = _iterHead;
   while((e)&&(idx--)) e = e->_iterNext;
   return e;
}

template <class KeyType, class ValueType, class HashFunctorType>
int
Hashtable<KeyType,ValueType,HashFunctorType>::CompareEntries(const HashtableEntry & left, const HashtableEntry & right, KeyCompareFunc optKeyFunc, ValueCompareFunc optValueFunc, void * cookie) const
{
   return optValueFunc ? optValueFunc(left._value, right._value, cookie) : (optKeyFunc ? optKeyFunc(left._key, right._key, cookie) : 0);
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
Hashtable<KeyType,ValueType,HashFunctorType>::SortByAux(KeyCompareFunc optKeyFunc, ValueCompareFunc optValueFunc, void * cookie)
{
   if ((optKeyFunc)||(optValueFunc))
   {
      HashtableIterator<KeyType,ValueType,HashFunctorType> * saveIterList = _iterList;
      _iterList = NULL;  // avoid modifying our iterators

      // First, remove all the nodes from the iteration list and put them into our private list instead.  
      HashtableEntry * privList = NULL;
      while(_iterHead)
      {
         HashtableEntry * temp = _iterHead; // save since RemoveIterationEntry() will change _iterHead
         RemoveIterationEntry(_iterHead);
         temp->_iterNext = privList;
         privList = temp;
      }

      // Now we go through our list and add everything back using the given function, 
      while(privList)
      {
         HashtableEntry * next = privList->_iterNext;
         InsertSortedIterationEntry(privList, optKeyFunc, optValueFunc, cookie);
         privList = next;
      } 

      _iterList = saveIterList;  // lastly restore our iterators
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
Hashtable<KeyType,ValueType,HashFunctorType>::GrowTable()
{
   // 1. Initialize the scratch space for our active iterators.
   {
      HashtableIterator<KeyType,ValueType,HashFunctorType> * nextIter = _iterList;
      while(nextIter)
      {
         nextIter->_scratchSpace[0] = nextIter->_scratchSpace[1] = NULL;  // these will hold our switch-to-on-success values
         nextIter = nextIter->_nextIter;
      }
   }
    
   // 2. Create a new, bigger table, and fill it with a copy of all of our data.
   Hashtable<KeyType,ValueType,HashFunctorType> biggerTable(NextPrime(2*_tableSize), _loadFactor);
   biggerTable.SetKeyCompareFunction(_userKeyCompareFunc);
   biggerTable.SetValueCompareFunction(_userValueCompareFunc);
   biggerTable.SetCompareCookie(_compareCookie);
   biggerTable.SetAutoSortMode(_autoSortMode);
   {
      HashtableEntry * next = _iterHead;
      while(next)
      {
         HashtableEntry * hisClone = biggerTable.PutAux(next->_key, next->_value, NULL, NULL);
         if (hisClone)
         {
            // Mark any iterators that will need to be redirected to point to the new nodes.
            HashtableIterator<KeyType,ValueType,HashFunctorType> * nextIter = _iterList;
            while(nextIter)
            {
               if (nextIter->_nextKeyCookie   == next) nextIter->_scratchSpace[0] = hisClone;
               if (nextIter->_nextValueCookie == next) nextIter->_scratchSpace[1] = hisClone;
               nextIter = nextIter->_nextIter;
            }
         }
         else return B_ERROR;  // oops, out of mem, too bad.  

         next = next->_iterNext;
      }
   }

   // 3. Swap contents with the bigger table, but don't swap iterator lists (we want to keep ours!)
   {
      HashtableIterator<KeyType,ValueType,HashFunctorType> * temp = _iterList;
      _iterList = NULL;
      SwapContents(biggerTable);
      _iterList = temp;
   }

   // 4. Lastly, fix up our iterators to point to their new entries.
   {
      HashtableIterator<KeyType,ValueType,HashFunctorType> * nextIter = _iterList;
      while(nextIter)
      {
         nextIter->_nextKeyCookie   = nextIter->_scratchSpace[0];
         nextIter->_nextValueCookie = nextIter->_scratchSpace[1];
         nextIter = nextIter->_nextIter;
      }
   }

   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
void
Hashtable<KeyType,ValueType,HashFunctorType>::SwapContents(Hashtable<KeyType,ValueType,HashFunctorType> & swapMe)
{
   muscleSwap(_count,                swapMe._count);
   muscleSwap(_tableSize,            swapMe._tableSize);
   muscleSwap(_threshold,            swapMe._threshold);
   muscleSwap(_loadFactor,           swapMe._loadFactor);
   muscleSwap(_table,                swapMe._table);
   muscleSwap(_iterHead,             swapMe._iterHead);
   muscleSwap(_iterTail,             swapMe._iterTail);
   muscleSwap(_userKeyCompareFunc,   swapMe._userKeyCompareFunc);
   muscleSwap(_userValueCompareFunc, swapMe._userValueCompareFunc);
   muscleSwap(_autoSortMode,         swapMe._autoSortMode);
   muscleSwap(_compareCookie,        swapMe._compareCookie);
   muscleSwap(_iterList,             swapMe._iterList);

   // Lastly, swap the owners of all iterators, so that they will unregister from the correct table when they die
   {
      HashtableIterator<KeyType,ValueType,HashFunctorType> * next = _iterList;
      while(next)
      {
         next->_owner = &swapMe;
         next = next->_nextIter;
      }
   }
   {
      HashtableIterator<KeyType,ValueType,HashFunctorType> * next = swapMe._iterList;
      while(next)
      {
         next->_owner = this;
         next = next->_nextIter;
      }
   }
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
Hashtable<KeyType,ValueType,HashFunctorType>::MoveToFront(const KeyType & moveMe)
{
   HashtableEntry * e = GetEntry(moveMe, NULL);
   if (e == NULL) return B_ERROR;
   if (e->_iterPrev)
   {
      RemoveIterationEntry(e);
      InsertIterationEntry(e, NULL);
   }
   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
Hashtable<KeyType,ValueType,HashFunctorType>::MoveToBack(const KeyType & moveMe)
{
   HashtableEntry * e = GetEntry(moveMe, NULL);
   if (e == NULL) return B_ERROR;
   if (e->_iterNext)
   {
      RemoveIterationEntry(e);
      InsertIterationEntry(e, _iterTail);
   }
   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
Hashtable<KeyType,ValueType,HashFunctorType>::MoveToBefore(const KeyType & moveMe, const KeyType & toBeforeMe)
{
   HashtableEntry * e = GetEntry(moveMe, NULL);
   HashtableEntry * f = GetEntry(toBeforeMe, NULL);
   if ((e == NULL)||(f == NULL)||(e == f)) return B_ERROR;
   if (e->_iterNext != f)
   {
      RemoveIterationEntry(e);
      InsertIterationEntry(e, f->_iterPrev);
   }
   return B_NO_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t 
Hashtable<KeyType,ValueType,HashFunctorType>::MoveToBehind(const KeyType & moveMe, const KeyType & toBehindMe)
{
   HashtableEntry * d = GetEntry(toBehindMe, NULL);
   HashtableEntry * e = GetEntry(moveMe, NULL);
   if ((d == NULL)||(e == NULL)||(d == e)) return B_ERROR;
   if (e->_iterPrev != d)
   {
      RemoveIterationEntry(e);
      InsertIterationEntry(e, d);
   }
   return B_NO_ERROR;
}

// Adds (e) to the end of our iteration linked list, or to an appropriate location to maintain sorting, if a sorting function is specified.
template <class KeyType, class ValueType, class HashFunctorType>
void
Hashtable<KeyType,ValueType,HashFunctorType>::InsertSortedIterationEntry(HashtableEntry * e, KeyCompareFunc optKeyFunc, ValueCompareFunc optValueFunc, void * cookie)
{
   HashtableEntry * insertAfter = _iterTail;  // default to appending to the end of the list
   if ((_iterHead)&&((optKeyFunc)||(optValueFunc)))
   {
      // We're in sorted mode, so we'll try to place this guy in the correct position.
           if (CompareEntries(*e, *_iterHead, optKeyFunc, optValueFunc, cookie) < 0) insertAfter = NULL;  // easy; append to the head of the list
      else if (CompareEntries(*e, *_iterTail, optKeyFunc, optValueFunc, cookie) < 0)  // only iterate through if we're before the tail, otherwise the tail is fine
      {
         HashtableEntry * prev = _iterHead;
         HashtableEntry * next = _iterHead->_iterNext;  // more difficult;  find where to insert into the middle
         while(next)
         {
            if (CompareEntries(*e, *next, optKeyFunc, optValueFunc, cookie) < 0)
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

// Adds (e) to the our iteration linked list, behind (optBehindThis), or at the head if (optBehindThis) is NULL.
template <class KeyType, class ValueType, class HashFunctorType>
void
Hashtable<KeyType,ValueType,HashFunctorType>::InsertIterationEntry(HashtableEntry * e, HashtableEntry * optBehindThis)
{
   e->_iterPrev = optBehindThis;
   e->_iterNext = optBehindThis ? optBehindThis->_iterNext : _iterHead;
   if (e->_iterPrev) e->_iterPrev->_iterNext = e;
                else _iterHead = e;
   if (e->_iterNext) e->_iterNext->_iterPrev = e;
                else _iterTail = e;
}

// Remove (e) from our iteration linked list
template <class KeyType, class ValueType, class HashFunctorType>
void 
Hashtable<KeyType,ValueType,HashFunctorType>::RemoveIterationEntry(HashtableEntry * e)
{
   // Update any iterators that were pointing at (e), so that they now point to the entry after e.
   HashtableIterator<KeyType, ValueType, HashFunctorType> * next = _iterList;
   while(next)
   {
      if (next->_nextKeyCookie   == e) (void) next->GetNextKey();
      if (next->_nextValueCookie == e) (void) next->GetNextValue();
      next = next->_nextIter;
   }

   if (_iterHead == e) _iterHead = e->_iterNext;
   if (_iterTail == e) _iterTail = e->_iterPrev;
   if (e->_iterPrev) e->_iterPrev->_iterNext = e->_iterNext;
   if (e->_iterNext) e->_iterNext->_iterPrev = e->_iterPrev;
   e->_iterPrev = e->_iterNext = NULL; 
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
Hashtable<KeyType,ValueType,HashFunctorType>::Put(const KeyType& key, const ValueType& value)
{
   return (PutAux(key, value, NULL, NULL) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
Hashtable<KeyType,ValueType,HashFunctorType>::Put(const KeyType& key, const ValueType& value, ValueType & setPreviousValue, bool * optReplacedFlag)
{
   return (PutAux(key, value, &setPreviousValue, optReplacedFlag) != NULL) ? B_NO_ERROR : B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
typename Hashtable<KeyType,ValueType,HashFunctorType>::HashtableEntry *
Hashtable<KeyType,ValueType,HashFunctorType>::PutAux(const KeyType& key, const ValueType& value, ValueType * optSetPreviousValue, bool * optReplacedFlag)
{
   if (optReplacedFlag) *optReplacedFlag = false;

   if (_table == NULL)  // demand-allocate this
   {
      _table = newnothrow HashtableEntry[_tableSize];
      if (_table == NULL) {WARN_OUT_OF_MEMORY; return NULL;}
   }

   uint32 hash = _functor(key);
   uint32 index = hash % _tableSize;

   // If we already have an entry for this key in the table, we can just replace its contents
   HashtableEntry * e = GetEntry(key, NULL);
   if (e)
   {
      if (optSetPreviousValue) *optSetPreviousValue = e->_value;
      if (optReplacedFlag)     *optReplacedFlag     = true;
      e->_value = value;
      if ((_autoSortMode == AUTOSORT_BY_VALUE)&&(_userValueCompareFunc))
      {
         // If our new value has changed our position in the sort-order, then adjust the traversal list
         if ((e->_iterPrev)&&(_userValueCompareFunc(e->_value, e->_iterPrev->_value, _compareCookie) < 0))
         {
            const HashtableEntry * moveToBefore = e->_iterPrev;
            while((moveToBefore->_iterPrev)&&(_userValueCompareFunc(e->_value, moveToBefore->_iterPrev->_value, _compareCookie) < 0)) moveToBefore = moveToBefore->_iterPrev;
            (void) MoveToBefore(e->_key, moveToBefore->_key);
         }
         else if ((e->_iterNext)&&(_userValueCompareFunc(e->_value, e->_iterNext->_value, _compareCookie) > 0))
         {
            const HashtableEntry * moveToBehind = e->_iterNext;
            while((moveToBehind->_iterNext)&&(_userValueCompareFunc(e->_value, moveToBehind->_iterNext->_value, _compareCookie) > 0)) moveToBehind = moveToBehind->_iterNext;
            (void) MoveToBehind(e->_key, moveToBehind->_key);
         }
      }
      return e;
   }

   // Rehash the table if the threshold is exceeded
   if (_count >= _threshold) return (GrowTable() == B_NO_ERROR) ? PutAux(key, value, optSetPreviousValue, optReplacedFlag) : NULL;

   HashtableEntry * slot = &_table[index];
   if (slot->_valid)
   {
      // Creates new entry in linked list for this slot
      HashtableEntry * newEntry = newnothrow HashtableEntry();
      if (newEntry == NULL) {WARN_OUT_OF_MEMORY; return NULL;}
      newEntry->_hash  = hash;
      newEntry->_key   = key;
      newEntry->_value = value;
      newEntry->_valid = true;
      newEntry->_next  = slot->_next;
      slot->_next = newEntry;
      InsertSortedIterationEntry(newEntry, (_autoSortMode == AUTOSORT_BY_KEY) ? _userKeyCompareFunc : NULL, (_autoSortMode == AUTOSORT_BY_VALUE) ? _userValueCompareFunc : NULL, _compareCookie);
      e = newEntry;
   }
   else
   {
      // First entry in slot; just copy data over
      slot->_hash  = hash;
      slot->_key   = key;
      slot->_value = value;  
      slot->_next  = NULL;
      slot->_valid = true;
      e = slot;
      InsertSortedIterationEntry(slot, (_autoSortMode == AUTOSORT_BY_KEY) ? _userKeyCompareFunc : NULL, (_autoSortMode == AUTOSORT_BY_VALUE) ? _userValueCompareFunc : NULL, _compareCookie);
   }
   _count++;

   return e; 
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
Hashtable<KeyType,ValueType,HashFunctorType>::Remove(const KeyType& key)
{
   return RemoveAux(key, NULL);
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
Hashtable<KeyType,ValueType,HashFunctorType>::Remove(const KeyType& key, ValueType & setValue)
{
   return RemoveAux(key, &setValue);
}


template <class KeyType, class ValueType, class HashFunctorType>
status_t
Hashtable<KeyType,ValueType,HashFunctorType>::RemoveAux(const KeyType& key, ValueType * optSetValue)
{
   HashtableEntry * prev;
   HashtableEntry * e = GetEntry(key, &prev);
   if (e)
   {
      if (optSetValue) *optSetValue = e->_value;
      HashtableEntry * next = e->_next;
      if (prev)
      {
         prev->_next = next;
         RemoveIterationEntry(e);
         delete e;
      }
      else
      {
         if (next) 
         {
            // Remove (e) from the iteration list (it's not in the correct position; it needs to be where (next) is)
            RemoveIterationEntry(e);

            // Replace (next) with (e) in the iteration list
            {
               e->_iterPrev = next->_iterPrev;
               e->_iterNext = next->_iterNext;
               if (e->_iterPrev) e->_iterPrev->_iterNext = e;
               if (e->_iterNext) e->_iterNext->_iterPrev = e;
               if (_iterHead == next) _iterHead = e;
               if (_iterTail == next) _iterTail = e;
            }

            // e takes over next's data
            *e = *next;
            e->_valid = true;

            // Update any iterators that were pointing at (next), so that they now point to e.
            HashtableIterator<KeyType, ValueType, HashFunctorType> * nextIter = _iterList;
            while(nextIter)
            {
               if (nextIter->_nextKeyCookie   == next) nextIter->_nextKeyCookie   = e;
               if (nextIter->_nextValueCookie == next) nextIter->_nextValueCookie = e;
               nextIter = nextIter->_nextIter;
            }

            delete next;
         }
         else 
         {
            RemoveIterationEntry(e);
            KeyType   blankKey = KeyType();
            ValueType blankVal = ValueType();
            e->Invalidate(blankKey, blankVal);
         }
      }
      _count--;
      return B_NO_ERROR;
   }
   return B_ERROR;
}

template <class KeyType, class ValueType, class HashFunctorType>
void
Hashtable<KeyType,ValueType,HashFunctorType>::Clear()
{
   if (_count > 0)
   {
      // First go through our list of active iterators, and let them all know they are now invalid
      while(_iterList)
      {
         HashtableIterator<KeyType,ValueType,HashFunctorType> * next = _iterList->_nextIter;
         *_iterList = HashtableIterator<KeyType,ValueType,HashFunctorType>();
         _iterList = next;
      }

      // Then go through our list of data and remove/reset it all
      KeyType   blankKey   = KeyType();
      ValueType blankValue = ValueType();
      while(_iterHead)
      {
         HashtableEntry * next = _iterHead->_iterNext;  // save for later
         if ((_iterHead >= _table)&&(_iterHead < _table+_tableSize)) _iterHead->Invalidate(blankKey, blankValue);
                                                                else delete _iterHead;
         _iterHead = next;
      }
      _iterTail = NULL;
      _count = 0;
   }
}

template <class KeyType, class ValueType, class HashFunctorType>
uint32
Hashtable<KeyType,ValueType,HashFunctorType>::NextPrime(uint32 start) const
{
   if (start % 2 == 0) start++;
   uint32 i;
   for(; ; start += 2)
   {
      for(i = 3; i * i <= start; i += 2) if (start % i == 0) break;
      if (i * i > start) return start;
   }
}

//===============================================================
// Implementation of HashtableIterator
//===============================================================

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::HashtableIterator() : _nextKeyCookie(NULL), _nextValueCookie(NULL), _owner(NULL)
{
   // empty
}

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::HashtableIterator(const HashtableIterator<KeyType, ValueType, HashFunctorType> & rhs) : _owner(NULL)
{
   *this = rhs;
}

template <class KeyType, class ValueType, class HashFunctorType>
HashtableIterator<KeyType, ValueType, HashFunctorType>::HashtableIterator(const Hashtable<KeyType, ValueType, HashFunctorType> & owner, void * startCookie, bool backwards) : _nextKeyCookie(startCookie), _nextValueCookie(startCookie), _backwards(backwards), _owner(&owner)
{
   _owner->RegisterIterator(this);
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
   if (_owner != rhs._owner)
   {
      if (_owner) _owner->UnregisterIterator(this);
      _owner = rhs._owner;
      if (_owner) _owner->RegisterIterator(this);
   }
   _backwards       = rhs._backwards;
   _nextKeyCookie   = rhs._nextKeyCookie;
   _nextValueCookie = rhs._nextValueCookie;
   return *this;
}


template <class KeyType, class ValueType, class HashFunctorType>
bool
HashtableIterator<KeyType,ValueType,HashFunctorType>::HasMoreKeys() const
{
   return (_nextKeyCookie != NULL);
}

template <class KeyType, class ValueType, class HashFunctorType>
bool
HashtableIterator<KeyType,ValueType,HashFunctorType>::HasMoreValues() const
{
   return (_nextValueCookie != NULL);
}

template <class KeyType, class ValueType, class HashFunctorType>
status_t
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKey(KeyType& key) 
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
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextValue(ValueType& val)
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
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextKey(KeyType& key) const
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
HashtableIterator<KeyType,ValueType,HashFunctorType>::PeekNextValue(ValueType& val) const
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
ValueType *
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextValue()
{
   ValueType * val = PeekNextValue();
   if (val)
   {
      _owner->IterateCookie(&_nextValueCookie, _backwards);
      return val;
   }
   return NULL;
}

template <class KeyType, class ValueType, class HashFunctorType>
const KeyType *
HashtableIterator<KeyType,ValueType,HashFunctorType>::GetNextKey() 
{
   const KeyType * ret = PeekNextKey();
   if (ret)
   {
      _owner->IterateCookie(&_nextKeyCookie, _backwards);
      return ret;
   }
   return NULL;
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

};  // end namespace muscle

#endif
