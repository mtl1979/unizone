/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleAbstractReflectSession_h
#define MuscleAbstractReflectSession_h

#include "dataio/TCPSocketDataIO.h"
#include "iogateway/AbstractMessageIOGateway.h"
#include "reflector/AbstractSessionIOPolicy.h"
#include "reflector/ServerComponent.h"
#include "util/Queue.h"
#include "util/RefCount.h"

BEGIN_NAMESPACE(muscle);

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
class AbstractReflectSession : public ServerComponent, public AbstractGatewayMessageReceiver
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

   /** Marks this session for immediate termination and removal from the server. */
   void EndSession();

   /** Forces the disconnection of this session's TCP connection to its client.
    *  Calling this will cause ClientConnectionClosed() to be called, as if the
    *  TCP connection had been severed externally.
    *  @returns the value that ClientConnectionClosed() returned.
    */
   bool DisconnectSession();

   /**
    * Causes this session to be terminated (similar to EndSession(), 
    * and the session specified in (newSessionRef) to take its 
    * place using the same socket connection & message IO gateway.  
    * @param newSession the new session object that is to take the place of this one.
    * @return B_NO_ERROR on success, B_ERROR if the new session refused to be attached.
    */
   status_t ReplaceSession(AbstractReflectSessionRef newSession);

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
    * @param newPolicy Reference to the new policy to use to control the incoming byte stream
    *                  for this session.  May be a NULL reference if you just want to remove the existing policy.
    */
   void SetInputPolicy(PolicyRef newPolicy);
   
   /** Returns a reference to the current input policy for this session.  
     * May be a NULL reference, if there is no input policy installed (which is the default state)
     */
   PolicyRef GetInputPolicy() const {return _inputPolicyRef;}

   /**
    * Set a new output I/O policy for this session.
    * @param newPolicy Reference to the new policy to use to control the outgoing byte stream
    *                 for this session.  May be a NULL reference if you just want to remove the existing policy.
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
   void SetGateway(AbstractMessageIOGatewayRef ref) {_gateway = ref; _outputStallLimit = _gateway()?_gateway()->GetOutputStallLimit():MUSCLE_TIME_NEVER;}

   /**
    * Returns a pointer to our internally held message IO gateway object, 
    * or NULL reference if there is none.  The returned gateway remains 
    * the property of this session.
    */
   AbstractMessageIOGateway * GetGateway() const {return _gateway();}

   /** As above, only returns a reference to the gateway instead of the raw pointer. */
   AbstractMessageIOGatewayRef GetGatewayRef() const {return _gateway;}

   /** Should return true iff we have data pending for output.
    *  Default implementation calls HasBytesToOutput() on our installed AbstractDataIOGateway object, 
    *  if we have one, or returns false if we don't.
    */
   virtual bool HasBytesToOutput() const;

   /**
     * Should return true iff we are willing to read more bytes from our
     * client connection at this time.  Default implementation calls
     * IsReadyForInput() on our install AbstractDataIOGateway object, if we 
     * have one, or returns false if we don't.
     * 
     */
   virtual bool IsReadyForInput() const;

   /** Called by the ReflectServer when it wants us to read some more bytes from our client.
     * Default implementation simply calls DoInput() on our Gateway object (if any).
     * @param receiver Object to call CallMessageReceivedFromGateway() on when new Messages are ready to be looked at.
     * @param maxBytes Maximum number of bytes to read before returning.
     * @returns The total number of bytes read, or -1 if there was a fatal error.
     */
   virtual int32 DoInput(AbstractGatewayMessageReceiver & receiver, uint32 maxBytes);

   /** Called by the ReflectServer when it wants us to push some more bytes out to our client.
     * Default implementation simply calls DoOutput() on our Gateway object (if any).
     * @param maxBytes Maximum number of bytes to write before returning.
     * @returns The total number of bytes written, or -1 if there was a fatal error.
     */
   virtual int32 DoOutput(uint32 maxBytes);

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

   /** May be overridden to return the host name string we should be assigned
    *  if no host name could be automatically determined by the ReflectServer
    *  (i.e. if we had no associated socket at the time).
    *  Default implementation returns "<unknown>".
    */
   virtual String GetDefaultHostName() const;

   /** Convenience method -- returns a human-readable string describing our
    *  type, our hostname, our session ID, and what port we are connected to.
    */
   String GetSessionDescriptionString() const;

   /** Returns the IP address we connected asynchronously to.
    *  The returned value is meaningful only if we were added
    *  with AddNewConnectSession().
    */
   uint32 GetAsyncConnectIP() const {return _asyncConnectIP;}

   /** Returns the remote port we connected asynchronously to.
    *  The returned value is meaningful only if we were added
    *  with AddNewConnectSession().
    */
   uint16 GetAsyncConnectPort() const {return _asyncConnectPort;}

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

   /**
    * Closes this session's current TCP connection (if any), and creates a new
    * TCP socket that will then try to asynchronously connect back to the previous
    * socket's host and port.  Note that this method is only useful for sessions
    * that were added with AddNewConnectSession(); for other types of session,
    * it will return B_ERROR with no side effects.
    * @note This method will call CreateDataIO() to make a new DataIO object for
    *       the newly created socket.
    * @returns B_NO_ERROR on success, or B_ERROR on failure.
    *          On success, the asynchronous connection result will be reported back
    *          later, either via a call to AsyncConnectCompleted() (if the connection
    *          succeeds) or a call to ClientConnectionClosed() (if the connection fails)
    */
   status_t Reconnect();

   /** Returns true iff we are currently in the middle of an asynchronous TCP connection */
   bool IsConnectingAsync() const {return _connectingAsync;}

private:
   void SetPolicyAux(PolicyRef & setRef, uint32 & setChunk, PolicyRef newRef, bool isInput);

   friend class ReflectServer;
   char _idString[32];
   uint16 _port;
   bool _connectingAsync;
   String _hostName;
   uint32 _asyncConnectIP;
   uint16 _asyncConnectPort;
   AbstractMessageIOGatewayRef _gateway;
   uint64 _lastByteOutputAt;
   PolicyRef _inputPolicyRef;
   PolicyRef _outputPolicyRef;
   uint32 _maxInputChunk;   // as determined by our Policy object
   uint32 _maxOutputChunk;  // and stored here for convenience
   uint64 _outputStallLimit;
   bool _scratchReconnected; // scratch, watched by ReflectServer() during ClientConnectionClosed() calls.
};

// VC++ can't handle partial template specialization, so for VC++ we define this explicitely.
#ifdef _MSC_VER
template <> class HashFunctor<Ref<AbstractReflectSession> >
{
public:
   uint32 operator() (const Ref<AbstractReflectSession> & x) const {return (uint32)x();}
};
#endif

END_NAMESPACE(muscle);

#endif
