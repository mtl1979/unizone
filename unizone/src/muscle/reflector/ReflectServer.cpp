/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#if defined(__linux__) || defined(__APPLE__)
# include <signal.h>
#endif

#include "reflector/ReflectServer.h"
#include "reflector/StorageReflectConstants.h"
#include "util/NetworkUtilityFunctions.h"
#include "util/MemoryAllocator.h"

#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
# include "system/GlobalMemoryAllocator.h"
#endif

static volatile bool _signalCaught = false;
void MuscleSignalHandlerCallbackFunc(int /*signum*/) {_signalCaught = true;}

BEGIN_NAMESPACE(muscle);

status_t
ReflectServer ::
AddNewSession(const AbstractReflectSessionRef & ref, int s)
{
   TCHECKPOINT;

   AbstractReflectSession * newSession = ref();
   if (newSession == NULL) return B_ERROR;

   newSession->_owner = this;  // in case CreateGateway() needs to use the owner

   // Create the gateway object for this session, if it isn't already set up
   if (s >= 0)
   {
      AbstractMessageIOGatewayRef gatewayRef = newSession->GetGatewayRef();
      if (gatewayRef() == NULL) gatewayRef.SetRef(newSession->CreateGateway());
      if (gatewayRef())  // don't combine these ifs!
      {
         if (gatewayRef()->GetDataIO() == NULL)
         {
            // create the new DataIO for the gateway; this must always be done on the fly
            // since it depends on the socket being used.
            DataIO * io = newSession->CreateDataIO(s);
            if (io) 
            {
               // success!
               gatewayRef()->SetDataIO(DataIORef(io));
               newSession->SetGateway(gatewayRef);
            }
            else {newSession->_owner = NULL; return B_ERROR;}
         }
      }
      else {newSession->_owner = NULL; return B_ERROR;}
   }

   TCHECKPOINT;

   // Set our hostname (IP address) string if it isn't already set
   if (newSession->_hostName.Length() == 0)
   {
      int socket = GetSocketFor(newSession);
      if (socket >= 0)
      {
         uint32 ip = GetPeerIPAddress(socket, true);
         const String * remapString = _remapIPs.Get(ip);
         char ipbuf[16]; Inet_NtoA(ip, ipbuf);
         newSession->_hostName = remapString ? *remapString : ((ip > 0) ? String(ipbuf) : newSession->GetDefaultHostName());
      }
      else newSession->_hostName = newSession->GetDefaultHostName();
   }

        if (AddNewSession(ref) == B_NO_ERROR) return B_NO_ERROR;
   else if (newSession) newSession->_owner = NULL;

   TCHECKPOINT;

   return B_ERROR;
}

status_t
ReflectServer ::
AddNewConnectSession(const AbstractReflectSessionRef & ref, uint32 destIP, uint16 port)
{
   AbstractReflectSession * session = ref();
   if (session)
   {
      bool isReady;
      int socket = ConnectAsync(destIP, port, isReady);
      if (socket >= 0)
      {
         session->_asyncConnectIP = destIP;
         session->_asyncConnectPort = port;
         char ipbuf[16]; Inet_NtoA(destIP, ipbuf);
         session->_hostName = (destIP > 0) ? ipbuf : "<unknown>";
         session->_connectingAsync = (isReady == false);
         status_t ret = AddNewSession(ref, socket);
         if (ret == B_NO_ERROR)
         {
            if (isReady) session->AsyncConnectCompleted();
            return B_NO_ERROR;
         }
      }
   }
   return B_ERROR;
}

status_t
ReflectServer ::
AddNewSession(const AbstractReflectSessionRef & ref)
{
   AbstractReflectSession * newSession = ref();
   if ((newSession)&&(_sessions.Put(newSession->GetSessionIDString(), ref) == B_NO_ERROR))
   {
      newSession->_owner = this;
      if (newSession->AttachedToServer() == B_NO_ERROR)
      {
         if (_doLogging) LogTime(MUSCLE_LOG_DEBUG, "New %s ("UINT32_FORMAT_SPEC" total)\n", newSession->GetSessionDescriptionString()(), _sessions.GetNumItems());
         return B_NO_ERROR;
      }
      else 
      {
         newSession->AboutToDetachFromServer();  // well, it *was* attached, if only for a moment
         newSession->DoOutput(MUSCLE_NO_LIMIT);  // one last chance for him to send any leftover data!
         if (_doLogging) LogTime(MUSCLE_LOG_DEBUG, "%s aborted startup ("UINT32_FORMAT_SPEC" left)\n", newSession->GetSessionDescriptionString()(), _sessions.GetNumItems()-1);
      }
      newSession->_owner = NULL;
      (void) _sessions.Remove(newSession->GetSessionIDString());
   }
   return B_ERROR;
}


