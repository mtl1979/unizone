/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "system/SetupSystem.h"

#ifdef WIN32
# include <winsock.h>
# include <signal.h>
#else
# if defined(__BEOS__) || defined(__CYGWIN__)
#  include <signal.h>
# else
#  include <sys/signal.h>  // changed signal.h to sys/signal.h to work with OS/X
#  include <sys/times.h>
# endif
#endif

namespace muscle {

static Mutex * _muscleLock = NULL;
Mutex * GetGlobalMuscleLock() {return _muscleLock;}

#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
const uint32 MUTEX_POOL_SIZE = 256;
static Mutex * _atomicMutexes = NULL;
#endif

static uint32 _threadSetupCount = 0;

ThreadSetupSystem :: ThreadSetupSystem()
{
   if (++_threadSetupCount == 1)
   {
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
      delete [] _atomicMutexes;
      _atomicMutexes = NULL;
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

/** Defined here since every MUSCLE program will have to include this anyway... */
uint64 GetRunTime64()
{
#ifdef WIN32
   static bool _gotFrequency = false;
   static uint64 _ticksPerSecond;
   if (_gotFrequency == false)
   {
      LARGE_INTEGER tps;
      _ticksPerSecond = (QueryPerformanceFrequency(&tps)) ? ((((uint64)tps.HighPart)<<32)|((uint64)tps.LowPart)) : 0;
      _gotFrequency = true;
   }
   if (_ticksPerSecond > 0)
   {
      LARGE_INTEGER curTicks;
      if (QueryPerformanceCounter(&curTicks))
      {
         uint64 cts = ((((uint64)curTicks.HighPart)<<32)|((uint64)curTicks.LowPart));
         return (cts*1000000)/_ticksPerSecond;
      }
      else return 0;
   }
   return ((uint64)timeGetTime())*1000;  // convert milliseconds to microseconds -- will wrap after 49.7 days, doh!
#else
   // default method:  use POSIX commands
   static clock_t _ticksPerSecond = 0;

   if (_ticksPerSecond <= 0) _ticksPerSecond = sysconf(_SC_CLK_TCK);

   struct tms junk;
   int64 curTicks = (int64) times(&junk);
   return ((_ticksPerSecond > 0)&&(curTicks >= 0)) ? ((curTicks*1000000)/_ticksPerSecond) : 0;
#endif
}

#endif


};  // end muscle namespace
