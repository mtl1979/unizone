/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "system/SetupSystem.h"

#ifdef WIN32
# include <winsock.h>
# include <signal.h>
#else
# if defined(__BEOS__) || defined(__CYGWIN__)
#  include <signal.h>
# else
#  include <sys/signal.h>  // changed signal.h to sys/signal.h to work with OS/X
# endif
#endif

namespace muscle {

#ifndef MUSCLE_MINIMALIST_LOGGING
extern Mutex * _muscleLogLock;
#endif

#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
const uint32 MUTEX_POOL_SIZE = 256;
static Mutex * _atomicMutexes = NULL;
#endif

ThreadSetupSystem :: ThreadSetupSystem()
{
#ifndef MUSCLE_MINIMALIST_LOGGING
   _muscleLogLock = &_logLock;
#endif
   
#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
   MASSERT(_atomicMutexes == NULL, "Don't create more than one ThreadSetupSystem!");
   _atomicMutexes = newnothrow Mutex[MUTEX_POOL_SIZE];
   MASSERT(_atomicMutexes, "Couldn't allocate atomic mutexes!");
#endif
}

ThreadSetupSystem :: ~ThreadSetupSystem()
{
#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
   delete [] _atomicMutexes;
   _atomicMutexes = NULL;
#endif
#ifndef MUSCLE_MINIMALIST_LOGGING
   _muscleLogLock = NULL;
#endif
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

NetworkSetupSystem :: NetworkSetupSystem()
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

NetworkSetupSystem :: ~NetworkSetupSystem()
{
#ifdef WIN32
   WSACleanup();
#endif
}

};  // end muscle namespace