ReflectServer :: ReflectServer(MemoryAllocator * optMemoryUsageTracker) : _keepServerGoing(true), _serverStartedAt(0), _doLogging(true), _watchMemUsage(optMemoryUsageTracker)
{
   // make sure _lameDuckSessions has plenty of memory available in advance (we need might need it in a tight spot later!)
   _lameDuckSessions.EnsureSize(256);
   _sessions.SetKeyCompareFunction(CStringCompareFunc);
}

ReflectServer :: ~ReflectServer()
{
   // empty
}

void
ReflectServer :: Cleanup()
{
   // Detach all sessions
   {
      HashtableIterator<const char *, AbstractReflectSessionRef> iter = GetSessions();
      const char * nextKey;
      AbstractReflectSessionRef nextValue;
      while(iter.GetNextKeyAndValue(nextKey, nextValue) == B_NO_ERROR)
      {
         if (nextValue()) 
         {
            nextValue()->AboutToDetachFromServer();
            nextValue()->DoOutput(MUSCLE_NO_LIMIT);  // one last chance for him to send any leftover data!
            nextValue()->_owner = NULL;
            _lameDuckSessions.AddTail(nextValue);  // we'll delete it below
            _sessions.Remove(nextKey);  // but prevent other sessions from accessing it now that it's detached
         }
      }
   }
  
   // Detach all factories
   RemoveAcceptFactory(0);

   // This will dereference everything so they can be safely deleted here
   _lameDuckSessions.Clear();
   _lameDuckFactories.Clear();
}

status_t
ReflectServer ::
ReadyToRun()
{
   return B_NO_ERROR;
}

const char *
ReflectServer ::
GetServerName() const
{
   return "MUSCLE";
}

/** Makes sure the given policy has its BeginIO() called, if necessary, and returns it */
uint32
ReflectServer :: 
CheckPolicy(Hashtable<PolicyRef, bool> & policies, const PolicyRef & policyRef, const PolicyHolder & ph, uint64 now) const
{
   AbstractSessionIOPolicy * p = policyRef();
   if (p)
   {
      // Any policy that is found attached to a session goes into our temporary policy set
       policies.Put(policyRef, true);

      // If the session is ready, and BeginIO() hasn't been called on this policy already, do so now
      if ((ph.GetSession())&&(p->_hasBegun == false))
      {
         CallSetCycleStartTime(*p, now);
         p->BeginIO(now); 
         p->_hasBegun = true;
      }
   }
   return ((ph.GetSession())&&((p == NULL)||(p->OkayToTransfer(ph)))) ? MUSCLE_NO_LIMIT : 0;
}

void ReflectServer :: CheckForOutOfMemory(const AbstractReflectSessionRef & optSessionRef)
{
   if ((_watchMemUsage)&&(_watchMemUsage->HasAllocationFailed()))
   {
      _watchMemUsage->SetAllocationHasFailed(false);  // clear the memory-failed flag
      if (optSessionRef())
      {
         if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Low Memory!  Aborting %s to get some back!\n", optSessionRef()->GetSessionDescriptionString()());
         AddLameDuckSession(optSessionRef);
      }
      else if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Low Memory!  Dumping bogged sessions!\n");

      DumpBoggedSessions();       // see what other cleanup we can do
   }
}

