/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "dataio/ChildProcessDataIO.h"

// BeOS doesn't include the tty default stuff
#ifndef MUSCLE_AVOID_TTYDEFS
# if defined(__BEOS__)
#  define MUSCLE_AVOID_TTYDEFS 1
# endif
#endif

#if defined(WIN32) || defined(__CYGWIN__)
# include <process.h>  // for _beginthreadex()
# define USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
#else
# include <errno.h>
# include <fcntl.h>
# include <grp.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/wait.h>
#ifndef TTYDEFCHARS
# define TTYDEFCHARS 1 // to enable declaration of ttydefchars array in sys/ttydefaults.h
#endif
# include <termios.h>
# include <unistd.h>
# include <sys/ioctl.h>
#endif

#include "util/NetworkUtilityFunctions.h"
#include "util/String.h"
#include "util/StringTokenizer.h"

BEGIN_NAMESPACE(muscle);

#ifndef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION

static int ptym_open(char *pts_name)
{
#ifdef __BEOS__
   strcpy(pts_name, "/dev/pt/");
#else
   strcpy(pts_name, "/dev/pty");
#endif

   int base = strlen(pts_name);

   /* array index: 0123456789 (for references in following code) */
   for (const char * ptr1 = "pqrstuvwxyzPQRST"; *ptr1 != 0; ptr1++) 
   {
      pts_name[base] = *ptr1;
      for (const char * ptr2 = "0123456789abcdef"; *ptr2 != 0; ptr2++) 
      {
         pts_name[base+1] = *ptr2;
         pts_name[base+2] = '\0';

         /* try to open master */
         int fdm = open(pts_name, O_RDWR);
         if (fdm < 0) 
         {
            if (errno == ENOENT) return -1; /* out of pty devices */
                            else continue;  /* try next pty device */
         }

         pts_name[5] = 't';  /* change "pty" to "tty" */

         // Make sure we have permission to open the associated tty also.  Note that we can't just 
         // open(pts_name) to see if it works, and then close it again, since that seems to break
         // the functionality.  Apparently the tty assumes it will only be opened once?
         struct stat s;         
         if (stat(pts_name, &s) == 0)
         {
            mode_t m = s.st_mode;
            if ((                          (m & (S_IROTH|S_IWOTH)) == (S_IROTH|S_IWOTH))  || 
                ((getegid() == s.st_gid)&&((m & (S_IRGRP|S_IWGRP)) == (S_IRGRP|S_IWGRP))) || 
                ((geteuid() == s.st_uid)&&((m & (S_IRUSR|S_IWUSR)) == (S_IRUSR|S_IWUSR)))) return fdm;  // success!
         }
         
         pts_name[5] = 'p';  /* oops!  Change it back */
         close(fdm);
      }
   }
   return -1;      /* out of pty devices */
}

static int ptys_open(int fdm, char *pts_name)
{
   struct group *grptr = getgrnam("tty");

   /* following two functions don't work unless we're root */
   chown(pts_name, getuid(), grptr?grptr->gr_gid:(gid_t)-1);
   chmod(pts_name, S_IRUSR | S_IWUSR | S_IWGRP);

   int fds = open(pts_name, O_RDWR);
   if (fds < 0) 
   {
      close(fdm);
      return -1;
   }
   return fds;
}

