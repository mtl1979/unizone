/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleQueue_h
#define MuscleQueue_h

#include "support/MuscleSupport.h"

namespace muscle {

#ifndef SMALL_QUEUE_SIZE
# define SMALL_QUEUE_SIZE 3
#endif

/** This class implements a templated double-ended queue data structure.
 *  Adding or removing items from the head or tail of a Queue is (on average)
 *  an O(1) operation.  A Queue also makes for a nice Vector, if that's all you need.
 */
template <class ItemType> class Queue
{
public:
   /** Constructor.
    *  @param initialSlots Specifies how many slots to pre-allocate.  Defaults to (SMALL_QUEUE_SIZE).
    */
   Queue(uint32 initialSlots = SMALL_QUEUE_SIZE);

   /** Copy constructor. */
   Queue(const Queue& copyMe);

   /** Destructor. */
   virtual ~Queue();

   /** Assigment operator. */
   Queue &operator=(const Queue &from);

   /** Equality operator.  Queues are equal if they are the same length, and
     * every nth item in this queue is == to the corresponding item in (rhs). */
   bool operator==(const Queue &rhs) const;

   /** Returns the negation of the equality operator */
   bool operator!=(const Queue &rhs) const {return !(*this == rhs);}

   /** Appends a default-constructed item to the end of the queue.  Queue size grows by one.
    *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
    */
   status_t AddTail() {ItemType blank = ItemType(); return AddTail(blank);}

   /** Appends (item) to the end of the queue.  Queue size grows by one.
    *  @param item The item to append. 
    *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
    */
   status_t AddTail(const ItemType & item);

   /** Appends all items in (queue) to the end of our queue.  Queue size
    *  grows by (queue.GetNumItems()).
    *  For example:
    *    Queue a;   // contains 1, 2, 3, 4
    *    Queue b;   // contains 5, 6, 7, 8
    *    a.AddTail(b);      // a now contains 1, 2, 3, 4, 5, 6, 7, 8
    *  @param item The queue to append to our queue.
    *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
    */
   status_t AddTail(const Queue<ItemType> & queue);

   /** Prepends a default-constructed item to the head of the queue.  Queue size grows by one.
    *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
    */
   status_t AddHead() {ItemType blank = ItemType(); return AddHead(blank);}

   /** Prepends (item) to the head of the queue.  Queue size grows by one.
    *  @param item The item to prepend. 
    *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
    */
   status_t AddHead(const ItemType &item);

   /** Concatenates (queue) to the head of our queue.
    *  Our queue size grows by (queue.GetNumItems()).
    *  For example:
    *    Queue a;   // contains 1, 2, 3, 4
    *    Queue b;   // contains 5, 6, 7, 8
    *    a.AddHead(b);      // a now contains 5, 6, 7, 8, 1, 2, 3, 4
    *  @param item The queue to prepend to our queue.
    *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
    */
   status_t AddHead(const Queue<ItemType> & queue);

   /** Removes the item at the head of the queue.
    *  @return B_NO_ERROR on success, B_ERROR if the queue was empty.
    */
   status_t RemoveHead();

   /** Removes the item at the head of the queue and places it into (returnItem).
    *  @param returnItem On success, the removed item is copied into this object.
    *  @return B_NO_ERROR on success, B_ERROR if the queue was empty
    */
   status_t RemoveHead(ItemType & returnItem);

   /** Removes the item at the tail of the queue.
    *  @return B_NO_ERROR on success, B_ERROR if the queue was empty.
    */
   status_t RemoveTail();

   /** Removes the item at the tail of the queue and places it into (returnItem).
    *  @param returnItem On success, the removed item is copied into this object.
    *  @return B_NO_ERROR on success, B_ERROR if the queue was empty
    */
   status_t RemoveTail(ItemType & returnItem);

   /** Removes the item at the (index)'th position in the queue.
    *  @param index Which item to remove--can range from zero 
    *               (head of the queue) to GetNumItems()-1 (tail of the queue).
    *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. bad index)
    *  Note that this method is somewhat inefficient for indices that
    *  aren't at the head or tail of the queue (i.e. O(n) time)
    */
   status_t RemoveItemAt(uint32 index);

   /** Removes the item at the (index)'th position in the queue, and copies it into (returnItem).
    *  @param index Which item to remove--can range from zero 
    *               (head of the queue) to (GetNumItems()-1) (tail of the queue).
    *  @param returnItem On success, the removed item is copied into this object.
    *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. bad index)
    */
   status_t RemoveItemAt(uint32 index, ItemType & returnItem);

   /** Copies the (index)'th item into (returnItem).
    *  @param index Which item to get--can range from zero 
    *               (head of the queue) to (GetNumItems()-1) (tail of the queue).
    *  @param returnItem On success, the retrieved item is copied into this object.
    *  @return B_NO_ERROR on success, B_ERROR on failure (e.g. bad index)
    */
   status_t GetItemAt(uint32 index, ItemType & returnItem) const;

   /** Returns a pointer to an item in the array (i.e. no copying of the item is done).  
    *  Included for efficiency; be careful with this: modifying the queue can invalidate 
    *  the returned pointer!  
    *  @param index Index of the item to return a pointer to.
    *  @return a pointer to the internally held item, or NULL if (index) was invalid.
    */
   ItemType * GetItemAt(uint32 index) const;

   /** Replaces the (index)'th item in the queue with a default-constructed item.
    *  @param index Which item to replace--can range from zero 
    *               (head of the queue) to (GetNumItems()-1) (tail of the queue).
    *  @return B_NO_ERROR on success, B_ERROR on failure (e.g. bad index)
    */
   status_t ReplaceItemAt(uint32 index) {ItemType blank = ItemType(); return ReplaceItemAt(index, blank);}
 
   /** Replaces the (index)'th item in the queue with (newItem).
    *  @param index Which item to replace--can range from zero 
    *               (head of the queue) to (GetNumItems()-1) (tail of the queue).
    *  @param newItem The item to place into the queue at the (index)'th position.
    *  @return B_NO_ERROR on success, B_ERROR on failure (e.g. bad index)
    */
   status_t ReplaceItemAt(uint32 index, const ItemType & newItem);
 
   /** Inserts a default-constructed item into the (nth) slot in the array.  InsertItemAt(0)
    *  is the same as AddHead(item), InsertItemAt(GetNumItems()) is the same
    *  as AddTail(item).  Other positions will involve an O(n) shifting of contents.
    *  @param index The position at which to insert the new item.
    *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. bad index).
    */
   status_t InsertItemAt(uint32 index) {ItemType blank = ItemType(); return InsertItemAt(index, blank);}
   
   /** Inserts (item) into the (nth) slot in the array.  InsertItemAt(0)
    *  is the same as AddHead(item), InsertItemAt(GetNumItems()) is the same
    *  as AddTail(item).  Other positions will involve an O(n) shifting of contents.
    *  @param index The position at which to insert the new item.
    *  @param newItem The item to insert into the queue.
    *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. bad index).
    */
   status_t InsertItemAt(uint32 index, const ItemType & newItem);
   
   /** Removes all items from the queue. */
   void Clear();

   /** Returns the number of items in the queue.  (This number does not include pre-allocated space) */
   uint32 GetNumItems() const {return _itemCount;}

   /** Returns true iff their are no items in the queue. */
   bool IsEmpty() const {return (_itemCount == 0);}

   /** Returns a read-only reference the head item in the queue.  You must not call this when the queue is empty! */
   const ItemType & Head() const {return *GetItemAt(0);}

   /** Returns a read-only reference the tail item in the queue.  You must not call this when the queue is empty! */
   const ItemType & Tail() const {return *GetItemAt(_itemCount-1);}

   /** Returns a writable reference the head item in the queue.  You must not call this when the queue is empty! */
   ItemType & Head() {return *GetItemAt(0);}

   /** Returns a writable reference the tail item in the queue.  You must not call this when the queue is empty! */
   ItemType & Tail() {return *GetItemAt(_itemCount-1);}

   /** Returns a pointer to the first item in the queue, or NULL if the queue is empty */
   ItemType * HeadPointer() const {return (_itemCount > 0) ? GetItemAt(0) : NULL;}

   /** Returns a pointer to the last item in the queue, or NULL if the queue is empty */
   ItemType * TailPointer() const {return (_itemCount > 0) ? GetItemAt(_itemCount-1) : NULL;}

   /** Convenient read-only array-style operator (be sure to only use valid indices!) */
   const ItemType & operator [](uint32 Index) const; 

   /** Convenient read-write array-style operator (be sure to only use valid indices!) */
   ItemType & operator [](uint32 Index);

   /** Deprecated synonym for GetItemAt().  Don't call this method in new code, call GetItemAt() instead!
     * @deprecated
     */
   ItemType * GetItemPointer(uint32 index) const {return GetItemAt(index);}

   /** Makes sure there is enough space allocated for at least (numSlots) items.  
    *  You only need to call this if you wish to minimize the number of data re-allocations done,
    *  or wish to add or remove a large number of default items at once (by specifying setNumItems=true).
    *  @param numSlots the minimum amount of items to pre-allocate space for in the Queue.
    *  @param setNumItems If true, the length of the Queue will be altered by adding or removing
    *                     items to (from) the tail of the Queue until the Queue is the specified size.
    *                     If false (the default), more slots may be pre-allocated, but the 
    *                     number of items officially in the Queue remains the same as before.
    *  @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory)
    */
   status_t EnsureSize(uint32 numSlots, bool setNumItems = false);

   /** Returns the last index of the given (item), or -1 if (item) is
    *  not found in the list.  O(n) search time.
    *  @param item The item to look for.
    *  @return The index of (item), or -1 if no such item is present.
    */
   int32 IndexOf(const ItemType & item) const;

   /**
    *  Swaps the values of the two items at the given indices.  This operation
    *  will involve three copies of the held items.
    *  @param fromIndex First index.   0 < fromIndex < GetNumItems().
    *  @param toIndex.  Second index.  0 < toIndex   < GetNumItems().
    */
   void Swap(uint32 fromIndex, uint32 toIndex);

   /**
    *  Reverses the ordering of the items in the given subrange.
    *  (e.g. if the items were A,B,C,D,E, this would change them to E,D,C,B,A)
    *  @param from Index of the start of the subrange.  Defaults to zero.
    *  @param to Index of the next item after the end of the subrange.  If greater than
    *         the number of items currently in the queue, this value will be clipped
    *         to be equal to that number.  Defaults to the largest possible uint32.
    */
   void ReverseItemOrdering(uint32 from=0, uint32 to = ((uint32)-1)); 

   /**
    *  This is the signature of the type of callback function that you must pass 
    *  into the Sort() method.  This function should work like strcmp(), returning
    *  a negative value if (item1) is less than item2, or zero if the items are
    *  equal, or a positive value if (item1) is greater than item2.
    *  @param item1 The first item
    *  @param item2 The second item
    *  @param cookie A user-defined value that was passed in to the Sort() method.
    *  @return A value indicating which item is "larger", as defined above.
    */
   typedef int (*ItemCompareFunc)(const ItemType & item1, const ItemType & item2, void * cookie);

   /**
    *  Does an in-place, stable sort on a subrange of the contents of this Queue.
    *  (The sort algorithm is a smart, in-place merge sort).
    *  @param compareFunc A function that compares two items in this queue and returns
    *         a value indicating which is "larger".  (negative indicates
    *         that the second parameter is larger, positive indicate that the
    *         first is larger, and zero indicates that the two parameters are equal)
    *  @param from Index of the start of the subrange.  Defaults to zero.
    *  @param to Index of the next item after the end of the subrange.  
    *         If greater than the number of items currently in the queue, 
    *         the subrange will extend to the last item.  Defaults to the largest possible uint32.
    *  @param optCookie A user-defined value that will be passed to the (compareFunc).
    */
   void Sort(ItemCompareFunc compareFunc, uint32 from=0, uint32 to = ((uint32)-1), void * optCookie = NULL);

   /**
    *  Goes through the array and removes every item that is equal to (val).
    *  @param val the item to look for and remove
    *  @return The number of instances of (val) that were found and removed during this operation.
    */
   uint32 RemoveAllInstancesOf(const ItemType & val);

   /**
    *  Goes through the array and removes the first item that is equal to (val).
    *  @param val the item to look for and remove
    *  @return B_NO_ERROR if a matching item was found and removed, or B_ERROR if it wasn't found.
    */
   status_t RemoveFirstInstanceOf(const ItemType & val);

   /**
    *  Goes through the array and removes the last item that is equal to (val).
    *  @param val the item to look for and remove
    *  @return B_NO_ERROR if a matching item was found and removed, or B_ERROR if it wasn't found.
    */
   status_t RemoveLastInstanceOf(const ItemType & val);

private:
   inline uint32 NextIndex(uint32 idx) const {return (idx >= _queueSize-1) ? 0 : idx+1;}
   inline uint32 PrevIndex(uint32 idx) const {return (idx == 0) ? _queueSize-1 : idx-1;}

   // Translates a user-index into an index into the _queue array.
   inline uint32 InternalizeIndex(uint32 idx) const {return (_headIndex + idx) % _queueSize;}

   // Helper methods, used for sorting (stolen from http://www-ihm.lri.fr/~thomas/VisuTri/inplacestablesort.html)
   void Merge(ItemCompareFunc compareFunc, uint32 from, uint32 pivot, uint32 to, uint32 len1, uint32 len2, void * cookie);
   uint32 Lower(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType & val, void * cookie) const;
   uint32 Upper(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType & val, void * cookie) const;

   ItemType _smallQueue[SMALL_QUEUE_SIZE];  // small queues can be stored inline in this array
   ItemType * _queue;                       // points to _smallQueue, or to a dynamically alloc'd array
   uint32 _queueSize;  // number of slots in the _queue array
   uint32 _itemCount;  // number of valid items in the array
   uint32 _headIndex;  // index of the first filled slot (meaningless if _itemCount is zero)
   uint32 _tailIndex;  // index of the last filled slot (meaningless if _itemCount is zero)
   const uint32 _initialSize;  // as specified in ctor
};

template <class ItemType>
Queue<ItemType>::Queue(uint32 initialSize)
   : _queue(NULL), _queueSize(0), _itemCount(0), _initialSize(initialSize)
{
   // empty
}

template <class ItemType>
Queue<ItemType>::Queue(const Queue& rhs)
   : _queue(NULL), _queueSize(0), _itemCount(0), _initialSize(rhs._initialSize)
{
   *this = rhs;
}

template <class ItemType>
bool
Queue<ItemType>::operator ==(const Queue& rhs) const
{
   if (this == &rhs) return true;
   if (GetNumItems() != rhs.GetNumItems()) return false;

   for (int i = GetNumItems()-1; i>=0; i--) if ((*this)[i] != rhs[i]) return false;

   return true;
}


template <class ItemType>
Queue<ItemType> &
Queue<ItemType>::operator =(const Queue& rhs)
{
   if (this != &rhs)
   {
      Clear();
      uint32 numItems = rhs.GetNumItems();
      if (EnsureSize(numItems) == B_NO_ERROR)
      {
         for (uint32 i=0; i<numItems; i++) 
         {
            if (AddTail(*rhs.GetItemAt(i)) == B_ERROR) break;  // trouble!?  Shouldn't happen...
         }
      }
   }
   return *this;
}

template <class ItemType>
ItemType &
Queue<ItemType>::operator[](uint32 i)
{
   MASSERT(i<_itemCount, "Invalid index to Queue::[]");
   return _queue[InternalizeIndex(i)];
}

template <class ItemType>
const ItemType &
Queue<ItemType>::operator[](uint32 i) const
{
   MASSERT(i<_itemCount, "Invalid index to Queue::[]");
   return _queue[InternalizeIndex(i)];
}             

template <class ItemType>
ItemType * 
Queue<ItemType>::GetItemAt(uint32 i) const
{
   return &_queue[InternalizeIndex(i)];
}

template <class ItemType>
Queue<ItemType>::~Queue()
{
   if (_queue != _smallQueue) delete [] _queue;
}

template <class ItemType>
status_t 
Queue<ItemType>::
AddTail(const ItemType &item)
{
   if (EnsureSize(_itemCount+1) == B_ERROR) return B_ERROR;
   if (_itemCount == 0) _headIndex = _tailIndex = 0;
                   else _tailIndex = NextIndex(_tailIndex);
   _queue[_tailIndex] = item;
   _itemCount++;
   return B_NO_ERROR;
}


template <class ItemType>
status_t 
Queue<ItemType>::
AddTail(const Queue<ItemType> &queue)
{
   uint32 qSize = queue.GetNumItems();
   for (uint32 i=0; i<qSize; i++) if (AddTail(queue[i]) == B_ERROR) return B_ERROR;
   return B_NO_ERROR;
}

template <class ItemType>
status_t 
Queue<ItemType>::
AddHead(const ItemType &item)
{
   if (EnsureSize(_itemCount+1) == B_ERROR) return B_ERROR;
   if (_itemCount == 0) _headIndex = _tailIndex = 0;
                   else _headIndex = PrevIndex(_headIndex);
   _queue[_headIndex] = item;
   _itemCount++;
   return B_NO_ERROR;
}

template <class ItemType>
status_t 
Queue<ItemType>::
AddHead(const Queue<ItemType> &queue)
{
   for (int i=((int)queue.GetNumItems())-1; i>=0; i--) if (AddHead(queue[i]) == B_ERROR) return B_ERROR;
   return B_NO_ERROR;
}

template <class ItemType>
status_t 
Queue<ItemType>::
RemoveHead(ItemType & returnItem)
{
   if (_itemCount == 0) return B_ERROR;
   returnItem = _queue[_headIndex];
   return RemoveHead();
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveHead()
{
   if (_itemCount == 0) return B_ERROR;
   int oldHeadIndex = _headIndex;
   _headIndex = NextIndex(_headIndex);
   _itemCount--;
   ItemType blank = ItemType();
   _queue[oldHeadIndex] = blank;  // this must be done last, as queue state must be coherent when we do this
   return B_NO_ERROR;
}

template <class ItemType>
status_t 
Queue<ItemType>::
RemoveTail(ItemType & returnItem)
{
   if (_itemCount == 0) return B_ERROR;
   returnItem = _queue[_tailIndex];
   return RemoveTail();
}


template <class ItemType>
status_t 
Queue<ItemType>::
RemoveTail()
{
   if (_itemCount == 0) return B_ERROR;
   int removedItemIndex = _tailIndex;
   _tailIndex = PrevIndex(_tailIndex);
   _itemCount--;
   ItemType blank = ItemType();
   _queue[removedItemIndex] = blank;  // this must be done last, as queue state must be coherent when we do this
   return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
GetItemAt(uint32 index, ItemType & returnItem) const
{
   if (index >= _itemCount) return B_ERROR;
   returnItem = _queue[InternalizeIndex(index)];
   return B_NO_ERROR;
}

template <class ItemType>
status_t 
Queue<ItemType>::
RemoveItemAt(uint32 index, ItemType & returnItem)
{
   if (index >= _itemCount) return B_ERROR;
   returnItem = _queue[InternalizeIndex(index)];
   return RemoveItemAt(index);
}

template <class ItemType>
status_t 
Queue<ItemType>::
RemoveItemAt(uint32 index)
{
   if (index >= _itemCount) return B_ERROR;

   uint32 internalizedIndex = InternalizeIndex(index);
   uint32 indexToClear;

   if (index < _itemCount/2)
   {
      // item is closer to the head:  shift everything forward one, ending at the head
      while(internalizedIndex != _headIndex)
      {
         uint32 prev = PrevIndex(internalizedIndex);
         _queue[internalizedIndex] = _queue[prev];
         internalizedIndex = prev;
      }
      indexToClear = _headIndex;
      _headIndex = NextIndex(_headIndex); 
   }
   else
   {
      // item is closer to the tail:  shift everything back one, ending at the tail
      while(internalizedIndex != _tailIndex)
      {
         uint32 next = NextIndex(internalizedIndex);
         _queue[internalizedIndex] = _queue[next];
         internalizedIndex = next;
      }
      indexToClear = _tailIndex;
      _tailIndex = PrevIndex(_tailIndex); 
   }

   _itemCount--;
   ItemType blank = ItemType();
   _queue[indexToClear] = blank;  // this must be done last, as queue state must be coherent when we do this
   return B_NO_ERROR; 
}

template <class ItemType>
status_t 
Queue<ItemType>::
ReplaceItemAt(uint32 index, const ItemType & newItem)
{
   if (index >= _itemCount) return B_ERROR;
   _queue[InternalizeIndex(index)] = newItem;
   return B_NO_ERROR;
}

template <class ItemType>
status_t 
Queue<ItemType>::
InsertItemAt(uint32 index, const ItemType & newItem)
{
   // Simple cases
   if (index >  _itemCount) return B_ERROR;
   if (index == _itemCount) return AddTail(newItem);
   if (index == 0)          return AddHead(newItem);

   // Harder case:  inserting into the middle of the array
   if (index < _itemCount/2)
   {
      // Add a space at the front, and shift things back
      if (AddHead() != B_NO_ERROR) return B_ERROR;  // allocate an extra slot
      for (uint32 i=0; i<index; i++) ReplaceItemAt(i, *GetItemAt(i+1));
   }
   else
   {
      // Add a space at the rear, and shift things forward
      if (AddTail() != B_NO_ERROR) return B_ERROR;  // allocate an extra slot
      for (int32 i=((int32)_itemCount)-1; i>((int32)index); i--) ReplaceItemAt(i, *GetItemAt(i-1));
   }
   return ReplaceItemAt(index, newItem);
}

template <class ItemType>
void 
Queue<ItemType>::
Clear()
{
   while(RemoveTail() == B_NO_ERROR) {/* empty */}
}

template <class ItemType>
status_t 
Queue<ItemType>::
EnsureSize(uint32 size, bool setNumItems)
{
   if ((_queue == NULL)||(_queueSize < size))
   {
      const uint32 sqLen = ARRAYITEMS(_smallQueue);
      uint32 newQLen = ((setNumItems)||(size <= sqLen)) ? muscleMax(sqLen,size) : (size*2);  // if we're gonna do an implicit new[], we might as well double it to avoid another reallocation later
      if (newQLen < _initialSize) newQLen = _initialSize;

      ItemType * newQueue = ((_queue == _smallQueue)||(newQLen > sqLen)) ? newnothrow ItemType[newQLen] : _smallQueue;
      if (newQueue == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
      if (newQueue == _smallQueue) newQLen = sqLen;
      
      for (uint32 i=0; i<_itemCount; i++) (void) GetItemAt(i, newQueue[i]);  // we know that (_itemCount < size)
      if (setNumItems) _itemCount = size;
      _headIndex = 0;
      _tailIndex = _itemCount-1;

      if (_queue == _smallQueue) 
      {
         ItemType blank = ItemType();
         for (uint32 i=0; i<sqLen; i++) _smallQueue[i] = blank;
      }
      else delete [] _queue;

      _queue = newQueue;
      _queueSize = newQLen;
   }
   else if (setNumItems) while(_itemCount > size) (void) RemoveTail();

   return B_NO_ERROR;
}

template <class ItemType>
int32 
Queue<ItemType>::
IndexOf(const ItemType & item) const
{
   if (_queue) for (int i=((int)GetNumItems())-1; i>=0; i--) if (*GetItemAt(i) == item) return i;
   return -1;
}


template <class ItemType>
void 
Queue<ItemType>::
Swap(uint32 fromIndex, uint32 toIndex) 
{
   ItemType temp = *(GetItemAt(fromIndex));
   ReplaceItemAt(fromIndex, *(GetItemAt(toIndex)));
   ReplaceItemAt(toIndex,   temp);
}

template <class ItemType>
void 
Queue<ItemType>::
Sort(ItemCompareFunc compareFunc, uint32 from, uint32 to, void * cookie) 
{ 
   uint32 size = GetNumItems();
   if (to > size) to = size;
   if (to > from)
   {
      if (to < from+12) 
      {
         // too easy, just do a bubble sort (base case)
         if (to > from+1) 
         {
            for (uint32 i=from+1; i<to; i++) 
            {
               for (uint32 j=i; j>from; j--) 
               {
                  int ret = compareFunc(*(GetItemAt(j)), *(GetItemAt(j-1)), cookie);
                  if (ret < 0) Swap(j, j-1); 
                          else break; 
               }
            } 
         } 
      }
      else
      {
         // Okay, do the real thing
         uint32 middle = (from + to)/2; 
         Sort(compareFunc, from, middle, cookie); 
         Sort(compareFunc, middle, to, cookie); 
         Merge(compareFunc, from, middle, to, middle-from, to-middle, cookie); 
      }
   }
}

template <class ItemType>
void 
Queue<ItemType>::
ReverseItemOrdering(uint32 from, uint32 to) 
{
   uint32 size = GetNumItems();
   if (size > 0)
   {
      to--;  // make it inclusive
      if (to >= size) to = size-1;
      while (from < to) Swap(from++, to--); 
   }
} 

template <class ItemType>
status_t 
Queue<ItemType>::
RemoveFirstInstanceOf(const ItemType & val) 
{
   uint32 ni = GetNumItems();
   for (uint32 i=0; i<ni; i++)
   {
      if ((*this)[i] == val) 
      {
         RemoveItemAt(i);
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

template <class ItemType>
status_t 
Queue<ItemType>::
RemoveLastInstanceOf(const ItemType & val) 
{
   for (int32 i=((int32)GetNumItems())-1; i>=0; i--)
   {
      if ((*this)[i] == val) 
      {
         RemoveItemAt(i);
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

template <class ItemType>
uint32 
Queue<ItemType>::
RemoveAllInstancesOf(const ItemType & val) 
{
   // Efficiently collapse all non-matching slots up to the top of the list
   uint32 ret      = 0;
   uint32 writeTo  = 0;
   uint32 origSize = GetNumItems();
   for(uint32 readFrom=0; readFrom<origSize; readFrom++)
   {
      const ItemType & nextRead = (*this)[readFrom];
      if (nextRead == val) ret++;
      else 
      {
         if (readFrom > writeTo) (*this)[writeTo] = nextRead;
         writeTo++;
      }
   }

   // Now get rid of any now-surplus slots
   for (; writeTo<origSize; writeTo++) RemoveTail();

   return ret;
}

template <class ItemType>
void 
Queue<ItemType>::
Merge(ItemCompareFunc compareFunc, uint32 from, uint32 pivot, uint32 to, uint32 len1, uint32 len2, void * cookie) 
{
   if ((len1)&&(len2))
   {
      if (len1+len2 == 2) 
      { 
         if (compareFunc(*(GetItemAt(pivot)), *(GetItemAt(from)), cookie) < 0) Swap(pivot, from); 
      } 
      else
      {
         uint32 first_cut, second_cut; 
         uint32 len11, len22; 
         if (len1 > len2) 
         { 
            len11      = len1/2; 
            first_cut  = from + len11; 
            second_cut = Lower(compareFunc, pivot, to, *GetItemAt(first_cut), cookie); 
            len22      = second_cut - pivot; 
         } 
         else 
         { 
            len22      = len2/2; 
            second_cut = pivot + len22; 
            first_cut  = Upper(compareFunc, from, pivot, *GetItemAt(second_cut), cookie); 
            len11      = first_cut - from; 
         } 

         // do a rotation
         if ((pivot!=first_cut)&&(pivot!=second_cut)) 
         {
            // find the greatest common denominator of (pivot-first_cut) and (second_cut-first_cut)
            uint32 n = pivot-first_cut;
            {
               uint32 m = second_cut-first_cut;
               while(n!=0) 
               {
                  uint32 t = m % n; 
                  m=n; 
                  n=t;
               } 
               n = m;
            }

            while(n--) 
            {
               const ItemType val = *GetItemAt(first_cut+n); 
               uint32 shift = pivot - first_cut; 
               uint32 p1 = first_cut+n;
               uint32 p2 = p1+shift; 
               while (p2 != first_cut + n) 
               { 
                  ReplaceItemAt(p1, *GetItemAt(p2));
                  p1 = p2; 
                  if (second_cut - p2 > shift) p2 += shift; 
                                          else p2  = first_cut + (shift - (second_cut - p2)); 
               } 
               ReplaceItemAt(p1, val);
            }
         }

         uint32 new_mid = first_cut+len22; 
         Merge(compareFunc, from,    first_cut,  new_mid, len11,        len22,        cookie); 
         Merge(compareFunc, new_mid, second_cut, to,      len1 - len11, len2 - len22, cookie); 
      }
   }
}


template <class ItemType>
uint32 
Queue<ItemType>::
Lower(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType & val, void * cookie) const
{
   if (to > from)
   {
      uint32 len = to - from;
      while (len > 0) 
      {
         uint32 half = len/2; 
         uint32 mid  = from + half; 
         if (compareFunc(*(GetItemAt(mid)), val, cookie) < 0) 
         {
            from = mid+1; 
            len  = len - half - 1; 
         } 
         else len = half; 
      }
   }
   return from; 
} 

template <class ItemType>
uint32 
Queue<ItemType>::
Upper(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType & val, void * cookie) const 
{
   if (to > from)
   {
      uint32 len = to - from;
      while (len > 0) 
      { 
         uint32 half = len/2; 
         uint32 mid  = from + half; 
         if (compareFunc(val, *(GetItemAt(mid)), cookie) < 0) len = half; 
         else 
         {
            from = mid+1; 
            len  = len - half -1; 
         } 
      } 
   }
   return from; 
}

};  // end namespace muscle

#endif

