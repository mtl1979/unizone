/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

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
# include <iphlpapi.h>
# pragma warning(disable: 4800 4018)
#else
# include <unistd.h>
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
# ifndef BEOS_OLD_NETSERVER
#  include <net/if.h>
# endif
# include <sys/ioctl.h>
# ifdef SunOS
#  include <sys/sockio.h>     // for the SIOCGIFADDR definition on Solaris
# endif
# ifdef BEOS_OLD_NETSERVER
#  include <app/Roster.h>     // for the run-time bone check
#  include <storage/Entry.h>  // for the backup run-time bone check
#else
#  include <arpa/inet.h>
#  include <fcntl.h>
#  include <netinet/tcp.h>
# endif
#endif

#include <sys/stat.h>

#if defined(__FreeBSD__) || defined(BSD) || defined(__APPLE__) || defined(__linux__)
# define USE_GETIFADDRS 1
# define USE_SOCKETPAIR 1
# include <ifaddrs.h>
#endif

#include "system/GlobalMemoryAllocator.h"  // for muscleAlloc()/muscleFree()

// Different OS's use different types for pass-by-reference in accept(), etc.
// So I define my own muscle_socklen_t to avoid having to #ifdef all my code
#if defined(__amd64__) || defined(__FreeBSD__) || defined(BSD) || defined(__PPC64__)
typedef socklen_t muscle_socklen_t;
#elif defined(__BEOS__) || defined(__APPLE__) || defined(__CYGWIN__) || defined(WIN32) || defined(__QNX__) || defined(__osf__)
# ifdef _SOCKLEN_T
typedef socklen_t muscle_socklen_t;
# else
typedef int muscle_socklen_t;
# endif
#else
typedef size_t muscle_socklen_t;
#endif

BEGIN_NAMESPACE(muscle);

#ifdef MUSCLE_USE_IPV6
# define MUSCLE_SOCKET_FAMILY AF_INET6
static inline void GET_SOCKADDR_IP(const struct sockaddr_in6 & sockAddr, ip_address & ipAddr) {ipAddr.ReadFromNetworkArray(sockAddr.sin6_addr.s6_addr);}
static inline void SET_SOCKADDR_IP(struct sockaddr_in6 & sockAddr, const ip_address & ipAddr) {ipAddr.WriteToNetworkArray(sockAddr.sin6_addr.s6_addr);}
static inline uint16 GET_SOCKADDR_PORT(const struct sockaddr_in6 & addr) {return ntohs(addr.sin6_port);}
static inline void SET_SOCKADDR_PORT(struct sockaddr_in6 & addr, uint16 port) {addr.sin6_port = htons(port);}
static inline uint16 GET_SOCKADDR_FAMILY(const struct sockaddr_in6 & addr) {return addr.sin6_family;}
static inline void SET_SOCKADDR_FAMILY(struct sockaddr_in6 & addr, uint16 family) {addr.sin6_family = family;}
static void InitializeSockAddr6(struct sockaddr_in6 & addr, const ip_address * optFrom, uint16 port)
{
   memset(&addr, 0, sizeof(struct sockaddr_in6));
#ifdef SIN6_LEN
   addr.sin6_len = sizeof(struct sockaddr_in6);
#endif
   SET_SOCKADDR_FAMILY(addr, MUSCLE_SOCKET_FAMILY);
   if (optFrom) SET_SOCKADDR_IP(addr, *optFrom);
   if (port) SET_SOCKADDR_PORT(addr, port);
}
# define DECLARE_SOCKADDR(addr, ip, port) struct sockaddr_in6 addr; InitializeSockAddr6(addr, ip, port);
#else
# define MUSCLE_SOCKET_FAMILY AF_INET
static inline void GET_SOCKADDR_IP(const struct sockaddr_in & sockAddr, ip_address & ipAddr) {ipAddr = ntohl(sockAddr.sin_addr.s_addr);}
static inline void SET_SOCKADDR_IP(struct sockaddr_in & sockAddr, const ip_address & ipAddr) {sockAddr.sin_addr.s_addr = htonl(ipAddr);}
static inline uint16 GET_SOCKADDR_PORT(const struct sockaddr_in & addr) {return ntohs(addr.sin_port);}
static inline void SET_SOCKADDR_PORT(struct sockaddr_in & addr, uint16 port) {addr.sin_port = htons(port);}
static inline uint16 GET_SOCKADDR_FAMILY(const struct sockaddr_in & addr) {return addr.sin_family;}
static inline void SET_SOCKADDR_FAMILY(struct sockaddr_in & addr, uint16 family) {addr.sin_family = family;}
static void InitializeSockAddr4(struct sockaddr_in & addr, const ip_address * optFrom, uint16 port)
{
   memset(&addr, 0, sizeof(struct sockaddr_in));
   SET_SOCKADDR_FAMILY(addr, MUSCLE_SOCKET_FAMILY);
   if (optFrom) SET_SOCKADDR_IP(addr, *optFrom);
   if (port) SET_SOCKADDR_PORT(addr, port);
}
# define DECLARE_SOCKADDR(addr, ip, port) struct sockaddr_in addr; InitializeSockAddr4(addr, ip, port);
#endif

