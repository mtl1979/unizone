/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleReflectServer_h
#define MuscleReflectServer_h

#include "reflector/AbstractReflectSession.h"

namespace muscle {

class MemoryAllocator;

/** This class represents a MUSCLE server:  It runs on a centrally located machine,
 *  and many clients may connect to it simultaneously.  This server can then redirect messages
 *  uploaded by any client to other clients in a somewhat efficient manner.
 *  This class can be used as-is, or subclassed if necessary.
 *  There is typically only one ReflectServer object present in a given MUSCLE server program.
 */
class ReflectServer : public PulseNode, private PulseNodeManager
{
public: 
   /** Constructor. 
     * @param optMemoryUsageTracker If non-NULL, this tracker will be inspected by the server
     *                              report back to clients how much memory is currently available
     *                              on the server.  Note that this tracker does NOT become
     *                              owned by the ReflectServer!
     */
   ReflectServer(MemoryAllocator * optMemoryUsageTracker = NULL);

   /** Destructor.  */
   virtual ~ReflectServer();

   /** The main loop for the message reflection server.
    *  This method will not return until the server stops running (usually due to an error).
    *  @return B_NO_ERROR if the server has decided to exit peacefully, or B_ERROR if there was a 
    *                     fatal error during setup or execution.
    */
   virtual status_t ServerProcessLoop();

   /** This function may be called zero or more times before ServerProcessLoop()
    *  Each call adds one port that the server should listen on, and the factory object
    *  to use to create sessions when a connection is received on that port.
    *  @param port The TCP port the server will listen on.  (muscled's traditional port is 2960)
    *              If this port is zero, then the server will choose an available port number to use.
    *              If this port is the same as one specified in a previous call to PutAcceptFactory(),
    *              the old factory associated with that port will be replaced with this one.
    *  @param sessionFactoryRef Reference to a factory object that can generate new sessions when needed.
    *  @return B_NO_ERROR on success, B_ERROR on failure (couldn't bind to socket?)
    */
   virtual status_t PutAcceptFactory(uint16 port, ReflectSessionFactoryRef sessionFactoryRef);
    
   /** Remove a listening port callback that was previously added by PutAcceptFactory().
    *  @param port whose callback should be removed.  If (port) is set to zero, all callbacks will be removed.
    *  @returns B_NO_ERROR on success, or B_ERROR if a factory for the specified port was not found.
    */
   virtual status_t RemoveAcceptFactory(uint16 port);

   /**
    * Called after the server is set up, but just before accepting any connections.
    * Should return B_NO_ERROR if it's okay to continue, or B_ERROR to abort and shut down the server.
    * @return Default implementation returns B_NO_ERROR.
    */
   virtual status_t ReadyToRun();

   /**
    * Adds a new session that uses the given socket for I/O.
    * @param ref New session to add to the server.
    * @param socket The TCP socket that the new session will be using, or -1, if the new session has no TCP connection.
    * @return B_NO_ERROR if the new session was added successfully, or B_ERROR if there was an error setting it up.
    */
   virtual status_t AddNewSession(AbstractReflectSessionRef ref, int socket);

   /**
    * Like AddNewSession(), only creates a session that connects asynchronously to
    * the given IP address.  AttachedToServer() will be called immediately on the
    * session, and then when the connection is complete, AsyncConnectCompleted() will
    * be called.  Other than that, however, (session) will behave like any other session,
    * except any I/O messages for the client won't be transferred until the connection
    * completes.
    * @param ref New session to add to the server.
    * @param targetIPAddress IP address to connect to
    * @param port Port to connect to at that IP address.
    * @return B_NO_ERROR if the session was successfully added, or B_ERROR on error
    *                    (out-of-memory, or the connect attempt failed immediately)
    */
   status_t AddNewConnectSession(AbstractReflectSessionRef ref, uint32 targetIPAddress, uint16 port); 

   /**
    * Should be called just before the ReflectServer is to be destroyed.
    * this in a good place to put any cleanup code.  Be sure
    * to call Cleanup() of your parent class as well!
    * (We can't just do this in the destructor, as some cleanup
    * relies on the subclass still being functional, which it isn't
    * when our destructor gets called!)
    */
   virtual void Cleanup();

   /** Accessor to our central-state repository message */
   Message & GetCentralState() {return _centralState;}

   /** Set whether or not we should log informational messages when sessions are added and removed, etc.
     * Default state is true.
     */
   void SetDoLogging(bool log) {_doLogging = log;}

   /** Returns whether or not we should be logging informational messages. */
   bool GetDoLogging() const {return _doLogging;}

   /**
    * Returns a human-readable string that describes the type of server that is running.  
    * @return Default implementation returns "MUSCLE".
    */
   virtual const char * GetServerName() const;

   /** Returns an iterator that allows one to iterate over all the sessions currently running on this server. */
   HashtableIterator<const char *, AbstractReflectSessionRef> GetSessions() const {return _sessions.GetIterator();}

   /** Returns the number of sessions currently attached to this server */
   uint32 GetNumSessions() const {return _sessions.GetNumItems();}

   /** Given a session ID string, returns a reference to the session, or a NULL reference if no such session exists. */
   AbstractReflectSessionRef GetSession(const char * name) const;

