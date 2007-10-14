/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "util/DebugTimer.h"
#include "util/TimeUtilityFunctions.h"

BEGIN_NAMESPACE(muscle);

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
      HashtableIterator<uint32, uint64> iter(_modeToElapsedTime);
      uint32 nextMode;
      uint64 nextTime;
      while(iter.GetNextKeyAndValue(nextMode, nextTime) == B_NO_ERROR)
      {
         if (nextTime >= _minLogTime)
         {
            if (nextTime >= 1000) LogTime(_debugLevel, "%s: mode "UINT32_FORMAT_SPEC": " UINT64_FORMAT_SPEC " milliseconds elapsed\n", _title(), nextMode, nextTime/1000);
                             else LogTime(_debugLevel, "%s: mode "UINT32_FORMAT_SPEC": " UINT64_FORMAT_SPEC " microseconds elapsed\n", _title(), nextMode, nextTime);
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

END_NAMESPACE(muscle);