SocketRef CreateUDPSocket()
{
   return GetSocketRefFromPool(socket(MUSCLE_SOCKET_FAMILY, SOCK_DGRAM, 0));
}

status_t BindUDPSocket(const SocketRef & sock, uint16 port, uint16 * optRetPort, const ip_address & optFrom, bool allowShared)
{
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

   if (allowShared)
   {
      int one = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &one, sizeof(one));
#ifdef __APPLE__
      setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char *) &one, sizeof(one));
#endif
   }

   DECLARE_SOCKADDR(saSocket, &optFrom, port);
   if (bind(fd, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)
   {
      if (optRetPort)
      {
         muscle_socklen_t len = sizeof(saSocket);
         if (getsockname(fd, (struct sockaddr *)&saSocket, &len) == 0)
         {
            *optRetPort = GET_SOCKADDR_PORT(saSocket);
            return B_NO_ERROR;
         }
         else return B_ERROR;
      }
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetUDPSocketTarget(const SocketRef & sock, const ip_address & remoteIP, uint16 remotePort)
{
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

   DECLARE_SOCKADDR(saAddr, &remoteIP, remotePort);
   return (connect(fd, (struct sockaddr *) &saAddr, sizeof(saAddr)) == 0) ? B_NO_ERROR : B_ERROR;
}

status_t SetUDPSocketTarget(const SocketRef & sock, const char * remoteHostName, uint16 remotePort, bool expandLocalhost)
{
   ip_address hostIP = GetHostByName(remoteHostName, expandLocalhost);
   return (hostIP != invalidIP) ? SetUDPSocketTarget(sock, hostIP, remotePort) : B_ERROR;
}

SocketRef CreateAcceptingSocket(uint16 port, int maxbacklog, uint16 * optRetPort, const ip_address & optInterfaceIP)
{
   SocketRef ret = GetSocketRefFromPool(socket(MUSCLE_SOCKET_FAMILY, SOCK_STREAM, 0));
   if (ret())
   {
      int fd = ret.GetFileDescriptor();

#ifndef WIN32
      // (Not necessary under windows -- it has the behaviour we want by default)
      const long trueValue = 1;
      (void) setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &trueValue, sizeof(long));
#endif

      DECLARE_SOCKADDR(saSocket, &optInterfaceIP, port);
      if ((bind(fd, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)&&(listen(fd, maxbacklog) == 0)) 
      {
         if (optRetPort)
         {
            muscle_socklen_t len = sizeof(saSocket);
            *optRetPort = (getsockname(fd, (struct sockaddr *)&saSocket, &len) == 0) ? GET_SOCKADDR_PORT(saSocket) : 0;
         }
         return ret;
      }
   }
   return SocketRef();  // failure
}

int32 ReceiveData(const SocketRef & sock, void * buffer, uint32 size, bool bm)
{
   int fd = sock.GetFileDescriptor();
   return (fd >= 0) ? ConvertReturnValueToMuscleSemantics(recv(fd, (char *)buffer, size, 0L), size, bm) : -1;
}

int32 ReceiveDataUDP(const SocketRef & sock, void * buffer, uint32 size, bool bm, ip_address * optFromIP, uint16 * optFromPort)
{
   int fd = sock.GetFileDescriptor();
   if (fd >= 0)
   {
      int r;
      if ((optFromIP)||(optFromPort))
      {
         DECLARE_SOCKADDR(fromAddr, NULL, 0);
         muscle_socklen_t fromAddrLen = sizeof(fromAddr);
         r = recvfrom(fd, (char *)buffer, size, 0L, (struct sockaddr *) &fromAddr, &fromAddrLen);
         if ((r >= 0)&&(GET_SOCKADDR_FAMILY(fromAddr) == MUSCLE_SOCKET_FAMILY))
         {
            if (optFromIP) GET_SOCKADDR_IP(fromAddr, *optFromIP);
            if (optFromPort) *optFromPort = GET_SOCKADDR_PORT(fromAddr);
         }
      }
      else r = recv(fd, (char *)buffer, size, 0L);

      if (r == 0) return 0;  // for UDP, zero is a valid recv() size, since there is no EOS
      return ConvertReturnValueToMuscleSemantics(r, size, bm);
   }
   else return -1;
}

int32 SendData(const SocketRef & sock, const void * buffer, uint32 size, bool bm)
{
   int fd = sock.GetFileDescriptor();
   return (fd >= 0) ? ConvertReturnValueToMuscleSemantics(send(fd, (const char *)buffer, size, 0L), size, bm) : -1;
}

int32 SendDataUDP(const SocketRef & sock, const void * buffer, uint32 size, bool bm, const ip_address & optToIP, uint16 optToPort)
{
   int fd = sock.GetFileDescriptor();
   if (fd >= 0)
   {
       int s;
       if ((optToIP != invalidIP)||(optToPort))
       {
          DECLARE_SOCKADDR(toAddr, NULL, 0);
          if ((optToIP == invalidIP)||(optToPort == 0))
          {
             // Fill in the values with our socket's current target-values, as defaults
             muscle_socklen_t length = sizeof(sockaddr_in);
             if ((getpeername(fd, (struct sockaddr *)&toAddr, &length) != 0)||(GET_SOCKADDR_FAMILY(toAddr) != MUSCLE_SOCKET_FAMILY)) return -1;
          }
          if (optToIP != invalidIP) SET_SOCKADDR_IP(toAddr, optToIP);
          if (optToPort) SET_SOCKADDR_PORT(toAddr, optToPort);
          s = sendto(fd, (const char *)buffer, size, 0L, (struct sockaddr *)&toAddr, sizeof(toAddr));
       }
       else s = send(fd, (const char *)buffer, size, 0L);

       if (s == 0) return 0;  // for UDP, zero is a valid send() size, since there is no EOS
       return ConvertReturnValueToMuscleSemantics(s, size, bm);
   }
   else return -1;
}

status_t ShutdownSocket(const SocketRef & sock, bool dRecv, bool dSend)
{
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

   if ((dRecv == false)&&(dSend == false)) return B_NO_ERROR;  // there's nothing we need to do!

   // Since these constants aren't defined everywhere, I'll define my own:
   const int MUSCLE_SHUT_RD   = 0;
   const int MUSCLE_SHUT_WR   = 1;
   const int MUSCLE_SHUT_RDWR = 2;

   return (shutdown(fd, dRecv?(dSend?MUSCLE_SHUT_RDWR:MUSCLE_SHUT_RD):MUSCLE_SHUT_WR) == 0) ? B_NO_ERROR : B_ERROR;
}

SocketRef Accept(const SocketRef & sock, ip_address * optRetInterfaceIP)
{
   DECLARE_SOCKADDR(saSocket, NULL, 0);
   muscle_socklen_t nLen = sizeof(saSocket);
   int fd = sock.GetFileDescriptor();
   if (fd >= 0)
   {
      SocketRef ret = GetSocketRefFromPool(accept(fd, (struct sockaddr *)&saSocket, &nLen));
      if ((ret())&&(optRetInterfaceIP))
      {
         muscle_socklen_t len = sizeof(saSocket);
         if (getsockname(ret.GetFileDescriptor(), (struct sockaddr *)&saSocket, &len) == 0) GET_SOCKADDR_IP(saSocket, *optRetInterfaceIP);
                                                                                       else *optRetInterfaceIP = invalidIP;
      }
      return ret;
   }
   return SocketRef();  // failure
}

SocketRef Connect(const char * hostName, uint16 port, const char * debugTitle, bool errorsOnly, uint64 maxConnectTime, bool expandLocalhost) 
{
   ip_address hostIP = GetHostByName(hostName, expandLocalhost);
   if (hostIP != invalidIP) return Connect(hostIP, port, hostName, debugTitle, errorsOnly, maxConnectTime);
   else 
   {
      if (debugTitle) LogTime(MUSCLE_LOG_INFO, "%s: hostname lookup for [%s] failed!\n", debugTitle, hostName);
      return SocketRef();
   }
}

SocketRef Connect(const ip_address & hostIP, uint16 port, const char * optDebugHostName, const char * debugTitle, bool errorsOnly, uint64 maxConnectTime)
{
   char ipbuf[64]; Inet_NtoA(hostIP, ipbuf);

   if ((debugTitle)&&(errorsOnly == false))
   {
      LogTime(MUSCLE_LOG_INFO, "%s: Connecting to [%s:%u]: ", debugTitle, optDebugHostName?optDebugHostName:ipbuf, port);
      LogFlush();
   }

   bool socketIsReady = false;
   SocketRef s = (maxConnectTime == MUSCLE_TIME_NEVER) ? GetSocketRefFromPool(socket(MUSCLE_SOCKET_FAMILY, SOCK_STREAM, 0)) : ConnectAsync(hostIP, port, socketIsReady);
   if (s())
   {
      int fd = s.GetFileDescriptor();
      int ret = -1;
      if (maxConnectTime == MUSCLE_TIME_NEVER) 
      {
         DECLARE_SOCKADDR(saAddr, &hostIP, port);
         ret = connect(fd, (struct sockaddr *) &saAddr, sizeof(saAddr));
      }
      else
      {
         if (socketIsReady) ret = 0;  // immediate success, yay!
         else
         {
            // The harder case:  the user doesn't want the Connect() call to take more than (so many) microseconds.
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
               FD_ZERO(&writeSet); FD_SET(fd, &writeSet);
               if (exceptSetPointer) {FD_ZERO(exceptSetPointer); FD_SET(fd, exceptSetPointer);}

               struct timeval timeVal; Convert64ToTimeVal(deadline-now, timeVal);
               if ((select(fd+1, NULL, &writeSet, exceptSetPointer, &timeVal) < 0)&&(PreviousOperationWasInterrupted() == false)) break;  // error out!
               else
               {
                  if ((exceptSetPointer)&&(FD_ISSET(fd, exceptSetPointer))) break;  // Win32:  failed async connect detected!
                  if (FD_ISSET(fd, &writeSet))
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
   }
   else if (debugTitle)
   {
      if (errorsOnly) LogTime(MUSCLE_LOG_INFO, "%s: socket() failed!\n", debugTitle);
                 else Log(MUSCLE_LOG_INFO, "socket() failed!\n");
   }
   return SocketRef();
}

static bool IsIP4Address(const char * s)
{
   int numDots     = 0;
   int numDigits   = 0;
   bool prevWasDot = true;  // an initial dot is illegal
   while(*s)
   {
      if (*s == '.')
      {
         if ((prevWasDot)||(++numDots > 3)) return false;
         numDigits  = 0;
         prevWasDot = true;
      }
      else
      {
         if ((prevWasDot)&&(atoi(s) > 255)) return false;
         prevWasDot = false;
         if ((muscleInRange(*s, '0', '9') == false)||(++numDigits > 3)) return false;
      } 
      s++;
   }
   return (numDots == 3);
}

bool IsIPAddress(const char * s)
{
#ifdef MUSCLE_USE_IPV6
   struct in6_addr tmp;
   return ((inet_pton(MUSCLE_SOCKET_FAMILY, s, &tmp) > 0)||(IsIP4Address(s)));
#else
   return IsIP4Address(s);
#endif
}

static void ExpandLocalhostAddress(ip_address & ipAddress)
{
   if (ipAddress == localhostIP)
   {
      ip_address altRet = GetLocalHostIPOverride();  // see if the user manually specified a preferred local address
      if (altRet == invalidIP)
      {
         // If not, try to grab one from the OS
         Queue<NetworkInterfaceInfo> ifs;
         if ((GetNetworkInterfaceInfos(ifs) == B_NO_ERROR)&&(ifs.HasItems())) altRet = ifs.Head().GetLocalAddress();
      }
      if (altRet != invalidIP) ipAddress = altRet;
   }
}

ip_address GetHostByName(const char * name, bool expandLocalhost)
{
   ip_address ret;
   if (IsIPAddress(name)) ret = Inet_AtoN(name);
   else
   {
#ifdef MUSCLE_USE_IPV6
      ret = invalidIP;
      struct addrinfo * result;
      if (getaddrinfo(name, NULL, NULL, &result) == 0)
      {
         struct addrinfo * next = result;
         while(next)
         {
            switch(next->ai_family)
            {
               case AF_INET:
                  ret.SetBits(ntohl(((struct sockaddr_in *) next->ai_addr)->sin_addr.s_addr), 0);  // read IPv4 address into low bits of IPv6 address structure
               break;

               case AF_INET6:
                  ret.ReadFromNetworkArray(((struct sockaddr_in6 *) (next->ai_addr))->sin6_addr.s6_addr);
               break;
            }
            if (ret == invalidIP) next = next->ai_next;
                             else break;
         }
         freeaddrinfo(result);
      }
#else
      struct hostent * he = gethostbyname(name);
      ret = ntohl(he ? *((ip_address*)he->h_addr) : 0);
#endif
   }

   if (expandLocalhost) ExpandLocalhostAddress(ret);
   return ret;
}

SocketRef ConnectAsync(const ip_address & hostIP, uint16 port, bool & retIsReady)
{
   SocketRef s = GetSocketRefFromPool(socket(MUSCLE_SOCKET_FAMILY, SOCK_STREAM, 0));
   if (s())
   {
      if (SetSocketBlockingEnabled(s, false) == B_NO_ERROR)
      {
         DECLARE_SOCKADDR(saAddr, &hostIP, port);
         int result = connect(s.GetFileDescriptor(), (struct sockaddr *) &saAddr, sizeof(saAddr));
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
   }
   return SocketRef();
}

ip_address GetPeerIPAddress(const SocketRef & sock, bool expandLocalhost)
{
   ip_address ipAddress = invalidIP;
   int fd = sock.GetFileDescriptor();
   if (fd >= 0)
   {
      DECLARE_SOCKADDR(saTempAdd, NULL, 0);
      muscle_socklen_t length = sizeof(saTempAdd);
      if ((getpeername(fd, (struct sockaddr *)&saTempAdd, &length) == 0)&&(GET_SOCKADDR_FAMILY(saTempAdd) == MUSCLE_SOCKET_FAMILY))
      {
         GET_SOCKADDR_IP(saTempAdd, ipAddress);
         if (expandLocalhost) ExpandLocalhostAddress(ipAddress);
      }
   }
   return ipAddress;
}

/* See the header file for description of what this does */
status_t CreateConnectedSocketPair(SocketRef & socket1, SocketRef & socket2, bool blocking)
{
   TCHECKPOINT;

#if defined(USE_SOCKETPAIR)
   int temp[2];
   if (socketpair(AF_UNIX, SOCK_STREAM, 0, temp) == 0)
   { 
      socket1 = GetSocketRefFromPool(temp[0]);
      socket2 = GetSocketRefFromPool(temp[1]);
      if ((SetSocketBlockingEnabled(socket1, blocking) == B_NO_ERROR)&&(SetSocketBlockingEnabled(socket2, blocking) == B_NO_ERROR)) return B_NO_ERROR;
   }
#else
   uint16 port;
   socket1 = CreateAcceptingSocket(0, 1, &port, localhostIP);
   if (socket1())
   {
      socket2 = Connect(localhostIP, port);
      if (socket2())
      {
         SocketRef newfd = Accept(socket1);
         if (newfd())
         {
            socket1 = newfd;
            if ((SetSocketBlockingEnabled(socket1, blocking) == B_NO_ERROR)&&(SetSocketBlockingEnabled(socket2, blocking) == B_NO_ERROR))
            {
               (void) SetSocketNaglesAlgorithmEnabled(socket1, false);
               (void) SetSocketNaglesAlgorithmEnabled(socket2, false);
               return B_NO_ERROR;
            }
         }
      }
   }
#endif

   socket1.Reset();
   socket2.Reset();
   return B_ERROR;
}

status_t SetSocketBlockingEnabled(const SocketRef & sock, bool blocking)
{
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

#ifdef WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? B_NO_ERROR : B_ERROR;
#else
# ifdef BEOS_OLD_NETSERVER
   long b = blocking ? 0 : 1; 
   return (setsockopt(fd, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b)) == 0) ? B_NO_ERROR : B_ERROR;
# else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags < 0) return B_ERROR;
   flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? B_NO_ERROR : B_ERROR;
# endif
#endif
}

status_t SetUDPSocketBroadcastEnabled(const SocketRef & sock, bool broadcast)
{
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

   int val = (broadcast ? 1 : 0);
#ifdef BEOS_OLD_NETSERVER
   return (setsockopt(fd, SOL_SOCKET, INADDR_BROADCAST, (char *) &val, sizeof(val)) == 0) ? B_NO_ERROR : B_ERROR;
#else
   return (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,     (char *) &val, sizeof(val)) == 0) ? B_NO_ERROR : B_ERROR;
#endif
}

status_t SetSocketNaglesAlgorithmEnabled(const SocketRef & sock, bool enabled)
{
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

#ifdef BEOS_OLD_NETSERVER
   (void) enabled;  // prevent 'unused var' warning
   return B_ERROR;  // old networking stack doesn't support this flag
#else
   int enableNoDelay = enabled ? 0 : 1;
   return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &enableNoDelay, sizeof(enableNoDelay)) >= 0) ? B_NO_ERROR : B_ERROR;
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

status_t FinalizeAsyncConnect(const SocketRef & sock)
{
   TCHECKPOINT;

   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

#if defined(BEOS_OLD_NETSERVER)
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
      return (connect(fd, (struct sockaddr *) &junk, sizeof(junk)) == 0) ? B_NO_ERROR : B_ERROR;
   }
#elif defined(__FreeBSD__) || defined(BSD)
   // Nathan Whitehorn reports that send() doesn't do this trick under FreeBSD 7,
   // so for BSD we'll call getpeername() instead.  -- jaf
   struct sockaddr_in junk;
   socklen_t length = sizeof(junk);
   memset(&junk, 0, sizeof(junk));
   return (getpeername(fd, (struct sockaddr *)&junk, &length) == 0) ? B_NO_ERROR : B_ERROR;
#else
   // For most platforms (including BONE), the code below is all we need
   char junk;
   return (send(fd, &junk, 0, 0L) == 0) ? B_NO_ERROR : B_ERROR;
#endif
}

status_t SetSocketSendBufferSize(const SocketRef & sock, uint32 sendBufferSizeBytes)
{
#ifdef BEOS_OLD_NETSERVER
   (void) sock;
   (void) sendBufferSizeBytes;
   return B_ERROR;  // not supported!
#else
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

   int iSize = (int) sendBufferSizeBytes;
   return (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&iSize, sizeof(iSize)) >= 0) ? B_NO_ERROR : B_ERROR;
#endif
}

status_t SetSocketReceiveBufferSize(const SocketRef & sock, uint32 receiveBufferSizeBytes)
{
#ifdef BEOS_OLD_NETSERVER
   (void) sock;
   (void) receiveBufferSizeBytes;
   return B_ERROR;  // not supported!
#else
   int fd = sock.GetFileDescriptor();
   if (fd < 0) return B_ERROR;

   int iSize = (int) receiveBufferSizeBytes;
   return (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&iSize, sizeof(iSize)) >= 0) ? B_NO_ERROR : B_ERROR;
#endif
}

