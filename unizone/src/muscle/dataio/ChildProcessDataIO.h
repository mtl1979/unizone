/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef ChildProcessDataIO_h
#define ChildProcessDataIO_h

#include <errno.h>
#include "dataio/DataIO.h"
#include "util/Queue.h"

namespace muscle {

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
     * @param usePty If true (the default), ChildProcessDataIO will try to launch the child process using a pseudo-terminal (via forkpty()).
     *               If specified as false, ChildProcessDataIO will use good old fashioned fork() instead.
     *               Pty's allow for better control of interactive child processes, but are not always well supported on all platforms.
     *               Note that this argument is ignored when running under Windows.
     * @return B_NO_ERROR on success, or B_ERROR if the launch failed.
     */
   status_t LaunchChildProcess(int argc, const char * argv[], bool usePty = true) {return LaunchChildProcessAux(muscleMax(0,argc), argv, usePty);}

   /** As above, but the program name and all arguments are specified as a single string.
     * @param cmd String to launch the child process with
     * @param usePty If true (the default), ChildProcessDataIO will try to launch the child process using a pseudo-terminal (via forkpty()).
     *               If specified as false, ChildProcessDataIO will use good old fashioned fork() instead.
     *               Pty's allow for better control of interactive child processes, but are not always well supported on all platforms.
     *               Note that this argument is ignored when running under Windows.
     * @return B_NO_ERROR on success, or B_ERROR if the launch failed.
     */
   status_t LaunchChildProcess(const char * cmd, bool usePty = true) {return LaunchChildProcessAux(-1, cmd, usePty);}

   /** Convenience method.  Launches a child process using an (argc,argv) that is constructed from the passed in argument list.
     * @param argv A list of strings to construct the (argc,argv) from.  The first string should be the executable name, the second string
     *             should be the first argument to the executable, and so on.
     * @param usePty If true (the default), ChildProcessDataIO will try to launch the child process using a pseudo-terminal (via forkpty()).
     *               If specified as false, ChildProcessDataIO will use good old fashioned fork() instead.
     *               Pty's allow for better control of interactive child processes, but are not always well supported on all platforms.
     *               Note that this argument is ignored when running under Windows.
     * @return B_NO_ERROR on success, or B_ERROR if the launch failed.
     */
   status_t LaunchChildProcess(const Queue<String> & argv, bool usePty = true);

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

   /** Kills the child process, using the sequence described at SetChildProcessShutdownBehavior(). */
   virtual void Shutdown();

   /** Returns a socket that can be select()'d on for notifications of read/write availability.
    *  Even works under Windows (in non-blocking mode, anyway), despite Microsoft's best efforts 
    *  to make such a thing impossible :^P Note that you should only pass this socket to select(); 
    *  to read or write to/from the child process, call Read() and Write() on this object but don't
    *  try to recv()/send()/etc on this socket directly!
    */
   virtual const ConstSocketRef & GetSelectSocket() const;

   /** Returns true iff the child process is available (i.e. if startup succeeded). */
   bool IsChildProcessAvailable() const;

   /** Specify the series of actions we should take to gracefully shutdown the child process 
     * when this object is closed.  
     *
     * The shutdown sequence we use is the following:
     *   1) Close the socket to the child process
     *   2) (optional) send a signal to the child process (note:  this step is not available under Windows)
     *   3) (optional) wait up to a specified amount of time for the child process to exit voluntarily
     *   4) (optional) forcibly kill the child process if it hasn't exited by this time
     *
     * @param okayToKillChild If true, we are allowed to violently terminate the child if he still 
     *                        hasn't exited by the time we are done waiting for him to exit.  If false,
     *                        we will just let the child continue running if he feels he must.
     * @param sendSignalNumber If non-negative, we will prompt the child process to exit by
     *                         sending him this signal before beginning to wait.  Note that
     *                         this argument is not used under Windows.
     *                         Defaults to -1.
     * @param maxWaitTimeMicros If specified, this is the maximum amount of time (in microseconds) 
     *                          that we should wait for the child process to exit before continuing.
     *                          Defaults to MUSCLE_TIME_NEVER, meaning that we will wait indefinitely
     *                          for the child to exit, if necessary.
     *
     * Note that if this method is never called, then the default behavior is to immediately 
     * kill the child (with no signal sent and no wait time elapsed).
     */
   void SetChildProcessShutdownBehavior(bool okayToKillChild, int sendSignalNumber = -1, uint64 maxWaitTimeMicros = MUSCLE_TIME_NEVER);

