/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include "system/SetupSystem.h"
#include "support/Flattenable.h"
#include "dataio/DataIO.h"
#include "util/ObjectPool.h"

#ifdef WIN32
# include <windows.h>
# include <winsock.h>
# include <signal.h>
#else
# if defined(__BEOS__)
#  include <signal.h>
# elif defined(__CYGWIN__)
#  include <signal.h>
#  include <sys/signal.h>
#  include <sys/times.h>
# elif defined(__QNX__)
#  include <signal.h>
#  include <sys/times.h>
# elif defined(SUN) || defined(__sparc__) || defined(sun386)
#  include <signal.h>
#  include <sys/times.h>
#  include <limits.h>
# else
#  include <sys/signal.h>  // changed signal.h to sys/signal.h to work with OS/X
#  include <sys/times.h>
# endif
#endif

#if defined(__BORLANDC__)
# include <math.h>
# include <float.h>
#endif

#if defined(__APPLE__)
# include <CoreServices/CoreServices.h>
#endif

BEGIN_NAMESPACE(muscle);

#ifdef MUSCLE_SINGLE_THREAD_ONLY
bool _muscleSingleThreadOnly = true;
#else
bool _muscleSingleThreadOnly = false;
#endif

static Mutex * _muscleLock = NULL;
Mutex * GetGlobalMuscleLock() {return _muscleLock;}

#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
const uint32 MUTEX_POOL_SIZE = 256;
static Mutex * _atomicMutexes = NULL;
#endif

static uint32 _threadSetupCount = 0;

MathSetupSystem :: MathSetupSystem()
{
#if defined(__BORLANDC__)
   _control87(MCW_EM,MCW_EM);  // disable floating point exceptions
#endif
}

MathSetupSystem :: ~MathSetupSystem()
{
   // empty
}

ThreadSetupSystem :: ThreadSetupSystem(bool muscleSingleThreadOnly)
{
   if (++_threadSetupCount == 1)
   {
#ifdef MUSCLE_SINGLE_THREAD_ONLY
      (void) muscleSingleThreadOnly;  // shut the compiler up
#else
      _muscleSingleThreadOnly = muscleSingleThreadOnly;
      if (_muscleSingleThreadOnly) _lock.Neuter();  // if we're single-thread, then this Mutex can be a no-op!
#endif
      _muscleLock = &_lock;

#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
      _atomicMutexes = newnothrow_array(Mutex, MUTEX_POOL_SIZE);
      MASSERT(_atomicMutexes, "Couldn't allocate atomic mutexes!");
#endif
   }
}

ThreadSetupSystem :: ~ThreadSetupSystem()
{
   if (--_threadSetupCount == 0)
   {
#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
      delete [] _atomicMutexes; _atomicMutexes = NULL;
#endif
      _muscleLock = NULL;
   }
}

#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
int32 DoMutexAtomicIncrement(volatile int32 * count, int32 delta)
{
   MASSERT(_atomicMutexes, "Please declare a SetupSystem object before doing any atomic incrementing!");
   Mutex & mutex = _atomicMutexes[(((uint32)count)/sizeof(int))%MUTEX_POOL_SIZE];
   (void) mutex.Lock();
   int32 ret = *count = (*count + delta);
   (void) mutex.Unlock();
   return ret;
}
#endif

static uint32 _networkSetupCount = 0;

NetworkSetupSystem :: NetworkSetupSystem()
{
   if (++_networkSetupCount == 1)
   {
#ifdef WIN32
      WORD versionWanted = MAKEWORD(1, 1);
      WSADATA wsaData;
      int ret = WSAStartup(versionWanted, &wsaData);
      MASSERT((ret == 0), "NetworkSetupSystem:  Couldn't initialize Winsock!");
#else
      signal(SIGPIPE, SIG_IGN);  // avoid evil SIGPIPE signals from sending on a closed socket
#endif
   }
}

NetworkSetupSystem :: ~NetworkSetupSystem()
{
   if (--_networkSetupCount == 0)
   {
#ifdef WIN32
      WSACleanup();
#endif
   }
}