static pid_t pty_fork(int *ptrfdm, char *slave_name, const struct termios *slave_termios, const struct winsize *slave_winsize, bool & success)
{
   success = true;
   char pts_name[20];

   int fdm = ptym_open(pts_name);
   if (fdm < 0) 
   {
      LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  pty_fork() can't open master pty %s: (%s)\n", pts_name, strerror(errno));
      return -1;
   }
   if (slave_name) strcpy(slave_name, pts_name);   /* return name of slave */

   pid_t pid = fork();
   if (pid < 0) 
   {
      success = false;
      return -1;
   }
   else if (pid == 0) 
   {
      /* child */
      if (setsid() < 0) LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  pty_fork() setsid error: %s\n", strerror(errno));

      /* SVR4 acquires controlling terminal on open() */
      int fds = ptys_open(fdm, pts_name);
      close(fdm);      /* all done with master in child */
      if (fds < 0)
      {
         LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  pty_fork() can't open slave pty: %s on [%i, %s]\n", strerror(errno), fdm, pts_name);
         success = false;
         return 0;   // gotta return zero to indicate we are the child process!  Failure is indicated via separate success param
      }
       
#if defined(TIOCSCTTY) && !defined(CIBAUD)
      /* 44BSD way to acquire controlling terminal */
      /* !CIBAUD to avoid doing this under SunOS */
      if (ioctl(fds, TIOCSCTTY, (char *) 0) < 0) LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  pty_fork() TIOCSCTTY error: %s\n", strerror(errno));
#endif

      /* set slave's termios and window size */
      if ((slave_termios)&&(tcsetattr(fds, TCSANOW, slave_termios) < 0)) LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  pty_fork() tcsetattr error on slave pty: %s\n", strerror(errno));
      if ((slave_winsize)&&(ioctl(fds, TIOCSWINSZ,  slave_winsize) < 0)) LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  TIOCSWINSZ error on slave pty: %s\n", strerror(errno));

      /* slave becomes stdin/stdout/stderr of child */
      if (dup2(fds, STDIN_FILENO)  != STDIN_FILENO)  LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  dup2 error to stdin: %s\n", strerror(errno));
      if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO) LogTime(MUSCLE_LOG_ERROR, "ChildProcessDataIO:  dup2 error to stdout: %s\n", strerror(errno));
      if (fds > STDERR_FILENO) close(fds);
      return 0;      /* child returns 0 just like fork() */
   }
   else 
   {
      /* parent */
      *ptrfdm = fdm; /* return fd of master */
      return pid;    /* parent returns pid of child */
   }
}
#endif

ChildProcessDataIO :: ChildProcessDataIO(bool blocking) : _blocking(blocking), _killChildOnClose(true), _waitForChildOnClose(true)
#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
   , _readFromStdout(INVALID_HANDLE_VALUE), _writeToStdin(INVALID_HANDLE_VALUE), _ioThread(NULL), _wakeupSignal(INVALID_HANDLE_VALUE), _childProcess(INVALID_HANDLE_VALUE), _childThread(INVALID_HANDLE_VALUE), _masterNotifySocket(-1), _slaveNotifySocket(-1), _requestThreadExit(false)
#else
   , _handle(-1), _childPID(-1)
#endif
{
   // empty
}

#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
static void SafeCloseHandle(::HANDLE & h)
{
   if (h != INVALID_HANDLE_VALUE)
   {
      CloseHandle(h);
      h = INVALID_HANDLE_VALUE;
   }
}
#endif

// Parses a command line into a list of argv-style tokens
static status_t ParseLine(const String & line, Queue<String> & addTo)
{
   TCHECKPOINT;

   const String trimmed = line.Trim();
   uint32 len = trimmed.Length();

   // First, we'll pre-process the string into a StringTokenizer-friendly
   // form, by replacing all quoted spaces with gunk and removing the quotes
   String tokenizeThis;
   if (tokenizeThis.Prealloc(len) != B_NO_ERROR) return B_ERROR;

   const char GUNK_CHAR      = (char) 0x01;
   bool lastCharWasBackslash = false;
   bool inQuotes = false;
   for (uint32 i=0; i<len; i++)
   {
      char c = trimmed[i];
      if ((lastCharWasBackslash == false)&&(c == '\"')) inQuotes = !inQuotes;
                                                   else tokenizeThis += ((inQuotes)&&(c == ' ')) ? GUNK_CHAR : c;
      lastCharWasBackslash = (c == '\\');
   }

   StringTokenizer tok(tokenizeThis()," ");
   const char * next;
   while((next = tok()) != NULL)
   {
      String n(next);
      n.Replace(GUNK_CHAR, ' ');
      if (addTo.AddTail(n) != B_NO_ERROR) return B_ERROR;
   }
   return B_NO_ERROR;
}

