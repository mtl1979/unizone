/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "system/MessageTransceiverThread.h"
#include "iogateway/SignalMessageIOGateway.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/ReflectServer.h"
#include "util/SocketHolder.h"

namespace muscle {

MessageTransceiverThread :: MessageTransceiverThread() : _server(NULL)
{
   // empty
}

MessageTransceiverThread :: ~MessageTransceiverThread()
{
   MASSERT(IsInternalThreadRunning() == false, "You must call ShutdownInternalThread() on a MessageTransceiverThread before deleting it!");
   if (_server)
   {
      _server->Cleanup(); 
      delete _server;
   }
}

status_t MessageTransceiverThread :: EnsureServerAllocated()
{
   if (_server == NULL)
   {
      ReflectServer * server = CreateReflectServer();
      if (server)
      {
         server->SetDoLogging(false);
         int socket = GetInternalThreadWakeupSocket();
         if (socket >= 0)
         {
            ThreadSupervisorSession * controlSession = CreateSupervisorSession();
            if (controlSession)
            {
               controlSession->_mtt = this;
               controlSession->SetDefaultDistributionPath(GetDefaultDistributionPath());
               if (server->AddNewSession(AbstractReflectSessionRef(controlSession, NULL), socket) == B_NO_ERROR) 
               {
                  SetOkayToCloseInternalThreadWakeupSocket(false);  // server owns it now
                  _server = server;
                  return B_NO_ERROR;
               }
            }
            CloseSockets();  // close the other socket too
         }
         delete server;
      }
      return B_ERROR;
   }
   return B_NO_ERROR;
}

ReflectServer * MessageTransceiverThread :: CreateReflectServer()
{
   return newnothrow ReflectServer;
}

status_t MessageTransceiverThread :: StartInternalThread()
{
   return ((IsInternalThreadRunning() == false)&&(EnsureServerAllocated() == B_NO_ERROR)) ? Thread::StartInternalThread() : B_ERROR;
}

status_t MessageTransceiverThread :: SendMessageToSessions(MessageRef userMsg, const char * optPath)
{
   MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_SEND_USER_MESSAGE));
   return ((msgRef())&&(msgRef()->AddMessage(MTT_NAME_MESSAGE, userMsg) == B_NO_ERROR)&&((optPath==NULL)||(msgRef()->AddString(MTT_NAME_PATH, optPath) == B_NO_ERROR))) ? SendMessageToInternalThread(msgRef) : B_ERROR;
}

status_t MessageTransceiverThread :: AddNewSession(int socket, AbstractReflectSessionRef sessionRef)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      if (sessionRef() == NULL) sessionRef.SetRef(CreateDefaultWorkerSession(), NULL);
      return (sessionRef()) ? (IsInternalThreadRunning() ? SendAddNewSessionMessage(sessionRef, socket, NULL, 0, 0) : _server->AddNewSession(sessionRef, socket)) : B_ERROR;
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: AddNewConnectSession(uint32 targetIPAddress, uint16 port, AbstractReflectSessionRef sessionRef)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      if (sessionRef() == NULL) sessionRef.SetRef(CreateDefaultWorkerSession(), NULL);
      return (sessionRef()) ? (IsInternalThreadRunning() ? SendAddNewSessionMessage(sessionRef, -1, NULL, targetIPAddress, port) : _server->AddNewConnectSession(sessionRef, targetIPAddress, port)) : B_ERROR;
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef sessionRef)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      if (sessionRef() == NULL) sessionRef.SetRef(CreateDefaultWorkerSession(), NULL);
      if (sessionRef())
      {
         if (IsInternalThreadRunning())
         {
            return SendAddNewSessionMessage(sessionRef, -1, targetHostName(), 0, port);
         }
         else
         {
            uint32 ip = GetHostByName(targetHostName());
            return (ip > 0) ? _server->AddNewConnectSession(sessionRef, ip, port) : B_ERROR;
         }
      }
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: SendAddNewSessionMessage(AbstractReflectSessionRef sessionRef, int socket, const char * hostName, uint32 hostIP, uint16 port)
{
   MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_ADD_NEW_SESSION));
   SocketHolderRef socketRef((socket >= 0) ? newnothrow SocketHolder(socket) : NULL, NULL);

   if ((sessionRef())&&(msgRef())&&((socket < 0)||(socketRef()))&&
       (msgRef()->AddTag(MTT_NAME_SESSION, sessionRef.GetGeneric())                          == B_NO_ERROR) &&
       ((hostName == NULL)||(msgRef()->AddString(MTT_NAME_HOSTNAME,  hostName)               == B_NO_ERROR))&&
       ((hostIP   == 0)   ||(msgRef()->AddInt32(MTT_NAME_IP_ADDRESS, hostIP)                 == B_NO_ERROR))&&
       ((port     == 0)   ||(msgRef()->AddInt16(MTT_NAME_PORT,       port)                   == B_NO_ERROR))&&
       ((socket   < 0)    ||(msgRef()->AddTag(MTT_NAME_SOCKET,       socketRef.GetGeneric()) == B_NO_ERROR))&&
       (SendMessageToInternalThread(msgRef) == B_NO_ERROR)) return B_NO_ERROR;

   if (socketRef()) socketRef()->ReleaseSocket();  // so we won't close the user's socket on error
   return B_ERROR;
}