#if defined(MUSCLE_USE_POWERPC_INLINE_ASSEMBLY) && defined (MUSCLE_POWERPC_TIMEBASE_HZ)
static inline uint32 get_tbl() {uint32 tbl; asm volatile("mftb %0"  : "=r" (tbl) :); return tbl;}
static inline uint32 get_tbu() {uint32 tbu; asm volatile("mftbu %0" : "=r" (tbu) :); return tbu;}
#endif

// For BeOS, this is an in-line function, defined in util/TimeUtilityFunctions.h
#ifndef __BEOS__

/** Defined here since every MUSCLE program will have to include this file anyway... */
uint64 GetRunTime64()
{
#ifdef WIN32
   TCHECKPOINT;

   uint64 ret = 0;
   static Mutex _rtMutex;
   if (_rtMutex.Lock() == B_NO_ERROR)
   {
#ifdef MUSCLE_USE_QUERYPERFORMANCECOUNTER
      TCHECKPOINT;

      static int64 _brokenQPCOffset = 0;
      if (_brokenQPCOffset != 0) ret = (((uint64)timeGetTime())*1000) + _brokenQPCOffset;
      else
      {
         static bool _gotFrequency = false;
         static uint64 _ticksPerSecond;
         if (_gotFrequency == false)
         {
            LARGE_INTEGER tps;
            _ticksPerSecond = (QueryPerformanceFrequency(&tps)) ? tps.QuadPart : 0;
            _gotFrequency = true;
         }

         LARGE_INTEGER curTicks;
         if ((_ticksPerSecond > 0)&&(QueryPerformanceCounter(&curTicks)))
         {
            uint64 checkGetTime = ((uint64)timeGetTime())*1000;
            ret = (curTicks.QuadPart*1000000)/_ticksPerSecond;

            // Hack-around for evil Windows/hardware bug in QueryPerformanceCounter().
            // see http://support.microsoft.com/default.aspx?scid=kb;en-us;274323
            static uint64 _lastCheckGetTime = 0;
            static uint64 _lastCheckQPCTime = 0;
            if (_lastCheckGetTime > 0)
            {
               uint64 getTimeElapsed = checkGetTime - _lastCheckGetTime;
               uint64 qpcTimeElapsed = ret          - _lastCheckQPCTime;
               if ((muscleMax(getTimeElapsed, qpcTimeElapsed) - muscleMin(getTimeElapsed, qpcTimeElapsed)) > 500000)
               {
                  //LogTime(MUSCLE_LOG_DEBUG, "QueryPerformanceCounter() is buggy, reverting to timeGetTime() method instead!\n");
                  _brokenQPCOffset = (_lastCheckQPCTime-_lastCheckGetTime);
                  ret = (((uint64)timeGetTime())*1000) + _brokenQPCOffset;
               }
            }
            _lastCheckGetTime = checkGetTime;
            _lastCheckQPCTime = ret;
         }
      }
#endif
      if (ret == 0)
      {
         static uint32 _prevVal    = 0;
         static uint64 _wrapOffset = 0;
         
         uint32 newVal = (uint32) timeGetTime();
         if (newVal < _prevVal) _wrapOffset += (((uint64)1)<<32); 
         ret = (_wrapOffset+newVal)*1000;  // convert to microseconds
         _prevVal = newVal;
      }
      _rtMutex.Unlock();
   }
   return ret;
#elif defined(__APPLE__)
   UnsignedWide uw = AbsoluteToNanoseconds(UpTime());
   return ((((uint64)uw.hi)<<32)|(uw.lo))/1000;
#else
# if defined(MUSCLE_USE_POWERPC_INLINE_ASSEMBLY) && defined(MUSCLE_POWERPC_TIMEBASE_HZ)
   TCHECKPOINT;
   while(1)
   {
      uint32 hi1 = get_tbu();
      uint32 low = get_tbl();
      uint32 hi2 = get_tbu();
      if (hi1 == hi2) 
      {
         // FogBugz #3199
         uint64 cycles = ((((uint64)hi1)<<32)|((uint64)low));
         return ((cycles/MUSCLE_POWERPC_TIMEBASE_HZ)*1000000)+(((cycles%MUSCLE_POWERPC_TIMEBASE_HZ)*((uint64)1000000))/MUSCLE_POWERPC_TIMEBASE_HZ);
      }
   }
# else
   TCHECKPOINT;

   // default implementation:  use POSIX API
   static clock_t _ticksPerSecond = 0;
   if (_ticksPerSecond <= 0) _ticksPerSecond = sysconf(_SC_CLK_TCK);
   if (_ticksPerSecond > 0)
   {
      if (sizeof(clock_t) > 4) 
      {
         // Easy case:  with a wide clock_t, we don't need to worry about it wrapping
         struct tms junk; clock_t newTicks = (clock_t) times(&junk);
         return ((((uint64)newTicks)*1000000)/_ticksPerSecond);
      }
      else
      {
         // Oops, clock_t is skinny enough that it might wrap.  So we need to watch for that.
         static Mutex _rtMutex;
         if (_rtMutex.Lock() == B_NO_ERROR)
         {
            static uint32 _prevVal;
            static uint64 _wrapOffset = 0;
            
            struct tms junk; clock_t newTicks = (clock_t) times(&junk);
            uint32 newVal = (uint32) newTicks;
            if (newVal < _prevVal) _wrapOffset += (((uint64)1)<<32);
            uint64 ret = ((_wrapOffset+newVal)*1000000)/_ticksPerSecond;  // convert to microseconds
            _prevVal = newTicks;

            _rtMutex.Unlock();
            return ret;
         }
      }
   }
   return 0;  // Oops?
# endif
#endif
}

