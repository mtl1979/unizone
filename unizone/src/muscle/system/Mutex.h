/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

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

namespace muscle {

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
# if defined(MUSCLE_USE_PTHREADS)
      : _lockCount(0)
# elif defined(QT_THREAD_SUPPORT)
      : _locker(true)
# elif defined(WIN32)
      : _locker(CreateMutex(NULL, false, NULL))
# elif defined(__ATHEOS__)
      : _locker(NULL) 
# endif
#endif
   {
#ifndef MUSCLE_SINGLE_THREAD_ONLY
# if defined(MUSCLE_USE_PTHREADS)
      pthread_mutex_init(&_locker, NULL);
# endif
#endif
   }
 
   /** Destructor.  If a Lock is destroyed while another thread is blocking in its Lock() method,
     * the results are undefined.
     */
   ~Mutex() 
   {
#ifndef MUSCLE_SINGLE_THREAD_ONLY
# if defined(MUSCLE_USE_PTHREADS)
      pthread_mutex_destroy(&_locker);
# elif defined(QT_THREAD_SUPPORT)
      // do nothing
# elif defined(WIN32)
      CloseHandle(_locker);
# endif
#endif
   }

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
#elif defined(MUSCLE_USE_PTHREADS)
      pthread_t self = pthread_self();
      if ((_lockCount == 0)||(!pthread_equal(_lockHolder,self))) // note: assumes (_lockCount==0) is atomic
      {
         if (pthread_mutex_lock(&_locker) == 0) _lockHolder = self;
                                           else return B_ERROR;
      }
      _lockCount++;
      return B_NO_ERROR;
#elif defined(QT_THREAD_SUPPORT)
      _locker.lock();
      return B_NO_ERROR;
#elif defined(__BEOS__)
      return _locker.Lock() ? B_NO_ERROR : B_ERROR;
#elif defined(WIN32)
      return ((_locker)&&(WaitForSingleObject(_locker, INFINITE) == WAIT_FAILED)) ? B_ERROR : B_NO_ERROR;
#elif defined(__ATHEOS__)
      return _locker.Lock() ? B_ERROR : B_NO_ERROR;  // Is this correct?  Kurt's documentation sucks
#endif
   }

   /** Unlocks the lock.  Once this is done, any other thread that is blocked in the Lock()
     * method will gain ownership of the lock and return.
     * @returns B_NO_ERROR on success, or B_ERORR on failure (perhaps you tried to unlock a lock
     *          that wasn't locked?  This method should never fail in typical usage)
     */
   status_t Unlock() const
   {
#ifdef MUSCLE_SINGLE_THREAD_ONLY
      return B_NO_ERROR;
#elif defined(MUSCLE_USE_PTHREADS)
      return ((pthread_equal(_lockHolder,pthread_self()))&&((--_lockCount > 0)||(pthread_mutex_unlock(&_locker) == 0))) ? B_NO_ERROR : B_ERROR;
#elif defined(QT_THREAD_SUPPORT)
      _locker.unlock();
      return B_NO_ERROR;
#elif defined(__BEOS__)
      _locker.Unlock();
      return B_NO_ERROR;
#elif defined(WIN32)
      return ((_locker)&&(ReleaseMutex(_locker))) ? B_NO_ERROR : B_ERROR;
#elif defined(__ATHEOS__)
      return _locker.Unlock() ? B_ERROR : B_NO_ERROR;  // Is this correct?  Kurt's documentation sucks
#endif
   }

private:
#ifndef MUSCLE_SINGLE_THREAD_ONLY
# if defined(MUSCLE_USE_PTHREADS)
   mutable pthread_mutex_t _locker;  // gotta do some extra work to make the mutex
   mutable int _lockCount;           // recursive, since pthreads doesn't support
   mutable pthread_t _lockHolder;    // recursive mutexes directly (at least, not portably)
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

};  // end namespace muscle

#endif
