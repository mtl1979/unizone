/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleSetupSystem_h
#define MuscleSetupSystem_h

#include "system/AtomicCounter.h"
#include "system/Mutex.h"

BEGIN_NAMESPACE(muscle);

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
     */
   ThreadSetupSystem();

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
   /** Constructor.  No-op, but at this point both
     * a NetworkSetupSystem and ThreadSetupSystem
     * will be created. 
     */
   CompleteSetupSystem() {/* empty */}

   /** Destructor.  All held subsystems get destroyed here. */
   ~CompleteSetupSystem() {/* empty */}

private: 
   NetworkSetupSystem _network;
   ThreadSetupSystem  _threads;
   MathSetupSystem    _math;
};

/** Returns a pointer to a process-wide Mutex, or NULL if that Mutex
 *  hasn't been allocated (by a ThreadSetupSystem or CompleteSetupSystem
 *  object) yet.
 */
Mutex * GetGlobalMuscleLock();

END_NAMESPACE(muscle);

#endif