#ifdef WIN32
// Broken out so ParseHumanReadableTimeValues() can use it also
uint64 __Win32FileTimeToMuscleTime(const FILETIME & ft)
{
   union {
     uint64 ns100; /*time since 1 Jan 1601 in 100ns units */ 
     FILETIME ft; 
   } theTime; 
   theTime.ft = ft;

   static const uint64 TIME_DIFF = ((uint64)116444736)*((uint64)1000000000);
   struct timeval tv;
   tv.tv_usec = (long)((theTime.ns100 / ((uint64)10)) % ((uint64)1000000)); 
   tv.tv_sec  = (long)((theTime.ns100 - TIME_DIFF)    / ((uint64)10000000)); 
   return ConvertTimeValTo64(tv);
}
#endif

#endif  /* !__BEOS__ */

/** Defined here since every MUSCLE program will have to include this file anyway... */
uint64 GetCurrentTime64(uint32 timeType)
{
#ifdef WIN32
   FILETIME ft;
   GetSystemTimeAsFileTime(&ft);
   if (timeType == MUSCLE_TIMEZONE_LOCAL) (void) FileTimeToLocalFileTime(&ft, &ft);
   return __Win32FileTimeToMuscleTime(ft);
#else
# ifdef __BEOS__
   uint64 ret = real_time_clock_usecs();
# else
   struct timeval tv;
   gettimeofday(&tv, NULL);
   uint64 ret = ConvertTimeValTo64(tv);
# endif
   if (timeType == MUSCLE_TIMEZONE_LOCAL)
   {
      time_t now = time(NULL);
      struct tm * tm = gmtime(&now);
      if (tm) ret += ((int64)now-mktime(tm))*((int64)1000000);
   }
   return ret;
#endif
}

#if MUSCLE_TRACE_CHECKPOINTS > 0
static volatile uint32 _defaultTraceLocation[MUSCLE_TRACE_CHECKPOINTS];
volatile uint32 * _muscleTraceValues = _defaultTraceLocation;
uint32 _muscleNextTraceValueIndex = 0;

void SetTraceValuesLocation(volatile uint32 * location)
{
   _muscleTraceValues = location ? location : _defaultTraceLocation;
   _muscleNextTraceValueIndex = 0; 
   for (uint32 i=0; i<MUSCLE_TRACE_CHECKPOINTS; i++) _muscleTraceValues[i] = 0;
}
#endif

static AbstractObjectRecycler * _firstRecycler = NULL;

