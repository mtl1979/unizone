/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleGlobalMemoryAllocator_h
#define MuscleGlobalMemoryAllocator_h

#include "util/MemoryAllocator.h"

namespace muscle {

// You can't use these functions unless memory tracking is enabled!
// So if you are getting errors, make sure -DMUSCLE_ENABLE_MEMORY_TRACKING
// is specified in your Makefile.
#ifdef MUSCLE_ENABLE_MEMORY_TRACKING

/** Set the MemoryAllocator object that is to be called by the C++ global new and delete operators.
  * @note this function is only available is -DMUSCLE_ENABLE_MEMORY_TRACKING is defined in the Makefile.
  * @param maRef Reference to The new MemoryAllocator object to use.  May be a NULL reference
  *              if you just want to remove any current MemoryAllocator.
  */
void SetCPlusPlusGlobalMemoryAllocator(MemoryAllocatorRef maRef);

/** Returns a reference to the current MemoryAllocator object that is being used by the 
  * C++ global new and delete operators.  Will return a NULL reference if no MemoryAllocator is in use.
  * @note this function is only available is -DMUSCLE_ENABLE_MEMORY_TRACKING is defined in the Makefile.
  */
MemoryAllocatorRef GetCPlusPlusGlobalMemoryAllocator();

/** Returns the number of bytes currently dynamically allocated by this process. 
  * @note this function is only available is -DMUSCLE_ENABLE_MEMORY_TRACKING is defined in the Makefile.
  */
size_t GetNumAllocatedBytes();

#else

#endif

};  // end namespace muscle

#endif
