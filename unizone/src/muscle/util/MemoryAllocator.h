/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#ifndef MuscleMemoryAllocator_h
#define MuscleMemoryAllocator_h

#include "support/MuscleSupport.h"
#include "util/Queue.h"
#include "util/RefCount.h"

namespace muscle {

class MemoryAllocator;

/** Interface class representing an object that can allocate and free blocks of memory. */
class MemoryAllocator : public RefCountable
{
public:
   /** Default constructor; no-op */
   MemoryAllocator() : _hasAllocationFailed(false) {/* empty */}

   /** Virtual destructor, to keep C++ honest */
   virtual ~MemoryAllocator() {/* empty */}

   /** This method is called whenever we are about to allocate some memory.
    *  @param currentlyAllocatedBytes How many bytes the system has allocated currently
    *  @param allocRequestBytes How many bytes the system would like to allocate.
    *  @return Should return B_NO_ERROR if the allocation may proceed, or B_ERROR to abort the allocation.
    */
   virtual status_t AboutToAllocate(size_t currentlyAllocatedBytes, size_t allocRequestBytes) = 0;

   /** This method is called whenever we are about to free some memory.
    *  @param currentlyAllocatedBytes How many bytes the system has allocated currently
    *  @param freeBytes How many bytes the system is about to free.
    */
   virtual void AboutToFree(size_t currentlyAllocatedBytes, size_t freeSize) = 0;

   /** Called if an allocation fails (either because AboutToAllocate() returned other than B_NO_ERROR, or
    *  because malloc() returned NULL).   This method does not need to call SetMallocHasFailed(),
    *  because the system will have already called it.
    *  @param currentlyAllocatedBytes How many bytes the system has allocated currently
    *  @param allocRequestBytes How many bytes the system wanted to allocate, but couldn't.
    */
   virtual void AllocationFailed(size_t currentlyAllocatedBytes, size_t allocRequestBytes) = 0;

   /** Sets the state of the "allocation has failed" flag.  Typically this is set true just
    *  before a call to AllocationFailed(), and set false again later on, after the flag has
    *  been noticed and dealt with.
    */
   virtual void SetAllocationHasFailed(bool hasFailed) {_hasAllocationFailed = hasFailed;}

   /** Should be overridden to return the maximum amount of memory that may be allocated at once.
    *  If there is no set limit, this method should return MUSCLE_NO_LIMIT.
    */
   virtual size_t GetMaxNumBytes() const = 0;

   /** Should be overridden to return the number of bytes still available for allocation,
    *  given that (currentlyAllocated) bytes have already been allocated.
    *  If there is no set limit, this method should return MUSCLE_NO_LIMIT.
    */
   virtual size_t GetNumAvailableBytes(size_t currentlyAllocated) const = 0;

   /** Returns the current state of the "allocation has failed" flag. */
   bool HasAllocationFailed() const {return _hasAllocationFailed;}

private:
   bool _hasAllocationFailed;
};

typedef Ref<MemoryAllocator> MemoryAllocatorRef;

/** Convenience class, used for easy subclassing:  holds a slave MemoryAllocator and passes all method
  * calls on through to the slave.
  */
class ProxyMemoryAllocator : public MemoryAllocator
{
public:
   /** Constructor.
     * @param slaveRef Reference to another MemoryAllocator that we will pass all our method calls on through to.
     *                 If (slaveRef) is NULL, default behaviour (always allow, no-op failures) will be used.
     */
   ProxyMemoryAllocator(MemoryAllocatorRef slaveRef) : _slaveRef(slaveRef) {/* empty */}

   /** Destructor */
   virtual ~ProxyMemoryAllocator() {/* empty */}

   virtual status_t AboutToAllocate(size_t currentlyAllocatedBytes, size_t allocRequestBytes);
   virtual void AboutToFree(size_t currentlyAllocatedBytes, size_t allocRequestBytes);
   virtual void AllocationFailed(size_t currentlyAllocatedBytes, size_t allocRequestBytes);
   virtual void SetAllocationHasFailed(bool hasFailed);
   virtual size_t GetMaxNumBytes() const;
   virtual size_t GetNumAvailableBytes(size_t currentlyAllocated) const;

private:
   MemoryAllocatorRef _slaveRef;
};

/** This MemoryAllocator decorates its slave MemoryAllocator to 
  * enforce a user-defined per-process limit on how much memory may be allocated at any given time. 
  */
class UsageLimitProxyMemoryAllocator : public ProxyMemoryAllocator
{
public:
   /** Constructor.  
     * @param slaveRef Reference to a sub-MemoryAllocator whose methods we will call through to.
     * @param maxBytes The maximum number of bytes that we will allow (slave) to allocate.  Defaults to no limit.
     *                 This value may be reset later using SetMaxNumBytes().
     */
   UsageLimitProxyMemoryAllocator(MemoryAllocatorRef slaveRef, size_t maxBytes = MUSCLE_NO_LIMIT);

