/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include <stdio.h>

#include "util/NetworkUtilityFunctions.h"

#if __BEOS__
# include <kernel/OS.h>     // for snooze()
#elif __ATHEOS__
# include <atheos/kernel.h> // for snooze()
#endif

#ifdef WIN32
# include <winsock.h>
# pragma warning(disable: 4800 4018)
#else
# include <unistd.h>
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
# ifdef BEOS_OLD_NETSERVER
#  include <app/Roster.h>     // for the run-time bone check
#  include <storage/Entry.h>  // for the backup run-time bone check
#else
#  include <arpa/inet.h>
#  include <fcntl.h>
#  include <netinet/tcp.h>
#  include <netinet/in.h>
# endif
#endif

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

// On some OS's, calls like accept() take an int* rather than a uint32*
// So I define net_length_t to avoid having to #ifdef all my code
#if __BEOS__ || __APPLE__ || __CYGWIN__ || WIN32
typedef int net_length_t;
#else
typedef size_t net_length_t;
#endif

namespace muscle {

static uint32 _customLocalhostIP = 0;  // disabled by default

void SetLocalHostIPOverride(uint32 ip) {_customLocalhostIP = ip;}
uint32 GetLocalHostIPOverride() {return _customLocalhostIP;}

int CreateAcceptingSocket(uint16 port, int maxbacklog, uint16 * optRetPort, uint32 optFrom)
{
   struct sockaddr_in saSocket;
   memset(&saSocket, 0, sizeof(saSocket));
   saSocket.sin_family      = AF_INET;
   saSocket.sin_addr.s_addr = htonl(optFrom ? optFrom : INADDR_ANY);
   saSocket.sin_port        = htons(port);

   int acceptSocket;
   if ((acceptSocket = socket(AF_INET, SOCK_STREAM, 0)) >= 0)
   {
#ifndef WIN32
      // (Not necessary under windows -- it has the behaviour we want by default)
      const long trueValue = 1;
      (void) setsockopt(acceptSocket, SOL_SOCKET, SO_REUSEADDR, &trueValue, sizeof(long));
#endif

      if ((bind(acceptSocket, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)&&(listen(acceptSocket, maxbacklog) == 0)) 
      {
         if (optRetPort)
         {
            net_length_t len = sizeof(saSocket);
            *optRetPort = (uint16) ((getsockname(acceptSocket, (struct sockaddr *)&saSocket, &len) == 0) ? ntohs(saSocket.sin_port) : 0);
         }
         return acceptSocket;
      }
      CloseSocket(acceptSocket);
   }
   return -1;
}

void CloseSocket(int socket)
{
   if (socket >= 0) closesocket(socket);  // a little bit silly, perhaps...
}

int32 ReceiveData(int socket, void * buffer, uint32 size, bool bm)
{
   return (socket >= 0) ? ConvertReturnValueToMuscleSemantics(recv(socket, (char *)buffer, size, 0L), size, bm) : -1;
}

int32 SendData(int socket, const void * buffer, uint32 size, bool bm)
{
   return (socket >= 0) ? ConvertReturnValueToMuscleSemantics(send(socket, (const char *)buffer, size, 0L), size, bm) : -1;
}

status_t ShutdownSocket(int socket, bool dRecv, bool dSend)
{
   if (socket < 0) return B_ERROR;
   if ((dRecv == false)&&(dSend == false)) return B_NO_ERROR;  // there's nothing we need to do!

   // Since these constants aren't defined everywhere, I'll define my own:
   const int MUSCLE_SHUT_RD   = 0;
   const int MUSCLE_SHUT_WR   = 1;
   const int MUSCLE_SHUT_RDWR = 2;

   return (shutdown(socket, dRecv?(dSend?MUSCLE_SHUT_RDWR:MUSCLE_SHUT_RD):MUSCLE_SHUT_WR) == 0) ? B_NO_ERROR : B_ERROR;
}

int Accept(int socket)
{
   struct sockaddr_in saSocket;
   net_length_t nLen = sizeof(saSocket);
   return (socket >= 0) ? accept(socket, (struct sockaddr *)&saSocket, &nLen) : -1;
}

int Connect(const char * hostName, uint16 port, const char * debugTitle, bool errorsOnly) 
{
   uint32 hostIP = GetHostByName(hostName);
   return (hostIP) ? Connect(hostIP, port, hostName, debugTitle, errorsOnly) : -1;
}

int Connect(uint32 hostIP, uint16 port, const char * optDebugHostName, const char * debugTitle, bool errorsOnly)
{
   char ipbuf[16]; Inet_NtoA(hostIP, ipbuf);

   if ((debugTitle)&&(errorsOnly == false))
   {
      LogTime(MUSCLE_LOG_INFO, "%s: Connecting to [%s:%u]: ", debugTitle, optDebugHostName?optDebugHostName:ipbuf, port);
      LogFlush();
   }

   struct sockaddr_in saAddr;
   memset(&saAddr, 0, sizeof(saAddr));
   saAddr.sin_family      = AF_INET;
   saAddr.sin_port        = htons(port);
   saAddr.sin_addr.s_addr = htonl(hostIP);

   int s = socket(AF_INET, SOCK_STREAM, 0);
   if (s >= 0)
   {
      if (connect(s, (struct sockaddr *) &saAddr, sizeof(saAddr)) >= 0)
      {
         if ((debugTitle)&&(errorsOnly == false)) Log(MUSCLE_LOG_INFO, "Connected!\n");
         return s;
      }
      else if (debugTitle)
      {
         if (errorsOnly) LogTime(MUSCLE_LOG_INFO, "%s: connect() to [%s:%u] failed!\n", debugTitle, optDebugHostName?optDebugHostName:ipbuf, port);
                    else Log(MUSCLE_LOG_INFO, "Connection failed!\n");
      }
      CloseSocket(s);
   }
   else if (debugTitle)
   {
      if (errorsOnly) LogTime(MUSCLE_LOG_INFO, "%s: socket() failed!\n", debugTitle);
                 else Log(MUSCLE_LOG_INFO, "socket() failed!\n");
   }
   return -1;
}

uint32 GetHostByName(const char * name)
{
   uint32 ret = inet_addr(name);  // first see if we can parse it as a numeric address
   if ((ret == 0)||(ret == (uint32)-1))
   {
      struct hostent * he = gethostbyname(name);
      ret = ntohl(he ? *((uint32*)he->h_addr) : 0);
   }
   else ret = ntohl(ret);

   if ((ret == localhostIP)&&(_customLocalhostIP > 0)) ret = _customLocalhostIP;
   return ret;
}

int ConnectAsync(uint32 hostIP, uint16 port, bool & retIsReady)
{
   int s = socket(AF_INET, SOCK_STREAM, 0);
   if (s >= 0)
   {
      struct sockaddr_in saAddr;
      memset(&saAddr, 0, sizeof(saAddr));
      saAddr.sin_family      = AF_INET;
      saAddr.sin_port        = htons(port);
      saAddr.sin_addr.s_addr = htonl(hostIP);
      if (SetSocketBlockingEnabled(s, false) == B_NO_ERROR)
      {
         int result = connect(s, (struct sockaddr *) &saAddr, sizeof(saAddr));
#ifdef WIN32
         bool inProgress = ((result < 0)&&(WSAGetLastError() == WSAEWOULDBLOCK));
#else
         bool inProgress = ((result < 0)&&(errno == EINPROGRESS));
#endif
         if ((result == 0)||(inProgress)) 
         {
            retIsReady = (inProgress == false);
            return s;
         }
      }
      CloseSocket(s);
   }
   return -1;
}

void Inet_NtoA(uint32 addr, char * ipbuf)
{
   sprintf(ipbuf, "%li.%li.%li.%li", (addr>>24)&0xFF, (addr>>16)&0xFF, (addr>>8)&0xFF, (addr>>0)&0xFF);
}

uint32 Inet_AtoN(const char * buf)
{
   // net_server inexplicably doesn't have this function; so I'll just fake it
   uint32 ret = 0;
   int shift = 24;  // fill out the MSB first
   bool startQuad = true;
   while((shift >= 0)&&(*buf))
   {
      if (startQuad)
      {
         uint8 quad = (uint8) atoi(buf);
         ret |= (((uint32)quad) << shift);
         shift -= 8;
      }
      startQuad = (*buf == '.');
      buf++;
   }
   return ret;
}

uint32 GetPeerIPAddress(int socket)
{
   uint32 ipAddress = 0;
   if (socket >= 0)
   {
      struct sockaddr_in saTempAdd;
      net_length_t length = sizeof(sockaddr_in);
      if (getpeername(socket, (struct sockaddr *)&saTempAdd, &length) == 0)
      {
         ipAddress = ntohl(saTempAdd.sin_addr.s_addr);
         if ((ipAddress == localhostIP)&&(_customLocalhostIP > 0)) ipAddress = _customLocalhostIP;
      }
   }
   return ipAddress;
}

/* Source code stolen from UNIX Network Programming, Volume 1
 * Comments from the Unix FAQ
 */
#ifdef WIN32
status_t SpawnDaemonProcess(bool &, const char *, const char *, bool) 
{ 
   return B_ERROR;  // Win32 can't do this trick, he's too lame  :^(
}
#else
status_t SpawnDaemonProcess(bool & returningAsParent, const char * optNewDir, const char * optOutputTo, bool createIfNecessary)
{
   // Here are the steps to become a daemon:
   // 1. fork() so the parent can exit, this returns control to the command line or shell invoking
   //    your program. This step is required so that the new process is guaranteed not to be a process
   //    group leader. The next step, setsid(), fails if you're a process group leader.
   pid_t pid = fork();
   if (pid < 0) return B_ERROR;
   if (pid > 0) 
   {
      returningAsParent = true;
      return B_NO_ERROR;
   }
   else returningAsParent = false; 

   // 2. setsid() to become a process group and session group leader. Since a controlling terminal is
   //    associated with a session, and this new session has not yet acquired a controlling terminal
   //    our process now has no controlling terminal, which is a Good Thing for daemons.
   setsid();

   // 3. fork() again so the parent, (the session group leader), can exit. This means that we, as a
   //    non-session group leader, can never regain a controlling terminal.
   signal(SIGHUP, SIG_IGN);
   pid = fork();
   if (pid < 0) return B_ERROR;
   if (pid > 0) exit(0);

   // 4. chdir("/") can ensure that our process doesn't keep any directory in use. Failure to do this
   //    could make it so that an administrator couldn't unmount a filesystem, because it was our
   //    current directory. [Equivalently, we could change to any directory containing files important
   //    to the daemon's operation.]
   if (optNewDir) chdir(optNewDir);

   // 5. umask(0) so that we have complete control over the permissions of anything we write.
   //    We don't know what umask we may have inherited. [This step is optional]
   umask(0);

   // 6. close() fds 0, 1, and 2. This releases the standard in, out, and error we inherited from our parent
   //    process. We have no way of knowing where these fds might have been redirected to. Note that many
   //    daemons use sysconf() to determine the limit _SC_OPEN_MAX. _SC_OPEN_MAX tells you the maximun open
   //    files/process. Then in a loop, the daemon can close all possible file descriptors. You have to
   //    decide if you need to do this or not. If you think that there might be file-descriptors open you should
   //    close them, since there's a limit on number of concurrent file descriptors.
   // 7. Establish new open descriptors for stdin, stdout and stderr. Even if you don't plan to use them,
   //    it is still a good idea to have them open. The precise handling of these is a matter of taste;
   //    if you have a logfile, for example, you might wish to open it as stdout or stderr, and open `/dev/null'
   //    as stdin; alternatively, you could open `/dev/console' as stderr and/or stdout, and `/dev/null' as stdin,
   //    or any other combination that makes sense for your particular daemon.
   int nullfd = open("/dev/null", O_RDWR);
   if (nullfd >= 0) dup2(nullfd, STDIN_FILENO);

   int outfd = -1;
   if (optOutputTo) 
   {
      outfd = open(optOutputTo, O_WRONLY | (createIfNecessary ? O_CREAT : 0));
      if (outfd < 0) LogTime(MUSCLE_LOG_ERROR, "BecomeDaemonProcess():  Couldn't open %s to redirect stdout, stderr\n", optOutputTo);
   }
   if (outfd >= 0) dup2(outfd, STDOUT_FILENO);
   if (outfd >= 0) dup2(outfd, STDERR_FILENO);

   return B_NO_ERROR;
}
#endif

status_t BecomeDaemonProcess(const char * optNewDir, const char * optOutputTo, bool createIfNecessary)
{
   bool isParent;
   status_t ret = SpawnDaemonProcess(isParent, optNewDir, optOutputTo, createIfNecessary);
   if ((ret == B_NO_ERROR)&&(isParent)) exit(0);
   return ret;
}

/* See the header file for description of what this does */
status_t CreateConnectedSocketPair(int & socket1, int & socket2, bool blocking, bool useNagles)
{
   uint16 port;
   socket1 = CreateAcceptingSocket(0, 1, &port, localhostIP);
   if (socket1 >= 0)
   {
      socket2 = Connect(localhostIP, port);
      if (socket2 >= 0)
      {
         int newfd = Accept(socket1);
         if (newfd >= 0)
         {
            CloseSocket(socket1);
            socket1 = newfd;
            if ((SetSocketBlockingEnabled(socket1, blocking) == B_NO_ERROR)&&
                (SetSocketBlockingEnabled(socket2, blocking) == B_NO_ERROR))
            {
               (void) SetSocketNaglesAlgorithmEnabled(socket1, useNagles);
               (void) SetSocketNaglesAlgorithmEnabled(socket2, useNagles);
               return B_NO_ERROR;
            }
         }
         CloseSocket(socket2);
      }
      CloseSocket(socket1);
   }
   socket1 = socket2 = -1;
   return B_ERROR;
}

status_t SetSocketBlockingEnabled(int socket, bool blocking)
{
   if (socket < 0) return B_ERROR;

#ifdef WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(socket, FIONBIO, &mode) == 0) ? B_NO_ERROR : B_ERROR;
#else
# ifdef BEOS_OLD_NETSERVER
   long b = blocking ? 0 : 1; 
   return (setsockopt(socket, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b)) == 0) ? B_NO_ERROR : B_ERROR;
# else
   int flags = fcntl(socket, F_GETFL, 0);
   if (flags < 0) return B_ERROR;
   flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
   return (fcntl(socket, F_SETFL, flags) == 0) ? B_NO_ERROR : B_ERROR;
# endif
#endif
}

status_t SetSocketNaglesAlgorithmEnabled(int socket, bool enabled)
{
   if (socket < 0) return B_ERROR;

#ifdef BEOS_OLD_NETSERVER
   (void) enabled;  // prevent 'unused var' warning
   return B_ERROR;  // old networking stack doesn't support this flag
#else
   int enableNoDelay = enabled ? 0 : 1;
   return (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *) &enableNoDelay, sizeof(enableNoDelay)) >= 0) ? B_NO_ERROR : B_ERROR;