AbstractObjectRecycler :: AbstractObjectRecycler()
{
   Mutex * m = GetGlobalMuscleLock();
   if ((m)&&(m->Lock() != B_NO_ERROR)) m = NULL;

   // Append us to the front of the linked list
   if (_firstRecycler) _firstRecycler->_prev = this;
   _prev = NULL;
   _next = _firstRecycler;
   _firstRecycler = this;
   
   if (m) m->Unlock();
}

AbstractObjectRecycler :: ~AbstractObjectRecycler()
{
   Mutex * m = GetGlobalMuscleLock();
   if ((m)&&(m->Lock() != B_NO_ERROR)) m = NULL;

   // Remove us from the linked list
   if (_prev) _prev->_next = _next;
   if (_next) _next->_prev = _prev;
   if (_firstRecycler == this) _firstRecycler = _next;

   if (m) m->Unlock();
}

void AbstractObjectRecycler :: GlobalFlushAllCachedObjects()
{
   Mutex * m = GetGlobalMuscleLock();
   if ((m)&&(m->Lock() != B_NO_ERROR)) m = NULL;

   // We restart at the head of the list anytime anything is flushed,
   // for safety.  When we get to the end of the list, everything has
   // been flushed.
   AbstractObjectRecycler * r = _firstRecycler;
   while(r) r = (r->FlushCachedObjects() > 0) ? _firstRecycler : r->_next;

   if (m) m->Unlock();
}

CompleteSetupSystem :: ~CompleteSetupSystem()
{
   AbstractObjectRecycler::GlobalFlushAllCachedObjects();
}

uint32 DataIO :: WriteFully(const void * buffer, uint32 size)
{
   const uint8 * b = (const uint8 *)buffer;
   const uint8 * firstInvalidByte = b+size;
   while(b < firstInvalidByte)
   {
      int32 bytesWritten = Write(b, firstInvalidByte-b);
      if (bytesWritten <= 0) break;
      b += bytesWritten;
   }
   return (b-((const uint8 *)buffer));
}

uint32 DataIO :: ReadFully(void * buffer, uint32 size)
{
   uint8 * b = (uint8 *) buffer;
   uint8 * firstInvalidByte = b+size;
   while(b < firstInvalidByte)
   {
      int32 bytesRead = Read(b, firstInvalidByte-b);
      if (bytesRead <= 0) break;
      b += bytesRead;
   }
   return (b-((const uint8 *)buffer));
}

int64 DataIO :: GetLength()
{
   int64 origPos = GetPosition();
   if ((origPos >= 0)&&(Seek(0, IO_SEEK_END) == B_NO_ERROR))
   {
      int64 ret = GetPosition();
      if (Seek(origPos, IO_SEEK_SET) == B_NO_ERROR) return ret;
   }
   return -1;  // error!
}

