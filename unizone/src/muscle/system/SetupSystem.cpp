/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include "system/SetupSystem.h"
#include "support/Flattenable.h"
#include "dataio/DataIO.h"
#include "util/ObjectPool.h"
#include "util/MiscUtilityFunctions.h"  // for ExitWithoutCleanup()
#include "util/DebugTimer.h"

#ifdef WIN32
# include <signal.h>
# include <mmsystem.h>
#else
# if defined(__BEOS__) || defined(__HAIKU__)
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

#if !defined(MUSCLE_SINGLE_THREAD_ONLY) && defined(MUSCLE_QT_HAS_THREADS)
# if QT_VERSION >= 0x040000
#  include <QThread>
# else
#  include <qthread.h>
# endif
#endif

namespace muscle {

#ifdef MUSCLE_SINGLE_THREAD_ONLY
bool _muscleSingleThreadOnly = true;
#else
bool _muscleSingleThreadOnly = false;
#endif

#ifdef MUSCLE_CATCH_SIGNALS_BY_DEFAULT
bool _mainReflectServerCatchSignals = true;
#else
bool _mainReflectServerCatchSignals = false;
#endif

static Mutex * _muscleLock = NULL;
Mutex * GetGlobalMuscleLock() {return _muscleLock;}

#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
const uint32 MUTEX_POOL_SIZE = 256;
static Mutex * _atomicMutexes = NULL;
#endif

static uint32 _threadSetupCount = 0;
#ifndef MUSCLE_SINGLE_THREAD_ONLY
static unsigned long _mainThreadID;
#endif

static int swap_memcmp(const void * vp1, const void * vp2, uint32 numBytes)
{
   const uint8 * p1 = (const uint8 *) vp1;
   const uint8 * p2 = (const uint8 *) vp2;
   for (uint32 i=0; i<numBytes; i++)
   {
      int diff = p2[numBytes-(i+1)]-p1[i];
      if (diff) return diff;
   }
   return 0;
}

static void GoInsane(const char * why, const char * why2 = NULL)
{
   printf("SanitySetupSystem:  MUSCLE COMPILATION RUNTIME SANITY CHECK FAILED!\n");
   printf("REASON:  %s %s\n", why, why2?why2:"");
   printf("PLEASE CHECK YOUR COMPILATION SETTINGS!  THIS PROGRAM WILL NOW EXIT.\n");
   fflush(stdout);
   ExitWithoutCleanup(10);
}

static void CheckOp(uint32 numBytes, const void * orig, const void * swapOne, const void * swapTwo, const void * origOne, const void * origTwo, const char * why)
{
   if ((swapOne)&&(swap_memcmp(orig, swapOne, numBytes))) GoInsane(why, "(swapOne)");
   if ((swapTwo)&&(swap_memcmp(orig, swapTwo, numBytes))) GoInsane(why, "(swapTwo)");
   if ((origOne)&&(memcmp(orig, origOne, numBytes)))      GoInsane(why, "(origOne)");
   if ((origTwo)&&(memcmp(orig, origTwo, numBytes)))      GoInsane(why, "(origTwo)");
}

SanitySetupSystem :: SanitySetupSystem()
{
   // Make sure our data type lengths are as expected
   if (sizeof(uint8)  != 1) GoInsane("sizeof(uint8) !=1");
   if (sizeof(int8)   != 1) GoInsane("sizeof(int8) !=1");
   if (sizeof(uint16) != 2) GoInsane("sizeof(uint16) !=2");
   if (sizeof(int16)  != 2) GoInsane("sizeof(int16) !=2");
   if (sizeof(uint32) != 4) GoInsane("sizeof(uint32) !=4");
   if (sizeof(int32)  != 4) GoInsane("sizeof(int32) !=4");
   if (sizeof(uint64) != 8) GoInsane("sizeof(uint64) !=8");
   if (sizeof(int64)  != 8) GoInsane("sizeof(int64) !=8");

   // Make sure our endian-ness info is correct
   static const uint32 one = 1;
   bool testsLittleEndian =  (*((const uint8 *) &one) == 1);

   // Make sure our endian-swap macros do what we expect them to
#if B_HOST_IS_BENDIAN
   if (testsLittleEndian) GoInsane("MUSCLE is compiled for a big-endian CPU, but host CPU is little-endian!?");
   else
   {
      {
         uint16 orig = 0x1234;
         uint16 HtoL = B_HOST_TO_LENDIAN_INT16(orig);
         uint16 LtoH = B_LENDIAN_TO_HOST_INT16(orig);
         uint16 HtoB = B_HOST_TO_BENDIAN_INT16(orig);  // should be a no-op
         uint16 BtoH = B_BENDIAN_TO_HOST_INT16(orig);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoL, &LtoH, &HtoB, &BtoH, "16-bit swap macro doesn't work!");
      }

      {
         uint32 orig = 0x12345678;
         uint32 HtoL = B_HOST_TO_LENDIAN_INT32(orig);
         uint32 LtoH = B_LENDIAN_TO_HOST_INT32(orig);
         uint32 HtoB = B_HOST_TO_BENDIAN_INT32(orig);  // should be a no-op
         uint32 BtoH = B_BENDIAN_TO_HOST_INT32(orig);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoL, &LtoH, &HtoB, &BtoH, "32-bit swap macro doesn't work!");
      }