status_t MessageTransceiverThread :: PutAcceptFactory(uint16 port, ReflectSessionFactoryRef factoryRef)
{
   if (EnsureServerAllocated() == B_NO_ERROR)
   {
      if (factoryRef() == NULL) factoryRef.SetRef(CreateDefaultSessionFactory(), NULL);
      if (factoryRef())
      {
         if (IsInternalThreadRunning())
         {
            MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_PUT_ACCEPT_FACTORY));
            if ((msgRef())&&(msgRef()->AddInt16(MTT_NAME_PORT, port) == B_NO_ERROR)&&(msgRef()->AddTag(MTT_NAME_FACTORY, factoryRef.GetGeneric()) == B_NO_ERROR)&&(SendMessageToInternalThread(msgRef) == B_NO_ERROR)) return B_NO_ERROR;
         }
         else if (_server->PutAcceptFactory(port, factoryRef) == B_NO_ERROR) return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t MessageTransceiverThread :: RemoveAcceptFactory(uint16 port)
{
   if (_server)
   {
      if (IsInternalThreadRunning())
      {
         MessageRef msgRef(GetMessageFromPool(MTT_COMMAND_REMOVE_ACCEPT_FACTORY));
         return ((msgRef())&&(msgRef()->AddInt16(MTT_NAME_PORT, port) == B_NO_ERROR)) ? SendMessageToInternalThread(msgRef) : B_ERROR;
      }
      else return _server->RemoveAcceptFactory(port);
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

int32 MessageTransceiverThread :: GetNextEventFromInternalThread(uint32 & code, MessageRef * optRetRef, String * optFromSession, uint16 * optFromPort)
{
   // First, default values for everyone
   if (optRetRef)      optRetRef->Reset();
   if (optFromSession) *optFromSession = "";
   if (optFromPort)    *optFromPort    = 0;

   MessageRef msgRef;
   int32 ret = GetNextReplyFromInternalThread(msgRef);
   if (ret >= 0)
   {
      if (msgRef())
      {
         code = msgRef()->what;
         if (optRetRef)      (void) msgRef()->FindMessage(MTT_NAME_MESSAGE, *optRetRef);
         if (optFromSession) (void) msgRef()->FindString(MTT_NAME_FROMSESSION, *optFromSession);
         if (optFromPort)    (void) msgRef()->FindInt16(MTT_NAME_PORT, (int16 *)optFromPort);
      }
      else ret = -1;  // NULL event message should never happen, but just in case
   }
   return ret;
}

status_t MessageTransceiverThread :: RequestOutputQueuesDrainedNotification(MessageRef notifyRef, const char * optDistPath, DrainTag * optDrainTag)
{
   // Send a command to the supervisor letting him know we are waiting for him to assure
   // us that all the matching worker sessions have dequeued their messages.  Preallocate
   // as much as possible so we won't have to worry about out-of-memory later on, when
   // it's too late to handle it properly.
   MessageRef commandRef = GetMessageFromPool(MTT_COMMAND_NOTIFY_ON_OUTPUT_DRAIN);
   MessageRef replyRef   = GetMessageFromPool(MTT_EVENT_OUTPUT_QUEUES_DRAINED);
   if ((commandRef())&&(replyRef())&&((notifyRef() == NULL)||(replyRef()->AddMessage(MTT_NAME_MESSAGE, notifyRef) == B_NO_ERROR)))
   {
      DrainTagRef drainTagRef(optDrainTag ? optDrainTag : newnothrow DrainTag, NULL);
      if (drainTagRef()) drainTagRef()->SetReplyMessage(replyRef);
      GenericRef genericRef = drainTagRef.GetGeneric();
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

status_t MessageTransceiverThread :: SetNewInputPolicy(PolicyRef pref, const char * optDistPath)
{
   return SetNewPolicyAux(MTT_COMMAND_SET_INPUT_POLICY, pref, optDistPath);
}

status_t MessageTransceiverThread :: SetNewOutputPolicy(PolicyRef pref, const char * optDistPath)
{
   return SetNewPolicyAux(MTT_COMMAND_SET_OUTPUT_POLICY, pref, optDistPath);
}

status_t MessageTransceiverThread :: SetNewPolicyAux(uint32 what, PolicyRef pref, const char * optDistPath)    
{
   MessageRef commandRef = GetMessageFromPool(what);
   return ((commandRef())&&
           ((optDistPath == NULL)||(commandRef()->AddString(MTT_NAME_PATH, optDistPath)     == B_NO_ERROR))&&
           ((pref() == NULL)||(commandRef()->AddTag(MTT_NAME_POLICY_TAG, pref.GetGeneric()) == B_NO_ERROR)))
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
   if (_server)
   {
      _server->Cleanup();
      delete _server;
      _server = NULL;
   }
   
   // Clear both message queues of any leftover messages.
   MessageRef junk;
   while(WaitForNextMessageFromOwner(junk, 0) >= 0) {/* empty */}
   while(GetNextReplyFromInternalThread(junk) >= 0) {/* empty */}
}

ThreadSupervisorSession * MessageTransceiverThread :: CreateSupervisorSession()
{
   ThreadSupervisorSession * ret = newnothrow ThreadSupervisorSession();
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return ret;
}

AbstractReflectSession * MessageTransceiverThread :: CreateDefaultWorkerSession()
{
   AbstractReflectSession * ret = newnothrow ThreadWorkerSession();
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return ret;
}

ReflectSessionFactory * MessageTransceiverThread :: CreateDefaultSessionFactory()
{
   ReflectSessionFactory * ret = newnothrow ThreadWorkerSessionFactory();
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return ret;
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

AbstractReflectSession * ThreadWorkerSessionFactory :: CreateSession(const String &)
{
   ThreadWorkerSession * ret = newnothrow ThreadWorkerSession();
   if ((ret)&&(SetMaxIncomingMessageSizeFor(ret) == B_NO_ERROR))
   {
      MessageRef notify(GetMessageFromPool(MTT_EVENT_SESSION_ACCEPTED));
      if (notify()) 
      {
         notify()->AddString(MTT_NAME_FROMSESSION, ret->GetSessionIDString());
         BroadcastToAllSessions(notify);
         return ret;
      }
   }
   else WARN_OUT_OF_MEMORY;

   delete ret;
   return NULL;
}

void MessageTransceiverThread :: InternalThreadEntry()
{
   if (_server) 
   {
      (void) _server->ServerProcessLoop();
      _server->Cleanup();
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
   BroadcastToAllSessions(GetMessageFromPool(MTT_EVENT_SESSION_CONNECTED));
}

status_t ThreadWorkerSession :: AttachedToServer()
{
   if (StorageReflectSession::AttachedToServer() == B_NO_ERROR)
   {
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
   if (_drainedNotifiers.GetNumItems() > 0)
   {
      AbstractMessageIOGateway * gw = GetGateway();
      if ((gw == NULL)||(gw->HasBytesToOutput() == false)) _drainedNotifiers.Clear();
   }
   return ret;
}

void ThreadWorkerSession :: MessageReceivedFromGateway(MessageRef msg, void * userData)
{
   // Wrap it up so the supervisor knows its for him, and send it out
   MessageRef wrapper = GetMessageFromPool(MTT_EVENT_INCOMING_MESSAGE);
   if ((wrapper())&&(wrapper()->AddMessage(MTT_NAME_MESSAGE, msg) == B_NO_ERROR)) BroadcastToAllSessions(wrapper, userData);
}

void ThreadWorkerSession :: MessageReceivedFromSession(AbstractReflectSession & from, MessageRef msgRef, void * userData)
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
               // If we have any messages pending, we'll save this message reference until our
               // outgoing message queue becomes empty.  That way the DrainTag item held by the 
               // referenced message won't be deleted until the appropriate time, and hence
               // the supervisor won't be notified until all the specified queues have drained.
               AbstractMessageIOGateway * gw = GetGateway();
               if ((gw)&&(gw->HasBytesToOutput())) _drainedNotifiers.AddTail(msgRef);
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
               GenericRef tagRef;
               (void) msg->FindTag(MTT_NAME_POLICY_TAG, tagRef);
               PolicyRef pref(tagRef, true);
               if (msg->what == MTT_COMMAND_SET_INPUT_POLICY) SetInputPolicy(pref);
                                                         else SetOutputPolicy(pref);
            }
            break;

            case MTT_COMMAND_SET_OUTGOING_ENCODING:
            {
               int32 enc;
               if (msg->FindInt32(MTT_NAME_ENCODING, &enc) == B_NO_ERROR)
               {
                  MessageIOGateway * gw = dynamic_cast<MessageIOGateway *>(GetGateway());
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
   HashtableIterator<DrainTag *, bool> tagIter = _drainTags.GetIterator();
   DrainTag * nextKey;
   while(tagIter.GetNextKey(nextKey) == B_NO_ERROR) nextKey->SetNotify(NULL);

   StorageReflectSession :: AboutToDetachFromServer();
}

void ThreadSupervisorSession :: DrainTagIsBeingDeleted(DrainTag * tag)
{
   if (_drainTags.Remove(tag) == B_NO_ERROR) _mtt->SendMessageToOwner(tag->GetReplyMessage());
}

AbstractMessageIOGateway * ThreadSupervisorSession :: CreateGateway()
{
   AbstractMessageIOGateway * gw = newnothrow SignalMessageIOGateway();
   if (gw == NULL) WARN_OUT_OF_MEMORY;
   return gw;
}

void ThreadSupervisorSession :: MessageReceivedFromGateway(MessageRef, void *)
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

void ThreadSupervisorSession :: MessageReceivedFromSession(AbstractReflectSession & from, MessageRef msgRef, void *)
{
   if (msgRef()) msgRef()->AddString(MTT_NAME_FROMSESSION, from.GetSessionIDString());
   _mtt->SendMessageToOwner(msgRef);
}

void ThreadSupervisorSession :: MessageReceivedFromFactory(ReflectSessionFactory & from, MessageRef msgRef, void *)
{
   if (msgRef()) msgRef()->AddInt16(MTT_NAME_PORT, from.GetPort());
   _mtt->SendMessageToOwner(msgRef);
}

bool ThreadSupervisorSession :: ClientConnectionClosed()
{
   EndServer();
   return StorageReflectSession::ClientConnectionClosed();
}

status_t ThreadSupervisorSession :: AddNewWorkerConnectSession(AbstractReflectSessionRef sessionRef, uint32 hostIP, uint16 port)
{
   status_t ret = (hostIP > 0) ? AddNewConnectSession(sessionRef, hostIP, port) : B_ERROR;

   // For immediate failure: Since (sessionRef) never attached, we need to send the disconnect message ourself.
   if (ret != B_NO_ERROR) MessageReceivedFromSession(*sessionRef(), GetMessageFromPool(MTT_EVENT_SESSION_DISCONNECTED), NULL);
   return ret;
}

void ThreadSupervisorSession :: DistributeMessageToWorkers(MessageRef distMsg)
{
   String distPath;
   SendMessageToMatchingSessions(distMsg, (distMsg()->FindString(MTT_NAME_PATH, distPath) == B_NO_ERROR) ? distPath : _defaultDistributionPath, QueryFilterRef(), false); 
}

status_t ThreadSupervisorSession :: MessageReceivedFromOwner(MessageRef msgRef, uint32)
{
   const Message * msg = msgRef();
   if (msg)
   {
      switch(msg->what)
      {
         case MTT_COMMAND_SEND_USER_MESSAGE:
            DistributeMessageToWorkers(msgRef);
         break;

         case MTT_COMMAND_NOTIFY_ON_OUTPUT_DRAIN:
         {
            GenericRef genericRef;
            if (msg->FindTag(MTT_NAME_DRAIN_TAG, genericRef) == B_NO_ERROR)
            {
               DrainTagRef drainTagRef(genericRef, true);
               if ((drainTagRef())&&(_drainTags.Put(drainTagRef(), true) == B_NO_ERROR))
               {
                  drainTagRef()->SetNotify(this);
                  DistributeMessageToWorkers(msgRef);
               }
            }
         }
         break;

         case MTT_COMMAND_ADD_NEW_SESSION:
         {
            GenericRef tagRef;
            if (msg->FindTag(MTT_NAME_SESSION, tagRef) == B_NO_ERROR)
            {
               AbstractReflectSessionRef sessionRef(tagRef, true);
               if (sessionRef())
               {
                  const char * hostName;
                  uint32 hostIP;
                  uint16 port = 0; (void) msg->FindInt16(MTT_NAME_PORT, (int16*) &port);

                  GenericRef genericRef;
                  if (msg->FindTag(MTT_NAME_SOCKET, genericRef) == B_NO_ERROR)
                  {
                     SocketHolderRef socketRef(genericRef, true);
                     if (socketRef())
                     {
                        int fd = socketRef()->ReleaseSocket();
                        if (AddNewSession(sessionRef, fd) != B_NO_ERROR) CloseSocket(fd);
                     }
                     else LogTime(MUSCLE_LOG_ERROR, "ThreadSupervisorSession: Couldn't get SocketHolder!\n");
                  }
                  else if (msg->FindInt32(MTT_NAME_IP_ADDRESS, (int32*)&hostIP)  == B_NO_ERROR) (void) AddNewWorkerConnectSession(sessionRef, hostIP, port);
                  else if (msg->FindString(MTT_NAME_HOSTNAME, &hostName)         == B_NO_ERROR) (void) AddNewWorkerConnectSession(sessionRef, GetHostByName(hostName), port);
               }
               else LogTime(MUSCLE_LOG_ERROR, "ThreadSupervisorSession:  Couldn't get Session!\n");
            }
         }
         break;

         case MTT_COMMAND_PUT_ACCEPT_FACTORY:
         {
            GenericRef tagRef;
            if (msg->FindTag(MTT_NAME_FACTORY, tagRef) == B_NO_ERROR)
            {
               ReflectSessionFactoryRef factoryRef(tagRef, true);
               if (factoryRef())
               {
                  uint16 port = 0; (void) msg->FindInt16(MTT_NAME_PORT, (int16*)&port);
                  (void) PutAcceptFactory(port, factoryRef);
               }
               else LogTime(MUSCLE_LOG_ERROR, "ThreadSupervisorSession:  Couldn't get ReflectSessionFactory!\n");
            }
         }
         break;

         case MTT_COMMAND_REMOVE_ACCEPT_FACTORY:
         {
            uint16 port = 0; 
            if (msg->FindInt16(MTT_NAME_PORT, (int16*)&port) == B_NO_ERROR) (void) RemoveAcceptFactory(port);
         }
         break;

         case MTT_COMMAND_SET_DEFAULT_PATH:
         {
            String dpath;
            (void) msg->FindString(MTT_NAME_PATH, dpath);
            SetDefaultDistributionPath(dpath);
         }
         break;

         default:
            StorageReflectSession::MessageReceivedFromGateway(msgRef, NULL);
         break;
      }
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

};  // end namespace muscle
