/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include <limits.h>
#include "reflector/ReflectServer.h"
#include "reflector/StorageReflectSession.h"
#include "iogateway/MessageIOGateway.h"

namespace muscle {

#define DEFAULT_PATH_PREFIX "*/*"  // when we get a path name without a leading '/', prepend this
#define DEFAULT_MAX_SUBSCRIPTION_MESSAGE_SIZE   50   // no more than 50 items/update message, please

static Message _blankMessage;
static MessageRef _blankMessageRef(&_blankMessage, NULL, false);

// field under which we file our shared data in the central-state message
static const String SRS_SHARED_DATA = "srs_shared";

StorageReflectSession::DataNodeRef::ItemPool StorageReflectSession::_nodePool(100, StorageReflectSession::ResetNodeFunc);

StringMatcherRef::ItemPool StorageReflectSession::_stringMatcherPool;

StringMatcherQueueRef::ItemPool StorageReflectSession::_stringMatcherQueuePool(100, StorageReflectSession::ResetMatcherQueueFunc);

void StorageReflectSession :: DrainPools()
{
   _nodePool.Drain();
   _stringMatcherPool.Drain();
   _stringMatcherQueuePool.Drain();
}

StorageReflectSessionFactory :: StorageReflectSessionFactory() : _maxIncomingMessageSize(MUSCLE_NO_LIMIT)
{
   // empty
}

AbstractReflectSession * StorageReflectSessionFactory :: CreateSession(const String &)
{
   AbstractReflectSession * ret = newnothrow StorageReflectSession();
   if ((ret)&&(SetMaxIncomingMessageSizeFor(ret) == B_NO_ERROR)) return ret;
   else
   {
      WARN_OUT_OF_MEMORY;
      delete ret;
      return NULL;
   }
}

status_t StorageReflectSessionFactory :: SetMaxIncomingMessageSizeFor(AbstractReflectSession * session) const
{
   if (_maxIncomingMessageSize != MUSCLE_NO_LIMIT) 
   {
      if (session->GetGateway() == NULL) session->SetGateway(AbstractMessageIOGatewayRef(session->CreateGateway(), NULL));
      MessageIOGateway * gw = dynamic_cast<MessageIOGateway*>(session->GetGateway());
      if (gw) gw->SetMaxIncomingMessageSize(_maxIncomingMessageSize);
         else return B_ERROR;
   }
   return B_NO_ERROR;
}

StorageReflectSession ::
StorageReflectSession() : 
   _parameters(PR_RESULT_PARAMETERS), 
   _sharedData(NULL),
   _subscriptionsEnabled(true), 
   _maxSubscriptionMessageItems(DEFAULT_MAX_SUBSCRIPTION_MESSAGE_SIZE), 
   _indexingPresent(false),
   _currentNodeCount(0),
   _maxNodeCount(MUSCLE_NO_LIMIT)
{
   // empty
}

StorageReflectSession :: 
~StorageReflectSession() 
{
   // empty
}

StorageReflectSession::StorageReflectSessionSharedData * 
StorageReflectSession ::
InitSharedData()
{
   Message & state = GetCentralState();

   StorageReflectSession::StorageReflectSessionSharedData * sd;
   if (state.FindPointer(SRS_SHARED_DATA, (void **) &sd) == B_NO_ERROR) return sd;
  
   // oops, there's no shared data object!  We must be the first session.
   // So we'll create the root node and the shared data object, and
   // add it to the central-state Message ourself.
   DataNode * globalRoot = GetNewDataNode("", _blankMessageRef);
   if (globalRoot) 
   {
      sd = newnothrow StorageReflectSessionSharedData(globalRoot);
      if (sd)
      {
         if (state.AddPointer(SRS_SHARED_DATA, sd) == B_NO_ERROR) return sd;
         delete sd;
      }
      else WARN_OUT_OF_MEMORY;

      ReleaseDataNode(globalRoot);
   }
   return NULL;
}

status_t
StorageReflectSession ::
AttachedToServer()
{
   if (DumbReflectSession::AttachedToServer() != B_NO_ERROR) return B_ERROR;

   _sharedData = InitSharedData();
   if (_sharedData == NULL) return B_ERROR;
   
   Message & state = GetCentralState();
   const char * hostname = GetHostName();
   const char * sessionid = GetSessionIDString();

   // Is there already a node for our hostname?
   DataNodeRef hostDir;
   if (GetGlobalRoot().GetChild(hostname, hostDir) != B_NO_ERROR)
   {
      // nope.... we'll add one then
      DataNode * hostNode = GetNewDataNode(hostname, _blankMessageRef);
      if (hostNode)
      {
         hostDir = DataNodeRef(hostNode, &_nodePool);
         if (GetGlobalRoot().PutChild(hostDir, this, this) != B_NO_ERROR) {WARN_OUT_OF_MEMORY; Cleanup(); return B_ERROR;}
      }
      else {WARN_OUT_OF_MEMORY; Cleanup(); return B_ERROR;}
   }

   // Create a new node for our session (we assume no such
   // node already exists, as session id's are supposed to be unique)
   DataNode * hostNode = hostDir.GetItemPointer();
   if (hostNode == NULL) {Cleanup(); return B_ERROR;}
   if (hostNode->HasChild(sessionid)) LogTime(MUSCLE_LOG_WARNING, "WARNING:  Non-unique session id [%s] being overwritten!\n", sessionid);

   DataNode * sessionNode = GetNewDataNode(sessionid, _blankMessageRef);
   if (sessionNode)
   {
      // See if we get any special privileges
      int32 privBits = 0;
      for (int p=0; p<=PR_NUM_PRIVILEGES; p++)
      {
         char temp[32];
         sprintf(temp, "priv%i", p);
         const char * privPattern;
         for (int q=0; (state.FindString(temp, q, &privPattern) == B_NO_ERROR); q++)
         {
            if (StringMatcher(privPattern).Match(hostname))
            {
               if (p == PR_NUM_PRIVILEGES) privBits = ~0;  // all privileges granted!
                                      else privBits |= (1L<<p);
               break;
            } 
         }
      }
      if (privBits != 0L) 
      {
         _parameters.RemoveName(PR_NAME_PRIVILEGE_BITS);
         _parameters.AddInt32(PR_NAME_PRIVILEGE_BITS, privBits);
      }

      _sessionDir = DataNodeRef(sessionNode, &_nodePool);
      if (hostNode->PutChild(_sessionDir, this, this) != B_NO_ERROR) {WARN_OUT_OF_MEMORY; Cleanup(); return B_ERROR;}
 
      // do subscription notifications here
      PushSubscriptionMessages();

      // Get our node-creation limit.  For now, this is the same for all sessions.
      // (someday maybe I'll work out a way to give different limits to different sessions)
      uint32 nodeLimit;
      if (state.FindInt32(PR_NAME_MAX_NODES_PER_SESSION, (int32*)&nodeLimit) == B_NO_ERROR) _maxNodeCount = nodeLimit;
   
      return B_NO_ERROR;
   }

   WARN_OUT_OF_MEMORY; 
   Cleanup(); 
   return B_ERROR;
}

void
StorageReflectSession ::
AboutToDetachFromServer()
{
   Cleanup();
   DumbReflectSession::AboutToDetachFromServer();
}

void
StorageReflectSession ::
Cleanup()
{
   if (_sharedData)
   {
      DataNodeRef hostNodeRef;
      if (GetGlobalRoot().GetChild(GetHostName(), hostNodeRef) == B_NO_ERROR)
      {
         DataNode * hostNode = hostNodeRef.GetItemPointer();
         if (hostNode)
         {
            // make sure our session node is gone
            hostNode->RemoveChild(GetSessionIDString(), this, true, NULL);

            // If our host node is now empty, it goes too
            if (hostNode->CountChildren() == 0) GetGlobalRoot().RemoveChild(hostNode->GetNodeName()(), this, true, NULL);
         }
      
         PushSubscriptionMessages();
      }

      // If the global root is now empty, it goes too
      if (GetGlobalRoot().CountChildren() == 0)
      {
         GetCentralState().RemoveName(SRS_SHARED_DATA);
         ReleaseDataNode(&GetGlobalRoot());  // do this first!
         delete _sharedData;
      }
      else 
      {
         // Remove all of our subscription-marks from neighbor's nodes
         _subscriptions.DoTraversal((PathMatchCallback)DoSubscribeRefCallbackFunc, this, GetGlobalRoot(), (void *)(-LONG_MAX));
      }
      _sharedData = NULL;
   }
   _nextSubscriptionMessage.Reset();
   _nextIndexSubscriptionMessage.Reset();
}

void 
StorageReflectSession ::
NotifySubscribersThatNodeChanged(DataNode & modifiedNode, bool isBeingRemoved)
{
   HashtableIterator<const char *, uint32> subIter = modifiedNode.GetSubscribers();
   const char * next;
   while(subIter.GetNextKey(next) == B_NO_ERROR)
   {
      AbstractReflectSessionRef nRef = GetSession(next);
      StorageReflectSession * next = (StorageReflectSession *)(nRef.GetItemPointer());
      if ((next)&&((next != this)||(GetReflectToSelf()))) next->NodeChanged(modifiedNode, isBeingRemoved);
   }
}

void 
StorageReflectSession ::
NotifySubscribersThatNodeIndexChanged(DataNode & modifiedNode, char op, uint32 index, const char * key)
{
   HashtableIterator<const char *, uint32> subIter = modifiedNode.GetSubscribers();
   const char * next;
   while(subIter.GetNextKey(next) == B_NO_ERROR) 
   {
      AbstractReflectSessionRef nRef = GetSession(next);
      StorageReflectSession * s = (StorageReflectSession *) nRef.GetItemPointer();
      if (s) s->NodeIndexChanged(modifiedNode, op, index, key);
   }
}

void
StorageReflectSession ::
NotifySubscribersOfNewNode(DataNode & newNode)
{
   HashtableIterator<const char *, AbstractReflectSessionRef> sessions = GetSessions();
   AbstractReflectSessionRef * lRef;
   while((lRef = sessions.GetNextValue()) != NULL)
   {
      StorageReflectSession * next = (StorageReflectSession *) (lRef->GetItemPointer());
      if (next) next->NodeCreated(newNode);  // always notify; !Self filtering will be done elsewhere
   }
}

void
StorageReflectSession ::
NodeChanged(DataNode & modifiedNode, bool isBeingRemoved)
{
   if (GetSubscriptionsEnabled())
   {
      if (_nextSubscriptionMessage() == NULL) _nextSubscriptionMessage = GetMessageFromPool(PR_RESULT_DATAITEMS);
      if (_nextSubscriptionMessage())
      {
         _sharedData->_subsDirty = true;
         String np;
         if (modifiedNode.GetNodePath(np) == B_NO_ERROR)
         {
            if (isBeingRemoved) _nextSubscriptionMessage()->AddString(PR_NAME_REMOVED_DATAITEMS, np);
                           else _nextSubscriptionMessage()->AddMessage(np, modifiedNode.GetData());
         }
         if (_nextSubscriptionMessage()->CountNames() >= _maxSubscriptionMessageItems) PushSubscriptionMessages(); 
      }
      else WARN_OUT_OF_MEMORY;
   }
}

void
StorageReflectSession ::
NodeIndexChanged(DataNode & modifiedNode, char op, uint32 index, const char * key)
{
   if (GetSubscriptionsEnabled())
   {
      if (_nextIndexSubscriptionMessage() == NULL) _nextIndexSubscriptionMessage = GetMessageFromPool(PR_RESULT_INDEXUPDATED);

      String np;
      if ((_nextIndexSubscriptionMessage())&&(modifiedNode.GetNodePath(np) == B_NO_ERROR))
      {
         _sharedData->_subsDirty = true;
         char temp[100];
         sprintf(temp, "%c%lu:%s", op, index, key);
         _nextIndexSubscriptionMessage()->AddString(np, temp);
      }
      else WARN_OUT_OF_MEMORY;
      // don't push subscription messages here.... it will be done elsewhere
   }
}

void
StorageReflectSession ::
NodeCreated(DataNode & newNode)
{
   newNode.IncrementSubscriptionRefCount(GetSessionIDString(), _subscriptions.GetMatchCount(newNode, 0));
}

status_t
StorageReflectSession ::
SetDataNode(const String & nodePath, MessageRef dataMsgRef, bool overwrite, bool create, bool quiet, bool appendToIndex)
{
   DataNode * node = _sessionDir.GetItemPointer();
   if (node == NULL) return B_ERROR;
 
   if ((nodePath.Length() > 0)&&(nodePath[0] != '/'))
   {
      int lastSlashPos = -1;
      int slashPos = 0;
      DataNodeRef childNodeRef;
      String nextClause;
      while(slashPos >= 0)
      {
         slashPos = nodePath.IndexOf('/', lastSlashPos+1);
         nextClause = nodePath.Substring(lastSlashPos+1, (slashPos >= 0) ? slashPos : nodePath.Length());
         const char * clause = nextClause();
         DataNode * allocedNode = NULL;  // default:  we haven't alloced anybody
         if (node->GetChild(clause, childNodeRef) != B_NO_ERROR)
         {
            if ((create)&&(_currentNodeCount < _maxNodeCount))
            {
               allocedNode = GetNewDataNode(clause, ((slashPos < 0)&&(appendToIndex == false)) ? dataMsgRef : _blankMessageRef);
               if (allocedNode)
               {
                  childNodeRef = DataNodeRef(allocedNode, &_nodePool);
                  if ((slashPos < 0)&&(appendToIndex))
                  {
                     if (node->InsertOrderedChild(dataMsgRef, NULL, this, quiet?NULL:this, NULL) == B_NO_ERROR)
                     {
                        _currentNodeCount++;
                        _indexingPresent = true;
                     }
                  }
                  else if (node->PutChild(childNodeRef, this, ((quiet)||(slashPos < 0)) ? NULL : this) == B_NO_ERROR) _currentNodeCount++;
               }
               else {WARN_OUT_OF_MEMORY; return B_ERROR;}
            }
            else return B_ERROR;
         }

         node = childNodeRef.GetItemPointer();
         if ((slashPos < 0)&&(appendToIndex == false))
         {
            if ((node == NULL)||((overwrite == false)&&(node != allocedNode))) return B_ERROR;
            node->SetData(dataMsgRef, quiet ? NULL : this);  // do this to trigger the changed-notification
         }
         lastSlashPos = slashPos;
      }
   }
   return B_NO_ERROR;
}

StorageReflectSession :: DataNode *
StorageReflectSession ::
GetDataNode(const String & nodePath) const
{
   DataNode * node = _sessionDir.GetItemPointer();
   if (nodePath.Length() > 0)
   {
      int lastSlashPos = -1;
      int slashPos = 0;
      DataNodeRef childNodeRef;
      String nextClause;

      if (nodePath[0] == '/') 
      {
         node = &GetGlobalRoot();
         lastSlashPos=0;
      }
      while(slashPos >= 0)
      {
         slashPos = nodePath.IndexOf('/', lastSlashPos+1);
         nextClause = nodePath.Substring(lastSlashPos+1, (slashPos >= 0) ? slashPos : nodePath.Length());
         const char * clause = nextClause();
         if ((node == NULL)||(node->GetChild(clause, childNodeRef) != B_NO_ERROR)) return NULL;  // lookup failure (404)
         node = childNodeRef.GetItemPointer();
         if (slashPos < 0) return node;
         lastSlashPos = slashPos;
      }
   }
   return node;
}

bool
StorageReflectSession ::
HasPrivilege(int priv) const
{
   int32 privBits;
   return (_parameters.FindInt32(PR_NAME_PRIVILEGE_BITS, &privBits) == B_NO_ERROR) ? ((privBits & (1L<<priv)) != 0L) : false;
}

void
StorageReflectSession ::
AfterMessageReceivedFromGateway(MessageRef msgRef)
{
   AbstractReflectSession::AfterMessageReceivedFromGateway(msgRef);
   PushSubscriptionMessages();
}

void 
StorageReflectSession :: 
MessageReceivedFromGateway(MessageRef msgRef)
{
   Message * msgp = msgRef.GetItemPointer();
   if (msgp == NULL) return;

   Message & msg = *msgp;
   if ((msg.what >= BEGIN_PR_COMMANDS)&&(msg.what <= END_PR_COMMANDS))
   {
      switch(msg.what)
      {
         case PR_COMMAND_JETTISONDATATREES:
         {
            if (msg.HasName(PR_NAME_TREE_REQUEST_ID, B_STRING_TYPE)) 
            {
               const char * str;
               for (int32 i=0; msg.FindString(PR_NAME_TREE_REQUEST_ID, i, &str) == B_NO_ERROR; i++) JettisonOutgoingSubtrees(str);
            }
            else JettisonOutgoingSubtrees(NULL);
         }
         break;

         case PR_COMMAND_SETDATATREES:
         break;

         case PR_COMMAND_GETDATATREES:
         {
            const char * id = NULL; (void) msg.FindString(PR_NAME_TREE_REQUEST_ID, &id);
            MessageRef reply = GetMessageFromPool(PR_RESULT_DATATREES);
            if ((reply())&&((id==NULL)||(reply()->AddString(PR_NAME_TREE_REQUEST_ID, id) == B_NO_ERROR)))
            {
msg.PrintToStream();
               if (msg.HasName(PR_NAME_KEYS, B_STRING_TYPE)) 
               {
                  NodePathMatcher matcher;
                  matcher.AddPathsFromMessage(PR_NAME_KEYS, msg, DEFAULT_PATH_PREFIX, &_stringMatcherPool, &_stringMatcherQueuePool);
                  matcher.DoTraversal((PathMatchCallback)GetSubtreesCallbackFunc, this, GetGlobalRoot(), reply());
               }
               MessageReceivedFromSession(*this, reply, NULL);  // send the result back to our client
            }
         }
         break;

         case PR_COMMAND_NOOP:
            // do nothing!
         break;

         case PR_COMMAND_BATCH:
         {
            MessageRef subRef;
            for (int i=0; msg.FindMessage(PR_NAME_KEYS, i, subRef) == B_NO_ERROR; i++) CallMessageReceivedFromGateway(subRef);
         }
         break;

         case PR_COMMAND_KICK:
         {
            if (HasPrivilege(PR_PRIVILEGE_KICK))
            { 
               if (msg.HasName(PR_NAME_KEYS, B_STRING_TYPE)) 
               {
                  NodePathMatcher matcher;
                  matcher.AddPathsFromMessage(PR_NAME_KEYS, msg, DEFAULT_PATH_PREFIX, &_stringMatcherPool, &_stringMatcherQueuePool);
                  matcher.DoTraversal((PathMatchCallback)KickClientCallbackFunc, this, GetGlobalRoot(), NULL);
               }
            }
            else BounceMessage(PR_RESULT_ERRORACCESSDENIED, msgRef);
         }
         break;

         case PR_COMMAND_ADDBANS: case PR_COMMAND_ADDREQUIRES:
            if (HasPrivilege(PR_PRIVILEGE_ADDBANS)) 
            {
               ReflectSessionFactoryRef factoryRef = GetFactory(GetPort());
               if (factoryRef()) factoryRef()->MessageReceivedFromSession(*this, msgRef, NULL);
            }
            else BounceMessage(PR_RESULT_ERRORACCESSDENIED, msgRef);
         break;

         case PR_COMMAND_REMOVEBANS: case PR_COMMAND_REMOVEREQUIRES:
            if (HasPrivilege(PR_PRIVILEGE_REMOVEBANS))  
            {
               ReflectSessionFactoryRef factoryRef = GetFactory(GetPort());
               if (factoryRef()) factoryRef()->MessageReceivedFromSession(*this, msgRef, NULL);
            }
            else BounceMessage(PR_RESULT_ERRORACCESSDENIED, msgRef);
         break;

         case PR_COMMAND_SETPARAMETERS:
         {
            bool subscribeQuietly = msg.HasName(PR_NAME_SUBSCRIBE_QUIETLY);
            MessageFieldNameIterator it = msg.GetFieldNameIterator();
            Message getMsg(PR_COMMAND_GETDATA);
            const char * fn;
            while((fn = it.GetNextFieldName()) != NULL)
            {
               bool copyField = true;
               if (strncmp(fn, "SUBSCRIBE:", 10) == 0)
               {
                  const char * path = &fn[10];
                  String fixPath(path);
                  _subscriptions.AdjustStringPrefix(fixPath, DEFAULT_PATH_PREFIX);
                  if (_subscriptions.ContainsPathString(fixPath) == false)
                  {
                     // This marks any currently existing matching nodes so they know to notify us
                     // It must be done once per subscription path, as it uses per-sub ref-counting
                     NodePathMatcher temp;
                     temp.AddPathString(fixPath, &_stringMatcherPool, &_stringMatcherQueuePool);
                     temp.DoTraversal((PathMatchCallback)DoSubscribeRefCallbackFunc, this, GetGlobalRoot(), (void *)1L);

                     _subscriptions.AddPathsFromMatcher(temp);
                  }
                  if (subscribeQuietly == false) getMsg.AddString(PR_NAME_KEYS, path);
               }
               else if (strcmp(fn, PR_NAME_REFLECT_TO_SELF) == 0)
               {
                  SetReflectToSelf(true);
               }
               else if (strcmp(fn, PR_NAME_DISABLE_SUBSCRIPTIONS) == 0)
               {
                  SetSubscriptionsEnabled(false);
               }
               else if (strcmp(fn, PR_NAME_KEYS) == 0)
               {
                  _defaultMessageRoute.Clear();
                  _defaultMessageRoute.AddPathsFromMessage(PR_NAME_KEYS, msg, DEFAULT_PATH_PREFIX, &_stringMatcherPool, &_stringMatcherQueuePool);
               }
               else if (strcmp(fn, PR_NAME_SUBSCRIBE_QUIETLY) == 0)
               {
                  // don't add this to the parameter set; it's just an "argument" for the SUBSCRIBE: fields.
                  copyField = false;
               }
               else if (strcmp(fn, PR_NAME_MAX_UPDATE_MESSAGE_ITEMS) == 0)
               {
                  (void) msg.FindInt32(PR_NAME_MAX_UPDATE_MESSAGE_ITEMS, (int32*)&_maxSubscriptionMessageItems);
               }
               else if (strcmp(fn, PR_NAME_PRIVILEGE_BITS) == 0)
               {
                  // don't add this to the parameter set; clients aren't allowed to change
                  // their privilege bits (maybe someday, for some clients, but not now)
                  copyField = false;
               }

               if (copyField) msg.CopyName(fn, _parameters);
            }
            if (getMsg.HasName(PR_NAME_KEYS)) DoGetData(getMsg);  // return any data that matches the subscription
         }
         break;

         case PR_COMMAND_GETPARAMETERS:
         {
            DataNode * sessionNode = _sessionDir.GetItemPointer();
            if (sessionNode)
            {
               String np;
               MessageRef resultMessage = GetMessageFromPool(_parameters);
               if ((resultMessage())&&(sessionNode->GetNodePath(np) == B_NO_ERROR))
               {
                  // Add hard-coded params 
                  resultMessage()->RemoveName(PR_NAME_SESSION_ROOT);
                  resultMessage()->AddString(PR_NAME_SESSION_ROOT, np);

                  resultMessage()->RemoveName(PR_NAME_SERVER_VERSION);
                  resultMessage()->AddString(PR_NAME_SERVER_VERSION, MUSCLE_VERSION_STRING);

                  resultMessage()->RemoveName(PR_NAME_SERVER_MEM_AVAILABLE);
                  resultMessage()->AddInt64(PR_NAME_SERVER_MEM_AVAILABLE, (uint64) GetNumAvailableBytes());

                  resultMessage()->RemoveName(PR_NAME_SERVER_MEM_USED);
                  resultMessage()->AddInt64(PR_NAME_SERVER_MEM_USED, (uint64) GetNumUsedBytes());

                  resultMessage()->RemoveName(PR_NAME_SERVER_MEM_MAX);
                  resultMessage()->AddInt64(PR_NAME_SERVER_MEM_MAX, (uint64) GetMaxNumBytes());

                  resultMessage()->RemoveName(PR_NAME_SERVER_UPTIME);
                  resultMessage()->AddInt64(PR_NAME_SERVER_UPTIME, (uint64) GetServerUptime());

                  resultMessage()->RemoveName(PR_NAME_MAX_NODES_PER_SESSION);
                  resultMessage()->AddInt64(PR_NAME_MAX_NODES_PER_SESSION, _maxNodeCount);
        
                  MessageReceivedFromSession(*this, resultMessage, NULL);
               }
               else WARN_OUT_OF_MEMORY;
            }
         }
         break;

         case PR_COMMAND_REMOVEPARAMETERS:
         {
            StringMatcher matcher;
            const char * nextName;
            Queue<String> toRemove;

            // Compile a list of parameters whose names match any key in the PR_NAME_KEYS field
            for (int i=0; msg.FindString(PR_NAME_KEYS, i, &nextName) == B_NO_ERROR; i++) 
            {
               // Search the parameters message for all parameters that match (nextName)...
               if (matcher.SetPattern(nextName) == B_NO_ERROR)
               {
                  MessageFieldNameIterator it = _parameters.GetFieldNameIterator();
                  const char * fn;
                  while((fn = it.GetNextFieldName()) != NULL) if (matcher.Match(fn)) (void) toRemove.AddTail(fn);
               }
               else LogTime(MUSCLE_LOG_ERROR, "PR_COMMAND_REMOVEPARAMETERS:  Error, StringMatcher couldn't match on [%s]\n",nextName);
            }

            // Now remove them from our parameters list (and do any side effects that go with that)
            for (int j=toRemove.GetNumItems()-1; j>=0; j--) 
            {
               const char * paramName = toRemove[j]();

               if (_parameters.RemoveName(paramName) == B_NO_ERROR)
               {
                  if (strncmp(paramName, "SUBSCRIBE:", 10) == 0) 
                  {
                     String str(&paramName[10]);
                     _subscriptions.AdjustStringPrefix(str, DEFAULT_PATH_PREFIX);
                     if (_subscriptions.RemovePathString(str) == B_NO_ERROR)
                     {
                        // Remove the references from this subscription from all nodes
                        NodePathMatcher temp;
                        temp.AddPathString(str, &_stringMatcherPool, &_stringMatcherQueuePool);
                        temp.DoTraversal((PathMatchCallback)DoSubscribeRefCallbackFunc, this, GetGlobalRoot(), (void *)-1L);
                     }
                  }
                  else if (strcmp(paramName, PR_NAME_REFLECT_TO_SELF) == 0)
                  {
                     SetReflectToSelf(false);
                  }
                  else if (strcmp(paramName, PR_NAME_DISABLE_SUBSCRIPTIONS) == 0)
                  {
                     SetSubscriptionsEnabled(true);
                  }
                  else if (strcmp(paramName, PR_NAME_MAX_UPDATE_MESSAGE_ITEMS) == 0)
                  {
                     _maxSubscriptionMessageItems = DEFAULT_MAX_SUBSCRIPTION_MESSAGE_SIZE;  // back to the default
                  }
               }
            }
         }
         break;

         case PR_COMMAND_SETDATA:
         {
            bool quiet = msg.HasName(PR_NAME_SET_QUIETLY);
            MessageFieldNameIterator it = msg.GetFieldNameIterator(B_MESSAGE_TYPE);
            const String * nextName;
            while((nextName = it.GetNextFieldNameString()) != NULL)
            {
               MessageRef dataMsgRef;
               if (msg.FindMessage(*nextName, dataMsgRef) == B_NO_ERROR) SetDataNode(*nextName, dataMsgRef, true, true, quiet);
            }
         }
         break;

         case PR_COMMAND_INSERTORDEREDDATA:
            (void) InsertOrderedData(msgRef, NULL);
         break;

         case PR_COMMAND_REORDERDATA:
         {
            // Because REORDERDATA operates solely on pre-existing nodes, we can allow wildcards in our node paths.
            DataNode * sessionNode = _sessionDir.GetItemPointer();
            if (sessionNode)
            {
               // Do each field as a separate operation (so they won't mess each other up)
               MessageFieldNameIterator iter = msg.GetFieldNameIterator(B_STRING_TYPE);
               const String * nextFieldName;
               while((nextFieldName = iter.GetNextFieldNameString()) != NULL)
               {
                  const char * value;
                  if (msg.FindString(*nextFieldName, &value) == B_NO_ERROR)
                  {
                     Message temp;
                     temp.AddString(PR_NAME_KEYS, *nextFieldName);

                     NodePathMatcher matcher;
                     matcher.AddPathsFromMessage(PR_NAME_KEYS, temp, NULL, &_stringMatcherPool, &_stringMatcherQueuePool);
                     matcher.DoTraversal((PathMatchCallback)ReorderDataCallbackFunc, this, *sessionNode, (void *)value);
                  }
               }
            }
         }
         break;

         case PR_COMMAND_GETDATA:
            DoGetData(msg);
         break;

         case PR_COMMAND_REMOVEDATA:
         {
               NodePathMatcher matcher;
               matcher.AddPathsFromMessage(PR_NAME_KEYS, msg, NULL, &_stringMatcherPool, &_stringMatcherQueuePool);
               DoRemoveData(matcher, msg.HasName(PR_NAME_REMOVE_QUIETLY));
         }
         break;

         case PR_RESULT_PARAMETERS:
            // fall-thru
         case PR_RESULT_DATAITEMS:
            LogTime(MUSCLE_LOG_WARNING, "Warning, client at [%s] sent me a PR_RESULT_* code.  Bad client!\n", GetHostName());
         break;

         case PR_COMMAND_JETTISONRESULTS:
         {
            if (msg.HasName(PR_NAME_KEYS, B_STRING_TYPE))
            {
               NodePathMatcher matcher;
               matcher.AddPathsFromMessage(PR_NAME_KEYS, msg, DEFAULT_PATH_PREFIX, &_stringMatcherPool, &_stringMatcherQueuePool);
               JettisonOutgoingResults(&matcher);
            }
            else JettisonOutgoingResults(NULL);
         }
         break;

         case PR_COMMAND_PING:
         {
            msg.what = PR_RESULT_PONG;                         // mark it as processed...
            MessageReceivedFromSession(*this, msgRef, NULL);  // and send it right back to our client
         }
         break;

         default:
            BounceMessage(PR_RESULT_ERRORUNIMPLEMENTED, msgRef);
         break;
      }
   }
   else
   {
      // New for v1.85; if the message has a PR_NAME_SESSION field in it, make sure it's the correct one!
      // This is to foil certain people (olorin ;^)) who would otherwise be spoofing messages from other people.
      msg.ReplaceString(false, PR_NAME_SESSION, GetSessionIDString());

      // what code not in our reserved range:  must be a client-to-client message
      if (msg.HasName(PR_NAME_KEYS, B_STRING_TYPE)) 
      {
         NodePathMatcher matcher;
         matcher.AddPathsFromMessage(PR_NAME_KEYS, msg, DEFAULT_PATH_PREFIX, &_stringMatcherPool, &_stringMatcherQueuePool);
         matcher.DoTraversal((PathMatchCallback)PassMessageCallbackFunc, this, GetGlobalRoot(), &msgRef);
      }
      else if (_parameters.HasName(PR_NAME_KEYS, B_STRING_TYPE)) 
      {
         _defaultMessageRoute.DoTraversal((PathMatchCallback)PassMessageCallbackFunc, this, GetGlobalRoot(), &msgRef);
      }
      else DumbReflectSession::MessageReceivedFromGateway(msgRef);
   }
}

status_t StorageReflectSession :: SendMessageToMatchingSessions(MessageRef msgRef, const String & nodePath)
{
   const char * s;
   String temp;
   if (nodePath.StartsWith("/")) s = nodePath()+1;
   else
   {
      temp = nodePath.Prepend(DEFAULT_PATH_PREFIX "/");
      s = temp();
   }

   NodePathMatcher matcher;
   if (matcher.AddPathString(s, &_stringMatcherPool, &_stringMatcherQueuePool) == B_NO_ERROR)
   {
      matcher.DoTraversal((PathMatchCallback)PassMessageCallbackFunc, this, GetGlobalRoot(), &msgRef);
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t StorageReflectSession :: InsertOrderedData(MessageRef msgRef, Hashtable<String, DataNodeRef> * optNewNodes)
{
   // Because INSERTORDEREDDATA operates solely on pre-existing nodes, we can allow wildcards in our node paths.
   DataNode * sessionNode = _sessionDir.GetItemPointer();
   if ((sessionNode)&&(msgRef()))
   {
      void * args[2] = {msgRef(), optNewNodes};
      NodePathMatcher matcher;
      matcher.AddPathsFromMessage(PR_NAME_KEYS, *msgRef(), NULL, &_stringMatcherPool, &_stringMatcherQueuePool);
      matcher.DoTraversal((PathMatchCallback)InsertOrderedDataCallbackFunc, this, *sessionNode, args);
      return B_NO_ERROR;
   }
   return B_ERROR;
}

void
StorageReflectSession :: 
BounceMessage(uint32 errorCode, MessageRef msgRef)
{
   // Unknown code; bounce it back to our client
   MessageRef bounce = GetMessageFromPool(errorCode);
   if (bounce())
   {
      bounce()->AddMessage(PR_NAME_REJECTED_MESSAGE, msgRef);
      MessageReceivedFromSession(*this, bounce, NULL);  // send rejection notice to client
   }
}

void
StorageReflectSession :: 
DoGetData(const Message & msg)
{
   NodePathMatcher matcher;
   matcher.AddPathsFromMessage(PR_NAME_KEYS, msg, DEFAULT_PATH_PREFIX, &_stringMatcherPool, &_stringMatcherQueuePool);

   MessageRef messageArray[2];  // first is the DATAITEMS message, second is the INDEXUPDATED message (both demand-allocated)
   matcher.DoTraversal((PathMatchCallback)GetDataCallbackFunc, this, GetGlobalRoot(), messageArray);

   // Send any still-pending "get" results...
   SendGetDataResults(messageArray[0]);
   SendGetDataResults(messageArray[1]);
}

void
StorageReflectSession ::
SendGetDataResults(MessageRef & replyMessage)
{
   if (replyMessage())
   {
      MessageReceivedFromSession(*this, replyMessage, NULL);
      replyMessage.Reset();  // force re-alloc later if need be
   }
}

void StorageReflectSession :: DoRemoveData(NodePathMatcher & matcher, bool quiet)
{
   DataNode * sessionNode = _sessionDir.GetItemPointer();
   if (sessionNode)
   {
      Queue<DataNodeRef> removeSet;
      matcher.DoTraversal((PathMatchCallback)RemoveDataCallbackFunc, this, *sessionNode, &removeSet);
      for (int i=removeSet.GetNumItems()-1; i>=0; i--)
      {
         DataNode * next = removeSet[i].GetItemPointer();
         DataNode * parent = next->GetParent();
         if ((next)&&(parent)) parent->RemoveChild(next->GetNodeName()(), quiet ? NULL : this, true, &_currentNodeCount);
      }
   }
}

status_t StorageReflectSession :: RemoveDataNodes(const String & nodePath, bool quiet)
{
   NodePathMatcher matcher;
   if (matcher.AddPathString(nodePath, &_stringMatcherPool, &_stringMatcherQueuePool) != B_NO_ERROR) return B_ERROR;
   DoRemoveData(matcher, quiet);
   return B_NO_ERROR;
}

void
StorageReflectSession ::
PushSubscriptionMessages()
{
   if (_sharedData->_subsDirty)
   {
      _sharedData->_subsDirty = false;

      // Send out any subscription results that were generated...
      HashtableIterator<const char *, AbstractReflectSessionRef> sessions = GetSessions();
      AbstractReflectSessionRef * lRef;
      while((lRef = sessions.GetNextValue()) != NULL)
      {
         StorageReflectSession * nextSession = (StorageReflectSession *) (lRef->GetItemPointer());
         if (nextSession)
         {
            nextSession->PushSubscriptionMessage(nextSession->_nextSubscriptionMessage);
            nextSession->PushSubscriptionMessage(nextSession->_nextIndexSubscriptionMessage);
         }
      }
      PushSubscriptionMessages();  // in case these generated even more messages...
   }
}

void
StorageReflectSession ::
PushSubscriptionMessage(MessageRef & ref)
{
   MessageRef oldRef = ref;  // (ref) is one of our _next*SubscriptionMessage members
   while(oldRef()) 
   {
      ref.Reset();  /* In case FromNeighbor() wants to add more suscriptions... */
      MessageReceivedFromSession(*this, oldRef, NULL);
      oldRef = ref;
   }
}

int
StorageReflectSession ::
PassMessageCallback(DataNode & node, void * userData)
{
   DataNode * n = &node;
   while(n->GetDepth() > 2) n = n->GetParent();  // go up to session level...
   AbstractReflectSessionRef sref = GetSession(n->GetNodeName()());
   StorageReflectSession * next = (StorageReflectSession *)(sref.GetItemPointer());
   if ((next)&&((next != this)||(GetReflectToSelf()))) next->MessageReceivedFromSession(*this, *((MessageRef *)userData), &node);
   return 2; // This causes the traversal to immediately skip to the next session
}

int
StorageReflectSession ::
KickClientCallback(DataNode & node, void * /*userData*/)
{
   DataNode * n = &node;
   while(n->GetDepth() > 2) n = n->GetParent();  // go up to session level...
   AbstractReflectSessionRef sref = GetSession(n->GetNodeName()());
   StorageReflectSession * next = (StorageReflectSession *)(sref.GetItemPointer());
   if ((next)&&(next != this)) 
   {
      LogTime(MUSCLE_LOG_INFO, "Session [%s/%s] is kicking session [%s/%s] off the server.\n", GetHostName(), GetSessionIDString(), next->GetHostName(), next->GetSessionIDString());
      next->EndSession();  // die!!
   }
   return 2; // This causes the traversal to immediately skip to the next session
}

int
StorageReflectSession ::
GetSubtreesCallback(DataNode & node, void * userData)
{
   bool inMyOwnSubtree = false;  // default:  actual value will only be calculated if it makes a difference
   bool reflectToSelf = GetReflectToSelf();
   if (reflectToSelf == false)
   {
      // Make sure (node) isn't part of our own tree!  If it is, move immediately to the next session
      const DataNode * n = &node;
      while(n->GetDepth() > 2) n = n->GetParent();
      if ((_indexingPresent == false)&&(GetSession(n->GetNodeName()())() == this)) return 2;  // skip to next session node
   }
   // Don't send our own data to our own client; he already knows what we have, because he uploaded it!
   if ((inMyOwnSubtree == false)||(reflectToSelf))
   {
      MessageRef subMsg = GetMessageFromPool();
      String nodePath;
      if ((subMsg() == NULL)||(node.GetNodePath(nodePath) != B_NO_ERROR)||(((Message*)userData)->AddMessage(nodePath, subMsg) != B_NO_ERROR)||(SaveNodeTreeToMessage(*subMsg(), &node, "", true) != B_NO_ERROR)) return 0;
   }
   return node.GetDepth();  // continue traversal as usual
}

int
StorageReflectSession ::
DoSubscribeRefCallback(DataNode & node, void * userData)
{
   node.IncrementSubscriptionRefCount(GetSessionIDString(), (long) userData);
   return node.GetDepth();  // continue traversal as usual
}

int
StorageReflectSession ::
GetDataCallback(DataNode & node, void * userData)
{
   MessageRef * messageArray = (MessageRef *) userData;

   bool inMyOwnSubtree = false;  // default:  actual value will only be calculated if it makes a difference
   bool reflectToSelf = GetReflectToSelf();
   if (reflectToSelf == false)
   {
      // Make sure (node) isn't part of our own tree!  If it is, move immediately to the next session
      const DataNode * n = &node;
      while(n->GetDepth() > 2) n = n->GetParent();
      if ((_indexingPresent == false)&&(GetSession(n->GetNodeName()())() == this)) return 2;  // skip to next session node
   }
 
   // Don't send our own data to our own client; he already knows what we have, because he uploaded it!
   if ((inMyOwnSubtree == false)||(reflectToSelf))
   {
      MessageRef & resultMsg = messageArray[0];
      if (resultMsg() == NULL) resultMsg = GetMessageFromPool(PR_RESULT_DATAITEMS);
      String np;
      if ((resultMsg())&&(node.GetNodePath(np) == B_NO_ERROR))
      {
         (void) resultMsg()->AddMessage(np, node.GetData());
         if (resultMsg()->CountNames() >= _maxSubscriptionMessageItems) SendGetDataResults(resultMsg);
      }
      else 
      {      
         WARN_OUT_OF_MEMORY;
         return 0;  // abort!
      }
   }

   // But indices we need to send to ourself no matter what, as they are generated on the server side.
   const Queue<const char *> * index = node.GetIndex();
   if (index)
   {
      uint32 indexLen = index->GetNumItems();
      if (indexLen > 0)
      {
         MessageRef & indexUpdateMsg = messageArray[1];
         if (indexUpdateMsg() == NULL) indexUpdateMsg = GetMessageFromPool(PR_RESULT_INDEXUPDATED);
         String np;
         if ((indexUpdateMsg())&&(node.GetNodePath(np) == B_NO_ERROR))
         {
            char clearStr[] = {INDEX_OP_CLEARED, '\0'};
            (void) indexUpdateMsg()->AddString(np, clearStr);
            for (uint32 i=0; i<indexLen; i++) 
            {
               char temp[100];
               sprintf(temp, "%c%lu:%s", INDEX_OP_ENTRYINSERTED, i, (*index)[i]);
               (void) indexUpdateMsg()->AddString(np, temp);
            }
            if (indexUpdateMsg()->CountNames() >= _maxSubscriptionMessageItems) SendGetDataResults(messageArray[1]);
         }
         else 
         {
            WARN_OUT_OF_MEMORY;
            return 0;  // abort!
         }
      }
   }

   return node.GetDepth();  // continue traveral as usual
}

int
StorageReflectSession ::
RemoveDataCallback(DataNode & node, void * userData)
{
   if (node.GetDepth() > 2)  // ensure that we never remove host nodes or session nodes this way
   {
      DataNodeRef nodeRef;
      if (node.GetParent()->GetChild(node.GetNodeName()(), nodeRef) == B_NO_ERROR) 
      {
         (void) ((Queue<DataNodeRef> *)userData)->AddTail(nodeRef);
         return node.GetDepth()-1;  // no sense in recursing down a node that we're going to delete anyway
      }
   }
   return node.GetDepth();
}

int
StorageReflectSession ::
InsertOrderedDataCallback(DataNode & node, void * userData)
{
   void ** args = (void **) userData;
   Message * insertMsg = (Message *) args[0];
   Hashtable<String, DataNodeRef> * optRetResults = (Hashtable<String, DataNodeRef> *) args[1];

   MessageFieldNameIterator iter = insertMsg->GetFieldNameIterator(B_MESSAGE_TYPE);
   const String * next;
   while((next = iter.GetNextFieldNameString()) != NULL)
   {
      MessageRef nextRef;
      for (int i=0; (insertMsg->FindMessage(*next, i, nextRef) == B_NO_ERROR); i++)
      {
         if ((_currentNodeCount < _maxNodeCount)&&(node.InsertOrderedChild(nextRef, (*next)(), this, this, optRetResults) == B_NO_ERROR))
         {
            _indexingPresent = true;  // disable optimization in GetDataCallback()
            _currentNodeCount++;
         }
      }
   }
   return node.GetDepth();
}

int
StorageReflectSession ::
ReorderDataCallback(DataNode & node, void * userData)
{
   DataNode * indexNode = node.GetParent();
   if (indexNode) indexNode->ReorderChild(node, (const char *) userData, this);
   return node.GetDepth();
}


StorageReflectSession :: DataNode ::
DataNode() : _children(NULL), _orderedIndex(NULL), _orderedCounter(0L)
{
   _subscribers.SetKeyCompareFunction(CStringCompareFunc);
}

void 
StorageReflectSession :: DataNode ::
Init(const char * name, MessageRef initData)
{
   _nodeName = name;
   _parent   = NULL;
   _depth    = 0;
   _data     = initData;
}

void
StorageReflectSession :: DataNode ::
Reset()
{
   if (_children) _children->Clear();
   if (_orderedIndex) _orderedIndex->Clear();
   _subscribers.Clear();
   _parent = NULL;
   _depth = 0;
   _data.Reset();
}

void
StorageReflectSession :: DataNode :: 
IncrementSubscriptionRefCount(const char * sessionID, long delta)
{
   if (delta > 0)
   {
      uint32 res = 0;
      (void) _subscribers.Get(sessionID, res);
      (void) _subscribers.Put(sessionID, res+delta);  // I'm not sure how to cleanly handle out-of-mem here??  --jaf
   }
   else if (delta < 0)
   {
      uint32 res = 0;
      if (_subscribers.Get(sessionID, res) == B_NO_ERROR)
      {
         uint32 decBy = (uint32) -delta;
         if (decBy >= res) _subscribers.Remove(sessionID);
                      else (void) _subscribers.Put(sessionID, res - decBy);  // out-of-mem shouldn't be possible
      }
   }
}

HashtableIterator<const char *, uint32> 
StorageReflectSession :: DataNode ::
GetSubscribers() const
{
   return _subscribers.GetIterator();
}

status_t
StorageReflectSession :: DataNode ::
InsertOrderedChild(MessageRef data, const char * optInsertBefore, StorageReflectSession * notifyWithOnSetParent, StorageReflectSession * optNotifyChangedData, Hashtable<String, DataNodeRef> * optRetAdded)
{
   if (_orderedIndex == NULL)
   {
      _orderedIndex = newnothrow Queue<const char *>;
      if (_orderedIndex == NULL)
      {
         WARN_OUT_OF_MEMORY; 
         return B_ERROR;
      }
   }

   // Find a unique ID string for our new kid
   char temp[50];
   while(true)
   {
      sprintf(temp, "I%lu", _orderedCounter++);
      if (HasChild(temp) == false) break;
   }

   DataNode * newNode = notifyWithOnSetParent->GetNewDataNode(temp, data);
   if (newNode == NULL)
   {
      WARN_OUT_OF_MEMORY; 
      return B_ERROR;
   }
   DataNodeRef dref(newNode, &_nodePool);

   uint32 insertIndex = _orderedIndex->GetNumItems();  // default to end of index
   if ((optInsertBefore)&&(optInsertBefore[0] == 'I'))  // only 'I''s could be in our index!
   {
      for (int i=_orderedIndex->GetNumItems()-1; i>=0; i--) 
      {
         if (strcmp(optInsertBefore, (*_orderedIndex)[i]) == 0)
         {
            insertIndex = i;
            break;
         }
      }
   }
 
   // Update the index
   if (PutChild(dref, notifyWithOnSetParent, optNotifyChangedData) == B_NO_ERROR)
   {
      if (_orderedIndex->InsertItemAt(insertIndex, newNode->GetNodeName()()) == B_NO_ERROR)
      {
         String np;
         if ((optRetAdded)&&(newNode->GetNodePath(np) == B_NO_ERROR)) (void) optRetAdded->Put(np, dref);

         // Notify anyone monitoring this node that the ordered-index has been updated
         notifyWithOnSetParent->NotifySubscribersThatNodeIndexChanged(*this, INDEX_OP_ENTRYINSERTED, insertIndex, newNode->GetNodeName()());
         return B_NO_ERROR;
      }
      else RemoveChild(dref()->GetNodeName()(), notifyWithOnSetParent, false, NULL);  // undo!
   }
   return B_ERROR;
}

status_t
StorageReflectSession :: DataNode ::
ReorderChild(const DataNode & child, const char * moveToBeforeThis, StorageReflectSession * optNotifyWith)
{
   // Only do anything if we have an index, and the node isn't going to be moved to before itself (silly) and (child) can be removed from the index
   if ((_orderedIndex)&&((moveToBeforeThis == NULL)||(strcmp(moveToBeforeThis, child.GetNodeName()())))&&(RemoveIndexEntry(child.GetNodeName()(), optNotifyWith) == B_NO_ERROR))
   {
      // Then re-add him to the index at the appropriate point
      uint32 targetIndex = _orderedIndex->GetNumItems();  // default to end of index
      if ((moveToBeforeThis)&&(HasChild(moveToBeforeThis)))
      {
         for (int i=_orderedIndex->GetNumItems()-1; i>=0; i--) 
         { 
            if (strcmp((*_orderedIndex)[i], moveToBeforeThis) == 0)
            {
               targetIndex = i;
               break; 
            }
         }
      }

      // Now add the child back into the index at his new position
      if (_orderedIndex->InsertItemAt(targetIndex, child.GetNodeName()()) == B_NO_ERROR)
      {
         // Notify anyone monitoring this node that the ordered-index has been updated
         if (optNotifyWith) optNotifyWith->NotifySubscribersThatNodeIndexChanged(*this, INDEX_OP_ENTRYINSERTED, targetIndex, child.GetNodeName()());
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t
StorageReflectSession :: DataNode ::
PutChild(DataNodeRef & node, StorageReflectSession * optNotifyWithOnSetParent, StorageReflectSession * optNotifyChangedData)
{
   status_t ret = B_ERROR;
   DataNode * child = node.GetItemPointer();
   if (child)
   {
      if (_children == NULL) 
      {
         _children = newnothrow Hashtable<const char *, DataNodeRef>;
         if (_children == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
         _children->SetKeyCompareFunction(CStringCompareFunc);
      }
      child->SetParent(this, optNotifyWithOnSetParent);
      ret = _children->Put(child->_nodeName(), node);
      if (optNotifyChangedData) optNotifyChangedData->NotifySubscribersThatNodeChanged(*child, false);
   }
   return ret;
}

void
StorageReflectSession :: DataNode ::
SetParent(DataNode * parent, StorageReflectSession * optNotifyWith)
{
   if ((_parent)&&(parent)) LogTime(MUSCLE_LOG_WARNING, "Warning, overwriting previous parent of node [%s]\n", GetNodeName()());
   _parent = parent;
   if (_parent == NULL) _subscribers.Clear();

   // Calculate our node's depth into the tree
   _depth = 0;
   if (_parent)
   {
      // Calculate the total length that our node path string will be
      const DataNode * node = this;
      while(node->_parent) 
      {
         _depth++;
         node = node->_parent;
      }
      if (optNotifyWith) optNotifyWith->NotifySubscribersOfNewNode(*this);
   }
}

const char *
StorageReflectSession :: DataNode ::
GetPathClause(uint32 depth) const
{
   if (depth <= _depth)
   {
      const DataNode * node = this;
      for (uint32 i=depth; i<_depth; i++) if (node) node = node->_parent;
      if (node) return node->GetNodeName()();
   }
   return NULL;
}

status_t
StorageReflectSession :: DataNode ::
GetNodePath(String & retPath) const
{
   // Calculate node path and node depth
   if (_parent)
   {
      // Calculate the total length that our node path string will be
      uint32 pathLen = 0;
      {
         const DataNode * node = this;
         while(node->_parent) 
         {
            pathLen += 1 + node->_nodeName.Length();  // the 1 is for the slash
            node = node->_parent;
         }
      }

      // Might as well make sure we have enough memory to return it, up front
      if (retPath.Prealloc(pathLen) != B_NO_ERROR) return B_ERROR;

      char * aBuf = NULL;
      const uint32 staticAllocSize = 256;
      char sBuf[staticAllocSize];      // try to do this without a dynamic allocation...
      if (pathLen >= staticAllocSize)  // but do a dynamic allocation if we have to (should be rare)
      {
         aBuf = newnothrow char[pathLen+1];
         if (aBuf == NULL) 
         { 
            WARN_OUT_OF_MEMORY; 
            return B_ERROR;
         }
      }

      char * writeAt = (aBuf ? aBuf : sBuf) + pathLen;  // points to last char in buffer
      *writeAt = '\0';  // terminate the string first (!)
      const DataNode * node = this;
      while(node->_parent)
      {
         int len = node->_nodeName.Length();
         writeAt -= len;
         memcpy(writeAt, node->_nodeName(), len);
         *(--writeAt) = '/';
         node = node->_parent;
      }
 
      retPath = (aBuf ? aBuf : sBuf);
      delete [] aBuf;
   }
   else retPath = "";

   return B_NO_ERROR;
}

status_t
StorageReflectSession :: DataNode ::   
RemoveChild(const char * key, StorageReflectSession * optNotifyWith, bool recurse, uint32 * optCurrentNodeCount)
{
   DataNodeRef childRef;
   if ((_children)&&(_children->Get(key, childRef) == B_NO_ERROR))
   {
      DataNode * child = childRef.GetItemPointer();
      if (child)
      {
         if (optNotifyWith) optNotifyWith->NotifySubscribersThatNodeChanged(*child, true);

         (void) RemoveIndexEntry(key, optNotifyWith);

         if (recurse)
         { 
            while(child->CountChildren() > 0)
            {
               DataNodeRefIterator it = child->GetChildIterator();
               const char * name;
               it.GetNextKey(name);
               child->RemoveChild(name, optNotifyWith, recurse, optCurrentNodeCount);
            }
         }

         child->SetParent(NULL, optNotifyWith);
      }
      if (optCurrentNodeCount) 
      {
//         MASSERT(*optCurrentNodeCount > 0, "RemoveChild:  node count was about to go negative!!");
         (*optCurrentNodeCount)--;
      }
      _children->Remove(key, childRef);
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t
StorageReflectSession :: DataNode ::
RemoveIndexEntry(const char * key, StorageReflectSession * optNotifyWith)
{
   // Update our ordered-node index & notify everyone about the change
   if ((_orderedIndex)&&(key[0] == 'I'))  // if it doesn't start with I, we know it's not part of our ordered-index!
   {
      for (int i=_orderedIndex->GetNumItems()-1; i>=0; i--)
      {
         if (strcmp(key, (*_orderedIndex)[i]) == 0)
         {
            _orderedIndex->RemoveItemAt(i);
            if (optNotifyWith) optNotifyWith->NotifySubscribersThatNodeIndexChanged(*this, INDEX_OP_ENTRYREMOVED, i, key);
            return B_NO_ERROR;
         }
      }
   }
   return B_ERROR;
}

void
StorageReflectSession :: DataNode ::   
SetData(MessageRef data, StorageReflectSession * optNotifyWith)
{
   _data = data;
   if (optNotifyWith) optNotifyWith->NotifySubscribersThatNodeChanged(*this, false);
}

StorageReflectSession :: DataNode ::
~DataNode() 
{
   delete _children;
   delete _orderedIndex;
}

bool
StorageReflectSession :: NodePathMatcher ::
MatchesNode(DataNode & node, int rootDepth) const
{
   for (int i=GetNumPaths()-1; i>=0; i--) if (PathMatches(node, i, rootDepth)) return true;
   return false;
}

int
StorageReflectSession :: NodePathMatcher ::
GetMatchCount(DataNode & node, int rootDepth) const
{
   int matchCount = 0;
   for (int i=GetNumPaths(); i>=0; i--) if (PathMatches(node, i, rootDepth)) matchCount++;
   return matchCount;
}

bool
StorageReflectSession :: NodePathMatcher ::
PathMatches(DataNode & node, int whichPath, int rootDepth) const
{
   const StringMatcherQueue * nextSubscription = GetParserQueue(whichPath);
   if (nextSubscription == NULL) return false;
   int pd = nextSubscription->GetNumItems();
   if (pd == ((int32)node.GetDepth())-rootDepth)  // only paths with the same number of clauses as the node's path (less rootDepth) can ever match
   {
      DataNode * travNode = &node;
      bool match = true;  // optimistic default
      for (int j=nextSubscription->GetNumItems()-1; j>=rootDepth; j--,travNode=travNode->GetParent())
      {
         StringMatcher * nextMatcher = nextSubscription->GetItemAt(j)->GetItemPointer();
         if ((nextMatcher)&&(nextMatcher->Match(travNode->GetNodeName()()) == false))
         {
            match = false;
            break; 
         }
      }
      if (match) return true;
   }
   return false;
}

void
StorageReflectSession :: NodePathMatcher ::
DoTraversal(PathMatchCallback cb, StorageReflectSession * This, DataNode & node, void * userData)
{
   (void) DoTraversalAux(cb, This, node, userData, node.GetDepth());
}

int 
StorageReflectSession :: NodePathMatcher ::
DoTraversalAux(PathMatchCallback cb, StorageReflectSession * This, DataNode & node, void * userData, int rootDepth)
{
   DataNodeRef nextChildRef;
   DataNodeRefIterator it = node.GetChildIterator();
   int depth = node.GetDepth();
   while(it.GetNextValue(nextChildRef) == B_NO_ERROR) 
   {
      DataNode * nextChild = nextChildRef.GetItemPointer();
      if (nextChild)
      {
         const char * nextChildName = nextChild->GetNodeName()();
         bool matched  = false;  // set if we have called the callback on this child already
         bool recursed = false;  // set if we have recursed to this child already

         // Try all parsers and see if any of them match at this level
         int numParsers = GetNumPaths();
         for (int i=0; i<numParsers; i++)
         {
            const StringMatcherQueue * nextQueue = GetParserQueue(i);
            if (nextQueue)
            {
               int numClausesInParser = nextQueue->GetNumItems();
               if (numClausesInParser > depth-rootDepth)
               {
                  StringMatcher * nextMatcher = nextQueue->GetItemAt(depth-rootDepth)->GetItemPointer();
                  if ((nextMatcher == NULL)||(nextMatcher->Match(nextChildName)))
                  {
                     // A match!  Now, depending on whether this match is the
                     // last clause in the path or not, we either do the callback or descend.
                     // But we make sure not to do either of these things more than once per node.
                     if (depth == rootDepth+numClausesInParser-1)
                     {
                        if (matched == false)
                        {
                           // when there is more than one string being used to match,
                           // it's possible that two or more strings can "conspire"
                           // to match a node even though any given string doesn't match it.
                           // For example, if we have the match-strings:
                           //    /j*/k*
                           //    /k*/j*
                           // The node /jeremy/jenny would match, even though it isn't
                           // specified by any of the subscription strings.  This is bad.
                           // So for multiple match-strings, we do an additional check 
                           // to make sure there is a NodePathMatcher for this node.
                           if ((numParsers == 1)||(MatchesNode(*nextChild, rootDepth)))
                           {
                              int nextDepth = cb(This, *nextChild, userData);
                              if (nextDepth < ((int)nextChild->GetDepth())-1) return nextDepth;
                              matched = true;
                              if (recursed) break;  // done both possibile actions, so be lazy
                           }
                        }
                     }
                     else
                     {
                        if (recursed == false)
                        {
                           // If we match a non-terminal clause in the path, recurse to the child.
                           int nextDepth = DoTraversalAux(cb, This, *nextChild, userData, rootDepth);
                           if (nextDepth < ((int)nextChild->GetDepth())-1) return nextDepth;
                           recursed = true;
                           if (matched) break;  // done both possible actions, so be lazy
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return node.GetDepth();
}

StorageReflectSession::DataNode *
StorageReflectSession ::
GetNewDataNode(const char * name, MessageRef initialValue)
{
   DataNode * n = _nodePool.ObtainObject();
   if (n) n->Init(name, initialValue);
   return n;
}

void
StorageReflectSession ::
ReleaseDataNode(DataNode * node)
{
   _nodePool.ReleaseObject(node);
}

void
StorageReflectSession ::
JettisonOutgoingSubtrees(const char * optMatchString)
{
   AbstractMessageIOGateway * gw = GetGateway();
   if (gw)
   {
      StringMatcher sm(optMatchString?optMatchString:"");
      Queue<MessageRef> & oq = gw->GetOutgoingMessageQueue();
      for (int i=oq.GetNumItems()-1; i>=0; i--)  // must do this backwards!
      {
         Message * msg = oq.GetItemAt(i)->GetItemPointer();
         if ((msg)&&(msg->what == PR_RESULT_DATATREES))
         {
            bool removeIt = false;
            const char * batchID = NULL; (void) msg->FindString(PR_NAME_TREE_REQUEST_ID, &batchID);
            if (optMatchString)
            {
               if ((batchID)&&(sm.Match(batchID))) removeIt = true;
            }
            else if (batchID == NULL) removeIt = true;

            if (removeIt) oq.RemoveItemAt(i);
         }
      }
   }
}

void
StorageReflectSession ::
JettisonOutgoingResults(const NodePathMatcher * matcher)
{
   AbstractMessageIOGateway * gw = GetGateway();
   if (gw)
   {
      Queue<MessageRef> & oq = gw->GetOutgoingMessageQueue();
      for (int i=oq.GetNumItems()-1; i>=0; i--)  // must do this backwards!
      {
         Message * msg = oq.GetItemAt(i)->GetItemPointer();
         if ((msg)&&(msg->what == PR_RESULT_DATAITEMS))
         {
            if (matcher)
            {
               // Remove any PR_NAME_REMOVED_DATAITEMS entries that match... 
               int nextr = 0;
               const char * rname;
               while(msg->FindString(PR_NAME_REMOVED_DATAITEMS, nextr, &rname) == B_NO_ERROR)
               {
                  if (matcher->MatchesPath(rname)) msg->RemoveData(PR_NAME_REMOVED_DATAITEMS, nextr);
                                              else nextr++;
               }
   
               // Remove all matching items from the Message.  (Yes, the iterator can handle this!  :^))
               const char * nextFieldName;
               MessageFieldNameIterator iter = msg->GetFieldNameIterator(B_MESSAGE_TYPE);
               while((nextFieldName = iter.GetNextFieldName()) != NULL) if (matcher->MatchesPath(nextFieldName)) msg->RemoveName(nextFieldName);
            }
            else msg->Clear();

            if (msg->CountNames() == 0) (void) oq.RemoveItemAt(i);
         }
      }
   }
}

status_t
StorageReflectSession :: CloneDataNodeSubtree(const DataNode & node, const String & destPath)
{
   // First clone the given node
   if (SetDataNode(destPath, node.GetData()) != B_NO_ERROR) return B_ERROR;

   // Then clone all of his children
   DataNodeRefIterator iter = node.GetChildIterator();
   const char * nextChildName;
   DataNodeRef nextChild; 
   while((iter.GetNextKey(nextChildName) == B_NO_ERROR)&&(iter.GetNextValue(nextChild) == B_NO_ERROR))
   {
      const DataNode * child = nextChild();
      if (child)
      {
         String childPath = destPath;
         childPath += '/';
         childPath += nextChildName;
         if (CloneDataNodeSubtree(*child, childPath) != B_NO_ERROR) return B_ERROR;
      }
   }

   return B_NO_ERROR;
}

// Recursive helper function
status_t
StorageReflectSession :: SaveNodeTreeToMessage(Message & msg, const DataNode * node, const String & path, bool saveData) const
{
   if ((saveData)&&(msg.AddMessage(PR_NAME_NODEDATA, node->GetData()) != B_NO_ERROR)) return B_NO_ERROR;
   
   if (node->CountChildren() > 0)
   {
      // Save the node-index, if there is one
      const Queue<const char *> * index = node->GetIndex();
      if (index)
      {
         uint32 indexSize = index->GetNumItems();
         if (indexSize > 0)
         {
            MessageRef indexMsgRef(GetMessageFromPool());
            if ((indexMsgRef() == NULL)||(msg.AddMessage(PR_NAME_NODEINDEX, indexMsgRef) != B_NO_ERROR)) return B_ERROR;
            Message * indexMsg = indexMsgRef();
            for (uint32 i=0; i<indexSize; i++) if (indexMsg->AddString(PR_NAME_KEYS, (*index)[i]) != B_NO_ERROR) return B_ERROR;
         }
      }

      // Then save the children, recursing to each one as necessary
      MessageRef childrenMsgRef(GetMessageFromPool());
      if ((childrenMsgRef() == NULL)||(msg.AddMessage(PR_NAME_NODECHILDREN, childrenMsgRef) != B_NO_ERROR)) return B_ERROR;
      DataNodeRefIterator childIter = node->GetChildIterator();
      DataNodeRef nextChildRef;
      while(childIter.GetNextValue(nextChildRef) == B_NO_ERROR) 
      {
         DataNode * child = nextChildRef();
         if (child)
         {
            String childPath(path);
            if (childPath.Length() > 0) childPath += '/';
            childPath += child->GetNodeName();

            MessageRef childMsgRef(GetMessageFromPool());
            if ((childMsgRef() == NULL)||(childrenMsgRef()->AddMessage(child->GetNodeName(), childMsgRef) != B_NO_ERROR)||(SaveNodeTreeToMessage(*childMsgRef(), child, childPath, true) != B_NO_ERROR)) return B_ERROR;
         }
      }
   }
   return B_NO_ERROR;
}

status_t
StorageReflectSession :: RestoreNodeTreeFromMessage(const Message & msg, const String & path, bool loadData, bool appendToIndex)
{
   if (loadData)
   {
      // Load in data payload for the current node
      MessageRef dataRef;
      if ((msg.FindMessage(PR_NAME_NODEDATA, dataRef) == B_NO_ERROR)&&(SetDataNode(path, dataRef, true, true, false, appendToIndex) != B_NO_ERROR)) return B_ERROR;
   }

   MessageRef childrenRef;
   if ((msg.FindMessage(PR_NAME_NODECHILDREN, childrenRef) == B_NO_ERROR)&&(childrenRef()))
   {
      // First recurse to the indexed nodes, adding them as indexed children
      Hashtable<const char *, uint32> indexLookup;
      indexLookup.SetKeyCompareFunction(CStringCompareFunc);
      {
         MessageRef indexRef;
         if (msg.FindMessage(PR_NAME_NODEINDEX, indexRef) == B_NO_ERROR)
         {
            const char * nextFieldName;
            for (int i=0; indexRef()->FindString(PR_NAME_KEYS, i, &nextFieldName) == B_NO_ERROR; i++)
            {
               MessageRef nextChildRef;
               if (childrenRef()->FindMessage(nextFieldName, nextChildRef) == B_NO_ERROR) 
               {
                  String childPath(path);
                  if (childPath.Length() > 0) childPath += '/';
                  childPath += nextFieldName;
                  if (RestoreNodeTreeFromMessage(*nextChildRef(), childPath, true, true) != B_NO_ERROR) return B_ERROR;
                  if (indexLookup.Put(nextFieldName, i) != B_NO_ERROR) return B_ERROR;
               }
            }
         }
      }

      // Then recurse to the non-indexed child nodes 
      {
         MessageFieldNameIterator iter = childrenRef()->GetFieldNameIterator(B_MESSAGE_TYPE);
         const char * nextFieldName;
         while((nextFieldName = iter.GetNextFieldName()) != NULL)
         {
            if (indexLookup.ContainsKey(nextFieldName) == false)
            {
               MessageRef nextChildRef;
               if ((childrenRef()->FindMessage(nextFieldName, nextChildRef) == B_NO_ERROR)&&(nextChildRef()))
               {
                  String childPath(path);
                  if (childPath.Length() > 0) childPath += '/';
                  childPath += nextFieldName;
                  if (RestoreNodeTreeFromMessage(*nextChildRef(), childPath, true) != B_NO_ERROR) return B_ERROR;
               }
            }
         }
      }
   }
   return B_NO_ERROR;   
}

};  // end namespace muscle

