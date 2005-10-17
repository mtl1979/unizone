/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#include <stdio.h>

#include "util/NetworkUtilityFunctions.h"

#if __BEOS__
# include <kernel/OS.h>     // for snooze()
#elif __ATHEOS__
# include <atheos/kernel.h> // for snooze()
#endif

#ifdef WIN32
# include <windows.h>
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

#include <string.h>
#include <sys/stat.h>

// On some OS's, calls like accept() take an int* rather than a uint32*
// So I define net_length_t to avoid having to #ifdef all my code
#if defined(__amd64__)
typedef socklen_t net_length_t;
#elif defined(__BEOS__) || defined(__APPLE__) || defined(__CYGWIN__) || defined(WIN32) || defined(__QNX__) || defined(__osf__)
# ifdef _SOCKLEN_T
typedef socklen_t net_length_t;
# else
typedef int net_length_t;
# endif
#else
typedef size_t net_length_t;
#endif

BEGIN_NAMESPACE(muscle);

int CreateUDPSocket()
{
   return socket(AF_INET, SOCK_DGRAM, 0);
}

status_t BindUDPSocket(int sock, uint16 port, uint16 * optRetPort, uint32 optFrom, bool allowShared)
{
   if (allowShared)
   {
      int one = 1;
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &one, sizeof(one));
#ifdef __APPLE__
      setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char *) &one, sizeof(one));
