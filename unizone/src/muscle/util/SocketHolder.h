/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleSockHolder_h
#define MuscleSockHolder_h

#include "util/RefCount.h"
#include "util/NetworkUtilityFunctions.h"

BEGIN_NAMESPACE(muscle);

/** A simple socket-holder class to make sure that socket fd's 
  * added to Messages get properly closed leaked if said 
  * Messages never get processed.
  */
class SocketHolder : public RefCountable
{
public:
   /** Constructor.
     * @param fd File descriptor of a socket.  (fd) becomes property of this SocketHolder object.
     */
   SocketHolder(int fd) : _fd(fd) {/* empty */}

   /** Destructor.  Calls CloseSocket() on the held file descriptor. */
   virtual ~SocketHolder() {CloseSocket(_fd);}

   /** Returns and releases our held socket.   
     * When this method returns, ownership of the socket is transferred to the calling code.
     */
   int ReleaseSocket() {int ret = _fd; _fd = -1; return ret;}

   /** Returns the held socket fd, but does not release ownership of it. */  
   int GetSocket() const {return _fd;}

private:
   int _fd;
};

typedef Ref<SocketHolder> SocketHolderRef;

END_NAMESPACE(muscle);

#endif
