/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

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
    *  @param argc for the child process.
    *  @param argv for the child process.
    *  @param blocking If true, I/O will be blocking; else non-blocking.
    */
   ChildProcessDataIO(int argc, char ** argv, bool blocking);

   /** Constructor.
    *  @param Command line for the child process (including any arguments)
    *  @param blocking If true, I/O will be blocking; else non-blocking.
    */
   ChildProcessDataIO(const char * cmdline, bool blocking);

   /** Destructor */
   virtual ~ChildProcessDataIO();
   
   virtual int32 Read(void * buffer, uint32 size);

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
    *  to read or write to/from the serial port, call Read() and Write() instead.
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

private:
   void Close();
   status_t LaunchChildProcess(int argc, const void * args);

   bool _blocking;
   bool _killChildOnClose;
   bool _waitForChildOnClose;

#if defined(WIN32) || defined(CYGWIN)
   void DoWindowsInit();
   void IOThreadEntry();
   void IOThreadAbort();
   static DWORD WINAPI IOThreadEntryFunc(LPVOID This) {((ChildProcessDataIO*)This)->IOThreadEntry(); return 0;}
   HANDLE _readFromStdout;
   HANDLE _writeToStdin;
   HANDLE _ioThread;
   HANDLE _wakeupSignal;
   HANDLE _childProcess;
   HANDLE _childThread;
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
