/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */ 

#include "util/MemoryAllocator.h"

namespace muscle {

bool ProxyMemoryAllocator :: IsOkayToAllocate(size_t currentlyAllocatedBytes, size_t allocRequestBytes) const
{
   return _slaveRef() ? _slaveRef()->IsOkayToAllocate(currentlyAllocatedBytes, allocRequestBytes) : true;
}

void ProxyMemoryAllocator :: AllocationFailed(size_t currentlyAllocatedBytes, size_t allocRequestBytes)
{
   if (_slaveRef()) 
   {
      _slaveRef()->SetAllocationHasFailed(true);
      _slaveRef()->AllocationFailed(currentlyAllocatedBytes, allocRequestBytes);
   }
}

void ProxyMemoryAllocator :: SetAllocationHasFailed(bool hasFailed)
{
   MemoryAllocator::SetAllocationHasFailed(hasFailed);
   if (_slaveRef()) _slaveRef()->SetAllocationHasFailed(hasFailed);
}

UsageLimitProxyMemoryAllocator :: UsageLimitProxyMemoryAllocator(MemoryAllocatorRef slaveRef, size_t maxBytes) : ProxyMemoryAllocator(slaveRef), _maxBytes(maxBytes)
{
   // empty
}
 
UsageLimitProxyMemoryAllocator :: ~UsageLimitProxyMemoryAllocator()
{
   // empty
}
 
bool UsageLimitProxyMemoryAllocator :: IsOkayToAllocate(size_t currentlyAllocatedBytes, size_t allocRequestBytes) const
{
   return ((allocRequestBytes < _maxBytes)&&(currentlyAllocatedBytes + allocRequestBytes <= _maxBytes));
}

void AutoCleanupProxyMemoryAllocator :: AllocationFailed(size_t, size_t)
{
   uint32 nc = _callbacks.GetNumItems();
   for (uint32 i=0; i<nc; i++) if (_callbacks[i]()) (_callbacks[i]())->OutOfMemory();
}

};  // end namespace muscle
