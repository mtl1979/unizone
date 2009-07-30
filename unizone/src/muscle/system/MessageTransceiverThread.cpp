/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include "system/MessageTransceiverThread.h"
#include "iogateway/SignalMessageIOGateway.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/ReflectServer.h"

namespace muscle {

static status_t FindIPAddressInMessage(const Message & msg, const String & fieldName, ip_address & ip)
{
   const String * s = NULL;
   if (msg.FindString(fieldName, &s) != B_NO_ERROR) return B_ERROR;

   ip = Inet_AtoN(s->Cstr());
   return B_NO_ERROR;
}

static status_t AddIPAddressToMessage(Message & msg, const String & fieldName, const ip_address & ip)
{
   return msg.AddString(fieldName, Inet_NtoA(ip));
}

MessageTransceiverThread :: MessageTransceiverThread()
{
   // empty
}

MessageTransceiverThread :: ~MessageTransceiverThread()
{
   MASSERT(IsInternalThreadRunning() == false, "You must call ShutdownInternalThread() on a MessageTransceiverThread before deleting it!");
   if (_server()) _server()->Cleanup(); 
}

status_t MessageTransceiverThread :: EnsureServerAllocated()
{
   if (_server() == NULL)
   {
      ReflectServerRef server = CreateReflectServer();
      if (server())
      {
         server()->SetDoLogging(false);
         const ConstSocketRef & socket = GetInternalThreadWakeupSocket();
         if (socket())
         {
            ThreadSupervisorSessionRef controlSession = CreateSupervisorSession();
            if (controlSession())
            {
               controlSession()->_mtt = this;
               controlSession()->SetDefaultDistributionPath(GetDefaultDistributionPath());
               if (server()->AddNewSession(AbstractReflectSessionRef(controlSession.GetRefCountableRef(), true), socket) == B_NO_ERROR)
               {
                  _server = server;
                  return B_NO_ERROR;
               }
            }
            CloseSockets();  // close the other socket too
         }
         server()->Cleanup();
      }
      return B_ERROR;
   }
   return B_NO_ERROR;
}

ReflectServerRef MessageTransceiverThread :: CreateReflectServer()
{
   ReflectServer * rs = newnothrow ReflectServer;
   if (rs == NULL) WARN_OUT_OF_MEMORY;
   return ReflectServerRef(rs);
}

status_t MessageTransceiverThread :: StartInternalThread()
{
   return ((IsInternalThreadRunning() == false)&&(EnsureServerAllocated() == B_NO_ERROR)) ? Thread::StartInternalThread() : B_ERROR;
}

status_t MessageTransceiverThread :: SendMessageToSessions(const MessageRef & userMsg, const char * optPath)
{
   MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_SEND_USER_MESSAGE));
   return ((msgRef())&&(msgRef()->AddMessage(MTT_NAME_MESSAGE, userMsg) == B_NO_ERROR)&&((optPath==NULL)||(msgRef()->AddString(MTT_NAME_PATH, optPath) == B_NO_ERROR))) ? SendMessageToInternalThread(msgRef) : B_ERROR;
}

