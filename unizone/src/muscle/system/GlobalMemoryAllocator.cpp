/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#include <new>
#include <typeinfo>
#include <string.h>

#include "system/GlobalMemoryAllocator.h"

namespace muscle {

static MemoryAllocatorRef _globalAllocatorRef;

void SetCPlusPlusGlobalMemoryAllocator(MemoryAllocatorRef maRef) {_globalAllocatorRef = maRef;}
MemoryAllocatorRef GetCPlusPlusGlobalMemoryAllocator() {return _globalAllocatorRef;}

static size_t _currentlyAllocatedBytes = 0;  // our count of the total malloc'd memory in the system

size_t GetNumAllocatedBytes() {return _currentlyAllocatedBytes;}

};  // end namespace muscle

// Metrowerk's compiler crashes at run-time if this memory-tracking
// code is enabled.  I haven't been able to figure out why (everything
// looks okay, but after a few dozen malloc()/free() calls, free()
// crashes!) so I'm just going to disable this code for PowerPC/Metrowerks
// machines.  Sorry!   --jaf 12/01/00
#ifdef __MWERKS__
# ifdef MUSCLE_ENABLE_MEMORY_TRACKING
#  undef MUSCLE_ENABLE_MEMORY_TRACKING
# endif
#endif

#ifdef MUSCLE_ENABLE_MEMORY_TRACKING

// VC++ doesn't know from exceptions  :^P
#ifdef WIN32
# ifndef MUSCLE_NO_EXCEPTIONS
#  define MUSCLE_NO_EXCEPTIONS
# endif
#endif

// No exceptions?  Then make all of these keywords go away
#ifdef MUSCLE_NO_EXCEPTIONS
# define BAD_ALLOC
# define THROW
# define LPAREN
# define RPAREN
#else
# define BAD_ALLOC bad_alloc
# define THROW     throw
# define LPAREN    (
# define RPAREN    )
#endif

static void * muscleAlloc(size_t s, bool isRecursion)
{
   using namespace muscle;

   size_t allocSize = s + sizeof(size_t);  // requested size, plus extra bytes for our tag
   bool mallocFailed = false;

   void * ret = NULL;
   MemoryAllocator * ma = _globalAllocatorRef();
   if ((ma == NULL)||(ma->IsOkayToAllocate(_currentlyAllocatedBytes, allocSize)))
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
      if ((mallocFailed)&&(isRecursion == false)) ret = muscleAlloc(s, true);
   }
   return ret;
}

static void muscleFree(void * p)
{
   using namespace muscle;
   if (p)
   {
      size_t * s = (((size_t*)p)-1);
      _currentlyAllocatedBytes -= *s;
      free(s);
//printf("-%lu = %lu\n", (uint32)s, (uint32)_currentlyAllocatedBytes);
   }
}

void * operator new(size_t s) THROW LPAREN BAD_ALLOC RPAREN
{
   void * ret = muscleAlloc(s, false);
   if (ret == NULL) {THROW BAD_ALLOC LPAREN RPAREN;}
   return ret;
}

void * operator new[](size_t s) THROW LPAREN BAD_ALLOC RPAREN
{
   void * ret = muscleAlloc(s, false);
   if (ret == NULL) {THROW BAD_ALLOC LPAREN RPAREN;}
   return ret;
}

// Borland and VC++ don't like separate throw/no-throw operators, it seems
#ifndef WIN32
void * operator new(  size_t s, nothrow_t const &) THROW LPAREN RPAREN {return muscleAlloc(s, false);}
void * operator new[](size_t s, nothrow_t const &) THROW LPAREN RPAREN {return muscleAlloc(s, false);}
#endif

void operator delete(  void * p) THROW LPAREN RPAREN {muscleFree(p);}
void operator delete[](void * p) THROW LPAREN RPAREN {muscleFree(p);}

#endif