      {
         uint64 orig = (((uint64)0x12345678)<<32)|(((uint64)0x12312312));
         uint64 HtoL = B_HOST_TO_LENDIAN_INT64(orig);
         uint64 LtoH = B_LENDIAN_TO_HOST_INT64(orig);
         uint64 HtoB = B_HOST_TO_BENDIAN_INT64(orig);  // should be a no-op
         uint64 BtoH = B_BENDIAN_TO_HOST_INT64(orig);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoL, &LtoH, &HtoB, &BtoH, "64-bit swap macro doesn't work!");
      }

      {
         float orig  = -1234567.89012345f;
         uint32 HtoL = B_HOST_TO_LENDIAN_IFLOAT(orig);
         float  LtoH = B_LENDIAN_TO_HOST_IFLOAT(HtoL);
         uint32 HtoB = B_HOST_TO_BENDIAN_IFLOAT(orig);  // should be a no-op
         float  BtoH = B_BENDIAN_TO_HOST_IFLOAT(HtoB);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoL, NULL, &HtoB, &BtoH, "float swap macro doesn't work!");
         CheckOp(sizeof(orig), &orig, NULL,  NULL, &LtoH,  NULL, "float swap macro doesn't work!");
      }

      {
         double orig = ((double)-1234567.89012345) * ((double)987654321.0987654321);
         uint64 HtoL = B_HOST_TO_LENDIAN_IDOUBLE(orig);
         double LtoH = B_LENDIAN_TO_HOST_IDOUBLE(HtoL);
         uint64 HtoB = B_HOST_TO_BENDIAN_IDOUBLE(orig);  // should be a no-op
         double BtoH = B_BENDIAN_TO_HOST_IDOUBLE(HtoB);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoL, NULL, &HtoB, &BtoH, "double swap macro doesn't work!");
         CheckOp(sizeof(orig), &orig,  NULL, NULL, &LtoH,  NULL, "double swap macro doesn't work!");
      }
   }