   /** Set whether or not the child process we spawn should inherit our
     * open file descriptors.  Default value is false.
     */
   void SetChildProcessInheritFileDescriptors(bool cpifds) {_childProcessInheritFileDescriptors = cpifds;}

   /** Returns true iff the child process we spawn will inherit our
     * open file descriptors.  Default value is false.
     */
   bool GetChildProcessInheritFileDescriptors() const {return _childProcessInheritFileDescriptors;}

   /** Called within the child process, just before the child process's
     * executable image is loaded in.  Default implementation is a no-op.
     * @note This method is not called when running under Windows!
     */
   virtual void ChildProcessReadyToRun();

   /** Returns the process ID of the child process.  Not available under Windows. */
#if defined(WIN32) || defined(CYGWIN)
   uint32 GetChildProcessID() const {return (uint32)GetProcessId(_childProcess);}
#else
   pid_t GetChildProcessID() const {return _childPID;}
#endif

   /** Tries to forcibly kill the child process immediately. 
     * @returns B_NO_ERROR on success, or B_ERROR on failure.
     */
   status_t KillChildProcess();

   /** Sends the specified signal to the child process.
     * Note that this method is not currently implemented under Windows, 
     * and thus under Windows this method is a no-op that just returns B_ERROR.
     * @param sigNum a signal number, e.g. SIGINT or SIGHUP.
     * @returns B_NO_ERROR on success, or B_ERROR on failure.
     */
   status_t SignalChildProcess(int sigNum);

   /** Will not return until our child process has exited.
     * If the child process is not currently running, returns immediately.
     * @param maxWaitTime The maximum amount of time to wait, in microseconds.
     *                    Defaults to MUSCLE_TIME_NEVER, indicating no timeout.
     * @returns true iff the child process is known to be gone, or false 
     *          otherwise (e.g. our timeout period elapsed and the child
     *          process still hadn't exited)
     */
   bool WaitForChildProcessToExit(uint64 maxWaitTime = MUSCLE_TIME_NEVER);

   /** Convenience method:  acts similar to the POSIX system() call, but
     * implemented internally via a ChildProcessDataIO object.  In particular,
     * this static method will launch the specified process and not return
     * until that process has completed.
     * @param argc Number of items in the (argv) array
     * @param argv A standard argv array for the child process to use
     * @param usePty If true (the default), ChildProcessDataIO will try to launch the child process using a pseudo-terminal (via forkpty()).
     * @returns B_NO_ERROR if the child process was launched, or B_ERROR
     *          if the child process could not be launched.
     */
   static status_t System(int argc, const char * argv[], bool usePty=true);

   /** Convenience method:  acts similar to the POSIX system() call, but
     * implemented internally via a ChildProcessDataIO object.  In particular,
     * this static method will launch the specified process and not return
     * until that process has completed.
     * @param argv A list of strings to construct the (argc,argv) from.  The first string should be the executable name, the second string
     *             should be the first argument to the executable, and so on.
     * @param usePty If true (the default), ChildProcessDataIO will try to launch the child process using a pseudo-terminal (via forkpty()).
     *               If specified as false, ChildProcessDataIO will use good old fashioned fork() instead.
     *               Pty's allow for better control of interactive child processes, but are not always well supported on all platforms.
     *               Note that this argument is ignored when running under Windows.
     * @return B_NO_ERROR on success, or B_ERROR if the launch failed.
     */
   status_t System(const Queue<String> & argv, bool usePty = true);

   /** Convenience method:  acts similar to the POSIX system() call, but
     * implemented internally via a ChildProcessDataIO object.  In particular,
     * this static method will launch the specified process and not return
     * until that process has completed.
     * @param cmdLine The command string to launch (as if typed into a shell)
     * @param usePty If true (the default), ChildProcessDataIO will try to launch the child process using a pseudo-terminal (via forkpty()).
     * @returns B_NO_ERROR if the child process was launched, or B_ERROR
     *          if the child process could not be launched.
     */
   static status_t System(const char * cmdLine, bool usePty=true);

private:
   void Close();
   status_t LaunchChildProcessAux(int argc, const void * argv, bool usePty);
   void DoGracefulChildShutdown();

   bool _blocking;

   bool _killChildOkay;
   uint64 _maxChildWaitTime;
   int _signalNumber;

   bool _childProcessInheritFileDescriptors;

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
   ConstSocketRef _masterNotifySocket;
   ConstSocketRef _slaveNotifySocket;
   volatile bool _requestThreadExit;
#else
   void RunChildProcess(int argc, const void * args);
   ConstSocketRef _handle;
   pid_t _childPID;
#endif
};

}; // end namespace muscle

#endif