#endif
   }

   struct sockaddr_in saSocket;
   memset(&saSocket, 0, sizeof(saSocket));
   saSocket.sin_family      = AF_INET;
   saSocket.sin_addr.s_addr = htonl(optFrom ? optFrom : INADDR_ANY);
   saSocket.sin_port        = htons(port);

   if (bind(sock, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)
   {
      if (optRetPort)
      {
         net_length_t len = sizeof(saSocket);
         if (getsockname(sock, (struct sockaddr *)&saSocket, &len) == 0)
         {
            *optRetPort = (uint16) ntohs(saSocket.sin_port);
            return B_NO_ERROR;
         }
         else return B_ERROR;
      }
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetUDPSocketTarget(int sock, uint32 remoteIP, uint16 remotePort)
{
   struct sockaddr_in saAddr;
   memset(&saAddr, 0, sizeof(saAddr));
   saAddr.sin_family      = AF_INET;
   saAddr.sin_port        = htons(remotePort);
   saAddr.sin_addr.s_addr = htonl(remoteIP);
   return (connect(sock, (struct sockaddr *) &saAddr, sizeof(saAddr)) == 0) ? B_NO_ERROR : B_ERROR;
}

status_t SetUDPSocketTarget(int sock, const char * remoteHostName, uint16 remotePort)
{
   uint32 hostIP = GetHostByName(remoteHostName);
   return (hostIP) ? SetUDPSocketTarget(sock, hostIP, remotePort) : B_ERROR;
}

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

void CloseSocket(int sock)
{
   if (sock >= 0) closesocket(sock);  // a little bit silly, perhaps...
}

int32 ReceiveData(int sock, void * buffer, uint32 size, bool bm)
{
   return (sock >= 0) ? ConvertReturnValueToMuscleSemantics(recv(sock, (char *)buffer, size, 0L), size, bm) : -1;
}

int32 ReceiveDataUDP(int sock, void * buffer, uint32 size, bool bm, uint32 * optFromIP, uint16 * optFromPort)
{
   if (sock >= 0)
   {
      int r;
      if ((optFromIP)||(optFromPort))
      {
         struct sockaddr_in fromAddr;
         net_length_t fromAddrLen = sizeof(fromAddr);
         r = recvfrom(sock, (char *)buffer, size, 0L, (struct sockaddr *) &fromAddr, &fromAddrLen);
         if (r >= 0)
         {
            if (optFromIP)   *optFromIP   = ntohl(fromAddr.sin_addr.s_addr);
            if (optFromPort) *optFromPort = ntohs(fromAddr.sin_port);
         }
      }
      else r = recv(sock, (char *)buffer, size, 0L);

      if (r == 0) return 0;  // for UDP, zero is a valid recv() size, since there is no EOS
      return ConvertReturnValueToMuscleSemantics(r, size, bm);
   }
   else return -1;
}

int32 SendData(int sock, const void * buffer, uint32 size, bool bm)
{
   return (sock >= 0) ? ConvertReturnValueToMuscleSemantics(send(sock, (const char *)buffer, size, 0L), size, bm) : -1;
}

int32 SendDataUDP(int sock, const void * buffer, uint32 size, bool bm, uint32 optToIP, uint16 optToPort)
{
   if (sock >= 0)
   {
       int s;
       if ((optToIP)||(optToPort))
       {
          struct sockaddr_in toAddr;
          if ((optToIP == 0)||(optToPort == 0))
          {
             // Fill in the values with our socket's current target-values, as defaults
             net_length_t length = sizeof(sockaddr_in);
             if (getpeername(sock, (struct sockaddr *)&toAddr, &length) != 0) return -1;
          }
          else memset(&toAddr, 0, sizeof(toAddr));

          toAddr.sin_family = AF_INET;
          if (optToIP)   toAddr.sin_addr.s_addr = htonl(optToIP);
          if (optToPort) toAddr.sin_port        = htons(optToPort);
          s = sendto(sock, (const char *)buffer, size, 0L, (struct sockaddr *)&toAddr, sizeof(toAddr));
       }
       else s = send(sock, (const char *)buffer, size, 0L);

       if (s == 0) return 0;  // for UDP, zero is a valid send() size, since there is no EOS
       return ConvertReturnValueToMuscleSemantics(s, size, bm);
   }
   else return -1;
}

status_t ShutdownSocket(int sock, bool dRecv, bool dSend)
{
   if (sock < 0) return B_ERROR;
   if ((dRecv == false)&&(dSend == false)) return B_NO_ERROR;  // there's nothing we need to do!

   // Since these constants aren't defined everywhere, I'll define my own:
   const int MUSCLE_SHUT_RD   = 0;
   const int MUSCLE_SHUT_WR   = 1;
   const int MUSCLE_SHUT_RDWR = 2;

   return (shutdown(sock, dRecv?(dSend?MUSCLE_SHUT_RDWR:MUSCLE_SHUT_RD):MUSCLE_SHUT_WR) == 0) ? B_NO_ERROR : B_ERROR;
}

int Accept(int sock)
{
   struct sockaddr_in saSocket;
   net_length_t nLen = sizeof(saSocket);
   return (sock >= 0) ? accept(sock, (struct sockaddr *)&saSocket, &nLen) : -1;
}

int Connect(const char * hostName, uint16 port, const char * debugTitle, bool errorsOnly, uint64 maxConnectTime) 
{
   uint32 hostIP = GetHostByName(hostName);
   if (hostIP > 0) return Connect(hostIP, port, hostName, debugTitle, errorsOnly, maxConnectTime);
   else 
   {
      if (debugTitle) LogTime(MUSCLE_LOG_INFO, "%s: hostname lookup for [%s] failed!\n", debugTitle, hostName);
      return -1;
   }
}

int Connect(uint32 hostIP, uint16 port, const char * optDebugHostName, const char * debugTitle, bool errorsOnly, uint64 maxConnectTime)
{
   char ipbuf[16]; Inet_NtoA(hostIP, ipbuf);

   if ((debugTitle)&&(errorsOnly == false))
   {
      LogTime(MUSCLE_LOG_INFO, "%s: Connecting to [%s:%u]: ", debugTitle, optDebugHostName?optDebugHostName:ipbuf, port);
      LogFlush();
   }

   bool socketIsReady = false;
   int s = (maxConnectTime == MUSCLE_TIME_NEVER) ? socket(AF_INET, SOCK_STREAM, 0) : ConnectAsync(hostIP, port, socketIsReady);
   if (s >= 0)
   {
      int ret = -1;
      if (maxConnectTime == MUSCLE_TIME_NEVER) 
      {
         struct sockaddr_in saAddr;
         memset(&saAddr, 0, sizeof(saAddr));
         saAddr.sin_family      = AF_INET;
         saAddr.sin_port        = htons(port);
         saAddr.sin_addr.s_addr = htonl(hostIP);
         ret = connect(s, (struct sockaddr *) &saAddr, sizeof(saAddr));
      }
      else
      {
         if (socketIsReady) ret = 0;  // immediate success, yay!
         else
         {
            // The harder case:  the user doesn't want the Connect() call to take more than so many microseconds.
            // For this, we'll need to go into non-blocking mode and run a select() loop to get the desired behaviour!
            const uint64 deadline = GetRunTime64()+maxConnectTime;
            fd_set * exceptSetPointer = NULL;
            fd_set writeSet;
#ifdef WIN32
            fd_set exceptSet;  // Under Windows, failed asynchronous connect()'s are communicated via the exceptions fd_set
            exceptSetPointer = &exceptSet;
#endif
            uint64 now;
            while((now = GetRunTime64()) < deadline)
            {
               FD_ZERO(&writeSet); FD_SET(s, &writeSet);
               if (exceptSetPointer) {FD_ZERO(exceptSetPointer); FD_SET(s, exceptSetPointer);}

               struct timeval timeVal; Convert64ToTimeVal(deadline-now, timeVal);
               if ((select(s+1, NULL, &writeSet, exceptSetPointer, &timeVal) < 0)&&(PreviousOperationWasInterrupted() == false)) break;  // error out!
               else
               {
                  if ((exceptSetPointer)&&(FD_ISSET(s, exceptSetPointer))) break;  // Win32:  failed async connect detected!
                  if (FD_ISSET(s, &writeSet))
                  {
                     if ((FinalizeAsyncConnect(s) == B_NO_ERROR)&&(SetSocketBlockingEnabled(s, true) == B_NO_ERROR)) ret = 0;
                     break;
                  }
               }
            }
         }
      }

      if (ret == 0)
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
   if ((ret == 0)||(ret == ((uint32)-1)))
   {
      struct hostent * he = gethostbyname(name);
      ret = ntohl(he ? *((uint32*)he->h_addr) : 0);
   }
   else ret = ntohl(ret);

   if (ret == localhostIP) 
   {
      uint32 altRet = (GetLocalHostIPOverride() > 0) ? GetLocalHostIPOverride() : GetLocalIPAddress();
      if (altRet > 0) ret = altRet;
   }
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

uint32 GetPeerIPAddress(int sock)
{
   uint32 ipAddress = 0;
   if (sock >= 0)
   {
      struct sockaddr_in saTempAdd;
      net_length_t length = sizeof(sockaddr_in);
      if (getpeername(sock, (struct sockaddr *)&saTempAdd, &length) == 0)
      {
         ipAddress = ntohl(saTempAdd.sin_addr.s_addr);
         if (ipAddress == localhostIP) 
         {
            uint32 altRet = (GetLocalHostIPOverride() > 0) ? GetLocalHostIPOverride() : GetLocalIPAddress();
            if (altRet > 0) ipAddress = altRet;
         }
      }
   }
   return ipAddress;
}

/* See the header file for description of what this does */
status_t CreateConnectedSocketPair(int & socket1, int & socket2, bool blocking, bool useNagles)
{
   TCHECKPOINT;

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

status_t SetSocketBlockingEnabled(int sock, bool blocking)
{
   if (sock < 0) return B_ERROR;

#ifdef WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(sock, FIONBIO, &mode) == 0) ? B_NO_ERROR : B_ERROR;
#else
# ifdef BEOS_OLD_NETSERVER
   long b = blocking ? 0 : 1; 
   return (setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b)) == 0) ? B_NO_ERROR : B_ERROR;
# else
   int flags = fcntl(sock, F_GETFL, 0);
   if (flags < 0) return B_ERROR;
   flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
   return (fcntl(sock, F_SETFL, flags) == 0) ? B_NO_ERROR : B_ERROR;
# endif
#endif
}

status_t SetUDPSocketBroadcastEnabled(int sock, bool broadcast)
{
   if (sock < 0) return B_ERROR;

   int val = (broadcast ? 1 : 0);
#ifdef BEOS_OLD_NETSERVER
   return (setsockopt(sock, SOL_SOCKET, INADDR_BROADCAST, (char *) &val, sizeof(val)) == 0) ? B_NO_ERROR : B_ERROR;
#else
   return (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *) &val, sizeof(val)) == 0) ? B_NO_ERROR : B_ERROR;
#endif
}

status_t SetSocketNaglesAlgorithmEnabled(int sock, bool enabled)
{
   if (sock < 0) return B_ERROR;

#ifdef BEOS_OLD_NETSERVER
   (void) enabled;  // prevent 'unused var' warning
   return B_ERROR;  // old networking stack doesn't support this flag
#else
   int enableNoDelay = enabled ? 0 : 1;
   return (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &enableNoDelay, sizeof(enableNoDelay)) >= 0) ? B_NO_ERROR : B_ERROR;
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

status_t FinalizeAsyncConnect(int sock)
{
   TCHECKPOINT;

   if (sock < 0) return B_ERROR;

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
      return (connect(sock, (struct sockaddr *) &junk, sizeof(junk)) == 0) ? B_NO_ERROR : B_ERROR;
   }
#endif

   // For most platforms (including BONE), the code below is all we need
   char junk;
   return (send(sock, &junk, 0, 0L) == 0) ? B_NO_ERROR : B_ERROR;
}