status_t ChildProcessDataIO :: LaunchChildProcessAux(int argc, const void * args)
{
   TCHECKPOINT;

#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
   SECURITY_ATTRIBUTES saAttr;
   {
      memset(&saAttr, 0, sizeof(saAttr));
      saAttr.nLength        = sizeof(saAttr);
      saAttr.bInheritHandle = true;
   }

   ::HANDLE childStdoutRead, childStdoutWrite;
   if (CreatePipe(&childStdoutRead, &childStdoutWrite, &saAttr, 0))
   {
      if (DuplicateHandle(GetCurrentProcess(), childStdoutRead, GetCurrentProcess(), &_readFromStdout, 0, false, DUPLICATE_SAME_ACCESS))
      {
         SafeCloseHandle(childStdoutRead);  // we'll use the dup from now on

         ::HANDLE childStdinRead, childStdinWrite;
         if (CreatePipe(&childStdinRead, &childStdinWrite, &saAttr, 0))
         {
            if (DuplicateHandle(GetCurrentProcess(), childStdinWrite, GetCurrentProcess(), &_writeToStdin, 0, false, DUPLICATE_SAME_ACCESS))
            {
               SafeCloseHandle(childStdinWrite);  // we'll use the dup from now on

               PROCESS_INFORMATION piProcInfo; 
               memset(&piProcInfo, 0, sizeof(piProcInfo));

               STARTUPINFOA siStartInfo;         
               {
                  memset(&siStartInfo, 0, sizeof(siStartInfo));
                  siStartInfo.cb         = sizeof(siStartInfo);
                  siStartInfo.hStdError  = childStdoutWrite;
                  siStartInfo.hStdOutput = childStdoutWrite;
                  siStartInfo.hStdInput  = childStdinRead;
                  siStartInfo.dwFlags    = STARTF_USESTDHANDLES;
               }

               String cmd;
               if (argc < 0) cmd = (const char *) args;
               else
               {
                  // TODO:  find a less hackish way to do this
                  const char ** argv = (const char **) args;
                  for (int i=0; i<argc; i++)
                  {
                     if ((cmd.Length() > 0)&&(cmd.EndsWith(" ") == false)) cmd += ' ';
                     cmd += argv[i];
                  }
               }

               if (CreateProcessA(NULL, (char *)cmd(), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
               {
                  _childProcess   = piProcInfo.hProcess;
                  _childThread    = piProcInfo.hThread;

                  if (_blocking) return B_NO_ERROR;  // done!
                  else 
                  {
                     // For non-blocking, we must have a separate proxy thread do the I/O for us :^P
                     _wakeupSignal = CreateEvent(0, false, false, 0);
                     if ((_wakeupSignal != INVALID_HANDLE_VALUE)&&(CreateConnectedSocketPair(_masterNotifySocket, _slaveNotifySocket, false) == B_NO_ERROR))
                     {
                        DWORD junkThreadID;
                        typedef unsigned (__stdcall *PTHREAD_START) (void *);
                        if ((_ioThread = (::HANDLE) _beginthreadex(NULL, 0, (PTHREAD_START)IOThreadEntryFunc, this, 0, (unsigned *) &junkThreadID)) != NULL) return B_NO_ERROR;
                     }
                  }
               }
            }
            SafeCloseHandle(childStdinRead);     // cleanup
            SafeCloseHandle(childStdinWrite);    // cleanup
         }
      }
      SafeCloseHandle(childStdoutRead);    // cleanup
      SafeCloseHandle(childStdoutWrite);   // cleanup
   }
   Close();  // free all allocated object state we may have
   return B_ERROR;
#else
   bool okay = false;

   struct termios orig_termios;
   struct winsize size;
   if (tcgetattr(STDIN_FILENO, &orig_termios) < 0)
   {  
      // Fill in reasonable default values so that this can still work...
      // Note that I got these values empirically by running wtrxd from the shell...
      memset(&orig_termios, 0, sizeof(orig_termios));
#ifndef MUSCLE_AVOID_TTYDEFS
      orig_termios.c_iflag = TTYDEF_IFLAG;
      orig_termios.c_oflag = TTYDEF_OFLAG;
      orig_termios.c_lflag = TTYDEF_LFLAG;
      orig_termios.c_cflag = TTYDEF_CFLAG;
      for (uint32 i=0; i<ARRAYITEMS(orig_termios.c_cc); i++) orig_termios.c_cc[i] = 0;
      for (uint32 j=0; j<ARRAYITEMS(ttydefchars); j++) orig_termios.c_cc[j] = ttydefchars[j];
#endif
   }
   if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &size) < 0) memset(&size, 0, sizeof(size));

   int ptySock;
   bool success;
   pid_t pid = pty_fork(&ptySock, NULL, &orig_termios, &size, success);
        if (pid < 0) return -1;  // fork failure!
   else if (pid == 0)
   {
      (void) signal(SIGHUP, SIG_DFL);  // FogBugz #2918

      // we are the child process -- turn off echo and execute the command
      struct termios stermios; 
      if (tcgetattr(STDIN_FILENO, &stermios) < 0) LogTime(MUSCLE_LOG_DEBUG, "tcgetattr error: %s\n", strerror(errno)); 
      stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL); 
      stermios.c_oflag &= ~(ONLCR); /* also turn off NL to CR/NL mapping on output */ 
      if (tcsetattr(STDIN_FILENO, TCSANOW, &stermios) < 0) LogTime(MUSCLE_LOG_DEBUG, "tcsetattr error: %s\n", strerror(errno));
      if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) LogTime(MUSCLE_LOG_DEBUG, "dup2() error: %s\n", strerror(errno));

      if (argc < 0) 
      {
         // I can't use system() here because it spawns an extra
         // process, which means that the parent process tries
         // to kill the wrong _childPID.  We need to have it all
         // execute in _this_ process, which means we gotta use exec()!
         int ret = 20;
         Queue<String> argv;
         if (ParseLine((const char *)args, argv) == B_NO_ERROR)
         {
            argc = argv.GetNumItems();
            char ** newArgv = newnothrow_array(char *, argc+1);
            if (newArgv)
            {
               for (int i=0; i<argc; i++) newArgv[i] = (char *) argv[i]();
               newArgv[argc] = NULL;   // make sure it's terminated!
               ChildProcessReadyToRun();
               ret = execvp(argv[0](), newArgv);
               delete [] newArgv;  // only executed if execvp() fails
            }
         }
         _exit(ret);
      }
      else
      {
         const char ** argv = (const char **) args;
         char ** newArgv = newnothrow_array(char *, argc+1);
         if (newArgv)
         {
            memcpy(newArgv, argv, argc*sizeof(char *));
            newArgv[argc] = NULL;   // make sure it's terminated!
            ChildProcessReadyToRun();
            int ret = execvp(argv[0], newArgv);
            delete [] newArgv;  // only executed if execvp() fails
            _exit(ret);
         }
	 else _exit(20);  // oopsie!
      }
   }
   else
   {
      // we are the parent process
      _childPID = pid;
      SetSocketBlockingEnabled(ptySock, _blocking);
      _handle = ptySock;
      okay = (ptySock >= 0);
   }
   if (okay) return B_NO_ERROR;
   else
   {
      Close();
      return B_ERROR;
   }
