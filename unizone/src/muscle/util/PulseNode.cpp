/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "util/PulseNode.h"

namespace muscle {

PulseNode :: PulseNode() : _parent(NULL), _nextPulseAtValid(false), _nextPulseAt(MUSCLE_TIME_NEVER), _localPulseAtValid(false), _localPulseAt(MUSCLE_TIME_NEVER), _cycleStartedAt(0), _maxTimeSlice(MUSCLE_TIME_NEVER), _timeSlicingSuggested(false)
{
   // empty
}

PulseNode :: ~PulseNode() 
{
   // unlink everybody, but don't delete anyone; no ownership is implied here!
   if (_parent) _parent->RemovePulseChild(this);
   ClearPulseChildren();
}

uint64 PulseNode :: GetPulseTime(uint64, uint64) 
{
   return MUSCLE_TIME_NEVER;
}

void PulseNode ::Pulse(uint64, uint64) 
{
   // empty
}

void PulseNode :: InvalidateGroupPulseTime()
{
   if (_nextPulseAtValid)
   {
      _nextPulseAtValid = false;
      if (_parent) _parent->InvalidateGroupPulseTime(); // tell our parent one of his kids has changed
   }
}

void PulseNode :: InvalidatePulseTime(bool clearPrevResult)
{
   if (_localPulseAtValid) 
   {
      _localPulseAtValid = false;
      if (clearPrevResult) _localPulseAt = MUSCLE_TIME_NEVER;
      InvalidateGroupPulseTime();
   }
}

void PulseNode :: GetPulseTimeAux(uint64 now, uint64 & minPulseTime)
{
   if (_localPulseAtValid == false)
   {
      _localPulseAt = GetPulseTime(now, _localPulseAt);
      _localPulseAtValid = true;
   }
   if (_nextPulseAtValid == false)
   {
      _nextPulseAt = _localPulseAt;
      if (_children.GetNumItems() > 0)
      {
         HashtableIterator<PulseNode *, bool> iter = _children.GetIterator();
         PulseNode * nextKey;
         while(iter.GetNextKey(nextKey) == B_NO_ERROR) nextKey->GetPulseTimeAux(now, _nextPulseAt);
      }
      _nextPulseAtValid = true;
   }
   if (minPulseTime > _nextPulseAt) minPulseTime = _nextPulseAt;
}

void PulseNode :: PulseAux(uint64 now)
{
   if (now >= _nextPulseAt)
   {
      if (now >= _localPulseAt)
      {
         Pulse(now, _localPulseAt);
         InvalidatePulseTime(false);
      }
      if (_children.GetNumItems() > 0)
      {
         HashtableIterator<PulseNode *, bool> iter = _children.GetIterator();
         PulseNode * nextKey;
         while(iter.GetNextKey(nextKey) == B_NO_ERROR) nextKey->PulseAux(now);
      }
   }
}

status_t PulseNode :: PutPulseChild(PulseNode * child)
{
   MASSERT((child->_parent == NULL), "PulseNode::PutPulseChild(): new child already has a parent!\n");
   if (_children.Put(child, true) != B_NO_ERROR) return B_ERROR;
   InvalidateGroupPulseTime();  // new child may change our timing!
   child->_parent = this;
   return B_NO_ERROR;
}

status_t PulseNode :: RemovePulseChild(PulseNode * child)
{
   if (_children.Remove(child) == B_NO_ERROR)
   {
      InvalidateGroupPulseTime();  // lack of a child may change our timing!
      child->_parent = NULL;
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

void PulseNode :: ClearPulseChildren()
{
   if (_children.GetNumItems() > 0) 
   {
      InvalidateGroupPulseTime();  // lack of children may change our timing!
      HashtableIterator<PulseNode *, bool> iter = _children.GetIterator();
      PulseNode * next;
      while(iter.GetNextKey(next) == B_NO_ERROR) next->_parent = NULL;  // don't delete the kids!
      _children.Clear();
   }
}

};  // end namespace muscle
