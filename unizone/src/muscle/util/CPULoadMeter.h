#include "support/MuscleSupport.h"

BEGIN_NAMESPACE(muscle);

/** This class knows how to measure the total load on the host computer's CPU.
  * Note that the internal implementation of this class is OS-specific and so
  * it will not work properly on all OS's.  
  *
  * To use this class, just instantiate a CPULoadMeter object, and then call
  * GetCPULoad() every so often (i.e. whenever you want to update your CPU
  * load statistic)
  */
class CPULoadMeter
{
public:
   /** Default constructor */
   CPULoadMeter();

   /** Destructor. */
   ~CPULoadMeter();

   /** Returns the percentage CPU load, measured since the last time this method was called.
     * @note Currently this method is implemented only for Linux, OS/X, and Windows.
     *       For other operating systems, this method will always return a negative value.
     * @returns 0.0f if the CPU was idle, 1.0f if the CPU was fully loaded, or something
     *          in between.  Returns a negative value if the CPU time could not be measured.
     */
   float GetCPULoad();

private:
   float CalculateCPULoad(uint64 idleTicks, uint64 totalTicks); 

   uint64 _previousTotalTicks;
   uint64 _previousIdleTicks;

#ifdef WIN32
   typedef WINBASEAPI BOOL WINAPI (*GetSystemTimesProc) (OUT LPFILETIME t1, OUT LPFILETIME t2, OUT LPFILETIME t3);
   GetSystemTimesProc _getSystemTimesProc;
   HMODULE _winKernelLib;
#endif
};

END_NAMESPACE(muscle);