#endif
}

ChildProcessDataIO :: ~ChildProcessDataIO()
{
   TCHECKPOINT;
   Close();
}

bool ChildProcessDataIO :: IsChildProcessAvailable() const
{
#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
   return (_readFromStdout != INVALID_HANDLE_VALUE);
#else
   return (_handle >= 0);
#endif
} 

void ChildProcessDataIO :: Close()
{
   TCHECKPOINT;

#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
   if (_ioThread != NULL)  // if this is valid, _wakeupSignal is guaranteed valid too
   {
      _requestThreadExit = true;                // set the "Please go away" flag
      SetEvent(_wakeupSignal);                  // wake the thread up so he'll check the flag
      WaitForSingleObject(_ioThread, INFINITE); // then wait for him to go away
      _ioThread = NULL;
   }
   CloseSocket(_masterNotifySocket); _masterNotifySocket = -1;
   CloseSocket(_slaveNotifySocket);  _slaveNotifySocket  = -1;
   SafeCloseHandle(_wakeupSignal);
   SafeCloseHandle(_readFromStdout);
   SafeCloseHandle(_writeToStdin);
   if (_childProcess != INVALID_HANDLE_VALUE)
   {
      if (_killChildOnClose)    (void) TerminateProcess(_childProcess, 0);
      if (_waitForChildOnClose) (void) WaitForSingleObject(_childProcess, INFINITE);
   }
   SafeCloseHandle(_childProcess);
   SafeCloseHandle(_childThread);
#else
   CloseSocket(_handle); _handle = -1;
   if (_childPID >= 0)
   {
      if (_killChildOnClose)    (void) kill(_childPID, SIGKILL);
      if (_waitForChildOnClose) (void) waitpid(_childPID, NULL, 0);
      _childPID = -1;
   }
#endif
}