status_t 
ReflectServer ::
ServerProcessLoop()
{
   TCHECKPOINT;

   _serverStartedAt = GetRunTime64();

   if (_doLogging)
   {
      LogTime(MUSCLE_LOG_DEBUG, "This %s server was compiled on " __DATE__ " " __TIME__ "\n", GetServerName());
      LogTime(MUSCLE_LOG_DEBUG, "The server was compiled with MUSCLE version %s\n", MUSCLE_VERSION_STRING);
   }

   if (ReadyToRun() != B_NO_ERROR) 
   {
      if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Server:  ReadyToRun() failed, aborting.\n");
      return B_ERROR;
   }

   TCHECKPOINT;

   // Print an informative startup message
   if (_doLogging)
   {
      int numFuncs = _factories.GetNumItems();
      uint32 myIP = GetLocalIPAddress();
      if (myIP > 0)
      {
         char bbuf[16]; Inet_NtoA(myIP, bbuf);
         LogTime(MUSCLE_LOG_DEBUG, "%s Server is active on %s", GetServerName(), bbuf); 
      }
      else LogTime(MUSCLE_LOG_DEBUG, "%s Server is active", GetServerName()); 

      if (numFuncs > 0)
      {
         Log(MUSCLE_LOG_DEBUG, " and listening on port%s ", (numFuncs > 1) ? "s" : "");
         HashtableIterator<uint16, ReflectSessionFactoryRef> iter(_factories);
         uint16 port=0;  // just to shut the compiler up
         int which=0;
         while(iter.GetNextKey(port) == B_NO_ERROR) Log(MUSCLE_LOG_DEBUG, "%u%s", port, (++which<numFuncs)?",":"");
      }
      else Log(MUSCLE_LOG_DEBUG, " but not listening to any ports");
      Log(MUSCLE_LOG_DEBUG, ".\n");
   }

   // The primary event loop for any MUSCLE-based server!
   // These variables are used as scratch space, but are declared outside the loop to avoid having to reinitialize them all the time.
   Hashtable<PolicyRef, bool> policies;
   fd_set readSet;
   fd_set writeSet;
#ifdef WIN32
   fd_set exceptionSet;
#endif

   while(ClearLameDucks() == B_NO_ERROR)
   {
      TCHECKPOINT;

      fd_set * exceptionSetPtr = NULL;  // set to point to exceptionSet if we should be watching for exceptions

      // Initialize our fd sets of events-to-watch-for
      FD_ZERO(&readSet);
      FD_ZERO(&writeSet);
      int maxSocket = -1;
      uint64 nextPulseAt = MUSCLE_TIME_NEVER; // running minimum of everything that wants to be Pulse()'d

      // Set up fd set entries and Pulse() timing info for all our different components
      {
         const uint64 now = GetRunTime64(); // nothing in this scope is supposed to take a significant amount of time to execute, so just calculate this once

         TCHECKPOINT;

         // Set up the session factories so we can be notified when a new connection is received
         if (_factories.GetNumItems() > 0)
         {
            HashtableIterator<uint16, ReflectSessionFactoryRef> iter(_factories);
            ReflectSessionFactoryRef * next; 
            while((next = iter.GetNextValue()) != NULL)
            {
               ReflectSessionFactory * factory = (*next)();
               int nextAcceptSocket = factory->_socket;
               if (nextAcceptSocket >= 0)
               {
                  FD_SET(nextAcceptSocket, &readSet);
                  if (nextAcceptSocket > maxSocket) maxSocket = nextAcceptSocket;
               }
               CallGetPulseTimeAux(*factory, now, nextPulseAt);
            }
         }

         TCHECKPOINT;

         // Set up the sessions, their associated IO-gateways, and their IOPolicies
         if (_sessions.GetNumItems() > 0)
         {
            AbstractReflectSessionRef * nextRef;
            HashtableIterator<const char *, AbstractReflectSessionRef> iter = GetSessions();
            while((nextRef = iter.GetNextValue()) != NULL)
            {
               AbstractReflectSession * session = nextRef->GetItemPointer();
               if (session)
               {
                  session->_maxInputChunk = session->_maxOutputChunk = 0;
                  AbstractMessageIOGateway * g = session->GetGateway();
                  if (g)
                  {
                     int sessionSocket = GetSocketFor(session);
                     if (sessionSocket >= 0)
                     {
                        bool in, out;
                        if (session->_connectingAsync) 
                        {
                           in  = false;
                           out = true;  // so we can watch for the async-connect event

#ifdef WIN32
                           // Under windows, failed asynchronous connect()'s are communicated via the exceptions fd_set
                           if (exceptionSetPtr == NULL)
                           {
                              FD_ZERO(&exceptionSet);
                              exceptionSetPtr = &exceptionSet;
                           }
                           FD_SET(sessionSocket, &exceptionSet);
#endif
                        }
                        else
                        {
                           session->_maxInputChunk  = CheckPolicy(policies, session->GetInputPolicy(),  PolicyHolder(session->IsReadyForInput()  ? session : NULL, true),  now);
                           session->_maxOutputChunk = CheckPolicy(policies, session->GetOutputPolicy(), PolicyHolder(session->HasBytesToOutput() ? session : NULL, false), now);
                           in  = (session->_maxInputChunk  > 0);
                           out = ((session->_maxOutputChunk > 0)||((g->GetDataIO())&&(g->GetDataIO()->HasBufferedOutput())));
                        }

                        if (in) FD_SET(sessionSocket, &readSet);
                        if (out) 
                        {
                           FD_SET(sessionSocket, &writeSet);
                           if (session->_lastByteOutputAt == 0) session->_lastByteOutputAt = now;  // the bogged-session-clock starts ticking when we first want to write...
                           if (session->_outputStallLimit != MUSCLE_TIME_NEVER) nextPulseAt = muscleMin(nextPulseAt, session->_lastByteOutputAt+session->_outputStallLimit);
                        }
                        else session->_lastByteOutputAt = 0;  // If we no longer want to write, then the bogged-session-clock-timeout is cancelled

                        if (((in)||(out))&&(sessionSocket > maxSocket)) maxSocket = sessionSocket;
                     }
                     TCHECKPOINT;
                     CallGetPulseTimeAux(*g, now, nextPulseAt);
                     TCHECKPOINT;
                  }
                  TCHECKPOINT;
                  CallGetPulseTimeAux(*session, now, nextPulseAt);
                  TCHECKPOINT;
               }
            }
         }

         TCHECKPOINT;
         CallGetPulseTimeAux(*this, now, nextPulseAt);
         TCHECKPOINT;

         // Set up the Session IO Policies
         if (policies.GetNumItems() > 0)
         {
            // Now that the policies know *who* amongst their policyholders will be reading/writing,
            // let's ask each activated policy *how much* each policyholder should be allowed to read/write.
            {
               AbstractReflectSessionRef * nextRef;
               HashtableIterator<const char *, AbstractReflectSessionRef> iter = GetSessions();
               while((nextRef = iter.GetNextValue()) != NULL)
               {
                  AbstractReflectSession * session = nextRef->GetItemPointer();
                  if (session)
                  {
                     AbstractSessionIOPolicy * inPolicy  = session->GetInputPolicy()();
                     AbstractSessionIOPolicy * outPolicy = session->GetOutputPolicy()();
                     if ((inPolicy)&&( session->_maxInputChunk  > 0)) session->_maxInputChunk  = inPolicy->GetMaxTransferChunkSize(PolicyHolder(session, true));
                     if ((outPolicy)&&(session->_maxOutputChunk > 0)) session->_maxOutputChunk = outPolicy->GetMaxTransferChunkSize(PolicyHolder(session, false));
                  }
               }
            }

            // Now that all is prepared, calculate all the policies' wakeup times
            {
               TCHECKPOINT;
               HashtableIterator<PolicyRef, bool> iter(policies);
               const PolicyRef * next;
               while((next = iter.GetNextKey()) != NULL) CallGetPulseTimeAux(*(next->GetItemPointer()), now, nextPulseAt);
               TCHECKPOINT;
            }
         }
      }

      TCHECKPOINT;

      // This block is the center of the MUSCLE server's universe -- where we wait for the next event, inside select()
      {
         // Calculate timeout value for the next call to Pulse() (if any)
         struct timeval waitTime;
         if (nextPulseAt != MUSCLE_TIME_NEVER) 
         {
            uint64 now = GetRunTime64();
            uint64 waitTime64 = (nextPulseAt > now) ? (nextPulseAt - now) : 0;
            Convert64ToTimeVal(waitTime64, waitTime);
         }

#ifdef WIN32
         if (maxSocket < 0)
         {
            TCHECKPOINT;

            // Stupid Win32 can't handle it when all three fd_sets are empty!
            // So we have to do some special-case logic for that scenario here
            Sleep((nextPulseAt == MUSCLE_TIME_NEVER) ? INFINITE : ((waitTime.tv_sec*1000)+(waitTime.tv_usec/1000)));
         }
         else
#endif
         {
            TCHECKPOINT;

            // We sleep here until the next I/O or pulse event becomes ready
            if ((select(maxSocket+1, (maxSocket >= 0) ? &readSet : NULL, (maxSocket >= 0) ? &writeSet : NULL, exceptionSetPtr, (nextPulseAt == MUSCLE_TIME_NEVER) ? NULL : &waitTime) < 0)&&(PreviousOperationWasInterrupted() == false))
            {
               if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "select() failed, aborting!\n");
               ClearLameDucks();
               return B_ERROR;
            }
         }
      }

      // Each event-loop cycle officially "starts" as soon as select() returns
      CallSetCycleStartTime(*this, GetRunTime64());

      TCHECKPOINT;

      // Before we do any session I/O, make sure there hasn't been a generalized memory failure
      CheckForOutOfMemory(AbstractReflectSessionRef());

      TCHECKPOINT;

      // Do I/O for each of our attached sessions
      {
         AbstractReflectSessionRef sessionRef;
         HashtableIterator<const char *, AbstractReflectSessionRef> iter = GetSessions();
         while(iter.GetNextValue(sessionRef) == B_NO_ERROR)
         {
            TCHECKPOINT;

            AbstractReflectSession * session = sessionRef();
            if (session)
            {
               if (_watchMemUsage) (void) _watchMemUsage->SetAllocationHasFailed(false);  // (session)'s responsibility for starts here!  If we run out of mem on his watch, he's history

               TCHECKPOINT;

               CallSetCycleStartTime(*session, GetRunTime64());
               TCHECKPOINT;
               CallPulseAux(*session, session->GetCycleStartTime());
               {
                  AbstractMessageIOGateway * gateway = session->GetGateway();
                  if (gateway) 
                  {
                     TCHECKPOINT;

                     CallSetCycleStartTime(*gateway, GetRunTime64());
                     CallPulseAux(*gateway, gateway->GetCycleStartTime());
                  }
               }

               TCHECKPOINT;

               int socket = GetSocketFor(session);
               if (socket >= 0)
               {
                  int32 readBytes = 0, wroteBytes = 0;
                  if (FD_ISSET(socket, &readSet))
                  {
                     readBytes = session->DoInput(*session, session->_maxInputChunk);  // session->MessageReceivedFromGateway() gets called here
                     AbstractSessionIOPolicy * p = session->GetInputPolicy()();
                     if ((p)&&(readBytes >= 0)) p->BytesTransferred(PolicyHolder(session, true), (uint32)readBytes);
                  }

                  TCHECKPOINT;

                  if (FD_ISSET(socket, &writeSet))
                  {
                     if (session->_connectingAsync) wroteBytes = (FinalizeAsyncConnect(sessionRef) == B_NO_ERROR) ? 0 : -1;
                     else
                     {
#ifdef MUSCLE_MAX_OUTPUT_CHUNK
                        // HACK: Certain BeOS configurations (i.e. behind routers??) don't handle large outgoing send()'s very
                        // well.  If this is the case with your machine, one easy fix is to put -DMUSCLE_MAX_OUTPUT_CHUNK=1000
                        // or so in your Makefile; that will force all send sizes to be 1000 bytes or less.
                        // (shatty reports the problem with a net_server -> Ethernet -> linksys -> pppoe -> dsl config)
                        if (session->_maxOutputChunk > MUSCLE_MAX_OUTPUT_CHUNK) session->_maxOutputChunk = MUSCLE_MAX_OUTPUT_CHUNK;
#endif
                        
                        // if the session's DataIO object is still has bytes buffered for output, try to send them now
                        {
                           AbstractMessageIOGateway * g = session->GetGateway();
                           if (g)
                           {
                              DataIO * io = g->GetDataIO();
                              if (io) io->WriteBufferedOutput();
                           }
                        }

                        wroteBytes = session->DoOutput(session->_maxOutputChunk);

                        AbstractSessionIOPolicy * p = session->GetOutputPolicy()();
                        if ((p)&&(wroteBytes >= 0)) p->BytesTransferred(PolicyHolder(session, false), (uint32)wroteBytes);
                     }
                  }
#ifdef WIN32
                  if ((exceptionSetPtr)&&(FD_ISSET(socket, exceptionSetPtr))) wroteBytes = -1;  // async connect() failed!
#endif

                  TCHECKPOINT;

                  if ((readBytes < 0)||(wroteBytes < 0))
                  {
                     bool wasConnecting = session->_connectingAsync;
                     if ((DisconnectSession(session) == false)&&(_doLogging)) LogTime(MUSCLE_LOG_DEBUG, "Connection for %s %s.\n", session->GetSessionDescriptionString()(), wasConnecting?"failed":"was severed");
                  }
                  else if (session->_lastByteOutputAt > 0)
                  {
                     // Check for output stalls
                     const uint64 now = GetRunTime64();
                          if ((wroteBytes > 0)||(session->_maxOutputChunk == 0)) session->_lastByteOutputAt = now;  // reset the moribundness-timer
                     else if (now-session->_lastByteOutputAt > session->_outputStallLimit)
                     {
                        if (_doLogging) LogTime(MUSCLE_LOG_WARNING, "Connection for %s timed out (output stall, no data movement for %llu seconds).\n", session->GetSessionDescriptionString()(), (session->_outputStallLimit/1000000));
                        (void) DisconnectSession(session);
                     }
                  }
               }
            }
            TCHECKPOINT;
            CheckForOutOfMemory(sessionRef);  // if the session caused a memory error, give him the boot
         }
      }

      TCHECKPOINT;

      // Pulse() our other PulseNode objects, as necessary
      {
         // Tell the session policies we're done doing I/O (for now)
         if (policies.GetNumItems() > 0)
         {
            const PolicyRef * next;
            HashtableIterator<PolicyRef, bool> iter(policies);
            while((next = iter.GetNextKey()) != NULL) 
            {
               AbstractSessionIOPolicy * p = next->GetItemPointer();
               if (p->_hasBegun)
               {
                  p->EndIO(GetRunTime64());
                  p->_hasBegun = false;
               }
            }
         }

         // Pulse the Policies
         if (policies.GetNumItems() > 0)
         {
            const PolicyRef * next;
            HashtableIterator<PolicyRef, bool> iter(policies);
            while((next = iter.GetNextKey()) != NULL) CallPulseAux(*(next->GetItemPointer()), GetRunTime64());
         }

         // Pulse the Server
         CallPulseAux(*this, GetRunTime64());
      }
      policies.Clear();

      TCHECKPOINT;

      // Lastly, check our accepting ports to see if anyone is trying to connect...
      if ((_signalCaught == false)&&(_factories.GetNumItems() > 0))  // for some reason Accept() hangs once a signal is caught!?
      {
         HashtableIterator<uint16, ReflectSessionFactoryRef> iter(_factories);
         uint16 port=0;  // just to shut the compiler up
         ReflectSessionFactoryRef * acc;
         while(iter.GetNextKeyAndValue(port, acc) == B_NO_ERROR)
         {
            ReflectSessionFactory * factory = acc->GetItemPointer();
            CallSetCycleStartTime(*factory, GetRunTime64());
            CallPulseAux(*factory, factory->GetCycleStartTime());

            int acceptSocket = factory->_socket;
            if (FD_ISSET(acceptSocket, &readSet)) (void) DoAccept(port, acceptSocket, factory);
         }
      }

      TCHECKPOINT;
   }

   TCHECKPOINT;

   (void) ClearLameDucks();  // get rid of any leftover ducks

   TCHECKPOINT;
   return B_NO_ERROR;
}

