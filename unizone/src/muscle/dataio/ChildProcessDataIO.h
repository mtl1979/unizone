/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef ChildProcessDataIO_h
#define ChildProcessDataIO_h

#include <errno.h>
#include "dataio/DataIO.h"
#include "util/Queue.h"

BEGIN_NAMESPACE(muscle);

/** This DataIO class is a handy cross-platform way to spawn 
 *  and talk to a child process.  Any data that the child process
 *  prints to stdout can be read from this object, and any data
 *  that is written to this object will be send to the child process's
 *  stdin.  Note that this class is currently only guaranteed to work 
 *  under Windows, MacOS/X, BeOS, and Linux.
 */
class ChildProcessDataIO : public DataIO
{
public:
   /** Constructor.
    *  @param blocking If true, I/O will be blocking; else non-blocking.
    *  @note that you will need to call LaunchChildProcess() to actually start the child process going.
    */
   ChildProcessDataIO(bool blocking);

   /** Destructor */
   virtual ~ChildProcessDataIO();
   
   /** Launch the child process.  Note that this method should only be called once!
     * @param argc The argc variable to be passed to the child process
     * @param argv The argv variable to be passed to the child process
     * @return B_NO_ERROR on success, or B_ERROR if the launch failed.
     */
   status_t LaunchChildProcess(int argc, char ** argv) {return LaunchChildProcessAux(muscleMax(0,argc), argv);}

   /** As above, but the program name and all arguments are specified as a single string.
     * @param cmdline String to launch the child process with
     * @return B_NO_ERROR on success, or B_ERROR if the launch failed.
     */
   status_t LaunchChildProcess(const char * cmd) {return LaunchChildProcessAux(-1, cmd);}

   /** Read data from the child process's stdout stream. 
     * @param buffer The read bytes will be placed here
     * @param size Maximum number of bytes that may be placed into (buffer).
     * @returns The number of bytes placed into (buffer), or a negative value if there was an error.
     */
   virtual int32 Read(void * buffer, uint32 size);

   /** Write data to the child process's stdin stream. 
     * @param buffer The bytes to write to the child process's stdin.
     * @param size Maximum number of bytes to read from (buffer) and written to the child process's stdin.
     * @returns The number of bytes written, or a negative value if there was an error.
     */
   virtual int32 Write(const void * buffer, uint32 size);

   /** Always returns B_ERROR, since you can't seek on a child process! */
   virtual status_t Seek(int64, int) {return B_ERROR;}

   /** Always returns -1, since a child process has no position to speak of */
   virtual int64 GetPosition() const {return -1;}

   /** Doesn't return until all outgoing have been sent */
   virtual void FlushOutput();

   /** Kills the child process */
   virtual void Shutdown();

   /** Returns a socket that can be select()'d on for notifications of read/write availability.
    *  Even works under Windows (in non-blocking mode, anyway), despite Microsoft's best efforts 
    *  to make such a thing impossible :^P Note that you should only use this socket with select(); 
    *  to read or write to/from the child process, call Read() and Write() instead.
    */
   virtual int GetSelectSocket() const;

   /** Returns true iff the child process is available (i.e. if startup succeeded). */
   bool IsChildProcessAvailable() const;

   /** Set whether or not we should forcibly kill the child process
     * when this DataIO object is shut down or deleted.  Default value 
     * is true.
     */
   void SetKillChildOnClose(bool kcoc) {_killChildOnClose = kcoc;}
   bool GetKillChildOnClose() const {return _killChildOnClose;}

   /** Set whether or not we should block (waiting until the child process
     * is gone) inside this DataIO object's destructor or Shutdown()
     * call.  Default value is true.
     */
   void SetWaitForChildOnClose(bool wcoc) {_waitForChildOnClose = wcoc;}
   bool GetWaitForChildOnClose() const {return _waitForChildOnClose;}

   /** Called within the child process, just before the child process's
     * executable image is loaded in.  Default implementation is a no-op.
     * @note This method is not called when running under Windows!
     */
   virtual void ChildProcessReadyToRun();

private:
   void Close();
   status_t LaunchChildProcessAux(int argc, const void * argv);

   bool _blocking;
   bool _killChildOnClose;
   bool _waitForChildOnClose;

#if defined(WIN32) || defined(CYGWIN)
   void IOThreadEntry();
   void IOThreadAbort();
   static DWORD WINAPI IOThreadEntryFunc(LPVOID This) {((ChildProcessDataIO*)This)->IOThreadEntry(); return 0;}
   ::HANDLE _readFromStdout;
   ::HANDLE _writeToStdin;
   ::HANDLE _ioThread;
   ::HANDLE _wakeupSignal;
   ::HANDLE _childProcess;
   ::HANDLE _childThread;
   int _masterNotifySocket;
   int _slaveNotifySocket;
   volatile bool _requestThreadExit;
#else
   int _handle;
   pid_t _childPID;
#endif
};

END_NAMESPACE(muscle);

#endif