status_t SetSocketSendBufferSize(int sock, uint32 sendBufferSizeBytes)
{
#ifdef BEOS_OLD_NETSERVER
   (void) sock;
   (void) sendBufferSizeBytes;
   return B_ERROR;  // not supported!
#else
   if (sock < 0) return B_ERROR;

   int iSize = (int) sendBufferSizeBytes;
   return (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&iSize, sizeof(iSize)) >= 0) ? B_NO_ERROR : B_ERROR;
#endif
}

status_t SetSocketReceiveBufferSize(int sock, uint32 receiveBufferSizeBytes)
{
#ifdef BEOS_OLD_NETSERVER
   (void) sock;
   (void) receiveBufferSizeBytes;
   return B_ERROR;  // not supported!
#else
   if (sock < 0) return B_ERROR;

   int iSize = (int) receiveBufferSizeBytes;
   return (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&iSize, sizeof(iSize)) >= 0) ? B_NO_ERROR : B_ERROR;
#endif
}

uint32 GetLocalIPAddress(uint32 which)
{
   char ac[512];
   if (gethostname(ac, sizeof(ac)) >= 0)
   {
      struct hostent *phe = gethostbyname(ac);
      if (phe)
      {
         for (int i = 0; phe->h_addr_list[i] != 0; ++i)
         {
            struct in_addr addr;
            memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
            uint32 ip = ntohl(addr.s_addr);
            if ((ip != 0)&&(ip != localhostIP)&&(which-- == 0)) return ip;
         }
      }
   }
   return 0;  // failure
}

END_NAMESPACE(muscle);


