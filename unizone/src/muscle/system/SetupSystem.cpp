/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "system/SetupSystem.h"

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
# elif defined(__sparc__) || defined(sun386)
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
      _atomicMutexes = newnothrow Mutex[MUTEX_POOL_SIZE];
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

// For BeOS, this is an in-line function, defined in util/TimeUtilityFunctions.h
#ifndef __BEOS__

/** Defined here since every MUSCLE program will have to include this file anyway... */
uint64 GetRunTime64()
{
#ifdef WIN32
   static Mutex _rtMutex;
   if (_rtMutex.Lock() == B_NO_ERROR)
   {
      static int64 _brokenQPCOffset = 0;
      uint64 ret = 0;
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
            // see http://support.microsoft.com:80/support/kb/articles/Q274/3/23.ASP&NoWebContent=1
            static uint64 _lastCheckGetTime = 0;
            static uint64 _lastCheckQPCTime = 0;
            if (_lastCheckGetTime > 0)
            {
               uint64 getTimeElapsed = checkGetTime - _lastCheckGetTime;
               uint64 qpcTimeElapsed = ret          - _lastCheckQPCTime;
               if ((muscleMax(getTimeElapsed, qpcTimeElapsed) - muscleMin(getTimeElapsed, qpcTimeElapsed)) > 500000)
               {
                  LogTime(MUSCLE_LOG_WARNING, "QueryPerformanceCounter() is buggy, reverting to timeGetTime() method instead!\n");
                  _brokenQPCOffset = (_lastCheckQPCTime-_lastCheckGetTime);
uint64 origRet = ret;
                  ret = (((uint64)timeGetTime())*1000) + _brokenQPCOffset;
               }
            }
            _lastCheckGetTime = checkGetTime;
            _lastCheckQPCTime = ret;
         }
      }
      _rtMutex.Unlock();
      if (ret > 0) return ret;
   }

   // fallback method: convert milliseconds to microseconds -- will wrap after 49.7 days, doh!
   return (((uint64)timeGetTime())*1000);
#else
# if defined(MUSCLE_USE_POWERPC_INLINE_ASSEMBLY) && defined (MUSCLE_POWERPC_TIMEBASE_HZ)
   // http://lists.linuxppc.org/linuxppc-embedded/200110/msg00338.html
   uint32 hiWord, loWord, wrapCheck;
   __asm__ __volatile__("0:; mftbu %0; mftb %1; mftbu %2; cmpw %0,%2; bne 0" : "=&r" (hiWord), "=&r" (loWord), "=&r" (wrapCheck));
   return (((((uint64)hiWord)<<32)|((uint64)loWord))*((uint64)1000000))/((uint64)MUSCLE_POWERPC_TIMEBASE_HZ);
# else
   // default method:  use POSIX commands
   static clock_t _ticksPerSecond = 0;

   if (_ticksPerSecond <= 0) _ticksPerSecond = sysconf(_SC_CLK_TCK);

   struct tms junk;
   int64 curTicks = (int64) times(&junk);
   return ((_ticksPerSecond > 0)&&(curTicks >= 0)) ? ((curTicks*1000000)/_ticksPerSecond) : 0;
# endif
#endif
}

/** Defined here since every MUSCLE program will have to include this file anyway... */
uint64 GetCurrentTime64()
{
   struct timeval tv;
#ifdef WIN32
   union {
     uint64 ns100; /*time since 1 Jan 1601 in 100ns units */ 
     FILETIME ft; 
   } now; 
   GetSystemTimeAsFileTime(&now.ft);
   static const uint64 TIME_DIFF = ((uint64)116444736)*((uint64)1000000000);
   tv.tv_usec = (long)((now.ns100 / ((uint64)10)) % ((uint64)1000000)); 
   tv.tv_sec  = (long)((now.ns100 - TIME_DIFF)    / ((uint64)10000000)); 
#else
   gettimeofday(&tv, NULL);
#endif
   return ConvertTimeValTo64(tv);
}

#endif

END_NAMESPACE(muscle);
