/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleMutex_h
#define MuscleMutex_h

#ifndef MUSCLE_SINGLE_THREAD_ONLY
# if defined(MUSCLE_USE_PTHREADS)
#  include <pthread.h>
# elif defined(QT_THREAD_SUPPORT)
#  include <qthread.h>
# elif defined(__BEOS__)
#  include <support/Locker.h>
# elif defined(WIN32)
#  include <windows.h>
# elif defined(__ATHEOS__)
#  include <util/locker.h>
# else
#  error "Lock:  threading support not implemented for this platform.  You'll need to add code to the MUSCLE Lock class for your platform, or add -DMUSCLE_SINGLE_THREAD_ONLY to your build line if your program is single-threaded or for some other reason doesn't need to worry about locking"
# endif
#endif

#include "support/MuscleSupport.h"

BEGIN_NAMESPACE(muscle);

// If false, then we must not assume that we are running in single-threaded mode.
// This variable should be set by the ThreadSetupSystem constructor ONLY!
extern bool _muscleSingleThreadOnly;

/** This class is a platform-independent API for a recursive mutual exclusion semaphore (a.k.a mutex). 
  * Typically used to serialize the execution of critical sections in a multithreaded API 
  * (e.g. the MUSCLE ObjectPool or Thread classes)
  * When compiling with the MUSCLE_SINGLE_THREAD_ONLY preprocessor flag defined, this class becomes a no-op.
  */
class Mutex
{
public:
   /** Constructor */
   Mutex()
#ifndef MUSCLE_SINGLE_THREAD_ONLY
      : _isEnabled(_muscleSingleThreadOnly == false)
# if defined(MUSCLE_USE_PTHREADS)
      // empty
# elif defined(QT_THREAD_SUPPORT)
      , _locker(true)
# elif defined(WIN32)
      , _locker(_isEnabled ? CreateMutex(NULL, false, NULL) : NULL)
# elif defined(__ATHEOS__)
      , _locker(NULL) 
# endif
#endif
   {
#ifndef MUSCLE_SINGLE_THREAD_ONLY
      if (_isEnabled)
      {
# if defined(MUSCLE_USE_PTHREADS)
         pthread_mutexattr_t mutexattr;
         pthread_mutexattr_init(&mutexattr);                              // Note:  If this code doesn't compile, then
         pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);  // you may need to add -D_GNU_SOURCE to your
         pthread_mutex_init(&_locker, &mutexattr);                        // Linux Makefile to enable it properly.
# endif
      }
#endif
   }
 
   /** Destructor.  If a Lock is destroyed while another thread is blocking in its Lock() method,
     * the results are undefined.
     */
   ~Mutex() {Cleanup();}

   /** Attempts to lock the lock. 
     * Any thread that tries to Lock() this object while it is already locked by another thread
     * will block until the other thread unlocks the lock.  The lock is recursive, however;
     * if a given thread calls Lock() twice in a row it won't deadlock itself (although it will
     * need to call Unlock() twice in a row in order to truly unlock the lock)
     * @returns B_NO_ERROR on success, or B_ERROR if the lock could not be locked for some reason.
     */
   status_t Lock() const
   {
#ifdef MUSCLE_SINGLE_THREAD_ONLY
      return B_NO_ERROR;
#else
      if (_isEnabled == false) return B_NO_ERROR;
# if defined(MUSCLE_USE_PTHREADS)
      return (pthread_mutex_lock(&_locker) == 0) ? B_NO_ERROR : B_ERROR;
# elif defined(QT_THREAD_SUPPORT)
      _locker.lock();
      return B_NO_ERROR;
# elif defined(__BEOS__)
      return _locker.Lock() ? B_NO_ERROR : B_ERROR;
# elif defined(WIN32)
      return ((_locker)&&(WaitForSingleObject(_locker, INFINITE) == WAIT_FAILED)) ? B_ERROR : B_NO_ERROR;
# elif defined(__ATHEOS__)
      return _locker.Lock() ? B_ERROR : B_NO_ERROR;  // Is this correct?  Kurt's documentation sucks
# endif
#endif
   }

   /** Unlocks the lock.  Once this is done, any other thread that is blocked in the Lock()
     * method will gain ownership of the lock and return.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (perhaps you tried to unlock a lock
     *          that wasn't locked?  This method should never fail in typical usage)
     */
   status_t Unlock() const
   {
#ifdef MUSCLE_SINGLE_THREAD_ONLY
      return B_NO_ERROR;
#else
      if (_isEnabled == false) return B_NO_ERROR;
# if defined(MUSCLE_USE_PTHREADS)
      return (pthread_mutex_unlock(&_locker) == 0) ? B_NO_ERROR : B_ERROR;
# elif defined(QT_THREAD_SUPPORT)
      _locker.unlock();
      return B_NO_ERROR;
# elif defined(__BEOS__)
      _locker.Unlock();
      return B_NO_ERROR;
# elif defined(WIN32)
      return ((_locker)&&(ReleaseMutex(_locker))) ? B_NO_ERROR : B_ERROR;
# elif defined(__ATHEOS__)
      return _locker.Unlock() ? B_ERROR : B_NO_ERROR;  // Is this correct?  Kurt's documentation sucks
# endif
#endif
   }

   /** Turns this Mutex into a no-op object.  Irreversible! */
   void Neuter() {Cleanup();}

private:
   void Cleanup()
   {
#ifndef MUSCLE_SINGLE_THREAD_ONLY
      if (_isEnabled)
      {
# if defined(MUSCLE_USE_PTHREADS)
         pthread_mutex_destroy(&_locker);
# elif defined(QT_THREAD_SUPPORT)
         // do nothing
# elif defined(WIN32)
         CloseHandle(_locker);
# endif

         _isEnabled = false;
      }
#endif
   }

#ifndef MUSCLE_SINGLE_THREAD_ONLY
   bool _isEnabled;  // if false, this Mutex is a no-op
# if defined(MUSCLE_USE_PTHREADS)
   mutable pthread_mutex_t _locker;
# elif defined(QT_THREAD_SUPPORT)
   mutable QMutex _locker;
# elif defined(__BEOS__)
   mutable BLocker _locker;
# elif defined(WIN32)
   mutable HANDLE _locker;
# elif defined(__ATHEOS__)
   mutable os::Locker _locker;
# endif
#endif
};

#ifdef MUSCLE_USE_PTHREADS
// These calls are useful in conjunction with tests/deadlockfinder.cpp, for tracking
// down potential synchronization deadlocks in multithreaded code.  Note that they only
// work when using a pThreads environment, however.
#define PLOCK(  name,pointer) _PLOCKimp(  name,pointer,__FILE__,__LINE__)
#define PUNLOCK(name,pointer) _PUNLOCKimp(name,pointer,__FILE__,__LINE__)

// don't call these directly -- call the PLOCK() and PUNLOCK() macros instead!
static inline void _PLOCKimp(const char * n, const void * p, const char * f, int ln) {printf("%li plock p=%p [%s] %s:%i\n", (int32)pthread_self(), p, n, f, ln);}
static inline void _PUNLOCKimp(const char * n, const void * p, const char * f, int ln) {printf("%li punlock p=%p [%s] %s:%i\n", (int32)pthread_self(), p, n, f, ln);}
#endif

END_NAMESPACE(muscle);

#endif