void ReflectServer :: ShutdownIOFor(AbstractReflectSession * session)
{
   AbstractMessageIOGateway * gw = session->GetGateway();
   if (gw) 
   {
      DataIO * io = gw->GetDataIO();
      if (io) io->Shutdown();  // so we won't try to do I/O on this one anymore
   }
}

status_t ReflectServer :: ClearLameDucks()
{
   // Delete any factories that were previously marked for deletion
   _lameDuckFactories.Clear();

   // Remove any sessions that were previously marked for removal
   AbstractReflectSessionRef duckRef;
   while(_lameDuckSessions.RemoveHead(duckRef) == B_NO_ERROR)
   {
      AbstractReflectSession * duck = duckRef();
      if (duck)
      {
         const char * id = duck->GetSessionIDString();
         if (_sessions.ContainsKey(id))
         {
            duck->AboutToDetachFromServer();
            duck->DoOutput(MUSCLE_NO_LIMIT);  // one last chance for him to send any leftover data!
            if (_doLogging) LogTime(MUSCLE_LOG_DEBUG, "Closed %s ("UINT32_FORMAT_SPEC" left)\n", duck->GetSessionDescriptionString()(), _sessions.GetNumItems()-1);
            duck->_owner = NULL;
            (void) _sessions.Remove(id);
         }
      }
   }

   return _keepServerGoing ? B_NO_ERROR : B_ERROR;
}

