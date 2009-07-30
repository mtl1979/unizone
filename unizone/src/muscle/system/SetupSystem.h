/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleSetupSystem_h
#define MuscleSetupSystem_h

#include "system/AtomicCounter.h"
#include "system/Mutex.h"
#include "util/GenericCallback.h"
#include "util/Queue.h"

namespace muscle {

class AtomicCounter;

/** SetupSystem is the base class for an object that sets up
  * the environment to handle the sort of things we'll
  * be wanting to do.  Typically System objects are placed
  * on the stack at the beginning of main().  They do
  * the setup in their constructor, and tear it down
  * again in their destructor. 
  */
class SetupSystem
{
protected:
   /** Default constructor, a no-op.
    *  It's protected because you should never instantiate a SetupSystem object alone;
    *  it should always be subclassed to.  (consider it an abstract base class, except
    *  without any pure virtuals defined)
   */
   SetupSystem() {/* empty */}
  
public:
   /** Virtual destructor to keep C++ honest */
   virtual ~SetupSystem() {/* empty */}
};

/** This SetupSystem handles initializing the environment's threading mechanisms. */
class ThreadSetupSystem : public SetupSystem
{
public:
   /** Constructor.  Currently a no-op except for
     * when MUSCLE_USE_PTHREADS is defined; in that
     * case it will set up some mutexes for use
     * by the AtomicCounter class.
     * @param muscleSingleThreadOnly If set to true, the MUSCLE code will assume that
     *                      this process is going to be single-threaded, even if the 
     *                      code was not compiled with the -DMUSCLE_SINGLE_THREAD_ONLY flag!
     *                      This can be useful to gain a bit of extra efficiency, if you 
     *                      need to compile your code to be multithread-capable but can 
     *                      sometimes promise that this particular process will never 
     *                      spawn multiple threads.
     *                      If your code is NEVER multi-threaded, then it is more efficient to define
     *                      -DMUSCLE_SINGLE_THREAD_ONLY in your Makefile, rather than setting this flag to false.
     *                      If -DMUSCLE_SINGLE_THREAD_ONLY is set, then this argument is ignored.
     *                      Note:  DON'T SET THIS TO FLAG TRUE UNLESS YOU REALLY KNOW WHAT YOU ARE DOING!
     *                             If you're not sure, leave it set to false.  Most people won't need it,
     *                             and leaving it false won't cause any problems.
     */
   ThreadSetupSystem(bool muscleSingleThreadOnly = false);

   /** Destructor.  If MUSCLE_USE_PTHREADS is defined,
     * this will destroy the mutexes that were set up
     * in the constructor; otherwise it's a no-op
     */
   virtual ~ThreadSetupSystem();

private:
   friend class AtomicCounter;

private:
   Mutex _lock;  // Returned by GetGlobalMuscleLock()
};

#if defined(MUSCLE_USE_MUTEXES_FOR_ATOMIC_OPERATIONS)
/** Used by AtomicCounter to get (rather inefficient)
  * atomic counting via a small number of static mutexes.
  * @param count The value to adjust atomically 
  * @param delta The amount to add/subtract to/from (*count)
  * @returns the new state of (*count)
  */
int32 DoMutexAtomicIncrement(int32 * count, int32 delta);
#endif

/** This SetupSystem handles initializing the environment's TCP stack */
class NetworkSetupSystem : public SetupSystem
{
public:
   /** Constructor.  Under Windows, this calls WSAStartup()
     * to initialize the TCP stack.  Under all other OS's,
     * it calls signal(SIGPIPE, SIG_IGN) so that we won't
     * get signalled and killed if a remote client closes
     * his connection while we are sending to him.
     */
   NetworkSetupSystem(); 

