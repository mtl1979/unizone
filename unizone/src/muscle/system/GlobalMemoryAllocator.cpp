/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef NEW_H_NOT_AVAILABLE
# include <new>
# include <typeinfo>
#endif

#include <string.h>

#include "system/GlobalMemoryAllocator.h"

// Metrowerk's compiler crashes at run-time if this memory-tracking
// code is enabled.  I haven't been able to figure out why (everything
// looks okay, but after a few dozen malloc()/free() calls, free()
// crashes!) so I'm just going to disable this code for PowerPC/Metrowerks
// machines.  Sorry!   --jaf 12/01/00
#ifndef __osf__
# ifdef __MWERKS__
#  ifdef MUSCLE_ENABLE_MEMORY_TRACKING
#   undef MUSCLE_ENABLE_MEMORY_TRACKING
#  endif
# endif
#endif

#ifdef MUSCLE_ENABLE_MEMORY_TRACKING

// VC++ doesn't know from exceptions  :^P
# ifdef WIN32
#  ifndef MUSCLE_NO_EXCEPTIONS
#   define MUSCLE_NO_EXCEPTIONS
#  endif
# endif

// No exceptions?  Then make all of these keywords go away
# ifdef MUSCLE_NO_EXCEPTIONS
#  define BAD_ALLOC
#  define THROW
#  define LPAREN
#  define RPAREN
# else
#  define BAD_ALLOC bad_alloc
#  define THROW     throw
#  define LPAREN    (
#  define RPAREN    )
# endif

BEGIN_NAMESPACE(muscle);

static MemoryAllocatorRef _globalAllocatorRef;

void SetCPlusPlusGlobalMemoryAllocator(MemoryAllocatorRef maRef) {_globalAllocatorRef = maRef;}
MemoryAllocatorRef GetCPlusPlusGlobalMemoryAllocator() {return _globalAllocatorRef;}

static size_t _currentlyAllocatedBytes = 0;  // Running tally of how many bytes our process has allocated

size_t GetNumAllocatedBytes() {return _currentlyAllocatedBytes;}

void * muscleAlloc(size_t s, bool retryOnFailure)
{
   USING_NAMESPACE(muscle);

   size_t allocSize = s + sizeof(size_t);  // requested size, plus extra bytes for our tag
   bool mallocFailed = false;

   void * ret = NULL;
   MemoryAllocator * ma = _globalAllocatorRef();
   if ((ma == NULL)||(ma->AboutToAllocate(_currentlyAllocatedBytes, allocSize) == B_NO_ERROR))
   {
      size_t * a = (size_t *) malloc(allocSize);
      if (a)
      {
         *a = allocSize;  // our little header tag so that muscleFree() will know how big the allocation was
         _currentlyAllocatedBytes += allocSize;
//printf("+%lu = %lu\n", (uint32)allocSize, (uint32)_currentlyAllocatedBytes);
         ret = (void *)(a+1);  // user doesn't want to see the header tag, of course
      }
      else mallocFailed = true;
   }
   if ((ma)&&(ret == NULL)) 
   {
      ma->SetAllocationHasFailed(true);
      ma->AllocationFailed(_currentlyAllocatedBytes, allocSize);

      // Maybe the AllocationFailed() method was able to free up some memory; so we'll try it one more time
      // That way we might be able to recover without interrupting the operation that was in progress.
      if ((mallocFailed)&&(retryOnFailure)) ret = muscleAlloc(s, false);
   }
   return ret;
}

