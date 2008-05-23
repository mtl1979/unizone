/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleSocket_h
#define MuscleSocket_h

#include "util/RefCount.h"

BEGIN_NAMESPACE(muscle);

/** A simple socket-holder class to make sure that socket fd's 
  * added to Messages get properly closed and not leaked if said 
  * Messages never get processed.
  */
class Socket : public RefCountable
{
public:
   /** Default constructor. */
   Socket() : _fd(-1), _okayToClose(false) {/* empty */}

   /** Constructor.
     * @param fd File descriptor of a socket.  (fd) becomes property of this Socket object.
     * @param okayToClose If true (fd) will be closed by the destructor.
     *                    If false, we will not close (fd).  Defaults to true. 
     */
   explicit Socket(int fd, bool okayToClose = true) : _fd(fd), _okayToClose(okayToClose) {/* empty */}

   /** Destructor.  Closes our held file descriptor, if we have one. */
   virtual ~Socket();

   /** Returns and releases our held file descriptor.   
     * When this method returns, ownership of the socket is transferred to the calling code.
     */
   int ReleaseFileDescriptor() {int ret = _fd; _fd = -1; return ret;}

   /** Returns the held socket fd, but does not release ownership of it. */  
   int GetFileDescriptor() const {return _fd;}

   /** Sets our file descriptor.  Will close any old one if appropriate. */
   void SetFileDescriptor(int fd, bool okayToClose = true);

private:
   /** Copy constructor, private and unimplemented on purpose */
   Socket(const Socket &);

   int _fd;
   bool _okayToClose;
};

/** SocketRef is subclassed rather than typedef'd so that I can override the == and != operators
  * to check for equality based on the file descriptor value rather than on the address of the
  * referenced Socket object.  Doing it this way gives more intuitive hashing behavior (i.e.
  * multiple SocketRefs referencing the same file descriptor will hash to the same entry)
  */
class SocketRef : public Ref<Socket>
{
public:
   /** Default constructor */
   SocketRef() : Ref<Socket>() {/* empty */}

   /** This constructor takes ownership of the given Socket object.
     * @param item The Socket object to take ownership of.  This will be recycled/deleted when this SocketRef is destroyed, unless (doRefCount) is specified as false.
     * @param doRefCount If set false, we will not attempt to reference-count (item), and instead will only act like a dumb pointer.  Defaults to true.
     */
   SocketRef(Socket * item, bool doRefCount = true) : Ref<Socket>(item, doRefCount) {/* empty */}

   /** Copy constructor
     * @param copyMe The SocketRef to become a copy of.  Note that this doesn't copy (copyMe)'s underlying Socket object, but instead
     *               only adds one to its reference count.
     */
   SocketRef(const SocketRef & copyMe) : Ref<Socket>(copyMe) {/* empty */}

   /** Downcast constructor
     * @param ref A GenericRef object that hopefully holds a Socket object
     * @param junk This parameter doesn't mean anything; it is only here to differentiate this ctor from the other ctors.
     */
   SocketRef(const GenericRef & ref, bool junk) : Ref<Socket>(ref, junk) {/* empty */}

   /** Comparison operator.  Returns true iff (this) and (rhs) both contain the same file descriptor. */
   inline bool operator ==(const SocketRef &rhs) const {return GetFileDescriptor() == rhs.GetFileDescriptor();}

   /** Comparison operator.  Returns false iff (this) and (rhs) both contain the same file descriptor. */
   inline bool operator !=(const SocketRef &rhs) const {return GetFileDescriptor() != rhs.GetFileDescriptor();}

   /** Convenience method.  Returns the file descriptor we are holding, or -1 if we are a NULL reference. */
   int GetFileDescriptor() const {const Socket * s = GetItemPointer(); return s?s->GetFileDescriptor():-1;}

   /** When we're being used as a key in a Hashtable, key on the file descriptor we hold */
   uint32 HashCode() const {return (uint32)GetFileDescriptor();}
};
DECLARE_HASHTABLE_KEY_CLASS(SocketRef);

/** Returns a SocketRef from our SocketRef pool that references the passed in file descriptor.
  * @param fd The file descriptor that the returned SocketRef should be tracking.
  * @param okayToClose if true, (fd) will be closed when the last SocketRef 
  *                    that references it is destroyed.  If false, it won't be.
  * @param returnNULLIfInvalidSocket If left true and (fd) is negative, then a NULL SocketRef
  *                                  will be returned.  If set false, then we will return a
  *                                  non-NULL SocketRef object, with (fd)'s negative value in it.
  * @returns a SocketRef pointing to the specified socket on success, or a NULL SocketRef on
  *          failure (out of memory).  Note that in the failure case, (fd) will be closed unless
  *          (okayToClose) was false; that way you don't have to worry about closing it yourself.
  */
SocketRef GetSocketRefFromPool(int fd, bool okayToClose = true, bool returnNULLIfInvalidSocket = true);

/** Convenience method:  Returns a NULL socket reference. */
const SocketRef & GetNullSocket();

/** Convenience method:  Returns a reference to an invalid Socket (i.e. a Socket object with a negative file descriptor).  Note the difference between what this function returns and what GetNullSocket() returns!  If you're not sure which of these two functions to use, then GetNullSocket() is probably the one you want. */
const SocketRef & GetInvalidSocket();

END_NAMESPACE(muscle);

#endif