   /** Destructor.  Under Windows, this calls WSACleanup();
     * it's a no-op for everyone else.
     */
   virtual ~NetworkSetupSystem();
};

/** This SetupSystem handles initializing the system's 
  * math routines as necessary.
  */
class MathSetupSystem : public SetupSystem
{
public:
   /** Constructor.  Under Borland C++, this constructor
     * disables floating point exceptions so that if they
     * occur, they won't crash the program.  It's a no-op
     * for all other environments.
     */
   MathSetupSystem();

   /** Destructor.  A no-op.  */
   virtual ~MathSetupSystem();
};

/** This SetupSystem just does some basic sanity checks
  * to ensure that the code was compiled in a way that
  * has some chance of working (e.g. it makes sure that
  * sizeof(uint32)==4, etc)
  */
class SanitySetupSystem : public SetupSystem
{
public:
   /** Constructor.  Under Borland C++, this constructor
     * disables floating point exceptions so that if they
     * occur, they won't crash the program.  It's a no-op
     * for all other environments.
     */
   SanitySetupSystem();

   /** Destructor.  A no-op.  */
   virtual ~SanitySetupSystem();
};

/** This class is a global setup/tear-down class;
  * It contains one member variable of each of the
  * other SetupSystem classes, so that when you instantiate
  * one of these objects, all the systems MUSCLE uses
  * get set up.  It's recommended that you put one of
  * these guys on the stack right at the beginning of
  * main(), to ensure that everything gets initialized
  * correctly... or if you for some reason don't want
  * to initialize all subsystems, you can still put
  * individual smaller SetupSystem objects on the
  * stack, instead.  Your choice.
  */
class CompleteSetupSystem : public SetupSystem
{
public:
   /** Constructor.  No-op, all the other *SetupSystem objects are created at this point.
     * @param muscleSingleThreadOnly Passed to the ThreadSetupSystem constructor.  
     *                      See the ThreadSetupSystem documentation for details.
     *                      (If you don't know what this flag is, leave it set to false!)
     */
   CompleteSetupSystem(bool muscleSingleThreadOnly = false);

   /** Destructor.  Calls the Callback() method of any items that were previously added
     * to our cleanup-callbacks list (in the opposite order from how they were added), then
     * calls AbstractObjectRecycler::GlobalFlushAllCachedObjects() to ensure
     * that objects don't get recycled after their object pools have been deleted.
     */
   ~CompleteSetupSystem();

   /** Returns a reference to a list of cleanup callback items that will be called by
     * the CompleteSetupSystem destructor.  You can add items to this list if you want
     * things to happen on program exit.
     */
   Queue<GenericCallbackRef> & GetCleanupCallbacks() {return _cleanupCallbacks;}

   /** As above, but read-only.  */
   const Queue<GenericCallbackRef> & GetCleanupCallbacks() const {return _cleanupCallbacks;}

   /** If there are any CompleteSetupSystems anywhere on the stack, this method will
     * return a pointer to the current (most recently created) CompleteSetupSystem object. 
     * Otherwise, returns NULL.
     */ 
   static CompleteSetupSystem * GetCurrentCompleteSetupSystem();

private: 
   NetworkSetupSystem _network;
   ThreadSetupSystem  _threads;
   MathSetupSystem    _math;
   SanitySetupSystem  _sanity;
   Queue<GenericCallbackRef> _cleanupCallbacks;
   CompleteSetupSystem * _prevInstance;  // stack (via linked list) so that nested scopes are handled appropriately
};

/** Returns a pointer to a process-wide Mutex, or NULL if that Mutex
 *  hasn't been allocated (by a ThreadSetupSystem or CompleteSetupSystem
 *  object) yet.
 */
Mutex * GetGlobalMuscleLock();

/** Returns true iff the current thread is the process's main thread (i.e. the flow
  * of execution that started at main() and placed a SetupSystem object on the stack).
  * If MUSCLE_SINGLE_THREAD_ONLY is defined, then this function always returns true.
  */
bool IsCurrentThreadMainThread();

}; // end namespace muscle

#endif
