/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleAbstractReflectSession_h
#define MuscleAbstractReflectSession_h

#include "dataio/TCPSocketDataIO.h"
#include "iogateway/AbstractMessageIOGateway.h"
#include "reflector/AbstractSessionIOPolicy.h"
#include "reflector/ServerComponent.h"
#include "util/Queue.h"
#include "util/RefCount.h"

namespace muscle {

/** This is an interface for an object that knows how to create new
 *  AbstractReflectSession objects when needed.  It is used by the
 *  ReflectServer classes to generate sessions when connections are received.
 */
class ReflectSessionFactory : public ServerComponent
{
public:
   /** Constructor */
   ReflectSessionFactory();

   /** Destructor */
   virtual ~ReflectSessionFactory();

   /** Should be overriden to return a new ReflectSession object, or NULL on failure.  
    *  @param remoteIP The IP address of the remote peer, in ASCII format (e.g. "132.239.50.8")
    *  @returns a freshly allocated AbstractReflectSession object on success, or NULL on failure.
    */
   virtual AbstractReflectSession * CreateSession(const String & remoteIP) = 0;

   /** Returns the port we are attached to.  Result is only valid after this factory object has
     * been added to a ReflectServer via PutAcceptFactory(); will return 0 at other times.
     */
   uint16 GetPort() const {return _port;}

protected:
   /** 
    * Convenience method:  Calls MessageReceivedFromFactory() on all session
    * objects.  Saves you from having to do your own iteration every time you
    * want to broadcast something.
    * @param msgRef a reference to the Message you wish to broadcast
    * @param userData any userData value you care to include.  Defaults to NULL.
    */
   void BroadcastToAllSessions(MessageRef msgRef, void * userData = NULL);

   /** 
    * Convenience method:  Calls MessageReceivedFromFactory() on all session-factory
    * objects.  Saves you from having to do your own iteration every time you
    * want to broadcast something to the factories.
    * @param msgRef a reference to the Message you wish to broadcast
    * @param userData any userData value you care to include.  Defaults to NULL.
    * @param includeSelf Whether or not MessageReceivedFromFactory() should be called on 'this' factory.  Defaults to true.
    */
   void BroadcastToAllFactories(MessageRef msgRef, void * userData = NULL, bool includeSelf = true);

private:  
   friend class ReflectServer;  // sets _port and _socket directly
   uint16 _port;
   int _socket;
};


/** This is the abstract base class that defines the server side logic for a single
 *  client-server connection.  This class contains no message routing logic of its own, 
 *  but defines the interface so that subclasses can do so.
 */
class AbstractReflectSession : public ServerComponent
{
public:
   /** Default Constructor. */
   AbstractReflectSession();

   /** Destructor. */
   virtual ~AbstractReflectSession();

   /** Returns the hostname of this client that is associated with this session.
     * May only be called if this session is currently attached to a ReflectServer.
     */
   const char * GetHostName() const;

   /** Returns the server-side port that this session was accepted on, or 0 if 
     * we weren't accepted from a port (e.g. we were created locally) 
     * May only be called if this session is currently attached to a ReflectServer.
     */
   uint16 GetPort() const;

   /**
    * Returns a ID string to represent this session with.
    * The returned string is guaranteed to be unique across
    * all sessions in the server.
    */
   virtual const char * GetSessionIDString() const;

   /** Marks this session to be terminated ASAP, as if the TCP connection had been broken.  */
   void EndSession();

   /**
    * Causes this session to be terminated (similar to EndSession(), 
    * and the session specified in (newSessionRef) to take its 
    * place using the same socket connection & message IO gateway.  
    * @param newSession the new session object that is to take the place of this one.
    * @return B_NO_ERROR on success, B_ERROR if the new session refused to be attached.
    */
   status_t ReplaceSession(AbstractReflectSessionRef newSession);

   /**
    * Calling this method causes MessageReceivedFromGateway() to be called
    * with the correct semantics.  If you are 'faking' a call to MessageReceivedFromGateway,
    * it's better to call this then to call MessageReceivedFromGateway() yourself. 
    * (this method calls AfterMessageReceivedFromGateway() too).
    * @param msg Reference to the Message to be passed to MessageReceivedFromGateway().
    */
   void CallMessageReceivedFromGateway(MessageRef msg);

   /**
    * Called when a new message is received from our IO gateway.
    * @param msg A reference to the Message that has been received from our client.
    */
   virtual void MessageReceivedFromGateway(MessageRef msg) = 0;

   /**
    * This method is called after MessageReceivedFromGateway(msg) was called.
    * Default implementation does nothing, but this method may be overridden 
    * in a subclass to do cleanup work.
    * @param msg The MessageRef that was previously passed to MessageReceivedFromGateway().
    */
   virtual void AfterMessageReceivedFromGateway(MessageRef msg);

   /**
    * Called when the TCP connection to our client is broken.
    * If this method returns true, then this session will be removed and
    * deleted. 
    * @return If it returns false, then this session will continue, even
    *         though the client is no longer available.  Default implementation always returns true.
    */
   virtual bool ClientConnectionClosed();
  
   /**
    * For sessions that were added to the server with AddNewConnectSession(),
    * this method is called when the asynchronous connect process completes
    * successfully.  (if the asynchronous connect fails, ClientConnectionClosed()
    * is called instead).  Default implementation does nothing.
    */
   virtual void AsyncConnectCompleted();