#else
   if (testsLittleEndian)
   {
      {
         uint16 orig = 0x1234;
         uint16 HtoB = B_HOST_TO_BENDIAN_INT16(orig);
         uint16 BtoH = B_BENDIAN_TO_HOST_INT16(orig);
         uint16 HtoL = B_HOST_TO_LENDIAN_INT16(orig);  // should be a no-op
         uint16 LtoH = B_LENDIAN_TO_HOST_INT16(orig);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoB, &BtoH, &HtoL, &LtoH, "16-bit swap macro doesn't work!");
      }

      {
         uint32 orig = 0x12345678;
         uint32 HtoB = B_HOST_TO_BENDIAN_INT32(orig);
         uint32 BtoH = B_BENDIAN_TO_HOST_INT32(orig);
         uint32 HtoL = B_HOST_TO_LENDIAN_INT32(orig);  // should be a no-op
         uint32 LtoH = B_LENDIAN_TO_HOST_INT32(orig);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoB, &BtoH, &HtoL, &LtoH, "32-bit swap macro doesn't work!");
      }

      {
         uint64 orig = (((uint64)0x12345678)<<32)|(((uint64)0x12312312));
         uint64 HtoB = B_HOST_TO_BENDIAN_INT64(orig);
         uint64 BtoH = B_BENDIAN_TO_HOST_INT64(orig);
         uint64 HtoL = B_HOST_TO_LENDIAN_INT64(orig);  // should be a no-op
         uint64 LtoH = B_LENDIAN_TO_HOST_INT64(orig);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoB, &BtoH, &HtoL, &LtoH, "64-bit swap macro doesn't work!");
      }

      {
         float orig  = -1234567.89012345f;
         uint32 HtoB = B_HOST_TO_BENDIAN_IFLOAT(orig);
         float  BtoH = B_BENDIAN_TO_HOST_IFLOAT(HtoB);
         uint32 HtoL = B_HOST_TO_LENDIAN_IFLOAT(orig);  // should be a no-op
         float  LtoH = B_LENDIAN_TO_HOST_IFLOAT(HtoL);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoB, NULL, &HtoL, &LtoH, "float swap macro doesn't work!");
         CheckOp(sizeof(orig), &orig,  NULL, NULL, &BtoH,  NULL, "float swap macro doesn't work!");
      }

      {
         double orig = ((double)-1234567.89012345) * ((double)987654321.0987654321);
         uint64 HtoB = B_HOST_TO_BENDIAN_IDOUBLE(orig);
         double BtoH = B_BENDIAN_TO_HOST_IDOUBLE(HtoB);
         uint64 HtoL = B_HOST_TO_LENDIAN_IDOUBLE(orig);  // should be a no-op
         double LtoH = B_LENDIAN_TO_HOST_IDOUBLE(HtoL);  // should be a no-op
         CheckOp(sizeof(orig), &orig, &HtoB, NULL, &HtoL, &LtoH, "double swap macro doesn't work!");
         CheckOp(sizeof(orig), &orig,  NULL, NULL, &BtoH,  NULL, "double swap macro doesn't work!");
      }
   }
   else GoInsane("MUSCLE is compiled for a little-endian CPU, but host CPU is big-endian!?");
#endif
}

SanitySetupSystem :: ~SanitySetupSystem()
{
   // empty
}

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

#ifndef MUSCLE_SINGLE_THREAD_ONLY
static unsigned long Muscle_GetCurrentThreadID()
{
# if defined(MUSCLE_USE_PTHREADS)
   return (unsigned long) pthread_self();
# elif defined(WIN32)
   return (unsigned long) GetCurrentThreadId();
# elif defined(MUSCLE_QT_HAS_THREADS)
#  if QT_VERSION >= 0x040000
   return (unsigned long) QThread::currentThreadId();
#  else
   return (unsigned long) QThread::currentThread();
#  endif
# elif defined(__BEOS__) || defined(__HAIKU__) || defined(__ATHEOS__)
   return (unsigned long) find_thread(NULL);
# else
#  error "Muscle_GetCurrentThreadID():  No implementation found for this OS!"
# endif
}
#endif

