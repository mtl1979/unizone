/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "dataio/StdinDataIO.h"

#if defined(WIN32) || defined(CYGWIN)
# define USE_WIN32_STDINDATAIO_IMPLEMENTATION
# include <process.h>  // for _beginthreadex()
# include "system/SetupSystem.h"  // for GetGlobalMuscleLock()
#endif

namespace muscle {

#ifdef USE_WIN32_STDINDATAIO_IMPLEMENTATION
static uint32 _instanceCount = 0;
static DWORD _oldConsoleMode = 0;
#else
static Socket _stdinSocket(STDIN_FILENO, false);  // we generally don't want to close stdin
#endif

StdinDataIO :: StdinDataIO(bool blocking) : _stdinBlocking(blocking)
#ifdef USE_WIN32_STDINDATAIO_IMPLEMENTATION
 , _ioThread(INVALID_HANDLE_VALUE), _stdinHandle(INVALID_HANDLE_VALUE)
#else
 , _fdIO(ConstSocketRef(&_stdinSocket, false), true)
#endif
{
#ifdef USE_WIN32_STDINDATAIO_IMPLEMENTATION
   // Make a duplicate of the stdin handle so that we own the handle
   // and can close it when necessary to force the I/O thread to go away
   if (DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_INPUT_HANDLE), GetCurrentProcess(), &_stdinHandle, 0, false, DUPLICATE_SAME_ACCESS))
   {
      Mutex * m = GetGlobalMuscleLock();
      if (m == NULL) MCRASH("Please put a CompleteSetupSystem object on the stack at the top of main().");
      if (m->Lock() == B_NO_ERROR)
      {
         if (++_instanceCount == 1) 
         {
            GetConsoleMode(_stdinHandle, &_oldConsoleMode);
            SetConsoleMode(_stdinHandle, _oldConsoleMode | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
         }
         m->Unlock();
      }
      if (_stdinBlocking == false)
      {
         // For non-blocking I/O, we need to handle stdin in a separate thread. 
         // note that I freopen stdin to "nul" so that other code (read: Python)
         // won't try to muck about with stdin and interfere with StdinDataIO's
         // operation.  I don't know of any good way to restore it again after,
         // though... so a side effect of StdinDataIO under Windows is that
         // stdin gets redirected to nul. 
         DWORD junkThreadID;
         typedef unsigned (__stdcall *PTHREAD_START) (void *);
         if ((CreateConnectedSocketPair(_masterSocket, _slaveSocket, false) != B_NO_ERROR)||(SetSocketBlockingEnabled(_slaveSocket, true) != B_NO_ERROR)||(freopen("nul", "r", stdin) == NULL)||((_ioThread = (::HANDLE) _beginthreadex(NULL, 0, (PTHREAD_START)IOThreadEntryFunc, this, 0, (unsigned *) &junkThreadID)) == INVALID_HANDLE_VALUE))
         {
            LogTime(MUSCLE_LOG_ERROR, "StdinDataIO:  Error setting up I/O thread!\n");
            Close();
         }
      }
   }
   else LogTime(MUSCLE_LOG_ERROR, "StdinDataIO:  Error, couldn't duplicate stdin handle!\n");
#endif
}

StdinDataIO ::
~StdinDataIO() 
{
   Close();
}

void StdinDataIO :: Shutdown()
{
   Close();
}

void StdinDataIO :: Close()
{
#ifdef USE_WIN32_STDINDATAIO_IMPLEMENTATION
   _masterSocket.Reset();  // must do this first, in case I/O thread is blocked in SendData()

   HANDLE junkStdinHandleCopy = _stdinHandle;
   if (_stdinHandle != INVALID_HANDLE_VALUE) 
   {
      _stdinHandle = INVALID_HANDLE_VALUE;  // do this first, to avoid race!
      ::CloseHandle(junkStdinHandleCopy);  // should cause ReadFile() to exit with an error
   }

   // Wait until the I/O thread has exited, then close the thread handle too
   if (_ioThread != INVALID_HANDLE_VALUE)
   {
      WaitForSingleObject(_ioThread, INFINITE);
      ::CloseHandle(_ioThread);
   }

   if (junkStdinHandleCopy != INVALID_HANDLE_VALUE)
   {
      Mutex * m = GetGlobalMuscleLock();
      if (m == NULL) MCRASH("Please put a CompleteSetupSystem object on the staat the top of main().");
      if (m->Lock() == B_NO_ERROR)
      {
         // Restore the old console mode when the last StdinDataIO is gone
         if (--_instanceCount == 0) SetConsoleMode(_stdinHandle, _oldConsoleMode);
         m->Unlock();
      }
   }
#else
   _fdIO.Shutdown();
#endif
}

int32 StdinDataIO :: Read(void * buffer, uint32 size)
{
#ifdef USE_WIN32_STDINDATAIO_IMPLEMENTATION
        if (_stdinHandle == INVALID_HANDLE_VALUE) return -1;
   else if (_stdinBlocking)
   {
      DWORD actual_read;
      return ReadFile(_stdinHandle, buffer, size, &actual_read, 0) ? actual_read : -1;
   }
   else return ReceiveData(_masterSocket, buffer, size, _stdinBlocking);
#else
   // Turn off stdin's blocking I/O mode only during the Read() call.
   if (_stdinBlocking == false) (void) _fdIO.SetBlockingIOEnabled(false);
   int32 ret = _fdIO.Read(buffer, size);
   if (_stdinBlocking == false) (void) _fdIO.SetBlockingIOEnabled(true);
   return ret;
#endif
}

const ConstSocketRef & StdinDataIO :: GetSelectSocket() const
{
#ifdef USE_WIN32_STDINDATAIO_IMPLEMENTATION
   return _stdinBlocking ? GetNullSocket() : _masterSocket;
#else
   return _fdIO.GetSelectSocket();
#endif
}

#ifdef USE_WIN32_STDINDATAIO_IMPLEMENTATION

void StdinDataIO :: IOThreadEntry()
{
   // Note that this thread uses blocking I/O exclusively!
   while(1)
   {
      char buf[4096];
      DWORD numBytesRead;
      if ((ReadFile(_stdinHandle, buf, sizeof(buf), &numBytesRead, NULL) == false)||(SendData(_slaveSocket, buf, numBytesRead, true) != numBytesRead)) break;
   }
   _slaveSocket.Reset();  // this alerts the main thread that we are gone
}

#endif

}; // end namespace muscle