status_t ReflectServer :: DoAccept(uint16 port, int acceptSocket, ReflectSessionFactory * optFactory)
{
   // Accept a new connection and try to start up a session for it
   int newSocket = Accept(acceptSocket);
   if (newSocket >= 0)
   {
      uint32 remoteIP = GetPeerIPAddress(newSocket, true);
      if (remoteIP > 0)
      {
         char ipbuf[16]; Inet_NtoA(remoteIP, ipbuf);
         String remoteIPString = ipbuf;
         AbstractReflectSessionRef newSessionRef(optFactory ? optFactory->CreateSession(remoteIPString) : NULL);
         if (newSessionRef())
         {
            newSessionRef()->_port = port;
            if (AddNewSession(newSessionRef, newSocket) == B_NO_ERROR) return B_NO_ERROR;  // success!
         }
         else if ((optFactory)&&(_doLogging)) LogTime(MUSCLE_LOG_DEBUG, "Session creation denied for [%s] on port %u.\n", remoteIPString(), port);
      }
      else if (_doLogging) LogTime(MUSCLE_LOG_ERROR, "Couldn't get peer IP address for new accept session!\n");

      CloseSocket(newSocket);
   }
   else if (_doLogging) LogTime(MUSCLE_LOG_ERROR, "Accept() failed on port %u (socket %i)\n", port, acceptSocket);

   return B_ERROR;
}