void * muscleRealloc(void * ptr, size_t s, bool retryOnFailure)
{
   USING_NAMESPACE(muscle);

        if (ptr == NULL) return muscleAlloc(s, retryOnFailure);
   else if (s   == 0)
   {
      muscleFree(ptr);
      return NULL;
   }

   size_t allocSize = s + sizeof(size_t); // requested size, plus extra bytes for our tag
   size_t * oldPtr = (((size_t*)ptr)-1);  // how much we already have allocated, including tag bytes
   if (allocSize == *oldPtr) return ptr;  // same size as before?  Then we are already done!

   void * ret = NULL;
   bool reallocFailed = false;
   MemoryAllocator * ma = _globalAllocatorRef();
   if (allocSize > *oldPtr)
   {
      size_t growBy = allocSize-*oldPtr;
      if ((ma == NULL)||(ma->AboutToAllocate(_currentlyAllocatedBytes, growBy) == B_NO_ERROR))
      {
         size_t * a = (size_t *) realloc(oldPtr, allocSize);
         if (a)
         {
            *a = allocSize;  // our little header tag so that muscleFree() will know how big the allocation was
            _currentlyAllocatedBytes += growBy;  // only reflect the newly-allocated bytes
//printf("r+%lu(->%lu) = %lu\n", (uint32)growBy, (uint32)allocSize, (uint32)_currentlyAllocatedBytes);
            ret = (void *)(a+1);  // user doesn't want to see the header tag, of course
         }
         else reallocFailed = true;
      }
      if ((ma)&&(ret == NULL)) 
      {
         ma->SetAllocationHasFailed(true);
         ma->AllocationFailed(_currentlyAllocatedBytes, growBy);

         // Maybe the AllocationFailed() method was able to free up some memory; so we'll try it one more time
         // That way we might be able to recover without interrupting the operation that was in progress.
         if ((reallocFailed)&&(retryOnFailure)) ret = muscleRealloc(ptr, s, false);
      }
   }
   else
   {
      size_t shrinkBy = *oldPtr-allocSize;
      if (ma) ma->AboutToFree(_currentlyAllocatedBytes, shrinkBy);
      size_t * a = (size_t *) realloc(oldPtr, allocSize);
      if (a)
      {
         *a = allocSize;  // our little header tag so that muscleFree() will know how big the allocation is now
         _currentlyAllocatedBytes -= shrinkBy;
//printf("r-%lu(->%lu) = %lu\n", (uint32)shrinkBy, (uint32)allocSize, (uint32)_currentlyAllocatedBytes);
         ret = (void *)(a+1);  // use doesn't see the header tag though
      }
      else ret = ptr;  // I guess the best thing to do is just send back the old pointer?  Not sure what to do here.
   }
   return ret;
}

void muscleFree(void * p)
{
   USING_NAMESPACE(muscle);
   if (p)
   {
      size_t * s = (((size_t*)p)-1);
      _currentlyAllocatedBytes -= *s;

      MemoryAllocator * ma = _globalAllocatorRef();
      if (ma) ma->AboutToFree(_currentlyAllocatedBytes, *s);

//printf("-%lu = %lu\n", (uint32)*s, (uint32)_currentlyAllocatedBytes);
      free(s);
   }
}

END_NAMESPACE(muscle);

void * operator new(size_t s) THROW LPAREN BAD_ALLOC RPAREN
{
   USING_NAMESPACE(muscle);
   void * ret = muscleAlloc(s);
   if (ret == NULL) {THROW BAD_ALLOC LPAREN RPAREN;}
   return ret;
}

void * operator new[](size_t s) THROW LPAREN BAD_ALLOC RPAREN
{
   USING_NAMESPACE(muscle);
   void * ret = muscleAlloc(s);
   if (ret == NULL) {THROW BAD_ALLOC LPAREN RPAREN;}
   return ret;
}

// Borland, VC++, and OSF don't like separate throw/no-throw operators, it seems
# ifndef WIN32
#  ifndef __osf__
void * operator new(  size_t s, nothrow_t const &) THROW LPAREN RPAREN {USING_NAMESPACE(muscle); return muscleAlloc(s);}
void * operator new[](size_t s, nothrow_t const &) THROW LPAREN RPAREN {USING_NAMESPACE(muscle); return muscleAlloc(s);}
#  endif
# endif

void operator delete(  void * p) THROW LPAREN RPAREN {USING_NAMESPACE(muscle); muscleFree(p);}
void operator delete[](void * p) THROW LPAREN RPAREN {USING_NAMESPACE(muscle); muscleFree(p);}

#endif