NetworkInterfaceInfo :: NetworkInterfaceInfo() : _ip(0), _netmask(0), _broadcastIP(0)
{
   // empty
}

NetworkInterfaceInfo :: NetworkInterfaceInfo(const char * name, const char * desc, const ip_address & ip, const ip_address & netmask, const ip_address & remoteIP) : _ip(ip), _netmask(netmask), _broadcastIP(remoteIP)
{
   _name = name;
   _desc = desc;
}

#if defined(USE_GETIFADDRS)
static ip_address SockAddrToIPAddr(struct sockaddr * a)
{
   if (a)
   {
      switch(a->sa_family)   
      {
         case AF_INET:  return ip_address(ntohl(((struct sockaddr_in *)a)->sin_addr.s_addr));
#ifdef MUSCLE_USE_IPV6
         case AF_INET6:
         {
            ip_address ret;
            ret.ReadFromNetworkArray(((struct sockaddr_in6 *)a)->sin6_addr.s6_addr);
            return ret;
         }
#endif
      }
   }
   return invalidIP;
}
#endif

status_t GetNetworkInterfaceInfos(Queue<NetworkInterfaceInfo> & results)
{
#if defined(USE_GETIFADDRS)
   /////////////////////////////////////////////////////////////////////////////////////////////
   // "Apparently, getifaddrs() is gaining a lot of traction at becoming the One True Standard
   //  way of getting at interface info, so you're likely to find support for it on most modern
   //  Unix-like systems, at least..."
   //
   // http://www.developerweb.net/forum/showthread.php?t=5085
   /////////////////////////////////////////////////////////////////////////////////////////////

   struct ifaddrs * ifap;

   if (getifaddrs(&ifap) == 0)
   {
      status_t ret = B_NO_ERROR;
      {
         struct ifaddrs * p = ifap;
         while(p)
         {
            ip_address ifaAddr  = SockAddrToIPAddr(p->ifa_addr);
            ip_address maskAddr = SockAddrToIPAddr(p->ifa_netmask);
            ip_address dstAddr  = SockAddrToIPAddr(p->ifa_broadaddr);
            if ((ifaAddr != invalidIP)&&(results.AddTail(NetworkInterfaceInfo(p->ifa_name, "", ifaAddr, maskAddr, dstAddr)) != B_NO_ERROR))
            {
               ret = B_ERROR;  // out of memory!?
               break;
            }
            else p = p->ifa_next;
         }
      }
      freeifaddrs(ifap);
      return ret;
   }
#elif defined(WIN32)
   // Adapted from example code at http://msdn2.microsoft.com/en-us/library/aa365917.aspx
   status_t ret = B_NO_ERROR;  // optimistic default

   // Now get Windows' IPv4 addresses table.  We gotta call GetIpAddrTable()
   // multiple times in order to deal with potential race conditions properly.
   MIB_IPADDRTABLE * ipTable = NULL;
   {
      ULONG bufLen = 0;
      for (int i=0; i<5; i++) 
      {
         DWORD ipRet = GetIpAddrTable(ipTable, &bufLen, false);
         if (ipRet == ERROR_INSUFFICIENT_BUFFER)
         {
            muscleFree(ipTable);  // in case we had previously allocated it
            ipTable = (MIB_IPADDRTABLE *) muscleAlloc(bufLen);
            if (ipTable == NULL) {WARN_OUT_OF_MEMORY; break;}
         }
         else if (ipRet == NO_ERROR) break;
         else 
         {
            muscleFree(ipTable);
            ipTable = NULL;
            break;
         }
      }
   }

   if (ipTable)
   {
      // Try to get the Adapters-info table, so we can given useful names to the IP 
      // addresses we are returning.  Once again, we gotta call GetAdaptersInfo() up to 
      // 5 times to handle the potential race condition between the size-query call and 
      // the get-data call.  I love a well-designed API :^P
      IP_ADAPTER_INFO * pAdapterInfo = NULL;
      {
         ULONG bufLen = 0;
         for (int i=0; i<5; i++) 
         {
            DWORD apRet = GetAdaptersInfo(pAdapterInfo, &bufLen);
            if (apRet == ERROR_BUFFER_OVERFLOW)
            {
               muscleFree(pAdapterInfo);  // in case we had previously allocated it
               pAdapterInfo = (IP_ADAPTER_INFO *) muscleAlloc(bufLen);
               if (pAdapterInfo == NULL) {WARN_OUT_OF_MEMORY; break;}
            }
            else if (apRet == ERROR_SUCCESS) break;
            else 
            {
               muscleFree(pAdapterInfo);
               pAdapterInfo = NULL;
               break;
            }
         }
      }

      for (DWORD i=0; i<ipTable->dwNumEntries; i++)
      {
         const MIB_IPADDRROW & row = ipTable->table[i];
         
         // Now lookup the appropriate adaptor-name in the pAdaptorInfos, if we can find it
         const char * name = NULL;
         const char * desc = NULL;
         if (pAdapterInfo)
         {
            IP_ADAPTER_INFO * next = pAdapterInfo;
            while((next)&&(name==NULL))
            {
               IP_ADDR_STRING * ipAddr = &next->IpAddressList;
               while(ipAddr)
               {
                  if (Inet_AtoN(ipAddr->IpAddress.String) == ntohl(row.dwAddr))
                  {
                     name = next->AdapterName;
                     desc = next->Description;
                     break;
                  }
                  ipAddr = ipAddr->Next;
               }
               next = next->Next;
            }
         }
         char buf[128];
         if (name == NULL) 
         {
            sprintf(buf, "unnamed-%i", i);
            name = buf;
         }

         uint32 ipAddr  = ntohl(row.dwAddr);
         uint32 netmask = ntohl(row.dwMask);
         uint32 baddr   = ipAddr & netmask;
         if (row.dwBCastAddr) baddr |= ~netmask;
         if (results.AddTail(NetworkInterfaceInfo(name, desc?desc:"", ipAddr, netmask, baddr)) != B_NO_ERROR)
         {
            ret = B_ERROR;
            break;
         }
      }

      muscleFree(pAdapterInfo);
      muscleFree(ipTable);
   }
   else ret = B_ERROR;

   return ret;
#else
   (void) results;  // for other OS's, this function isn't implemented.
#endif

   return B_ERROR;
}