status_t MessageTransceiverThread :: AddNewSession(const ConstSocketRef & socket, const ThreadWorkerSessionRef & sessionRef)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      ThreadWorkerSessionRef sRef = sessionRef;
      if (sRef() == NULL) sRef = CreateDefaultWorkerSession();
      return (sRef()) ? (IsInternalThreadRunning() ? SendAddNewSessionMessage(sRef, socket, NULL, invalidIP, 0, false, MUSCLE_TIME_NEVER) : _server()->AddNewSession(AbstractReflectSessionRef(sRef.GetRefCountableRef(), false), socket)) : B_ERROR;
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: AddNewConnectSession(const ip_address & targetIPAddress, uint16 port, const ThreadWorkerSessionRef & sessionRef, uint64 autoReconnectDelay)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      ThreadWorkerSessionRef sRef = sessionRef;
      if (sRef() == NULL) sRef = CreateDefaultWorkerSession();
      return (sRef()) ? (IsInternalThreadRunning() ? SendAddNewSessionMessage(sRef, ConstSocketRef(), NULL, targetIPAddress, port, false, autoReconnectDelay) : _server()->AddNewConnectSession(AbstractReflectSessionRef(sRef.GetRefCountableRef(), false), targetIPAddress, port, autoReconnectDelay)) : B_ERROR;
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: AddNewConnectSession(const String & targetHostName, uint16 port, const ThreadWorkerSessionRef & sessionRef, bool expandLocalhost, uint64 autoReconnectDelay)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      ThreadWorkerSessionRef sRef = sessionRef;
      if (sRef() == NULL) sRef = CreateDefaultWorkerSession();
      if (sRef())
      {
         if (IsInternalThreadRunning()) return SendAddNewSessionMessage(sRef, ConstSocketRef(), targetHostName(), 0, port, expandLocalhost, autoReconnectDelay);
         else
         {
            ip_address ip = GetHostByName(targetHostName(), expandLocalhost);
            return (ip != invalidIP) ? _server()->AddNewConnectSession(AbstractReflectSessionRef(sRef.GetRefCountableRef(), true), ip, port, autoReconnectDelay) : B_ERROR;
         }
      }
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: SendAddNewSessionMessage(const ThreadWorkerSessionRef & sessionRef, const ConstSocketRef & socket, const char * hostName, const ip_address & hostIP, uint16 port, bool expandLocalhost, uint64 autoReconnectDelay)
{
   MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_ADD_NEW_SESSION));

   return ((sessionRef())&&(msgRef())&&
       (msgRef()->AddTag(MTT_NAME_SESSION, sessionRef.GetRefCountableRef())                       == B_NO_ERROR) &&
       ((hostName == NULL)||(msgRef()->AddString(MTT_NAME_HOSTNAME,  hostName)                    == B_NO_ERROR))&&
       ((hostIP   == invalidIP)||(AddIPAddressToMessage(*msgRef(), MTT_NAME_IP_ADDRESS, hostIP)   == B_NO_ERROR))&&
       ((port     == 0)   ||(msgRef()->AddInt16(MTT_NAME_PORT,       port)                        == B_NO_ERROR))&&
       ((expandLocalhost == false)||(msgRef()->AddBool(MTT_NAME_EXPANDLOCALHOST, true)            == B_NO_ERROR))&&
       ((socket() == NULL)||(msgRef()->AddTag(MTT_NAME_SOCKET,       CastAwayConstFromRef(socket.GetRefCountableRef())) == B_NO_ERROR))&&
       ((autoReconnectDelay == MUSCLE_TIME_NEVER)||(msgRef()->AddInt64(MTT_NAME_AUTORECONNECTDELAY, autoReconnectDelay) == B_NO_ERROR)))
       ? SendMessageToInternalThread(msgRef) : B_ERROR;
}

status_t MessageTransceiverThread :: PutAcceptFactory(uint16 port, const ThreadWorkerSessionFactoryRef & factoryRef, const ip_address & optInterfaceIP, uint16 * optRetPort)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      ThreadWorkerSessionFactoryRef fRef = factoryRef;
      if (fRef() == NULL) fRef = CreateDefaultSessionFactory();
      if (fRef())
      {
         if (IsInternalThreadRunning())
         {
            MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_PUT_ACCEPT_FACTORY));
            if ((msgRef())&&(msgRef()->AddInt16(MTT_NAME_PORT, port) == B_NO_ERROR)&&(msgRef()->AddTag(MTT_NAME_FACTORY, fRef.GetRefCountableRef()) == B_NO_ERROR)&&(AddIPAddressToMessage(*msgRef(), MTT_NAME_IP_ADDRESS, optInterfaceIP) == B_NO_ERROR)&&(SendMessageToInternalThread(msgRef) == B_NO_ERROR)) return B_NO_ERROR;
         }
         else if (_server()->PutAcceptFactory(port, ReflectSessionFactoryRef(fRef.GetRefCountableRef(), true), optInterfaceIP, optRetPort) == B_NO_ERROR) return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: RemoveAcceptFactory(uint16 port, const ip_address & optInterfaceIP)
{
   if (_server())
   {
      if (IsInternalThreadRunning())
      {
         MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_REMOVE_ACCEPT_FACTORY));
         return ((msgRef())&&(msgRef()->AddInt16(MTT_NAME_PORT, port) == B_NO_ERROR)&&(AddIPAddressToMessage(*msgRef(), MTT_NAME_IP_ADDRESS, optInterfaceIP) == B_NO_ERROR)) ? SendMessageToInternalThread(msgRef) : B_ERROR;
      }
      else return _server()->RemoveAcceptFactory(port, optInterfaceIP);
   }
   else return B_NO_ERROR;  // if there's no server, there's no port
}