void ReflectServer :: DumpBoggedSessions()
{
   TCHECKPOINT;

   // New for v1.82:  also find anyone whose outgoing message queue is getting too large.
   //                 (where "too large", for now, is more than 5 megabytes)
   // This could happen if someone has a really slow Internet connection, or has decided to
   // stop reading from their end indefinitely for some reason.
   const int MAX_MEGABYTES = 5;
   AbstractReflectSessionRef * lRef;
   HashtableIterator<const char *, AbstractReflectSessionRef> xiter = GetSessions();
   while((lRef = xiter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * asr = lRef->GetItemPointer();
      if (asr)
      {
         AbstractMessageIOGateway * gw = asr->GetGateway();
         if (gw)
         {
            uint32 qSize = 0;
            const Queue<MessageRef> & q = gw->GetOutgoingMessageQueue();
            for (int k=q.GetNumItems()-1; k>=0; k--)
            {
               const Message * qmsg = q[k]();
               if (qmsg) qSize += qmsg->FlattenedSize();
            }
            if (qSize > MAX_MEGABYTES*1024*1024)
            {
               if (_doLogging) LogTime(MUSCLE_LOG_CRITICALERROR, "Low Memory!  Aborting %s to get some back!\n", asr->GetSessionDescriptionString()());
               AddLameDuckSession(*lRef);
            }
         }
      }
   }
}

AbstractReflectSessionRef
ReflectServer ::
GetSession(const char * name) const
{
   AbstractReflectSessionRef ref;
   (void) _sessions.Get(name, ref); 
   return ref;
}

ReflectSessionFactoryRef
ReflectServer ::
GetFactory(uint16 port) const
{
   ReflectSessionFactoryRef ref;
   (void) _factories.Get(port, ref); 
   return ref;
}

status_t
ReflectServer ::
ReplaceSession(const AbstractReflectSessionRef & newSessionRef, AbstractReflectSession * oldSession)
{
   TCHECKPOINT;

   // move the gateway from the old session to the new one...
   AbstractReflectSession * newSession = newSessionRef();
   if (newSession == NULL) return B_ERROR;

   newSession->SetGateway(oldSession->GetGatewayRef());
   newSession->_hostName = oldSession->_hostName;
   newSession->_port     = oldSession->_port;

   if (AddNewSession(newSessionRef) == B_NO_ERROR)
   {
      oldSession->SetGateway(AbstractMessageIOGatewayRef());   /* gateway now belongs to newSession */
      EndSession(oldSession);
      return B_NO_ERROR;
   }
   else
   {
       // Oops, rollback changes and error out
       newSession->SetGateway(AbstractMessageIOGatewayRef());
       newSession->_hostName.Clear();
       newSession->_port = 0; 
       return B_ERROR;
   }
}

bool ReflectServer :: DisconnectSession(AbstractReflectSession * session)
{
   session->_connectingAsync    = false;  // he's not connecting anymore, by gum!
   session->_scratchReconnected = false;  // if the session calls Reconnect() this will be set to true below

   AbstractMessageIOGateway * oldGW = session->GetGateway();
   DataIO * oldIO = oldGW ? oldGW->GetDataIO() : NULL;

   bool ret = session->ClientConnectionClosed(); 

   AbstractMessageIOGateway * newGW = session->GetGateway();
   DataIO * newIO = newGW ? newGW->GetDataIO() : NULL;

        if (ret) AddLameDuckSession(session);
   else if ((session->_scratchReconnected == false)&&(newGW == oldGW)&&(newIO == oldIO)) ShutdownIOFor(session);

   return ret;
}

void
ReflectServer ::
EndSession(AbstractReflectSession * who)
{
   AddLameDuckSession(who);
}

void
ReflectServer ::
EndServer()
{
   _keepServerGoing = false;
}

status_t
ReflectServer ::
PutAcceptFactory(uint16 port, const ReflectSessionFactoryRef & factoryRef)
{
   (void) RemoveAcceptFactory(port); // Get rid of any previous acceptor on this port...

   ReflectSessionFactory * f = factoryRef();
   if (f)
   {
      int acceptSocket = CreateAcceptingSocket(port, 20, &port);
      if (acceptSocket >= 0)
      {
         if (_factories.Put(port, factoryRef) == B_NO_ERROR)
         {
            f->_owner  = this;
            f->_socket = acceptSocket;
            f->_port   = port;
            if (f->AttachedToServer() == B_NO_ERROR) return B_NO_ERROR;
            else
            {
               (void) RemoveAcceptFactory(port);
               return B_ERROR;
            }
         }
         CloseSocket(acceptSocket);
      }
   }
   return B_ERROR;
}

void
ReflectServer ::
RemoveAcceptFactoryAux(const ReflectSessionFactoryRef & ref)
{
   ReflectSessionFactory * factory = ref();
   if (factory) 
   {
      factory->AboutToDetachFromServer();
      CloseSocket(factory->_socket);
      factory->_owner  = NULL;
      factory->_port   = 0;
      factory->_socket = -1;
      _lameDuckFactories.AddTail(ref);  // we'll actually have (factory) deleted later on, since at the moment 
   }                                    // we could be in the middle of (factory)'s own method call!
}

status_t
ReflectServer ::
RemoveAcceptFactory(uint16 port)
{
   if (port > 0)
   {
      ReflectSessionFactoryRef acc;
      if (_factories.Get(port, acc) != B_NO_ERROR) return B_ERROR;

      RemoveAcceptFactoryAux(acc);
      (void) _factories.Remove(port);  // do this after the AboutToDetach callback
   }
   else
   {
      HashtableIterator<uint16, ReflectSessionFactoryRef> iter(_factories);
      uint16 nextKey;
      ReflectSessionFactoryRef nextValue;
      while(iter.GetNextKeyAndValue(nextKey, nextValue) == B_NO_ERROR)
      {
         RemoveAcceptFactoryAux(nextValue);
         _factories.Remove(nextKey);  // make sure nobody accesses the factory after it is detached
      }
   }
   return B_NO_ERROR;
}

status_t 
ReflectServer :: 
FinalizeAsyncConnect(const AbstractReflectSessionRef & ref)
{
   AbstractReflectSession * session = ref();
#ifdef MUSCLE_AVOID_NAMESPACES
   if ((session)&&(::FinalizeAsyncConnect(GetSocketFor(session)) == B_NO_ERROR))
#else
   if ((session)&&(muscle::FinalizeAsyncConnect(GetSocketFor(session)) == B_NO_ERROR))
#endif
   {
      session->_connectingAsync = false;  // we're legit now!  :^)
      session->AsyncConnectCompleted();
      return B_NO_ERROR;
   }
   return B_ERROR;
}

uint64
ReflectServer ::
GetServerUptime() const
{
   uint64 now = GetRunTime64();
   return (_serverStartedAt <= now) ? (now - _serverStartedAt) : 0;
}
                        
uint32 
ReflectServer ::
GetNumAvailableBytes() const
{
   return (_watchMemUsage) ? (uint32)_watchMemUsage->GetNumAvailableBytes(GetNumUsedBytes()) : MUSCLE_NO_LIMIT;
}
 
uint32 
ReflectServer ::
GetMaxNumBytes() const
{
   return _watchMemUsage ? (uint32)_watchMemUsage->GetMaxNumBytes() : MUSCLE_NO_LIMIT;
}
 
uint32 
ReflectServer ::
GetNumUsedBytes() const
{
#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
   return (uint32) GetNumAllocatedBytes();
#else
   return 0;  // if we're not tracking, there is no way to know!
#endif
}

void
ReflectServer :: AddLameDuckSession(const AbstractReflectSessionRef & ref)
{
   if ((_lameDuckSessions.IndexOf(ref) < 0)&&(_lameDuckSessions.AddTail(ref) != B_NO_ERROR)&&(_doLogging)) LogTime(MUSCLE_LOG_CRITICALERROR, "Server:  AddLameDuckSession() failed, I'm REALLY in trouble!  Aggh!\n");
}

void
ReflectServer ::
AddLameDuckSession(AbstractReflectSession * who)
{
   TCHECKPOINT;

   AbstractReflectSessionRef * lRef; 
   HashtableIterator<const char *, AbstractReflectSessionRef> xiter = GetSessions();
   while((lRef = xiter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * session = lRef->GetItemPointer();
      if (session == who)
      {
         AddLameDuckSession(*lRef);
         break;
      }
   }
}

int
ReflectServer :: GetSocketFor(AbstractReflectSession * session) const
{
   AbstractMessageIOGateway * gw = session->GetGateway();
   if (gw)
   {
      DataIO * io = gw->GetDataIO();
      if (io) return io->GetSelectSocket();
   }
   return -1;         
}

class SignalHandler : public PulseNode
{
public:
   SignalHandler() : _server(NULL) {/* empty */}

   virtual uint64 GetPulseTime(uint64 now, uint64)
   {
#if defined(__linux__) || defined(__APPLE__)
      return _server ? (_signalCaught ? 0 : (now+1000000)) : MUSCLE_TIME_NEVER;
#else
      (void) now;
      return MUSCLE_TIME_NEVER;
#endif
   }

   virtual void Pulse(uint64, uint64)
   {
      if ((_signalCaught)&&(_server))
      {
         LogTime(MUSCLE_LOG_INFO, "SignalHandler:  Signal caught, shutting down server!\n");
         _server->EndServer();
      }
   }

   void SetServer(ReflectServer * s) {_server = s;}
   ReflectServer * GetServer() const {return _server;}

private:
   ReflectServer * _server;
};
static SignalHandler _signalHandler;

status_t ReflectServer :: SetSignalHandlingEnabled(bool enabled)
{
   _signalCaught = false;

#if defined(__linux__) || defined(__APPLE__)
   struct sigaction newact;
   sigemptyset(&newact.sa_mask);           /*no other signals blocked*/
   newact.sa_flags=0;                      /*no special options*/

   if (enabled)
   {
      if (_signalHandler.GetServer() == NULL)
      {
         newact.sa_handler = MuscleSignalHandlerCallbackFunc;  /*set the new handler*/
         if (sigaction(SIGINT,  &newact, NULL) == -1) LogTime(MUSCLE_LOG_WARNING, "Couldn't install SIGINT signal handler\n");
         if (sigaction(SIGTERM, &newact, NULL) == -1) LogTime(MUSCLE_LOG_WARNING, "Couldn't install SIGTERM signal handler\n");
         if (sigaction(SIGHUP,  &newact, NULL) == -1) LogTime(MUSCLE_LOG_WARNING, "Couldn't install SIGHUP signal handler\n");
      }
      _signalHandler.SetServer(this);
      PutPulseChild(&_signalHandler);
   }
   else
   {
      RemovePulseChild(&_signalHandler);
      if (_signalHandler.GetServer())
      {
         newact.sa_handler = NULL;
         (void) sigaction(SIGINT,  NULL, NULL);
         (void) sigaction(SIGTERM, NULL, NULL);
         (void) sigaction(SIGHUP,  NULL, NULL);
      }
      _signalHandler.SetServer(NULL);
   }
   return B_NO_ERROR;
#else
   (void) enabled;
   return B_ERROR;
#endif
}

bool WasSignalCaught() {return _signalCaught;}

END_NAMESPACE(muscle);