status_t Flattenable :: FlattenToDataIO(DataIO & outputStream, bool addSizeHeader) const
{
   uint8 smallBuf[256];
   uint8 * bigBuf = NULL;
   uint32 fs = FlattenedSize();
   uint32 bufSize = fs+(addSizeHeader?sizeof(uint32):0);

   uint8 * b;
   if (bufSize<=ARRAYITEMS(smallBuf)) b = smallBuf;
   else
   {
      b = bigBuf = newnothrow_array(uint8, bufSize);
      if (bigBuf == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
   }

   // Populate the buffer
   if (addSizeHeader)
   {
      muscleCopyOut(b, B_HOST_TO_LENDIAN_INT32(fs));
      Flatten(b+sizeof(uint32));
   }
   else Flatten(b);

   // And finally, write out the buffer
   status_t ret = (outputStream.WriteFully(b, fs) == fs) ? B_NO_ERROR : B_ERROR;
   delete [] bigBuf;
   return ret;
}

status_t Flattenable :: UnflattenFromDataIO(DataIO & inputStream, int32 optReadSize, uint32 optMaxReadSize)
{
   uint32 readSize = (uint32) optReadSize;
   if (optReadSize < 0)
   {
      uint32 leSize;
      if (inputStream.ReadFully(&leSize, sizeof(leSize)) != sizeof(leSize)) return B_ERROR;
      readSize = (uint32) B_LENDIAN_TO_HOST_INT32(leSize);
      if (readSize > optMaxReadSize) return B_ERROR;
   }

   uint8 smallBuf[256];
   uint8 * bigBuf = NULL;
   uint8 * b;
   if (readSize<=ARRAYITEMS(smallBuf)) b = smallBuf;
   else
   {
      b = bigBuf = newnothrow_array(uint8, readSize);
      if (bigBuf == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
   }

   status_t ret = (inputStream.ReadFully(b, readSize) == readSize) ? Unflatten(b, readSize) : B_ERROR;
   delete [] bigBuf;
   return ret;
}

status_t Flattenable :: CopyFromImplementation(const Flattenable & copyFrom)
{
   uint8 smallBuf[256];
   uint8 * bigBuf = NULL;
   uint32 flatSize = copyFrom.FlattenedSize();
   if (flatSize > ARRAYITEMS(smallBuf))
   {
      bigBuf = newnothrow_array(uint8, flatSize);
      if (bigBuf == NULL)
      {
         WARN_OUT_OF_MEMORY;
         return B_ERROR;
      }
   }
   copyFrom.Flatten(bigBuf ? bigBuf : smallBuf);
   status_t ret = Unflatten(bigBuf ? bigBuf : smallBuf, flatSize);
   delete [] bigBuf;
   return ret;
}

void Inet_NtoA(uint32 addr, char * ipbuf)
{
   sprintf(ipbuf, "%li.%li.%li.%li", (addr>>24)&0xFF, (addr>>16)&0xFF, (addr>>8)&0xFF, (addr>>0)&0xFF);
}

uint32 Inet_AtoN(const char * buf)
{
   // net_server inexplicably doesn't have this function; so I'll just fake it
   uint32 ret = 0;
   int shift = 24;  // fill out the MSB first
   bool startQuad = true;
   while((shift >= 0)&&(*buf))
   {
      if (startQuad)
      {
         uint8 quad = (uint8) atoi(buf);
         ret |= (((uint32)quad) << shift);
         shift -= 8;
      }
      startQuad = (*buf == '.');
      buf++;
   }
   return ret;
}

static uint32 _customLocalhostIP = 0;  // disabled by default

void SetLocalHostIPOverride(uint32 ip) {_customLocalhostIP = ip;}
uint32 GetLocalHostIPOverride() {return _customLocalhostIP;}

/** These compare functions are useful for passing into Hashtables or Queues to keep them sorted */
int IntCompareFunc(   const int    & i1, const int    & i2, void *) {return muscleCompare(i1, i2);}
int Int8CompareFunc(  const int8   & i1, const int8   & i2, void *) {return muscleCompare(i1, i2);}
int Int16CompareFunc( const int16  & i1, const int16  & i2, void *) {return muscleCompare(i1, i2);}
int Int32CompareFunc( const int32  & i1, const int32  & i2, void *) {return muscleCompare(i1, i2);}
int Int64CompareFunc( const int64  & i1, const int64  & i2, void *) {return muscleCompare(i1, i2);}
int UIntCompareFunc(  const unsigned int & i1, const unsigned int & i2, void *) {return muscleCompare(i1, i2);}
int UInt8CompareFunc( const uint8  & i1, const uint8  & i2, void *) {return muscleCompare(i1, i2);}
int UInt16CompareFunc(const uint16 & i1, const uint16 & i2, void *) {return muscleCompare(i1, i2);}
int UInt32CompareFunc(const uint32 & i1, const uint32 & i2, void *) {return muscleCompare(i1, i2);}
int UInt64CompareFunc(const uint64 & i1, const uint64 & i2, void *) {return muscleCompare(i1, i2);}
int FloatCompareFunc( const float  & i1, const float  & i2, void *) {return muscleCompare(i1, i2);}
int DoubleCompareFunc(const double & i1, const double & i2, void *) {return muscleCompare(i1, i2);}

END_NAMESPACE(muscle);