status_t MessageTransceiverThread :: SetDefaultDistributionPath(const String & path)
{
   if (_defaultDistributionPath != path)
   {
      if (IsInternalThreadRunning())
      {
         MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_SET_DEFAULT_PATH));
         if ((msgRef() == NULL)||(msgRef()->AddString(MTT_NAME_PATH, path) != B_NO_ERROR)||(SendMessageToInternalThread(msgRef) != B_NO_ERROR)) return B_ERROR;
      }
      _defaultDistributionPath = path;
   }
   return B_NO_ERROR;
}

int32 MessageTransceiverThread :: GetNextEventFromInternalThread(uint32 & code, MessageRef * optRetRef, String * optFromSession, uint32 * optFromFactoryID, IPAddressAndPort * optLocation)
{
   // First, default values for everyone
   if (optRetRef)        optRetRef->Reset();
   if (optFromSession)   optFromSession->Clear();
   if (optFromFactoryID) *optFromFactoryID = 0;

   MessageRef msgRef;
   int32 ret = GetNextReplyFromInternalThread(msgRef);
   if (ret >= 0)
   {
      if (msgRef())
      {
         code = msgRef()->what;
         if (optRetRef)        (void) msgRef()->FindMessage(MTT_NAME_MESSAGE, *optRetRef);
         if (optFromSession)   (void) msgRef()->FindString(MTT_NAME_FROMSESSION, *optFromSession);
         if (optFromFactoryID) (void) msgRef()->FindInt32(MTT_NAME_FACTORY_ID, optFromFactoryID);
         if (optLocation)
         {
            const String * s;
            if (msgRef()->FindString(MTT_NAME_LOCATION, &s) == B_NO_ERROR) optLocation->SetFromString(*s, 0, false);
         }
      }
      else ret = -1;  // NULL event message should never happen, but just in case
   }
   return ret;
}