   /**  
    * Set a new input I/O policy for this session.
    * @param policy Reference to the new policy to use to control the incoming byte stream
    *               for this session.  May be a NULL reference if you just want to remove the existing policy.
    */
   void SetInputPolicy(PolicyRef newPolicy);
   
   /** Returns a reference to the current input policy for this session.  
     * May be a NULL reference, if there is no input policy installed (which is the default state)
     */
   PolicyRef GetInputPolicy() const {return _inputPolicyRef;}

   /**
    * Set a new output I/O policy for this session.
    * @param policy Reference to the new policy to use to control the outgoing byte stream
    *               for this session.  May be a NULL reference if you just want to remove the existing policy.
    */
   void SetOutputPolicy(PolicyRef newPolicy);
   
   /** Returns a reference to the current output policy for this session.  May be a NULL reference. 
     * May be a NULL reference, if there is no output policy installed (which is the default state)
     */
   PolicyRef GetOutputPolicy() const {return _outputPolicyRef;}

   /** Installs the given AbstractMessageIOGateway as the gateway we should use for I/O.
     * If this method isn't called, the ReflectServer will call our CreateGateway() method
     * to set our gateway for us when we are attached.
     * @param ref Reference to the I/O gateway to use, or a NULL reference to remove any gateway we have.
     */
   void SetGateway(AbstractMessageIOGatewayRef ref) {_gateway = ref;}

   /**
    * Returns a poitner to our internally held message IO gateway object, 
    * or NULL reference if there is none.  The returned gateway remains 
    * the property of this session.
    */
   AbstractMessageIOGateway * GetGateway() const {return _gateway();}

   /** As above, only returns a reference to the gateway instead of the raw pointer. */
   AbstractMessageIOGatewayRef GetGatewayRef() const {return _gateway;}

   /**
    * Gateway factory method.  Should return a new AbstractMessageIOGateway
    * for this session to use for communicating with its remote peer.  
    * Called by ReflectServer when this session object is added to the
    * server, but doesn't already have a valid gateway installed.
    * The default implementation returns a MessageIOGateway object.
    * @return a new message IO gateway object, or NULL on failure.
    */
   virtual AbstractMessageIOGateway * CreateGateway();

   /** DataIO factory method.  Should return a new non-blocking DataIO 
    *  object for our gateway to use, or NULL on failure.  Called by 
    *  ReflectServer when this session is added to the server. 
    *  The default implementation returns a non-blocking TCPSocketDataIO
    *  object, which is the correct behaviour 99% of the time.
    *  @param socket The socket to provide the DataIO object for.
    *                On success, the DataIO object becomes owner of (socket).
    *  @return A newly allocated DataIO object, or NULL on failure.
    */
   virtual DataIO * CreateDataIO(int socket);

   /** Should return a pretty, human readable string identifying this class.  */
   virtual const char * GetTypeName() const = 0;

protected:
   /**
    * Adds a MessageRef to our gateway's outgoing message queue.
    * (ref) will be sent back to our client when time permits.
    * @param ref Reference to a Message to send to our client.
    * @return B_NO_ERROR on success, B_ERROR if out-of-memory.
    */
   virtual status_t AddOutgoingMessage(MessageRef ref);

   /** 
    * Convenience method:  Calls MessageReceivedFromSession() on all session
    * objects.  Saves you from having to do your own iteration every time you
    * want to broadcast something.
    * @param msgRef a reference to the Message you wish to broadcast
    * @param userData any userData value you care to include.  Defaults to NULL.
    * @param includeSelf Whether or not MessageReceivedFromSession() should be called on 'this' session.  Defaults to true.
    * @see GetSessions(), AddNewSession(), GetSession()
    */
   void BroadcastToAllSessions(MessageRef msgRef, void * userData = NULL, bool includeSelf = true);

   /** 
    * Convenience method:  Calls MessageReceivedFromSession() on all installed
    * session-factory objects.  Saves you from having to do your own iteration 
    * every time you want to broadcast something to the factories.
    * @param msgRef a reference to the Message you wish to broadcast
    * @param userData any userData value you care to include.  Defaults to NULL.
    * @see GetFactories(), PutFactory(), GetFactory()
    */
   void BroadcastToAllFactories(MessageRef msgRef, void * userData = NULL);

private:
   void SetPolicyAux(PolicyRef & setRef, uint32 & setChunk, PolicyRef newRef, bool isInput);

   friend class ReflectServer;
   char _idString[32];
   uint16 _port;
   bool _connectingAsync;
   String _hostName;
   AbstractMessageIOGatewayRef _gateway; /* we own this object */
   uint64 _lastByteOutputAt;
   PolicyRef _inputPolicyRef;
   PolicyRef _outputPolicyRef;
   uint32 _maxInputChunk;   // as determined by our Policy object
   uint32 _maxOutputChunk;  // and stored here for convenience
};

// VC++ can't handle partial template specialization, so for VC++ we define this explicitely.
#ifdef _MSC_VER
template <> class HashFunctor<Ref<AbstractReflectSession> >
{
public:
   uint32 operator() (const Ref<AbstractReflectSession> & x) const {return (uint32)x();}
};
#endif

};  // end namespace muscle

#endif