int32 ChildProcessDataIO :: Read(void *buf, uint32 len)
{
   TCHECKPOINT;

   if (IsChildProcessAvailable())
   {
#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
      if (_blocking)
      {
         DWORD actual_read;
         if (ReadFile(_readFromStdout, buf, len, &actual_read, NULL)) return actual_read;
      }
      else 
      {
         int32 ret = ConvertReturnValueToMuscleSemantics(recv(_masterNotifySocket, (char *)buf, len, 0L), len, _blocking);
         if (ret >= 0) SetEvent(_wakeupSignal);  // wake up the thread in case he has more data to give us
         return ret;
      }
#else
      int r = read(_handle, buf, len);
      return _blocking ? r : ConvertReturnValueToMuscleSemantics(r, len, _blocking);
#endif
   }
   return -1;
}

int32 ChildProcessDataIO :: Write(const void *buf, uint32 len)
{
   TCHECKPOINT;

   if (IsChildProcessAvailable())
   {
#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
      if (_blocking)
      {
         DWORD actual_write;
         if (WriteFile(_writeToStdin, buf, len, &actual_write, 0)) return actual_write;
      }
      else 
      {
         int32 ret = ConvertReturnValueToMuscleSemantics(send(_masterNotifySocket, (char *)buf, len, 0L), len, _blocking);
         if (ret > 0) SetEvent(_wakeupSignal);  // wake up the thread so he'll check his socket for our new data
         return ret;
      }
#else
      return ConvertReturnValueToMuscleSemantics(write(_handle, buf, len), len, _blocking);
#endif
   }
   return -1;
}

void ChildProcessDataIO :: FlushOutput()
{ 
   // not implemented
}

void ChildProcessDataIO :: ChildProcessReadyToRun()
{
   // empty
}

int ChildProcessDataIO :: GetSelectSocket() const
{
#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION
   return _blocking ? -1 : _masterNotifySocket;
#else 
   return _handle;
#endif
}

void ChildProcessDataIO :: Shutdown()
{
   Close();
}

#ifdef USE_WINDOWS_CHILDPROCESSDATAIO_IMPLEMENTATION

const uint32 CHILD_BUFFER_SIZE = 1024;

// Used as a temporary holding area for data in transit
class ChildProcessBuffer
{
public:
   ChildProcessBuffer() : _length(0), _index(0) {/* empty */}

   char _buf[CHILD_BUFFER_SIZE];
   uint32 _length;  // how many bytes in _buf are actually valid
   uint32 _index;   // Index of the next byte to process
};

// to be called from within the I/O thread only!
void ChildProcessDataIO :: IOThreadAbort()
{
   // If we read zero bytes, that means EOF!  Child process has gone away!
   CloseSocket(_slaveNotifySocket); _slaveNotifySocket = -1;
   _requestThreadExit = true;  // this will cause the I/O thread to go away now
}

