/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleTCPSocketDataIO_h
#define MuscleTCPSocketDataIO_h

#include "support/MuscleSupport.h"

#ifdef WIN32
# include <winsock.h>
# pragma warning(disable: 4800 4018)
#else
# ifndef BEOS_OLD_NETSERVER
#  include <fcntl.h>
#  include <netinet/tcp.h>
#  include <netinet/in.h>
# endif
#include <sys/socket.h>
#endif

#include <string.h>
#include <errno.h>
#include "dataio/DataIO.h"
#include "util/NetworkUtilityFunctions.h"

namespace muscle {

#ifndef MUSCLE_DEFAULT_TCP_STALL_TIMEOUT
# define MUSCLE_DEFAULT_TCP_STALL_TIMEOUT (20*60*((uint64)1000000))  // 20 minutes is our default default
#endif

/**
 *  Data I/O to and from a TCP socket! 
 */
class TCPSocketDataIO : public DataIO
{
public:
   /**
    *  Constructor.
    *  @param sockfd The socket to use.  Becomes property of this TCPSocketDataIO object.
    *  @param blockingIO determines whether to use blocking or non-blocking socket I/O.
    *  If you will be using this object with a AbstractMessageIOGateway,
    *  and/or select(), then it's usually better to set blocking to false.
    */
   TCPSocketDataIO(int sockfd, bool blocking) : _sockfd(sockfd), _stallLimit(MUSCLE_DEFAULT_TCP_STALL_TIMEOUT)
   {
      SetBlockingIOEnabled(blocking);
   }

   /** Destructor.
    *  close()'s the held socket descriptor.
    */
   virtual ~TCPSocketDataIO() 
   {
      if (_sockfd >= 0) Shutdown();
   }

   /** Reads bytes from the socket and places them into (buffer).
    *  @param buffer Buffer to write the bytes into
    *  @param size Number of bytes in the buffer.
    *  @return Number of bytes read, or -1 on error.
    *  @see DataIO::Read()
    */
   virtual int32 Read(void * buffer, uint32 size)  
   {
      return (_sockfd >= 0) ? CalculateReturnValue(recv(_sockfd, (char *)buffer, size, 0L)) : -1;
   }

   /**
    * Reads bytes from (buffer) and sends them out to the TCP socket.
    *  @param buffer Buffer to read the bytes from.
    *  @param size Number of bytes in the buffer.
    *  @return Number of bytes writte, or -1 on error.
    *  @see DataIO::Write()    
    */
   virtual int32 Write(const void * buffer, uint32 size)
   {
      return (_sockfd >= 0) ? CalculateReturnValue(send(_sockfd, (const char *)buffer, size, 0L)) : -1;
   }

   /**
    *  This method always returns B_ERROR.
    */
   virtual status_t Seek(int64 /*seekOffset*/, int /*whence*/) {return B_ERROR;}

   /**
    * Stall limit for TCP streams is 20*60*1000000 microseconds (20 minutes) by default.
    * Or change it by calling SetOutputStallLimit().
    */
   virtual uint64 GetOutputStallLimit() const {return _stallLimit;}

   /** Set a new output stall time limit.  Set to MUSCLE_TIME_NEVER to disable stall limiting.  */
   void SetOutputStallLimit(uint64 limit) {_stallLimit = limit;}

   /**
    * Flushes the output buffer by turning off Nagle's Algorithm
    * and then turning it back on again
    */
   virtual void FlushOutput()
   {
      SetNaglesAlgorithmEnabled(false);
      SetNaglesAlgorithmEnabled(true);
   }
   
   /**
    * Closes our socket connection
    */
   virtual void Shutdown()
   {
      if (_sockfd >= 0)
      { 
         shutdown(_sockfd, 2);  // go away right now, dammit
         closesocket(_sockfd);  // I'm sticking with closesocket() instead of CloseSocket()
         _sockfd = -1;          // so that I can avoid having to link in NetworkUtilityFunctions.o
      }
   }

   /** Returns our socket descriptor */
   virtual int GetSelectSocket() const {return _sockfd;}

   /**
    * Enables or diables blocking I/O on this socket.
    * If this object is to be used by an AbstractMessageIOGateway,
    * then non-blocking I/O is usually better to use.
    * @param blocking If true, socket is set to blocking I/O mode.  Otherwise, non-blocking I/O.
    * @return B_NO_ERROR on success, B_ERROR on error.
    */
   status_t SetBlockingIOEnabled(bool blocking)
   {
      status_t ret = (_sockfd >= 0) ? SetSocketBlockingEnabled(_sockfd, blocking) : B_ERROR;
      if (ret == B_NO_ERROR) _blocking = blocking;
      return ret;
   }

   /**
    * Turns Nagle's algorithm (output packet buffering/coalescing) on or off.
    * @param enabled If true, data will be held momentarily before sending, to allow for bigger packets.
    *                If false, each Write() call will cause a new packet to be sent immediately.
    * @return B_NO_ERROR on success, B_ERROR on error.
    */
   status_t SetNaglesAlgorithmEnabled(bool enabled)
   {
      if (_sockfd >= 0)
      {
#ifdef BEOS_OLD_NETSERVER
         (void)enabled;   // prevent 'unused var' warning
         return B_ERROR;  // old networking stack doesn't support this flag
#else
         long delay = enabled ? 0 : 1;
         if (setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &delay, sizeof(int)) >= 0)
         {
            return B_NO_ERROR;
         }
         else
         {
            perror("TCPSocketDataIO:SetNaglesAlgorithmEnabled(): setsockopt() failed!");
            return B_ERROR;
         }
#endif
      }
      else return B_ERROR;
   }

   /**
    * Releases control of the contained socket to the calling code.
    * After this method returns, this object no longer owns or can
    * use or close the socket descriptor it once held.
    */
   void ReleaseSocket() {_sockfd = -1;}

   /**
    * Returns the socket descriptor held by this object, or
    * -1 if there is none.
    */
   int GetSocket() const {return _sockfd;}

private:
   int CalculateReturnValue(int ret) const
   {
      int retForBlocking = ((ret == 0) ? -1 : ret);
#ifdef WIN32
      bool isBlocking = (WSAGetLastError() == WSAEWOULDBLOCK);
#else
      bool isBlocking = (errno == EWOULDBLOCK);
#endif
      return (_blocking) ? retForBlocking : (((ret < 0)&&(isBlocking)) ? 0 : retForBlocking); 
   }

   int _sockfd;
   bool _blocking;
   uint64 _stallLimit;
};

};  // end namespace muscle

#endif
