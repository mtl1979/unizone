/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleServerComponent_h
#define MuscleServerComponent_h

#include "message/Message.h"
#include "util/RefCount.h"
#include "util/PulseNode.h"

BEGIN_NAMESPACE(muscle);

class AbstractReflectSession;
class ReflectServer;
class ReflectSessionFactory;

/** Type declaration for a reference-count-pointer to an AbstractReflectSession */
typedef Ref<AbstractReflectSession> AbstractReflectSessionRef;

/** Type declaration for a reference-count-pointer to a ReflectSessionFactory */
typedef Ref<ReflectSessionFactory> ReflectSessionFactoryRef;   

/** 
 *  This class represents any object that can be added to a ReflectServer object
 *  in one way or another, to help define the ReflectServer's behaviour.  This
 *  class provides callback wrappers that let you operate on the server's state.
 */
class ServerComponent : public RefCountable, public PulseNode
{
public:
   /** Default Constructor. */
   ServerComponent();

   /** Destructor. */
   virtual ~ServerComponent();

   /**
    * This method is called when this object has been added to
    * a ReflectServer object.  When this method is called, it
    * is okay to call the other methods in the ServerComponent API.  
    * Should return B_NO_ERROR if everything is okay; something
    * else if there is a problem and the attachment should be aborted.
    * Default implementation does nothing and returns B_NO_ERROR.
    * If you override this, be sure to call your superclass's implementation
    * of this method as the first thing in your implementation, and
    * if it doesn't return B_NO_ERROR, immediately return an error yourself.
    */
   virtual status_t AttachedToServer();

   /**
    * This method is called just before we are removed from the
    * ReflectServer object.  Methods in the ServerComponent API may
    * still be called at this time (but not after this method returns).
    * Default implementation does nothing.
    * If you override this, be sure to call you superclass's implementation
    * of this method as the last thing you do in your implementation.
    */
   virtual void AboutToDetachFromServer();

   /**
    * Called when a message is sent to us by an AbstractReflectSession object.
    * Default implementation is a no-op.
    * @param from The session who is sending the Message to us.
    * @param msg A reference to the message that was sent.
    * @param userData Additional data whose semantics are determined by the sending subclass.
    *                 (For StorageReflectSessions, this value, if non-NULL, is a pointer to the
    *                  DataNode in this Session's node subtree that was matched by the paths in (msg))
    */
   virtual void MessageReceivedFromSession(AbstractReflectSession & from, MessageRef msg, void * userData);

   /**
    * Called when a message is sent to us by a ReflectSessionFactory object.
    * Default implementation is a no-op.
    * @param from The session who is sending the Message to us.
    * @param msg A reference to the message that was sent.
    * @param userData Additional data whose semantics are determined by the sending subclass.
    */
   virtual void MessageReceivedFromFactory(ReflectSessionFactory & from, MessageRef msg, void * userData);

   /** Returns true if we are attached to the ReflectServer object, false if we are not.  */
   bool IsAttachedToServer() const {return (_owner != NULL);}

protected:
   /** Returns the number of milliseconds that the server has been running. */
   uint64 GetServerUptime() const;

   /** Returns the number of bytes that are currently available to be allocated */
   uint32 GetNumAvailableBytes() const;
 
   /** Returns the maximum number of bytes that may be allocated at any given time */
   uint32 GetMaxNumBytes() const;
 
   /** Returns the number of bytes that are currently allocated */
   uint32 GetNumUsedBytes() const;

   /** Passes through to ReflectServer::PutAcceptFactory() */
   status_t PutAcceptFactory(uint16 port, ReflectSessionFactoryRef factoryRef);

   /** Passes through to ReflectServer::RemoveAcceptFactory() */
   status_t RemoveAcceptFactory(uint16 port);

   /** Tells the whole server process to quit ASAP.  */
   void EndServer();

   /**
    * Returns a reference to a Message that is shared by all objects in
    * a single ReflectServer.  This message can be used for whatever 
    * purpose the ServerComponents care to; it is not used by the 
    * server itself.  (Note that StorageReflectSessions add data to
    * this Message and expect it to remain there, so be careful not
    * to remove or overwrite it if you are using StorageReflectSessions)
    */
   Message & GetCentralState() const;

   /**
    * Adds the given AbstractReflectSession to the server's session list.
    * If (socket) is less than zero, no TCP connection will be used,
    * and the session will be a pure server-side entity.
    * @param session A reference to the new session to add to the server.
    * @param socket the socket descriptor associated with the new session, or -1.
    * @return B_NO_ERROR if the session was successfully added, or B_ERROR on error.
    */
   status_t AddNewSession(AbstractReflectSessionRef session, int socket);

   /**
    * Like AddNewSession(), only creates a session that connects asynchronously to
    * the given IP address.  AttachedToServer() will be called immediately on the
    * session, and then when the connection is complete, AsyncConnectCompleted() will
    * be called.  Other than that, however, (session) will behave like any other session,
    * except any I/O messages for the client won't be transferred until the connection
    * completes.
    * @param session A reference to the new session to add to the server.
    * @param targetIPAddress IP address to connect to
    * @param port port to connect to at that address
    * @return B_NO_ERROR if the session was successfully added, or B_ERROR on error 
    *                    (out-of-memory or the connect attempt failed immediately).
    */
   status_t AddNewConnectSession(AbstractReflectSessionRef session, uint32 targetIPAddress, uint16 port);

   /** Returns an object that can be used to iterate over all the sessions currently connected to our ReflectServer.  */
   HashtableIterator<const char *, AbstractReflectSessionRef> GetSessions() const;

   /**
    * Looks up a session connected to our ReflectServer via its session ID string.
    * @param id The ID string of the session you are looking for.
    * @return A reference to the session with the given session ID, or a NULL reference on failure.
    */
   AbstractReflectSessionRef GetSession(const char * id) const;
   
   /** Returns the number of sessions currently active on the server. */
   uint32 GetNumSessions() const;

   /** Returns an iterator that allows one to iterate over all the session factories currently attached to this server. */
   HashtableIterator<uint16, ReflectSessionFactoryRef> GetFactories() const;

   /** Returns the number of session factories currently attached to this server */
   uint32 GetNumFactories() const;

   /** Given a port number, returns a reference to the factory of that port, or a NULL reference if no
such factory exists. */
   ReflectSessionFactoryRef GetFactory(uint16) const;         

private:
   friend class ReflectServer;
   friend class AbstractReflectSession;
   ReflectServer * _owner;  // set directly by the ReflectServer.
};

END_NAMESPACE(muscle);

#endif
