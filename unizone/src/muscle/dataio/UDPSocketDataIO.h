/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleUDPSocketDataIO_h
#define MuscleUDPSocketDataIO_h

#include "support/MuscleSupport.h"

#include "dataio/DataIO.h"
#include "util/NetworkUtilityFunctions.h"

BEGIN_NAMESPACE(muscle);

/**
 *  Data I/O to and from a UDP socket! 
 */
class UDPSocketDataIO : public DataIO
{
public:
   /**
    *  Constructor.
    *  @param sockfd The socket to use.  Becomes property of this UDPSocketDataIO object.
    *  @param blocking specifies whether to use blocking or non-blocking socket I/O.
    *  If you will be using this object with a AbstractMessageIOGateway,
    *  and/or select(), then it's usually better to set blocking to false.
    */
   UDPSocketDataIO(int sockfd, bool blocking) : _sockfd(sockfd)
   {
      SetBlockingIOEnabled(blocking);
   }

   /** Destructor.
    *  Closes the socket descriptor, if necessary.
    */
   virtual ~UDPSocketDataIO() 
   {
      if (_sockfd >= 0) Shutdown();
   }

   virtual int32 Read(void * buffer, uint32 size) {return ReceiveDataUDP(_sockfd, buffer, size, _blocking);}
   virtual int32 Write(const void * buffer, uint32 size) {return SendDataUDP(_sockfd, buffer, size, _blocking);}

   /**
    *  This method implementation always returns B_ERROR, because you can't seek on a socket!
    */
   virtual status_t Seek(int64 /*seekOffset*/, int /*whence*/) {return B_ERROR;}

   /** Always returns -1, since a socket has no position to speak of */
   virtual int64 GetPosition() const {return -1;}

   /** Implemented as a no-op:  UDP sockets are always flushed immediately anyway */
   virtual void FlushOutput() {/* empty */}
   
   /**
    * Closes our socket connection
    */
   virtual void Shutdown()
   {
      if (_sockfd >= 0)
      { 
         ShutdownSocket(_sockfd);  // go away right now, dammit
         CloseSocket(_sockfd);
         _sockfd = -1;
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
      status_t ret = SetSocketBlockingEnabled(_sockfd, blocking);
      if (ret == B_NO_ERROR) _blocking = blocking;
      return ret;
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

   /** Returns true iff our socket is set to use blocking I/O (as specified in
    *  the constructor or in our SetBlockingIOEnabled() method)
    */
   bool IsBlockingIOEnabled() const {return _blocking;}

private:
   int _sockfd;
   bool _blocking;
};

END_NAMESPACE(muscle);

#endif