   /** Returns an iterator that allows one to iterate over all the session factories currently attached to this server. */
   HashtableIterator<uint16, ReflectSessionFactoryRef> GetFactories() const {return _factories.GetIterator();}

   /** Returns the number of session factories currently attached to this server */
   uint32 GetNumFactories() const {return _factories.GetNumItems();}

   /** Given a port number, returns a reference to the factory of that port, or a NULL reference if no such factory exists. */
   ReflectSessionFactoryRef GetFactory(uint16 port) const;

   /** Call this and the server will quit ASAP */
   void EndServer();

   /** Returns the number of microseconds since our main loop started */
   uint64 GetServerUptime() const;

   /** Returns the number of bytes that are currently available to be allocated, or MUSCLE_NO_LIMIT
     * if no memory watcher was specified in the constructor.
     */
   uint32 GetNumAvailableBytes() const;
 
   /** Returns the maximum number of bytes that may be allocated at any given time, or MUSCLE_NO_LIMIT
     * if no memory watcher was specified in the constructor. 
     */
   uint32 GetMaxNumBytes() const;
 
   /** Returns the number of bytes that are currently allocated, or MUSCLE_NO_LIMIT
     * if no memory watcher was specified in the constructor. 
     */
   uint32 GetNumUsedBytes() const;

   /** Returns the time at which the current cycle of the server's
    *  event loop began.
    */
   inline uint64 GetCycleStartTime() const {return _cycleStartedAt;}

   /** Returns a reference to a table mapping IP addresses to custom strings...
     * This table may be examined or altered.  When a new connection is accepted,
     * the ReflectServer will consult this table for the address-level MUSCLE node's
     * name.  If an entry is found, it will be used verbatim; otherwise, a name will
     * be created based on the peer's IP address.  Useful for e.g. NAT remapping...
     */
   Hashtable<uint32, String> & GetAddressRemappingTable() {return _remapIPs;}

   /** Read-only implementation of the above */
   const Hashtable<uint32, String> & GetAddressRemappingTable() const {return _remapIPs;}

protected:
   /**
    * This version of AddNewSession (which is called by the previous 
    * version) assumes that the gateway, hostname, port, etc of the
    * new session have already been set up.
    * @param ref The new session to add.
    * @return B_NO_ERROR if the new session was added successfully, or B_ERROR if there was an error setting it up.
    */
   virtual status_t AddNewSession(AbstractReflectSessionRef ref);

   /** Called by a session to send a message to its factory.  
     * @see AbstractReflectSession::SendMessageToFactory() for details.
     */
   status_t SendMessageToFactory(AbstractReflectSession * session, MessageRef msgRef, void * userData);

   /**
    * Called by a session to get itself replaced (the
    * new session will continue using the same message io streams
    * as the old one)
    * @return B_NO_ERROR on success, B_ERROR if the new session
    * returns an error in its AttachedToServer() method.  If 
    * B_ERROR is returned, then this call is guaranteed not to
    * have had any effect on the old session.
    */
   status_t ReplaceSession(AbstractReflectSessionRef newSession, AbstractReflectSession * replaceThisOne);

   /** Called by a session to get itself removed & destroyed */
   void EndSession(AbstractReflectSession * which);

   /** Called by a session to force its TCP connection to be closed */
   void DisconnectSession(AbstractReflectSession * which);

private:
   friend class AbstractReflectSession;
   void CleanupSockets(Queue<int> & list);  // utility method
   void AddLameDuckSession(AbstractReflectSessionRef whoRef);
   void AddLameDuckSession(AbstractReflectSession * who);  // convenience method ... less efficient
   void ShutdownIOFor(AbstractReflectSession * session);
   status_t ClearLameDucks();  // returns B_NO_ERROR if the server should keep going, or B_ERROR otherwise
   void DumpBoggedSessions();
   void RemoveAcceptFactoryAux(ReflectSessionFactoryRef ref);
   status_t FinalizeAsyncConnect(AbstractReflectSessionRef ref);
   status_t DoAccept(uint16 port, int acceptSocket, ReflectSessionFactory * optFactory);
   uint32 CheckPolicy(Hashtable<PolicyRef, bool> & policies, PolicyRef policyRef, const PolicyHolder & ph, uint64 now) const;
   int GetSocketFor(AbstractReflectSession * session) const;
   void CheckForOutOfMemory(AbstractReflectSessionRef optSessionRef);

   Hashtable<uint16, ReflectSessionFactoryRef> _factories;
   Queue<ReflectSessionFactoryRef> _lameDuckFactories;  // for delayed-deletion of factories when they go away

   Message _centralState;
   Hashtable<const char *, AbstractReflectSessionRef> _sessions;
   Queue<AbstractReflectSessionRef> _lameDuckSessions;  // sessions that are due to be removed
   bool _keepServerGoing;
   uint64 _cycleStartedAt;
   uint64 _serverStartedAt;
   bool _doLogging;


   Hashtable<uint32, String> _remapIPs;  // for v2.20; custom strings for "special" IP addresses

   MemoryAllocator * _watchMemUsage; 
};

};  // end namespace muscle

#endif