   /** Destructor.  */
   virtual ~UsageLimitProxyMemoryAllocator();

   /** Overridden to return false if memory usage would go over maximum due to this allocation. */
   virtual status_t AboutToAllocate(size_t currentlyAllocatedBytes, size_t allocRequestBytes);

   /** Set a new maximum number of allocatable bytes */
   void SetMaxNumBytes(size_t mb) {_maxBytes = mb;}

   /** Implemented to return our hard-coded size limit, same as GetMaxNumBytes() */
   virtual size_t GetMaxNumBytes() const {return muscleMin(_maxBytes, ProxyMemoryAllocator::GetMaxNumBytes());}

   /** Implemented to return the difference between the maximum allocation and the current number allocated. */
   virtual size_t GetNumAvailableBytes(size_t ca) const {return muscleMin((_maxBytes>ca)?_maxBytes-ca:0, ProxyMemoryAllocator::GetNumAvailableBytes(ca));}

private:
   size_t _maxBytes;
};

/** Interface class representing an object that can be called by an AutoCleanupProxyMemoryAllocator
  * in case of an out-of-memory emergency.
  */
class OutOfMemoryCallback : public RefCountable
{
public:
   /** Constructor */
   OutOfMemoryCallback() {/* empty */}

   /** Virtual destructor, to keep C++ honest */
   virtual ~OutOfMemoryCallback() {/* empty */}

   /** Called by a MemoryAllocator when it is having trouble allocating memory.
     * Typically this method would be implemented to free up some unimportant memory, so that
     * the MemoryAllocator will have a better chance of being able to get memory when it tries again.
     */
   virtual void OutOfMemory() = 0;
};

/** Convenience type for referencing an OutOfMemoryCallback */
typedef Ref<OutOfMemoryCallback> OutOfMemoryCallbackRef;

/** A handy little class--instead of having to subclass, it will call the function you specify */
class FunctionOutOfMemoryCallback : public OutOfMemoryCallback
{
public:
   /** Signature of the function type that we know how to call */
   typedef void (*OutOfMemoryCallbackFunc)();   

   /** Constructor.
     * @param f The function to call if we run out of memory 
     */
   FunctionOutOfMemoryCallback(OutOfMemoryCallbackFunc f) : _func(f) {/* empty */}

   /** Calls the function specified in the header */
   virtual void OutOfMemory() {if (_func) _func();}

private:
   OutOfMemoryCallbackFunc _func;
};

/** This MemoryAllocator decorates its slave MemoryAllocator to call a list of
  * OutOfMemoryCallback objects when the slave's memory allocation fails.  These
  * OutOfMemoryCallback calls should try to free up some memory if possible;
  * then this MemoryAllocator will call the slave again and see if the memory
  * allocation can now succeed.
  */
class AutoCleanupProxyMemoryAllocator : public ProxyMemoryAllocator
{
public:
   /** Constructor.  
     * @param slaveRef Reference to a sub-MemoryAllocator whose methods we will call through to.
     */
   AutoCleanupProxyMemoryAllocator(MemoryAllocatorRef slaveRef) : ProxyMemoryAllocator(slaveRef) {/* empty */}

   /** Destructor.  Calls ClearCallbacks(). */
   virtual ~AutoCleanupProxyMemoryAllocator() {/* empty */}

   /** Overridden to call our callbacks in event of a failure. */
   virtual void AllocationFailed(size_t currentlyAllocatedBytes, size_t allocRequestBytes);

   /** Read-write access to our list of out-of-memory callbacks. */
   Queue<OutOfMemoryCallbackRef> & GetCallbacksQueue() {return _callbacks;}

   /** Write-only access to our list of out-of-memory callbacks. */
   const Queue<OutOfMemoryCallbackRef> & GetCallbacksQueue() const {return _callbacks;}

private:
   Queue<OutOfMemoryCallbackRef> _callbacks;
};

};  // end namespace muscle

#endif