status_t MessageTransceiverThread :: RequestOutputQueuesDrainedNotification(const MessageRef & notifyRef, const char * optDistPath, DrainTag * optDrainTag)
{
   // Send a command to the supervisor letting him know we are waiting for him to assure
   // us that all the matching worker sessions have dequeued their messages.  Preallocate
   // as much as possible so we won't have to worry about out-of-memory later on, when
   // it's too late to handle it properly.
   MessageRef commandRef = GetMessageFromPool(MTT_COMMAND_NOTIFY_ON_OUTPUT_DRAIN);
   MessageRef replyRef   = GetMessageFromPool(MTT_EVENT_OUTPUT_QUEUES_DRAINED);
   if ((commandRef())&&(replyRef())&&((notifyRef() == NULL)||(replyRef()->AddMessage(MTT_NAME_MESSAGE, notifyRef) == B_NO_ERROR)))
   {
      DrainTagRef drainTagRef(optDrainTag ? optDrainTag : newnothrow DrainTag);
      if (drainTagRef()) drainTagRef()->SetReplyMessage(replyRef);
      RefCountableRef genericRef = drainTagRef.GetRefCountableRef();
      if ((drainTagRef())&&
          ((optDistPath == NULL)||(commandRef()->AddString(MTT_NAME_PATH, optDistPath) == B_NO_ERROR))&&
          (commandRef()->AddTag(MTT_NAME_DRAIN_TAG, genericRef)                        == B_NO_ERROR)&&
          (SendMessageToInternalThread(commandRef)                                     == B_NO_ERROR)) return B_NO_ERROR;

      // User keeps ownership of his custom DrainTag on error, so we don't delete it.
      if ((drainTagRef())&&(drainTagRef() == optDrainTag)) 
      {
         drainTagRef()->SetReplyMessage(MessageRef());
         drainTagRef.Neutralize();
         genericRef.Neutralize();
      }
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: SetNewInputPolicy(const AbstractSessionIOPolicyRef & pref, const char * optDistPath)
{
   return SetNewPolicyAux(MTT_COMMAND_SET_INPUT_POLICY, pref, optDistPath);
}

status_t MessageTransceiverThread :: SetNewOutputPolicy(const AbstractSessionIOPolicyRef & pref, const char * optDistPath)
{
   return SetNewPolicyAux(MTT_COMMAND_SET_OUTPUT_POLICY, pref, optDistPath);
}

status_t MessageTransceiverThread :: SetNewPolicyAux(uint32 what, const AbstractSessionIOPolicyRef & pref, const char * optDistPath)    
{
   MessageRef commandRef = GetMessageFromPool(what);
   return ((commandRef())&&
           ((optDistPath == NULL)||(commandRef()->AddString(MTT_NAME_PATH, optDistPath)     == B_NO_ERROR))&&
           ((pref() == NULL)||(commandRef()->AddTag(MTT_NAME_POLICY_TAG, pref.GetRefCountableRef()) == B_NO_ERROR)))
           ? SendMessageToInternalThread(commandRef) : B_ERROR;
}

status_t MessageTransceiverThread :: SetOutgoingMessageEncoding(int32 encoding, const char * optDistPath)
{
   MessageRef commandRef = GetMessageFromPool(MTT_COMMAND_SET_OUTGOING_ENCODING);
   return ((commandRef())&&
           ((optDistPath == NULL)||(commandRef()->AddString(MTT_NAME_PATH, optDistPath) == B_NO_ERROR))&&
            (commandRef()->AddInt32(MTT_NAME_ENCODING, encoding)                        == B_NO_ERROR))
           ? SendMessageToInternalThread(commandRef) : B_ERROR;
}

status_t MessageTransceiverThread :: RemoveSessions(const char * optDistPath)
{
   MessageRef commandRef = GetMessageFromPool(MTT_COMMAND_REMOVE_SESSIONS);
   return ((commandRef())&&((optDistPath == NULL)||(commandRef()->AddString(MTT_NAME_PATH, optDistPath) == B_NO_ERROR))) ? SendMessageToInternalThread(commandRef) : B_ERROR;
}

void MessageTransceiverThread :: Reset()
{
   ShutdownInternalThread();
   if (_server())
   {
      _server()->Cleanup();
      _server.Reset();
   }
   
   // Clear both message queues of any leftover messages.
   MessageRef junk;
   while(WaitForNextMessageFromOwner(junk, 0) >= 0) {/* empty */}
   while(GetNextReplyFromInternalThread(junk) >= 0) {/* empty */}
}

ThreadSupervisorSessionRef MessageTransceiverThread :: CreateSupervisorSession()
{
   ThreadSupervisorSession * ret = newnothrow ThreadSupervisorSession();
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return ThreadSupervisorSessionRef(ret);
}

ThreadWorkerSessionRef MessageTransceiverThread :: CreateDefaultWorkerSession()
{
   ThreadWorkerSession * ret = newnothrow ThreadWorkerSession();
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return ThreadWorkerSessionRef(ret);
}

ThreadWorkerSessionFactoryRef MessageTransceiverThread :: CreateDefaultSessionFactory()
{
   ThreadWorkerSessionFactory * ret = newnothrow ThreadWorkerSessionFactory();
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return ThreadWorkerSessionFactoryRef(ret);
}

ThreadWorkerSessionFactory :: ThreadWorkerSessionFactory()
{
   // empty
}

status_t ThreadWorkerSessionFactory :: AttachedToServer()
{
   if (StorageReflectSessionFactory::AttachedToServer() == B_NO_ERROR)
   {
      BroadcastToAllSessions(GetMessageFromPool(MTT_EVENT_FACTORY_ATTACHED));
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

void ThreadWorkerSessionFactory :: AboutToDetachFromServer()
{
   BroadcastToAllSessions(GetMessageFromPool(MTT_EVENT_FACTORY_DETACHED));
   StorageReflectSessionFactory::AboutToDetachFromServer();
}

ThreadWorkerSessionRef ThreadWorkerSessionFactory :: CreateThreadWorkerSession(const String &, const IPAddressAndPort &)
{
   ThreadWorkerSession * ret = newnothrow ThreadWorkerSession();
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return ThreadWorkerSessionRef(ret);
}

AbstractReflectSessionRef ThreadWorkerSessionFactory :: CreateSession(const String & clientHostIP, const IPAddressAndPort & iap)
{
   ThreadWorkerSessionRef tws = CreateThreadWorkerSession(clientHostIP, iap);
   if ((tws())&&(SetMaxIncomingMessageSizeFor(tws()) == B_NO_ERROR))
   {
      tws()->_acceptedIAP = iap;  // gotta send the MTT_EVENT_SESSION_ACCEPTED Message from within AttachedToServer()
      return AbstractReflectSessionRef(tws.GetRefCountableRef(), true);
   }
   return AbstractReflectSessionRef();
}

void MessageTransceiverThread :: InternalThreadEntry()
{
   if (_server()) 
   {
      (void) _server()->ServerProcessLoop();
      _server()->Cleanup();
   }
   SendMessageToOwner(GetMessageFromPool(MTT_EVENT_SERVER_EXITED));
}

ThreadWorkerSession :: ThreadWorkerSession()
{
   // empty
}

ThreadWorkerSession :: ~ThreadWorkerSession()
{
   // empty
}

void ThreadWorkerSession :: AsyncConnectCompleted()
{
   StorageReflectSession::AsyncConnectCompleted();

   MessageRef msg = GetMessageFromPool(MTT_EVENT_SESSION_CONNECTED);
   if ((msg())&&(msg()->AddString(MTT_NAME_LOCATION, IPAddressAndPort(GetAsyncConnectIP(), GetAsyncConnectPort()).ToString()) == B_NO_ERROR)) BroadcastToAllSessions(msg);
}

status_t ThreadWorkerSession :: AttachedToServer()
{
   if (StorageReflectSession::AttachedToServer() == B_NO_ERROR)
   {
      if (_acceptedIAP.IsValid())
      {
         MessageRef msg = GetMessageFromPool(MTT_EVENT_SESSION_ACCEPTED);
         if ((msg())&&(msg()->AddString(MTT_NAME_LOCATION, _acceptedIAP.ToString()) == B_NO_ERROR)) BroadcastToAllSessions(msg);
      }
      BroadcastToAllSessions(GetMessageFromPool(MTT_EVENT_SESSION_ATTACHED));
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

bool ThreadWorkerSession :: ClientConnectionClosed()
{
   BroadcastToAllSessions(GetMessageFromPool(MTT_EVENT_SESSION_DISCONNECTED));
   _drainedNotifiers.Clear();
   return StorageReflectSession::ClientConnectionClosed();
}

void ThreadWorkerSession :: AboutToDetachFromServer()
{
   BroadcastToAllSessions(GetMessageFromPool(MTT_EVENT_SESSION_DETACHED));
   _drainedNotifiers.Clear();
   StorageReflectSession::AboutToDetachFromServer();
}

int32 ThreadWorkerSession :: DoOutput(uint32 maxBytes)
{
   int32 ret = StorageReflectSession::DoOutput(maxBytes);
   if (_drainedNotifiers.HasItems())
   {
      AbstractMessageIOGateway * gw = GetGateway()();
      if ((gw == NULL)||(gw->HasBytesToOutput() == false)) _drainedNotifiers.Clear();
   }
   return ret;
}

void ThreadWorkerSession :: MessageReceivedFromGateway(const MessageRef & msg, void * userData)
{
   // Wrap it up so the supervisor knows its for him, and send it out
   MessageRef wrapper = GetMessageFromPool(MTT_EVENT_INCOMING_MESSAGE);
   if ((wrapper())&&(wrapper()->AddMessage(MTT_NAME_MESSAGE, msg) == B_NO_ERROR)) BroadcastToAllSessions(wrapper, userData);
}

void ThreadWorkerSession :: MessageReceivedFromSession(AbstractReflectSession & from, const MessageRef & msgRef, void * userData)
{
   const Message * msg = msgRef();
   if (msg)
   {
      if ((msg->what >= MTT_COMMAND_SEND_USER_MESSAGE)&&(msg->what <= MTT_LAST_COMMAND))
      {
         switch(msg->what)
         {
            case MTT_COMMAND_NOTIFY_ON_OUTPUT_DRAIN:
            {
               RefCountableRef genericRef;
               if (msg->FindTag(MTT_NAME_DRAIN_TAG, genericRef) == B_NO_ERROR)
               {
                  DrainTagRef drainTagRef(genericRef, true);
                  if (drainTagRef())
                  {
                     // Add our session ID so that the supervisor session will know we received the drain tag
                     Message * rmsg = drainTagRef()->GetReplyMessage()();
                     if (rmsg) rmsg->AddString(MTT_NAME_FROMSESSION, GetSessionRootPath());

                     // If we have any messages pending, we'll save this message reference until our
                     // outgoing message queue becomes empty.  That way the DrainTag item held by the 
                     // referenced message won't be deleted until the appropriate time, and hence
                     // the supervisor won't be notified until all the specified queues have drained.
                     AbstractMessageIOGateway * gw = GetGateway()();
                     if ((gw)&&(gw->HasBytesToOutput())) _drainedNotifiers.AddTail(drainTagRef);
                  }
               }
            }
            break;

            case MTT_COMMAND_SEND_USER_MESSAGE:
            {
               MessageRef userMsg;
               if (msg->FindMessage(MTT_NAME_MESSAGE, userMsg) == B_NO_ERROR) AddOutgoingMessage(userMsg);
            }
            break;

            case MTT_COMMAND_SET_INPUT_POLICY:
            case MTT_COMMAND_SET_OUTPUT_POLICY:
            {
               RefCountableRef tagRef;
               (void) msg->FindTag(MTT_NAME_POLICY_TAG, tagRef);
               AbstractSessionIOPolicyRef pref(tagRef, true);
               if (msg->what == MTT_COMMAND_SET_INPUT_POLICY) SetInputPolicy(pref);
                                                         else SetOutputPolicy(pref);
            }
            break;

            case MTT_COMMAND_SET_OUTGOING_ENCODING:
            {
               int32 enc;
               if (msg->FindInt32(MTT_NAME_ENCODING, enc) == B_NO_ERROR)
               {
                  MessageIOGateway * gw = dynamic_cast<MessageIOGateway *>(GetGateway()());
                  if (gw) gw->SetOutgoingEncoding(enc);
               }
            }
            break;

            case MTT_COMMAND_REMOVE_SESSIONS:
               EndSession();
            break;
         }
      }
      else if ((msg->what >= MTT_EVENT_INCOMING_MESSAGE)&&(msg->what <= MTT_LAST_EVENT)) 
      {
         // ignore these; we don't care about silly MTT_EVENTS, those are for the supervisor and the user
      }
      else StorageReflectSession::MessageReceivedFromSession(from, msgRef, userData);
   }
}

ThreadSupervisorSession :: ThreadSupervisorSession()
{
   // empty
}

ThreadSupervisorSession :: ~ThreadSupervisorSession()
{
   // empty
}

void ThreadSupervisorSession :: AboutToDetachFromServer()
{
   // Neutralize all outstanding DrainTrags so that they won't try to call DrainTagIsBeingDeleted() on me after I'm gone.
   HashtableIterator<DrainTag *, bool> tagIter(_drainTags);
   DrainTag * nextKey = NULL;  // just to shut the compiler up
   while(tagIter.GetNextKey(nextKey) == B_NO_ERROR) nextKey->SetNotify(NULL);

   StorageReflectSession :: AboutToDetachFromServer();
}

void ThreadSupervisorSession :: DrainTagIsBeingDeleted(DrainTag * tag)
{
   if (_drainTags.Remove(tag) == B_NO_ERROR) _mtt->SendMessageToOwner(tag->GetReplyMessage());
}

AbstractMessageIOGatewayRef ThreadSupervisorSession :: CreateGateway()
{
   AbstractMessageIOGateway * gw = newnothrow SignalMessageIOGateway();
   if (gw == NULL) WARN_OUT_OF_MEMORY;
   return AbstractMessageIOGatewayRef(gw);
}

void ThreadSupervisorSession :: MessageReceivedFromGateway(const MessageRef &, void *)
{
   // The message from the gateway is merely a signal that we should check
   // the message queue from the main thread again, to see if there are
   // new messages from our owner waiting.  So we'll do that here.
   MessageRef msgFromOwner;
   int32 numLeft;
   while((numLeft = _mtt->WaitForNextMessageFromOwner(msgFromOwner, 0)) >= 0)
   {
      if (msgFromOwner()) MessageReceivedFromOwner(msgFromOwner, (uint32)numLeft);
      else
      {
         EndServer();  // this will cause our thread to exit
         break;
      }
   }
}

void ThreadSupervisorSession :: MessageReceivedFromSession(AbstractReflectSession & from, const MessageRef & msgRef, void *)
{
   if (msgRef()) msgRef()->AddString(MTT_NAME_FROMSESSION, from.GetSessionRootPath());
   _mtt->SendMessageToOwner(msgRef);
}

void ThreadSupervisorSession :: MessageReceivedFromFactory(ReflectSessionFactory & from, const MessageRef & msgRef, void *)
{
   if (msgRef()) msgRef()->AddInt32(MTT_NAME_FACTORY_ID, from.GetFactoryID());
   _mtt->SendMessageToOwner(msgRef);
}

bool ThreadSupervisorSession :: ClientConnectionClosed()
{
   EndServer();
   return StorageReflectSession::ClientConnectionClosed();
}

status_t ThreadSupervisorSession :: AddNewWorkerConnectSession(const ThreadWorkerSessionRef & sessionRef, const ip_address & hostIP, uint16 port, uint64 autoReconnectDelay)
{
   status_t ret = (hostIP != invalidIP) ? AddNewConnectSession(AbstractReflectSessionRef(sessionRef.GetRefCountableRef(), true), hostIP, port, autoReconnectDelay) : B_ERROR;

   // For immediate failure: Since (sessionRef) never attached, we need to send the disconnect message ourself.
   if ((ret != B_NO_ERROR)&&(sessionRef()))
   {
      // We have to synthesize the MTT_NAME_FROMSESSION path ourselves, since the session was never added to the server and thus its path isn't set
      MessageRef errorMsg = GetMessageFromPool(MTT_EVENT_SESSION_DISCONNECTED);
      if ((errorMsg())&&(errorMsg()->AddString(MTT_NAME_FROMSESSION, String("/%1/%2").Arg(Inet_NtoA(hostIP)).Arg(sessionRef()->GetSessionID())) == B_NO_ERROR)) _mtt->SendMessageToOwner(errorMsg);
   }
   return ret;
}

void ThreadSupervisorSession :: DistributeMessageToWorkers(const MessageRef & distMsg)
{
   String distPath;
   SendMessageToMatchingSessions(distMsg, (distMsg()->FindString(MTT_NAME_PATH, distPath) == B_NO_ERROR) ? distPath : _defaultDistributionPath, QueryFilterRef(), false); 
}

status_t ThreadSupervisorSession :: MessageReceivedFromOwner(const MessageRef & msgRef, uint32)
{
   const Message * msg = msgRef();
   if (msg)
   {
      if (muscleInRange(msg->what, (uint32) MTT_COMMAND_SEND_USER_MESSAGE, (uint32) (MTT_LAST_COMMAND-1)))
      {
         switch(msg->what)
         {
            case MTT_COMMAND_ADD_NEW_SESSION:
            {
               RefCountableRef tagRef;
               if (msg->FindTag(MTT_NAME_SESSION, tagRef) == B_NO_ERROR)
               {
                  ThreadWorkerSessionRef sessionRef(tagRef, true);
                  if (sessionRef())
                  {
                     const char * hostName;
                     ip_address hostIP;
                     uint16 port = 0; (void) msg->FindInt16(MTT_NAME_PORT, port);
                     uint64 autoReconnectDelay = MUSCLE_TIME_NEVER; (void) msg->FindInt64(MTT_NAME_AUTORECONNECTDELAY, autoReconnectDelay);

                          if (FindIPAddressInMessage(*msg, MTT_NAME_IP_ADDRESS, hostIP) == B_NO_ERROR) (void) AddNewWorkerConnectSession(sessionRef, hostIP, port, autoReconnectDelay);
                     else if (msg->FindString(MTT_NAME_HOSTNAME, &hostName)             == B_NO_ERROR) 
                     {
                        bool expandLocalhost = false; (void) msg->FindBool(MTT_NAME_EXPANDLOCALHOST, &expandLocalhost);
                        (void) AddNewWorkerConnectSession(sessionRef, GetHostByName(hostName, expandLocalhost), port, autoReconnectDelay);
                     }
                     else
                     {
                        RefCountableRef genericRef; (void) msg->FindTag(MTT_NAME_SOCKET, genericRef);
                        (void) AddNewSession(AbstractReflectSessionRef(sessionRef.GetRefCountableRef(), true), ConstSocketRef(genericRef, true));
                     }
                  }
                  else LogTime(MUSCLE_LOG_ERROR, "MTT_COMMAND_PUT_ACCEPT_FACTORY:  Couldn't get Session!\n");
               }
            }
            break;

            case MTT_COMMAND_PUT_ACCEPT_FACTORY:
            {
               RefCountableRef tagRef;
               if (msg->FindTag(MTT_NAME_FACTORY, tagRef) == B_NO_ERROR)
               {
                  ThreadWorkerSessionFactoryRef factoryRef(tagRef, true);
                  if (factoryRef())
                  {
                     uint16 port = 0; (void) msg->FindInt16(MTT_NAME_PORT, port);
                     ip_address ip = invalidIP; (void) FindIPAddressInMessage(*msg, MTT_NAME_IP_ADDRESS, ip);
                     (void) PutAcceptFactory(port, ReflectSessionFactoryRef(factoryRef.GetRefCountableRef(), true), ip);
                  }
                  else LogTime(MUSCLE_LOG_ERROR, "MTT_COMMAND_PUT_ACCEPT_FACTORY:  Couldn't get ReflectSessionFactory!\n");
               }
            }
            break;

            case MTT_COMMAND_REMOVE_ACCEPT_FACTORY:
            {
               uint16 port;
               ip_address ip;
               if ((msg->FindInt16(MTT_NAME_PORT, port) == B_NO_ERROR)&&(FindIPAddressInMessage(*msg, MTT_NAME_IP_ADDRESS, ip) == B_NO_ERROR)) (void) RemoveAcceptFactory(port, ip);
            }
            break;

            case MTT_COMMAND_SET_DEFAULT_PATH:
            {
               String dpath;
               (void) msg->FindString(MTT_NAME_PATH, dpath);
               SetDefaultDistributionPath(dpath);
            }
            break;

            case MTT_COMMAND_NOTIFY_ON_OUTPUT_DRAIN:
            {
               RefCountableRef genericRef;
               if (msg->FindTag(MTT_NAME_DRAIN_TAG, genericRef) == B_NO_ERROR)
               {
                  DrainTagRef drainTagRef(genericRef, true);
                  if ((drainTagRef())&&(_drainTags.Put(drainTagRef(), true) == B_NO_ERROR))
                  {
                     drainTagRef()->SetNotify(this);
                     DistributeMessageToWorkers(msgRef);

                     // Check the tag to see if anyone got it.  If not, we'll add the
                     // PR_NAME_KEY string to the reply field, to give the user thread
                     // a hint about which handler the reply should be directed back to.
                     Message * rmsg = drainTagRef()->GetReplyMessage()();
                     if ((rmsg)&&(rmsg->HasName(MTT_NAME_FROMSESSION) == false))
                     {
                        String t;
                        if (msg->FindString(MTT_NAME_PATH, t) == B_NO_ERROR) (void) rmsg->AddString(MTT_NAME_FROMSESSION, t);
                     }
                  }
               }
            }
            break;

            default:
               DistributeMessageToWorkers(msgRef);
            break;
         }
      } 
      else StorageReflectSession::MessageReceivedFromGateway(msgRef, NULL);

      return B_NO_ERROR;
   }
   else return B_ERROR;
}

DrainTag :: ~DrainTag() 
{
   if (_notify) _notify->DrainTagIsBeingDeleted(this);
}

}; // end namespace muscle
