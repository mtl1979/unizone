/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MusclePulseNode_h
#define MusclePulseNode_h

#include "util/TimeUtilityFunctions.h"
#include "util/Hashtable.h"

namespace muscle {

class PulseNode;
class PulseNodeManager;

/** Interface class for any object that can schedule Pulse() calls for itself via the ReflectServer. */
class PulseNode
{
public:
   /** Default constructor */
   PulseNode();

   /** Destructor.  Does not delete any attached child pulsables. */
   virtual ~PulseNode();

   /**
    * This method can be overridden to tell the PulseNodeManager when we would like
    * to have our Pulse() method called.  This method is guaranteed to be called only
    * during the following times:
    * <ol>
    *   <li>When the PulseNode is first probed by the PulseNodeManager</li>
    *   <li>Immediately after our Pulse() method has returned</li>
    *   <li>Soon after InvalidatePulseTime() has been called one or more times 
    *       (InvalidatePulseTime() calls are merged together for efficiency)</li>
    * </ol>
    * @param now The current wall-clock time in microseconds, for convenience.
    * @param prevResult The value that this method returned the last time it was
    *                   called.  The very first time this method is called, this value
    *                   will be passed in as MUSCLE_TIME_NEVER.
    * @return Return MUSCLE_TIME_NEVER if you don't wish to schedule a future call to Pulse();
    *         or return the time at which you want Pulse() to be called.  Returning values less
    *         than or equal to (now) will cause Pulse() to be called as soon as possible.
    *         The default implementation always returns MUSCLE_TIME_NEVER.
    */
   virtual uint64 GetPulseTime(uint64 now, uint64 prevResult);

   /**
    * Will be called at the time specified previously by GetPulseTime().  GetPulseTime()
    * will be called again immediately after this call, to check if you want to schedule
    * another Pulse() call for later.
    * Default implementation is a no-op.
    * @param now The current wall-clock time-value in microseconds, for convenience.
    * @param scheduledTime The time this Pulse() call was scheduled to occur at, in
    *                      microseconds, as previously returned by GetPulseTime(). Note
    *                      that unless your computer is infinitely fast, this time will
    *                      always be at least a bit less than (now), since there is a delay 
    *                      between when the program gets woken up to service the next Pulse() 
    *                      call, and when the call actually happens.  (you may be able to 
    *                      use this value to compensate for the slippage, if it bothers you)
    */
   virtual void Pulse(uint64 now, uint64 scheduledTime);

   /** 
    *  Adds the given child into our set of child PulseNodes.  Any PulseNode in our
    *  set of children will have its pulsing needs taken care of by us, but it is
    *  not considered "owned" by this PulseNode--it will not be deleted when we are.
    *  @param child The child to place into our set of child pulsables.
    *  @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory).
    */
   status_t PutPulseChild(PulseNode * child);

   /** Attempts to remove the given child from our set of child PulseNodes.
    *  @param cihld The child to remove
    *  @param B_NO_ERROR on success, or B_ERROR on failure (child wasn't in our set)
    */
   status_t RemovePulseChild(PulseNode * child);

   /** Removes all children from our set of child PulseNodes */
   void ClearPulseChildren();

   /** Returns true iff the given child is in our set of child PulseNodes. 
     * @param child the child to look for.
     */
   bool ContainsPulseChild(PulseNode * child) const {return _children.ContainsKey(child);}

   /** Returns an iterator that can be used to iterate over our set of child PulseNodes. */
   HashtableIterator<PulseNode *, bool> GetPulseChildrenIterator() const {return _children.GetIterator();}

   /** Returns when this object wants its call to Pulse() scheduled next, or MUSCLE_TIME_NEVER 
    *  if it has no call to Pulse() currently scheduled. 
    */
   uint64 GetScheduledPulseTime() const {return _localPulseAt;}

protected:
   /**
    * Sets a flag to indicate that GetPulseTime() to be called on this object.
    * Call this whenever you've decided to reschedule your pulse time outside
    * of a Pulse() event.
    * @param clearPrevResult if true, this call will also clear the stored prevResult
    *                        value, so that the next time GetPulseTime() is called,
    *                        prevResult is passed in as MUSCLE_TIME_NEVER.  If false,
    *                        the prevResult value will be left alone.
    */
   void InvalidatePulseTime(bool clearPrevResult = true); 

private:
   // Called by a PulseNodeManager let us update our pulse times, as well as his (managerPulseTime)
   void GetPulseTimeAux(uint64 now, uint64 & managerPulseTime);

   // Called by a PulseNodeManager to tell us to call Pulse() on ourself and our children, as necessary.
   void PulseAux(uint64 now);

   // Invalidates our cached cumulative pulse time, and recurses to our parent (if any).
   void InvalidateGroupPulseTime();

   PulseNode * _parent;

   bool _nextPulseAtValid;
   uint64 _nextPulseAt;     // when we want to our next PulseAux() call

   bool _localPulseAtValid;
   uint64 _localPulseAt;    // when this object wants its next Pulse() call

   Hashtable<PulseNode *, bool> _children;

   friend class PulseNodeManager;
};

/** Subclasses of this class are allowed to manage PulseNode objects by 
  * calling their GetPulseTimeAux() and PulseAux() methods (indirectly).  
  * Most code won't need to use this class.
  */
class PulseNodeManager
{
public:
   /** Default constructor */
   PulseNodeManager() {/* empty */}

   /** Destructor */
   ~PulseNodeManager() {/* empty */}

protected:
   /** Passes the call on through to the given PulseNode */
   inline void CallGetPulseTimeAux(PulseNode & p, uint64 now, uint64 & spt) {p.GetPulseTimeAux(now, spt);}

   /** Passes the call on through to the given PulseNode */
   inline void CallPulseAux(PulseNode & p, uint64 now) {p.PulseAux(now);}
};

};  // end namespace muscle

#endif
