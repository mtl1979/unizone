#include "util/SharedUsageLimitProxyMemoryAllocator.h"

namespace muscle {

static const int32 CACHE_BYTES = 100*1024;  // 100KB local cache size seems reasonable, no?

SharedUsageLimitProxyMemoryAllocator :: SharedUsageLimitProxyMemoryAllocator(const char * sharedAreaKey, int32 memberID, uint32 groupSize, const MemoryAllocatorRef & slaveRef, size_t maxBytes) : ProxyMemoryAllocator(slaveRef), _maxBytes(maxBytes), _memberID(memberID), _groupSize(groupSize), _localCachedBytes(0)
{
   if (_shared.SetArea(sharedAreaKey, (groupSize+1)*sizeof(size_t), true) == B_NO_ERROR)
   {
      if (_shared.IsCreatedLocally()) 
      {
         size_t * sa = (size_t *) _shared();
         if (sa)
         {
            uint32 gs = _shared.GetAreaSize() / sizeof(size_t);
            for (size_t i=0; i<gs; i++) sa[i] = 0;
         }
      }
      else ResetDaemonCounter();  // Clean up after previous daemon, just in case

      _shared.UnlockArea();  // because it was locked for us by SetArea()
   }
   else LogTime(MUSCLE_LOG_CRITICALERROR, "SharedUsageLimitProxyMemoryAllocator:  Couldn't initialize shared memory area [%s]!\n", sharedAreaKey);
}

SharedUsageLimitProxyMemoryAllocator :: ~SharedUsageLimitProxyMemoryAllocator()
{
   if (_shared.LockAreaReadWrite() == B_NO_ERROR)
   {
      ResetDaemonCounter();
      _shared.UnlockArea();
   }
}

void SharedUsageLimitProxyMemoryAllocator :: ResetDaemonCounter()
{
   size_t * sa = (size_t *) _shared();
   if ((sa)&&(_memberID >= 0))
   {
      size_t gs = _shared.GetAreaSize() / sizeof(size_t);
      if ((size_t)(_memberID+1) < gs)
      {
         size_t & mySize = sa[_memberID+1];
         if (mySize > 0)
         {
            size_t & cumSize = sa[0];
            if (mySize > cumSize)
            {
               LogTime(MUSCLE_LOG_WARNING, "SharedUsageLimitProxyMemoryAllocator::ResetDaemonCounter():  Daemon size %u was greater than cumulative size %u\n", mySize, cumSize);
               cumSize = 0;
            }
            else cumSize -= mySize;

            mySize = 0;
         }
      }
   }
   _localCachedBytes = 0;
}

status_t SharedUsageLimitProxyMemoryAllocator :: ChangeDaemonCounter(int32 byteDelta)
{
   if (byteDelta > 0)
   {
      if (byteDelta > _localCachedBytes)
      {
         // Hmm, we don't have enough bytes locally, better ask for some from the shared region
         int32 wantBytes = ((byteDelta/CACHE_BYTES)+1)*CACHE_BYTES; // round up to nearest multiple
         if (ChangeDaemonCounterAux(wantBytes) == B_NO_ERROR) _localCachedBytes += wantBytes;
      }
      if (byteDelta > _localCachedBytes) return B_ERROR;  // still not enough!?
      _localCachedBytes -= byteDelta;
   }
   else
   {
      _localCachedBytes -= byteDelta;  // actually adds, since byteDelta is negative
      if (_localCachedBytes > 2*CACHE_BYTES)
      {
         int32 diffBytes = _localCachedBytes-CACHE_BYTES;  // FogBugz #4569 -- reduce cache to our standard cache size
         if (ChangeDaemonCounterAux(-diffBytes) == B_NO_ERROR) _localCachedBytes -= diffBytes;
      }
   }
   return B_NO_ERROR;
}

static inline void AdjustValue(uint32 which, size_t * arr, int32 byteDelta)
{
   size_t & v = arr[which];

   if (byteDelta >= 0) v += byteDelta;
   else
   {
      uint32 reduceBy = -byteDelta;
      if (v >= reduceBy) v -= reduceBy;
      else
      {
         printf("Error, Attempted to reduce slot "INT32_FORMAT_SPEC"'s counter (currently "UINT32_FORMAT_SPEC") by "UINT32_FORMAT_SPEC"!  Setting counter at zero instead.\n", ((int32)which)-1, (uint32) v, reduceBy);
PrintStackTrace();
         v = 0;
      }
   }
}

// Locks the shared memory region and adjusts our counter there.  This is a bit expensive,
// so we try to minimize the number of times we do it.
status_t SharedUsageLimitProxyMemoryAllocator :: ChangeDaemonCounterAux(int32 byteDelta)
{
   status_t ret = B_ERROR;
   if ((_memberID >= 0)&&(_shared.LockAreaReadWrite() == B_NO_ERROR))
   {
      size_t * sa = (size_t *) _shared();
      if (sa)
      {
         size_t gs = _shared.GetAreaSize() / sizeof(size_t);
         if (((size_t)(_memberID+1) < gs)&&((byteDelta <= 0)||(sa[0] + byteDelta <= _maxBytes)))
         {
            AdjustValue(0,           sa, byteDelta);
            AdjustValue(_memberID+1, sa, byteDelta);
//printf("delta("INT32_FORMAT_SPEC"): slot "INT32_FORMAT_SPEC" is now "UINT32_FORMAT_SPEC", total is now "UINT32_FORMAT_SPEC"/"UINT32_FORMAT_SPEC"\n", byteDelta, _memberID, (uint32) sa[_memberID+1], (uint32) sa[0], (uint32) _maxBytes);
            ret = B_NO_ERROR;
         }
      }
      _shared.UnlockArea();
   }
   return ret;
}

status_t SharedUsageLimitProxyMemoryAllocator :: AboutToAllocate(size_t cab, size_t arb)
{
   status_t ret = B_ERROR;
   if (ChangeDaemonCounter(arb) == B_NO_ERROR)
   {
      ret = ProxyMemoryAllocator::AboutToAllocate(cab, arb);
      if (ret != B_NO_ERROR) (void) ChangeDaemonCounter(-((int32)arb)); // roll back!
   }
   return ret;
}

void SharedUsageLimitProxyMemoryAllocator :: AboutToFree(size_t cab, size_t arb)
{
   (void) ChangeDaemonCounter(-((int32)arb));
   ProxyMemoryAllocator::AboutToFree(cab, arb);
}

size_t SharedUsageLimitProxyMemoryAllocator :: GetNumAvailableBytes(size_t allocated) const
{
   size_t totalUsed = MUSCLE_NO_LIMIT;
   if (_shared.LockAreaReadOnly() == B_NO_ERROR)
   {
      const size_t * sa = (const size_t *) _shared();
      if ((sa)&&(_shared.GetAreaSize()/sizeof(size_t) > 0)) totalUsed = sa[0];
      _shared.UnlockArea();
   }
   return muscleMin((_maxBytes>totalUsed)?(_maxBytes-totalUsed):0, ProxyMemoryAllocator::GetNumAvailableBytes(allocated));
}

status_t SharedUsageLimitProxyMemoryAllocator :: GetCurrentMemoryUsage(size_t * retCounts, size_t * optRetTotal) const
{
   status_t ret = B_ERROR;
   if (_shared.LockAreaReadOnly() == B_NO_ERROR)
   {
      const size_t * sa = (const size_t *) _shared();
      if (sa)
      {
         uint32 num = (_shared.GetAreaSize()/sizeof(size_t));
         for (uint32 i=0; i<_groupSize; i++) retCounts[i] = (i+1<num) ? sa[i+1] : 0;
         ret = B_NO_ERROR;
      }
      if (optRetTotal) *optRetTotal = sa[0];
      _shared.UnlockArea();
   }
   return ret;
}

}; // end namespace muscle