String Inet_NtoA(const ip_address & ipAddress)
{
   char buf[64];
   Inet_NtoA(ipAddress, buf);
   return String(buf);
}

void Inet_NtoA(const ip_address & addr, char * ipbuf)
{
#ifdef MUSCLE_USE_IPV6
   uint8 ip6[16]; addr.WriteToNetworkArray(ip6);
   if (inet_ntop(AF_INET6, ip6, ipbuf, 46) == NULL) ipbuf[0] = '\0';
#else
   sprintf(ipbuf, INT32_FORMAT_SPEC"."INT32_FORMAT_SPEC"."INT32_FORMAT_SPEC"."INT32_FORMAT_SPEC, (addr>>24)&0xFF, (addr>>16)&0xFF, (addr>>8)&0xFF, (addr>>0)&0xFF);
#endif
}

static uint32 Inet4_AtoN(const char * buf)
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

ip_address Inet_AtoN(const char * buf)
{
#ifdef MUSCLE_USE_IPV6
   struct in6_addr dst;
   if (inet_pton(AF_INET6, buf, &dst) > 0)
   {
      ip_address ret;
      ret.ReadFromNetworkArray(dst.s6_addr);
      return ret;
   }
   else return (IsIP4Address(buf)) ? ip_address(Inet4_AtoN(buf), 0) : invalidIP;
#else
   return Inet4_AtoN(buf);
#endif
}

