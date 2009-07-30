/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleTimeUtilityFunctions_h
#define MuscleTimeUtilityFunctions_h

#include <time.h>

#include "support/MuscleSupport.h"

#ifndef WIN32
# include <sys/time.h>
#endif

#if defined(__BEOS__) || defined(__HAIKU__)
# include <kernel/OS.h>
#endif

#if defined(TARGET_PLATFORM_XENOMAI)
# include "native/timer.h"
#endif

namespace muscle {

/** A value that GetPulseTime() can return to indicate that Pulse() should never be called. */
#define MUSCLE_TIME_NEVER ((uint64)-1) // (9223372036854775807LL)

/** Given a timeval struct, returns the equivalent uint64 value (in microseconds).
 *  @param tv a timeval to convert
 *  @return A uint64 representing the same time.
 */
inline uint64 ConvertTimeValTo64(const struct timeval & tv)
{
   return (((uint64)tv.tv_sec)*1000000) + ((uint64)tv.tv_usec);
}

/** Given a uint64, writes the equivalent timeval struct into the second argument.
 *  @param val A uint64 time value in microseconds
 *  @param retStruct On return, contains the equivalent timeval struct to (val)
 */
inline void Convert64ToTimeVal(uint64 val, struct timeval & retStruct)
{
   retStruct.tv_sec  = (int32)(val / 1000000);
   retStruct.tv_usec = (int32)(val % 1000000);
}

/** Convenience function:  Returns true true iff (t1 < t2)
 *  @param t1 a time value
 *  @param t2 another time value
 *  @result true iff t1 < t2
 */
inline bool IsLessThan(const struct timeval & t1, const struct timeval & t2)
{
   return (t1.tv_sec == t2.tv_sec) ? (t1.tv_usec < t2.tv_usec) : (t1.tv_sec < t2.tv_sec);
}

/** Convenience function: Adds (addThis) to (addToThis)
 *  @param addToThis A time value.  On return, this time value will have been modified.
 *  @param addThis Another time value, that will be added to (addToThis).
 */
inline void AddTimeVal(struct timeval & addToThis, const struct timeval & addThis)
{
   addToThis.tv_sec  += addThis.tv_sec;
   addToThis.tv_usec += addThis.tv_usec;
   if (addToThis.tv_usec > 1000000L)
   {
      addToThis.tv_sec += addToThis.tv_usec / 1000000L;
      addToThis.tv_usec = addToThis.tv_usec % 1000000L;
   }
}

/** Convenience function: Subtracts (subtractThis) from (subtractFromThis)
 *  @param subtractFromThis A time value.  On return, this time value will have been modified.
 *  @param subtractThis Another time value, that will be subtracted from (subtractFromThis).
 */ 
inline void SubtractTimeVal(struct timeval & subtractFromThis, const struct timeval & subtractThis)
{
   subtractFromThis.tv_sec  -= subtractThis.tv_sec;
   subtractFromThis.tv_usec -= subtractThis.tv_usec;
   if (subtractFromThis.tv_usec < 0L)
   {
      subtractFromThis.tv_sec += (subtractFromThis.tv_usec / 1000000L)-1;
      while(subtractFromThis.tv_usec < 0L) subtractFromThis.tv_usec += 1000000L;
   }
}

/** timestamp types used by GetCurrentTime64(), etc */
enum {
   MUSCLE_TIMEZONE_UTC = 0, // Universal Co-ordinated Time (formerly Greenwhich Mean Time)
   MUSCLE_TIMEZONE_LOCAL    // Host machine's local time (depends on local time zone settings)
};

/** Returns the current real-time clock time as a uint64.  The returned value is expressed
 *  as microseconds since the beginning of the year 1970.
 *  @param timeType if left as MUSCLE_TIMEZONE_UTC (the default), the value returned will be the current UTC time.  
 *                  If set to MUSCLE_TIMEZONE_LOCAL, on the other hand, the returned value will include the proper 
 *                  offset to make the time be the current time as expressed in this machine's local time zone.
 *                  For any other value, the behaviour of this function is undefined.
 *  @note that the values returned by this function are NOT guaranteed to be monotonically increasing!
 *        Events like leap seconds, the user changing the system time, or even the OS tweaking the system
 *        time automatically to eliminate clock drift may cause this value to decrease occasionally!
 *        If you need a time value that is guaranteed to never decrease, you may want to call GetRunTime64() instead.
 */
uint64 GetCurrentTime64(uint32 timeType=MUSCLE_TIMEZONE_UTC);

/** Returns a current time value, in microseconds, that is guaranteed never to decrease.  The absolute
 *  values returned by this call are undefined, so you should only use it for measuring relative time
 *  (i.e. how much time has passed between two events).  For a "wall clock" type of result with
 *  a well-defined time-base, you can call GetCurrentTime64() instead.
 */
#if defined(__BEOS__) || defined(__HAIKU__)
inline uint64 GetRunTime64() {return system_time();}
#elif defined(TARGET_PLATFORM_XENOMAI)
inline uint64 GetRunTime64() {return rt_timer_ticks2ns(rt_timer_read())/1000;}
#else
uint64 GetRunTime64();
#endif

/** Convenience function:  Won't return for a given number of microsends.
 *  @param micros The number of microseconds to wait for.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
#if defined(__BEOS__) || defined(__HAIKU__)
inline status_t Snooze64(uint64 microseconds) {return snooze(microseconds);}
#else
status_t Snooze64(uint64 microseconds);
#endif

/** Convenience function:  Returns true no more often than once every (interval).
 *  Useful if you are in a tight loop, but don't want e.g. more than one debug output line per second, or something like that.
 *  @param interval The minimum time that must elapse between two calls to this function returning true.
 *  @param lastTime A state variable.  Pass in the same reference every time you call this function.
 *  @return true iff it has been at least (interval) since the last time this function returned true, else false.
 */
inline bool OnceEvery(const struct timeval & interval, struct timeval & lastTime)
{
   uint64 now64 = GetRunTime64();
   struct timeval now; 
   Convert64ToTimeVal(now64, now);
   if (!IsLessThan(now, lastTime))
   {
      lastTime = now;
      AddTimeVal(lastTime, interval);
      return true;
   }
   return false;
}

/** Convenience function:  Returns true no more than once every (interval).
 *  Useful if you are in a tight loop, but don't want e.g. more than one debug output line per second, or something like that.
 *  @param interval The minimum time that must elapse between two calls to this function returning true (in microseconds).
 *  @param lastTime A state variable.  Pass in the same reference every time you call this function.  (set to zero the first time)
 *  @return true iff it has been at least (interval) since the last time this function returned true, else false.
 */
inline bool OnceEvery(uint64 interval, uint64 & lastTime)
{
   uint64 now = GetRunTime64();
   if (now >= lastTime+interval)
   {
      lastTime = now;
      return true;
   }
   return false;
}

/** This handy macro will print out, twice a second, 
 *  the average number of times per second it is being called.
 */
#define PRINT_CALLS_PER_SECOND(x) \
{ \
   static uint32 count = 0; \
   static uint64 startTime = 0; \
   uint64 lastTime = 0; \
   uint64 now = GetCurrentTime64(); \
   if (startTime == 0) startTime = now; \
   count++; \
   if ((OnceEvery(500000, lastTime))&&(now>startTime)) printf("%s: " UINT64_FORMAT_SPEC "/s\n", x, (1000000*((uint64)count))/(now-startTime)); \
}

}; // end namespace muscle

#endif