#endif
}

status_t Snooze64(uint64 micros)
{
#if __BEOS__
   return snooze(micros);
#elif __ATHEOS__
   return (snooze(micros) >= 0) ? B_NO_ERROR : B_ERROR;
#elif WIN32
   Sleep((DWORD)((micros/1000)+(((micros%1000)!=0)?1:0)));
   return B_NO_ERROR;
#else
   /** We can use select(), if nothing else */
   struct timeval waitTime;
   Convert64ToTimeVal(micros, waitTime);
   return (select(0, NULL, NULL, NULL, &waitTime) >= 0) ? B_NO_ERROR : B_ERROR;
#endif
}

status_t FinalizeAsyncConnect(int socket)
{
   if (socket < 0) return B_ERROR;

#ifdef BEOS_OLD_NETSERVER
   // net_server and BONE behave COMPLETELY differently as far as finalizing async connects
   // go... so we have to do this horrible hack where we figure out whether we're in a 
   // true net_server or BONE environment at runtime.  Pretend you didn't see any of this, 
   // you'll sleep better.  :^P
   static bool userIsRunningBone    = false;
   static bool haveDoneRuntimeCheck = false;
   if (haveDoneRuntimeCheck == false)
   {
      userIsRunningBone = be_roster ? (!be_roster->IsRunning("application/x-vnd.Be-NETS")) : BEntry("/boot/beos/system/lib/libsocket.so").Exists();
      haveDoneRuntimeCheck = true;  // only do this check once, it's rather expensive!
   }
   if (userIsRunningBone == false)
   {
      // net_server just HAS to do things differently from everyone else :^P
      struct sockaddr_in junk;
      memset(&junk, 0, sizeof(junk));
      return (connect(socket, (struct sockaddr *) &junk, sizeof(junk)) == 0) ? B_NO_ERROR : B_ERROR;
   }
#endif

   // For most platforms (including BONE), the code below is all we need
   char junk;
   return (send(socket, &junk, 0, 0L) == 0) ? B_NO_ERROR : B_ERROR;
}

status_t SetSocketSendBufferSize(int socket, uint32 sendBufferSizeBytes)
{
   if (socket < 0) return B_ERROR;

   int iSize = (int) sendBufferSizeBytes;
   return (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&iSize, sizeof(iSize)) >= 0) ? B_NO_ERROR : B_ERROR;
}

status_t SetSocketReceiveBufferSize(int socket, uint32 receiveBufferSizeBytes)
{
   if (socket < 0) return B_ERROR;

   int iSize = (int) receiveBufferSizeBytes;
   return (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *)&iSize, sizeof(iSize)) >= 0) ? B_NO_ERROR : B_ERROR;
}

};  // end namespace muscle