static ip_address ResolveIP(const char * s, bool allowDNSLookups)
{
   return allowDNSLookups ? GetHostByName(s) : Inet_AtoN(s);
}

void IPAddressAndPort :: SetFromString(const String & s, uint16 defaultPort, bool allowDNSLookups)
{
#ifdef MUSCLE_USE_IPV6
   int lBracket = s.IndexOf('[');
   if (lBracket >= 0)
   {
      int rBracket = s.IndexOf(']', lBracket+1);
      int colIdx   = (rBracket>=0) ? s.IndexOf(':', rBracket+1) : -1;
      _ip   = ResolveIP(((rBracket>lBracket) ? s.Substring(lBracket+1,rBracket) : s.Substring(lBracket+1))(), allowDNSLookups);
      _port = ((colIdx >= 0)&&(muscleInRange(s()[colIdx+1], '0', '9'))) ? atoi(s()+colIdx+1) : defaultPort;
   }
   else
   {
      // If we see a colon after the last dot, then we'll assume that's a port separator.
      // That way "old style" IPv4 address:port combos (e.g. "192.168.0.1:2960") still work
      int lastDot = s.LastIndexOf('.');
      int colIdx = (lastDot >= 0) ? s.IndexOf(':', lastDot+1) : -1;
      if ((colIdx >= 0)&&(muscleInRange(s()[colIdx+1], '0', '9')))
      {
         _ip   = ResolveIP(s.Substring(0, colIdx)(), allowDNSLookups);
         _port = atoi(s()+colIdx+1);
      }
      else
      {
         _ip   = ResolveIP(s(), allowDNSLookups);
         _port = defaultPort;
      }
   }
#else
   // Old style IPv4 parsing (e.g. "192.168.0.1" or "192.168.0.1:2960")
   int colIdx = s.IndexOf(':');
   if ((colIdx >= 0)&&(muscleInRange(s()[colIdx+1], '0', '9')))
   {
      _ip   = ResolveIP(s.Substring(0, colIdx)(), allowDNSLookups);
      _port = atoi(s()+colIdx+1);
   }
   else
   {
      _ip   = ResolveIP(s(), allowDNSLookups);
      _port = defaultPort;
   }
#endif 
}


String IPAddressAndPort :: ToString(bool includePort) const
{
   char buf[128]; 
   Inet_NtoA(_ip, buf);

   if ((includePort)&&(_port > 0))
   {
      char buf2[128];
#ifdef MUSCLE_USE_IPV6
      sprintf(buf2, "[%s]:%u", buf, _port);
#else
      sprintf(buf2, "%s:%u", buf, _port);
#endif
      return buf2;
   }
   else return buf;
}

static ip_address _customLocalhostIP = invalidIP;  // disabled by default
void SetLocalHostIPOverride(const ip_address & ip) {_customLocalhostIP = ip;}
ip_address GetLocalHostIPOverride() {return _customLocalhostIP;}

END_NAMESPACE(muscle);