void ChildProcessDataIO :: IOThreadEntry()
{
   bool childProcessExited = false;

   ChildProcessBuffer inBuf;  // bytes from the child process's stdout, waiting to go to the _slaveNotifySocket
   ChildProcessBuffer outBuf; // bytes from the _slaveNotifySocket, waiting to go to the child process's stdin

   ::HANDLE events[] = {_wakeupSignal, _childProcess};
   while(_requestThreadExit == false)
   {
      // IOThread <-> UserThread i/o handling here
      {
         // While we have any data in inBuf, send as much of it as possible back to the user thread.  This won't block.
         while(inBuf._index < inBuf._length)
         {
            int32 bytesToWrite = inBuf._length-inBuf._index;
            int32 bytesWritten = (bytesToWrite > 0) ? ConvertReturnValueToMuscleSemantics(send(_slaveNotifySocket, &inBuf._buf[inBuf._index], bytesToWrite, 0L), bytesToWrite, false) : 0;
            if (bytesWritten > 0)
            {
               inBuf._index += bytesWritten;
               if (inBuf._index == inBuf._length) inBuf._index = inBuf._length = 0;
            }
            else
            {
               if (bytesWritten < 0) IOThreadAbort();  // use thread connection closed!?
               break;  // can't write any more, for now
            }
         }

         // While we have room in our outBuf, try to read some more data into it from the slave socket.  This won't block.
         while(outBuf._length < sizeof(outBuf._buf))
         {
            int32 maxLen = sizeof(outBuf._buf)-outBuf._length;
            int32 ret = ConvertReturnValueToMuscleSemantics(recv(_slaveNotifySocket, &outBuf._buf[outBuf._length], maxLen, 0L), maxLen, false);
            if (ret > 0) outBuf._length += ret;
            else
            {
               if (ret < 0) IOThreadAbort();  // user thread connection closed!?
               break;  // no more to read, for now
            }
         }
      }

      // IOThread <-> ChildProcess i/o handling (and blocking) here
      {
         if (childProcessExited)
         {
            if (inBuf._index == inBuf._length) IOThreadAbort();
            break;
         }

         // block here until an event happens... gotta poll because
         // the Window anonymous pipes system doesn't allow me to
         // to check for events on the pipe using WaitForMultipleObjects().
         // It may be worth it to use named pipes some day to get around this...
         int evt = WaitForMultipleObjects(ARRAYITEMS(events)-(childProcessExited?1:0), events, false, 250)-WAIT_OBJECT_0;
         if (evt == 1) childProcessExited = true;

         int32 numBytesToRead;
         while((numBytesToRead = sizeof(inBuf._buf)-inBuf._length) > 0)
         {
            // See if there is actually any data available for reading first
            DWORD pipeSize;
            if (PeekNamedPipe(_readFromStdout, NULL, 0, NULL, &pipeSize, NULL))
            {
               if (pipeSize > 0)
               {
                  DWORD numBytesRead;
                  if (ReadFile(_readFromStdout, &inBuf._buf[inBuf._length], numBytesToRead, &numBytesRead, NULL))
                  {
                     inBuf._length += numBytesRead;
                  }
                  else
                  {
                     IOThreadAbort();  // child process exited?
                     break;
                  }
               }
               else break;
            }
            else 
            {
               IOThreadAbort();  // child process exited?
               break;
            }
         }

         int32 numBytesToWrite;
         while((numBytesToWrite = outBuf._length-outBuf._index) > 0)
         {
            DWORD bytesWritten;
            if (WriteFile(_writeToStdin, &outBuf._buf[outBuf._index], numBytesToWrite, &bytesWritten, 0))
            {
               if (bytesWritten > 0)
               {
                  outBuf._index += bytesWritten;
                  if (outBuf._index == outBuf._length) outBuf._index = outBuf._length = 0;
               }
               else break;  // no more space to write to, for now
            }
            else IOThreadAbort();  // wtf?
         }
      }
   }
}
#endif

END_NAMESPACE(muscle);