ThreadSetupSystem :: ThreadSetupSystem(bool muscleSingleThreadOnly)
{
   if (++_threadSetupCount == 1)
   {
#ifdef MUSCLE_SINGLE_THREAD_ONLY
      (void) muscleSingleThreadOnly;  // shut the compiler up
#else
      _mainThreadID = Muscle_GetCurrentThreadID();
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
   Mutex & mutex = _atomicMutexes[(((uint32)((unsigned long)count))/sizeof(int))%MUTEX_POOL_SIZE];  // double-cast for AMD64
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
      (void) ret;  // avoid compiler warning
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
#if !(defined(__BEOS__) || defined(__HAIKU__) || defined(TARGET_PLATFORM_XENOMAI))
/** Defined here since every MUSCLE program will have to include this file anyway... */
uint64 GetRunTime64()
{
# ifdef WIN32
   TCHECKPOINT;

   uint64 ret = 0;
   static Mutex _rtMutex;
   if (_rtMutex.Lock() == B_NO_ERROR)
   {
#  ifdef MUSCLE_USE_QUERYPERFORMANCECOUNTER
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
#  endif
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
# elif defined(__APPLE__)
   UnsignedWide uw = AbsoluteToNanoseconds(UpTime());
   return ((((uint64)uw.hi)<<32)|(uw.lo))/1000;
# else
#  if defined(MUSCLE_USE_POWERPC_INLINE_ASSEMBLY) && defined(MUSCLE_POWERPC_TIMEBASE_HZ)
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
#  else
#   if defined(MUSCLE_USE_LIBRT) && defined(_POSIX_MONOTONIC_CLOCK)
   struct timespec ts;
   return (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) ? ((((uint64)ts.tv_sec)*1000000)+(((uint64)ts.tv_nsec)/1000)) : 0; 
#   else
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
#   endif
#  endif
# endif
}
#endif

#if !(defined(__BEOS__) || defined(__HAIKU__))
status_t Snooze64(uint64 micros)
{
#if __ATHEOS__
   return (snooze(micros) >= 0) ? B_NO_ERROR : B_ERROR;
#elif WIN32
   Sleep((DWORD)((micros/1000)+(((micros%1000)!=0)?1:0)));
   return B_NO_ERROR;
#elif defined(MUSCLE_USE_LIBRT) && defined(_POSIX_MONOTONIC_CLOCK)
   const struct timespec ts = {micros/1000000, (micros%1000000)*1000};
   return (clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL) == 0) ? B_NO_ERROR : B_ERROR;
#else
   /** We can use select(), if nothing else */
   struct timeval waitTime;
   Convert64ToTimeVal(micros, waitTime);
   return (select(0, NULL, NULL, NULL, &waitTime) >= 0) ? B_NO_ERROR : B_ERROR;
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

#endif  /* !__BEOS__ && !__HAIKU__ */

/** Defined here since every MUSCLE program will have to include this file anyway... */
uint64 GetCurrentTime64(uint32 timeType)
{
#ifdef WIN32
   FILETIME ft;
   GetSystemTimeAsFileTime(&ft);
   if (timeType == MUSCLE_TIMEZONE_LOCAL) (void) FileTimeToLocalFileTime(&ft, &ft);
   return __Win32FileTimeToMuscleTime(ft);
#else
# if defined(__BEOS__) || defined(__HAIKU__)
   uint64 ret = real_time_clock_usecs();
# else
   struct timeval tv;
   gettimeofday(&tv, NULL);
   uint64 ret = ConvertTimeValTo64(tv);
# endif
   if (timeType == MUSCLE_TIMEZONE_LOCAL)
   {
      time_t now = time(NULL);
# if defined(__BEOS__) && !defined(__HAIKU__)
      struct tm * tm = gmtime(&now);
# else
      struct tm gmtm;
      struct tm * tm = gmtime_r(&now, &gmtm);
# endif
      if (tm) 
      {
         ret += ((int64)now-mktime(tm))*((int64)1000000);
         if (tm->tm_isdst>0) ret += 60*60*((int64)1000000);  // FogBugz #4498
      }
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

static CompleteSetupSystem * _activeCSS = NULL;
CompleteSetupSystem * CompleteSetupSystem :: GetCurrentCompleteSetupSystem() {return _activeCSS;}

CompleteSetupSystem :: CompleteSetupSystem(bool muscleSingleThreadOnly) : _threads(muscleSingleThreadOnly), _prevInstance(_activeCSS)
{
   _activeCSS = this;  // push us onto the stack
}

CompleteSetupSystem :: ~CompleteSetupSystem()
{
   GenericCallbackRef r;
   while(_cleanupCallbacks.RemoveTail(r) == B_NO_ERROR) (void) r()->Callback(NULL);

   AbstractObjectRecycler::GlobalFlushAllCachedObjects();

   _activeCSS = _prevInstance;  // pop us off the stack
}

// Implemented here so that every program doesn't have to link
// in MiscUtilityFunctions.cpp just for this function.
void ExitWithoutCleanup(int exitCode)
{
   _exit(exitCode);
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
   status_t ret = (outputStream.WriteFully(b, bufSize) == bufSize) ? B_NO_ERROR : B_ERROR;
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

// This function is now a private one, since it should no longer be necessary to call it
// from user code.  Instead, attach any socket file descriptors you create to ConstSocketRef
// objects by calling GetConstSocketRefFromPool(fd), and the file descriptors will be automatically
// closed when the last ConstSocketRef that references them is destroyed.
static void CloseSocket(int fd)
{
   if (fd >= 0)
   {
#if defined(WIN32) || defined(BEOS_OLD_NETSERVER)
      ::closesocket(fd);
#else
      close(fd);
#endif
   }
}

const ConstSocketRef & GetNullSocket()
{
   static const ConstSocketRef _null;
   return _null;
}

const ConstSocketRef & GetInvalidSocket()
{
   static Socket _invalidSocket;
   static ConstSocketRef _ref(&_invalidSocket, false);
   return _ref;
}

// Our socket should be closed when we are recycled, not just when we are deleted!
static void RecycleSocketFunc(Socket * s, void *) {s->SetFileDescriptor(-1, false);}

static ConstSocketRef::ItemPool _socketPool(100, RecycleSocketFunc);
ConstSocketRef GetConstSocketRefFromPool(int fd, bool okayToClose, bool returnNULLOnInvalidFD)
{
   if ((fd < 0)&&(returnNULLOnInvalidFD)) return ConstSocketRef();
   else
   {
      Socket * s = _socketPool.ObtainObject();
      ConstSocketRef ret(s);

      if (s) s->SetFileDescriptor(fd, okayToClose);
        else if (okayToClose) CloseSocket(fd);
      return ret;
   }
}

Socket :: ~Socket()
{
   SetFileDescriptor(-1, false);
}

void Socket :: SetFileDescriptor(int newFD, bool okayToClose)
{
   if (newFD != _fd)
   {
      if ((_fd >= 0)&&(_okayToClose)) CloseSocket(_fd);
      _fd = newFD; 
   }
   _okayToClose = okayToClose;
}

uint32 CalculateChecksum(const uint8 * buffer, uint32 numBytes)
{
   uint32 ret      = numBytes;
   uint32 numWords = numBytes/sizeof(uint32);
   uint32 leftover = numBytes%sizeof(uint32);

   const uint32 * words = (const uint32 *) buffer;
   for (uint32 i=0; i<numWords; i++) ret += words[i];

   const uint8 * bytes = (const uint8 *) (((const uint32 *)buffer)+numWords);
   for (uint32 j=0; j<leftover; j++) ret += (uint32)bytes[j];

   return ret; 
}

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

static void FlushAsciiChars(FILE * file, int idx, char * ascBuf, char * hexBuf, uint32 count, uint32 numColumns)
{
   while(count<numColumns) ascBuf[count++] = ' ';
   ascBuf[count] = '\0';
   fprintf(file, "%04i: %s [%s]\n", idx, ascBuf, hexBuf);
   hexBuf[0] = '\0';
}

static void FlushLogAsciiChars(int lvl, int idx, char * ascBuf, char * hexBuf, uint32 count, uint32 numColumns)
{
   while(count<numColumns) ascBuf[count++] = ' ';
   ascBuf[count] = '\0';
   LogTime(lvl, "%04i: %s [%s]\n", idx, ascBuf, hexBuf);
   hexBuf[0] = '\0';
}

void PrintHexBytes(const void * vbuf, uint32 numBytes, const char * optDesc, uint32 numColumns, FILE * optFile)
{
   if (optFile == NULL) optFile = stdout;

   const uint8 * buf = (const uint8 *) vbuf;

   if (numColumns == 0)
   {
      // A simple, single-line format
      if (optDesc) fprintf(optFile, "%s: ", optDesc);
      fprintf(optFile, "[");
      for (uint32 i=0; i<numBytes; i++) fprintf(optFile, "%s%02x", (i==0)?"":" ", buf[i]);
      fprintf(optFile, "]\n");
   }
   else
   {
      // A more useful columnar format with ASCII sidebar
      char headBuf[256]; 
      sprintf(headBuf, "--- %s ("UINT32_FORMAT_SPEC" bytes): ", ((optDesc)&&(strlen(optDesc)<200))?optDesc:"", numBytes);
      fprintf(optFile, "%s", headBuf);

      const int hexBufSize = (numColumns*8)+1;
      int numDashes = 8+(4*numColumns)-strlen(headBuf);
      for (int i=0; i<numDashes; i++) putchar('-');
      putchar('\n');
      char * ascBuf = newnothrow_array(char, numColumns+1);
      char * hexBuf = newnothrow_array(char, hexBufSize);
      if ((ascBuf)&&(hexBuf))
      {
         ascBuf[0] = hexBuf[0] = '\0';

         uint32 idx = 0;
         while(idx<numBytes)
         {
            uint8 c = buf[idx];
            ascBuf[idx%numColumns] = muscleInRange(c,(uint8)' ',(uint8)'~')?c:'.';
            char temp[8]; sprintf(temp, "%s%02x", ((idx%numColumns)==0)?"":" ", (unsigned int)(((uint32)buf[idx])&0xFF));
            strncat(hexBuf, temp, hexBufSize);
            idx++;
            if ((idx%numColumns) == 0) FlushAsciiChars(optFile, idx-numColumns, ascBuf, hexBuf, numColumns, numColumns);
         }
         uint32 leftovers = (numBytes%numColumns);
         if (leftovers > 0) FlushAsciiChars(optFile, numBytes-leftovers, ascBuf, hexBuf, leftovers, numColumns);
      }
      else WARN_OUT_OF_MEMORY;

      delete [] ascBuf;
      delete [] hexBuf;
   }
}

void PrintHexBytes(const Queue<uint8> & buf, const char * optDesc, uint32 numColumns, FILE * optFile)
{
   if (optFile == NULL) optFile = stdout;

   uint32 numBytes = buf.GetNumItems();
   if (numColumns == 0)
   {
      // A simple, single-line format
      if (optDesc) fprintf(optFile, "%s: ", optDesc);
      fprintf(optFile, "[");
      for (uint32 i=0; i<numBytes; i++) fprintf(optFile, "%s%02x", (i==0)?"":" ", buf[i]);
      fprintf(optFile, "]\n");
   }
   else
   {
      // A more useful columnar format with ASCII sidebar
      char headBuf[256]; 
      sprintf(headBuf, "--- %s ("UINT32_FORMAT_SPEC" bytes): ", ((optDesc)&&(strlen(optDesc)<200))?optDesc:"", numBytes);
      fprintf(optFile, "%s", headBuf);

      const int hexBufSize = (numColumns*8)+1;
      int numDashes = 8+(4*numColumns)-strlen(headBuf);
      for (int i=0; i<numDashes; i++) putchar('-');
      putchar('\n');
      char * ascBuf = newnothrow_array(char, numColumns+1);
      char * hexBuf = newnothrow_array(char, hexBufSize);
      if ((ascBuf)&&(hexBuf))
      {
         ascBuf[0] = hexBuf[0] = '\0';

         uint32 idx = 0;
         while(idx<numBytes)
         {
            uint8 c = buf[idx];
            ascBuf[idx%numColumns] = muscleInRange(c,(uint8)' ',(uint8)'~')?c:'.';
            char temp[8]; sprintf(temp, "%s%02x", ((idx%numColumns)==0)?"":" ", (unsigned int)(((uint32)buf[idx])&0xFF));
            strncat(hexBuf, temp, hexBufSize);
            idx++;
            if ((idx%numColumns) == 0) FlushAsciiChars(optFile, idx-numColumns, ascBuf, hexBuf, numColumns, numColumns);
         }
         uint32 leftovers = (numBytes%numColumns);
         if (leftovers > 0) FlushAsciiChars(optFile, numBytes-leftovers, ascBuf, hexBuf, leftovers, numColumns);
      }
      else WARN_OUT_OF_MEMORY;

      delete [] ascBuf;
      delete [] hexBuf;
   }
}

void LogHexBytes(int logLevel, const void * vbuf, uint32 numBytes, const char * optDesc, uint32 numColumns)
{
   const uint8 * buf = (const uint8 *) vbuf;

   if (numColumns == 0)
   {
      // A simple, single-line format
      if (optDesc) LogTime(logLevel, "%s: ", optDesc);
      Log(logLevel, "[");
      for (uint32 i=0; i<numBytes; i++) Log(logLevel, "%s%02x", (i==0)?"":" ", buf[i]);
      Log(logLevel, "]\n");
   }
   else
   {
      // A more useful columnar format with ASCII sidebar
      char headBuf[256]; 
      sprintf(headBuf, "--- %s ("UINT32_FORMAT_SPEC" bytes): ", ((optDesc)&&(strlen(optDesc)<200))?optDesc:"", numBytes);
      LogTime(logLevel, "%s", headBuf);

      const int hexBufSize = (numColumns*8)+1;
      int numDashes = 8+(4*numColumns)-strlen(headBuf);
      for (int i=0; i<numDashes; i++) Log(logLevel, "-");
      Log(logLevel, "\n");
      char * ascBuf = newnothrow_array(char, numColumns+1);
      char * hexBuf = newnothrow_array(char, hexBufSize);
      if ((ascBuf)&&(hexBuf))
      {
         ascBuf[0] = hexBuf[0] = '\0';

         uint32 idx = 0;
         while(idx<numBytes)
         {
            uint8 c = buf[idx];
            ascBuf[idx%numColumns] = muscleInRange(c,(uint8)' ',(uint8)'~')?c:'.';
            char temp[8]; sprintf(temp, "%s%02x", ((idx%numColumns)==0)?"":" ", (unsigned int)(((uint32)buf[idx])&0xFF));
            strncat(hexBuf, temp, hexBufSize);
            idx++;
            if ((idx%numColumns) == 0) FlushLogAsciiChars(logLevel, idx-numColumns, ascBuf, hexBuf, numColumns, numColumns);
         }
         uint32 leftovers = (numBytes%numColumns);
         if (leftovers > 0) FlushLogAsciiChars(logLevel, numBytes-leftovers, ascBuf, hexBuf, leftovers, numColumns);
      }
      else WARN_OUT_OF_MEMORY;

      delete [] ascBuf;
      delete [] hexBuf;
   }
}

void LogHexBytes(int logLevel, const Queue<uint8> & buf, const char * optDesc, uint32 numColumns)
{
   uint32 numBytes = buf.GetNumItems();
   if (numColumns == 0)
   {
      // A simple, single-line format
      if (optDesc) LogTime(logLevel, "%s: ", optDesc);
      Log(logLevel, "[");
      for (uint32 i=0; i<numBytes; i++) Log(logLevel, "%s%02x", (i==0)?"":" ", buf[i]);
      Log(logLevel, "]\n");
   }
   else
   {
      // A more useful columnar format with ASCII sidebar
      char headBuf[256]; 
      sprintf(headBuf, "--- %s ("UINT32_FORMAT_SPEC" bytes): ", ((optDesc)&&(strlen(optDesc)<200))?optDesc:"", numBytes);
      Log(logLevel, "%s", headBuf);

      const int hexBufSize = (numColumns*8)+1;
      int numDashes = 8+(4*numColumns)-strlen(headBuf);
      for (int i=0; i<numDashes; i++) Log(logLevel, "-");
      Log(logLevel, "\n");
      char * ascBuf = newnothrow_array(char, numColumns+1);
      char * hexBuf = newnothrow_array(char, hexBufSize);
      if ((ascBuf)&&(hexBuf))
      {
         ascBuf[0] = hexBuf[0] = '\0';

         uint32 idx = 0;
         while(idx<numBytes)
         {
            uint8 c = buf[idx];
            ascBuf[idx%numColumns] = muscleInRange(c,(uint8)' ',(uint8)'~')?c:'.';
            char temp[8]; sprintf(temp, "%s%02x", ((idx%numColumns)==0)?"":" ", (unsigned int)(((uint32)buf[idx])&0xFF));
            strncat(hexBuf, temp, hexBufSize);
            idx++;
            if ((idx%numColumns) == 0) FlushLogAsciiChars(logLevel, idx-numColumns, ascBuf, hexBuf, numColumns, numColumns);
         }
         uint32 leftovers = (numBytes%numColumns);
         if (leftovers > 0) FlushLogAsciiChars(logLevel, numBytes-leftovers, ascBuf, hexBuf, leftovers, numColumns);
      }
      else WARN_OUT_OF_MEMORY;

      delete [] ascBuf;
      delete [] hexBuf;
   }
}

DebugTimer :: DebugTimer(const String & title, uint64 mlt, uint32 startMode, int debugLevel) : _currentMode(startMode+1), _title(title), _minLogTime(mlt), _debugLevel(debugLevel), _enableLog(true)
{
   SetMode(startMode);
   _startTime = MUSCLE_DEBUG_TIMER_CLOCK;  // re-set it here so that we don't count the Hashtable initialization!
}

DebugTimer :: ~DebugTimer() 
{
   if (_enableLog)
   {
      // Finish off the current mode
      uint64 * curElapsed = _modeToElapsedTime.Get(_currentMode);
      if (curElapsed) *curElapsed += MUSCLE_DEBUG_TIMER_CLOCK-_startTime;

      // And print out our stats
      for (HashtableIterator<uint32, uint64> iter(_modeToElapsedTime); iter.HasMoreKeys(); iter++)
      {
         uint64 nextTime = iter.GetValue();
         if (nextTime >= _minLogTime)
         {
            if (nextTime >= 1000) LogTime(_debugLevel, "%s: mode "UINT32_FORMAT_SPEC": " UINT64_FORMAT_SPEC " milliseconds elapsed\n", _title(), iter.GetKey(), nextTime/1000);
                             else LogTime(_debugLevel, "%s: mode "UINT32_FORMAT_SPEC": " UINT64_FORMAT_SPEC " microseconds elapsed\n", _title(), iter.GetKey(), nextTime);
         }
      }
   }
}

/** Set the timer to record elapsed time to a different mode. */
void DebugTimer :: SetMode(uint32 newMode)
{
   if (newMode != _currentMode)
   {
      uint64 * curElapsed = _modeToElapsedTime.Get(_currentMode);
      if (curElapsed) *curElapsed += MUSCLE_DEBUG_TIMER_CLOCK-_startTime;

      _currentMode = newMode;
      (void) _modeToElapsedTime.GetOrPut(_currentMode, 0);
      _startTime = MUSCLE_DEBUG_TIMER_CLOCK;
   }
}

#ifdef MUSCLE_SINGLE_THREAD_ONLY
bool IsCurrentThreadMainThread() {return true;}
#else
bool IsCurrentThreadMainThread() 
{
   if (_threadSetupCount > 0) return (Muscle_GetCurrentThreadID() == _mainThreadID);
   else 
   {
      MCRASH("IsCurrentThreadMainThread() cannot be called unless there is a CompleteSetupSystem object on the stack!");
      return false;  // to shut the compiler up
   }
}
#endif

}; // end namespace muscle
