/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include <limits.h>
#include "reflector/ReflectServer.h"
#include "reflector/StorageReflectSession.h"
#include "iogateway/MessageIOGateway.h"

namespace muscle {

#define DEFAULT_PATH_PREFIX "*/*"  // when we get a path name without a leading '/', prepend this
#define DEFAULT_MAX_SUBSCRIPTION_MESSAGE_SIZE   50   // no more than 50 items/update message, please

static void ResetNodeFunc(DataNode * node, void * /*userData*/) {node->Reset();}
static DataNodeRef::ItemPool _nodePool(100, ResetNodeFunc);

// field under which we file our shared data in the central-state message
static const String SRS_SHARED_DATA = "srs_shared";

StorageReflectSessionFactory :: StorageReflectSessionFactory() : _maxIncomingMessageSize(MUSCLE_NO_LIMIT)
{
   // empty
}

AbstractReflectSessionRef StorageReflectSessionFactory :: CreateSession(const String &, const IPAddressAndPort &)
{
   TCHECKPOINT;

   AbstractReflectSession * srs = newnothrow StorageReflectSession;
   AbstractReflectSessionRef ret(srs);
 
   if ((srs)&&(SetMaxIncomingMessageSizeFor(srs) == B_NO_ERROR)) return ret;
   else
   {
      WARN_OUT_OF_MEMORY;
      return AbstractReflectSessionRef();
   }
}

status_t StorageReflectSessionFactory :: SetMaxIncomingMessageSizeFor(AbstractReflectSession * session) const
{
   if (_maxIncomingMessageSize != MUSCLE_NO_LIMIT) 
   {
      if (session->GetGateway()() == NULL) session->SetGateway(session->CreateGateway());
      MessageIOGateway * gw = dynamic_cast<MessageIOGateway*>(session->GetGateway()());
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
   TCHECKPOINT;

   Message & state = GetCentralState();

   void * sp = NULL; (void) state.FindPointer(SRS_SHARED_DATA, sp);
   StorageReflectSession::StorageReflectSessionSharedData * sd = (StorageReflectSession::StorageReflectSessionSharedData *) sp;
   if (sd) return sd;
  
   // oops, there's no shared data object!  We must be the first session.
   // So we'll create the root node and the shared data object, and
   // add it to the central-state Message ourself.
   DataNodeRef globalRoot = GetNewDataNode("", GetEmptyMessageRef());
   if (globalRoot()) 
   {
      sd = newnothrow StorageReflectSessionSharedData(globalRoot);
      if (sd)
      {
         if (state.ReplacePointer(true, SRS_SHARED_DATA, sd) == B_NO_ERROR) return sd;
         delete sd;
      }
      else WARN_OUT_OF_MEMORY;
   }
   return NULL;
}

status_t
StorageReflectSession ::
AttachedToServer()
{
   TCHECKPOINT;

   if (DumbReflectSession::AttachedToServer() != B_NO_ERROR) return B_ERROR;

   _sharedData = InitSharedData();
   if (_sharedData == NULL) return B_ERROR;
   
   Message & state = GetCentralState();
   const String & hostname = GetHostName();
   const String & sessionid = GetSessionIDString();

   // Is there already a node for our hostname?
   DataNodeRef hostDir;
   if (GetGlobalRoot().GetChild(hostname, hostDir) != B_NO_ERROR)
   {
      // nope.... we'll add one then
      hostDir = GetNewDataNode(hostname, GetEmptyMessageRef());
      if ((hostDir() == NULL)||(GetGlobalRoot().PutChild(hostDir, this, this) != B_NO_ERROR)) {WARN_OUT_OF_MEMORY; Cleanup(); return B_ERROR;}
   }

   // Create a new node for our session (we assume no such
   // node already exists, as session id's are supposed to be unique)
   if (hostDir() == NULL) {Cleanup(); return B_ERROR;}
   if (hostDir()->HasChild(sessionid())) LogTime(MUSCLE_LOG_WARNING, "WARNING:  Non-unique session id [%s] being overwritten!\n", sessionid());

   SetSessionRootPath(hostname.Prepend("/") + "/" + sessionid);

   DataNodeRef sessionNode = GetNewDataNode(sessionid, GetEmptyMessageRef());
   if (sessionNode())
   {
      // See if we get any special privileges
      int32 privBits = 0;
      for (int p=0; p<=PR_NUM_PRIVILEGES; p++)
      {
         char temp[32];
         sprintf(temp, "priv%i", p);
         const String * privPattern;
         for (int q=0; (state.FindString(temp, q, &privPattern) == B_NO_ERROR); q++)
         {
            if (StringMatcher(*privPattern).Match(hostname()))
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

      _sessionDir = sessionNode;
      if (hostDir()->PutChild(_sessionDir, this, this) != B_NO_ERROR) {WARN_OUT_OF_MEMORY; Cleanup(); return B_ERROR;}
 
      // do subscription notifications here
      PushSubscriptionMessages();

      // Get our node-creation limit.  For now, this is the same for all sessions.
      // (someday maybe I'll work out a way to give different limits to different sessions)
      uint32 nodeLimit;
      if (state.FindInt32(PR_NAME_MAX_NODES_PER_SESSION, nodeLimit) == B_NO_ERROR) _maxNodeCount = nodeLimit;
   
      return B_NO_ERROR;
   }

   WARN_OUT_OF_MEMORY; 
   Cleanup(); 

   TCHECKPOINT;
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
   TCHECKPOINT;

   if (_sharedData)
   {
      DataNodeRef hostNodeRef;
      if (GetGlobalRoot().GetChild(GetHostName(), hostNodeRef) == B_NO_ERROR)
      {
         DataNode * hostNode = hostNodeRef();
         if (hostNode)
         {
            // make sure our session node is gone
            hostNode->RemoveChild(GetSessionIDString(), this, true, NULL);

            // If our host node is now empty, it goes too
            if (hostNode->HasChildren() == false) GetGlobalRoot().RemoveChild(hostNode->GetNodeName(), this, true, NULL);
         }
      
         PushSubscriptionMessages();
      }

      // If the global root is now empty, it goes too
      if (GetGlobalRoot().HasChildren() == false)
      {
         GetCentralState().RemoveName(SRS_SHARED_DATA);
         _sharedData->_root.Reset(); // do this first!
         delete _sharedData;
      }
      else 
      {
         // Remove all of our subscription-marks from neighbor's nodes
         _subscriptions.DoTraversal((PathMatchCallback)DoSubscribeRefCallbackFunc, this, GetGlobalRoot(), false, (void *)(-LONG_MAX));
      }
      _sharedData = NULL;
   }
   _nextSubscriptionMessage.Reset();
   _nextIndexSubscriptionMessage.Reset();

   TCHECKPOINT;
}

void 
StorageReflectSession ::
NotifySubscribersThatNodeChanged(DataNode & modifiedNode, const MessageRef & oldData, bool isBeingRemoved)
{
   TCHECKPOINT;

   HashtableIterator<const String *, uint32> subIter = modifiedNode.GetSubscribers();
   const String * nextKey = NULL;  // set just to shut the compiler up
   while(subIter.GetNextKey(nextKey) == B_NO_ERROR)
   {
      StorageReflectSession * next = dynamic_cast<StorageReflectSession *>(GetSession(*nextKey)());
      if ((next)&&((next != this)||(GetReflectToSelf()))) next->NodeChanged(modifiedNode, oldData, isBeingRemoved);
   }

   TCHECKPOINT;
}

void 
StorageReflectSession ::
NotifySubscribersThatNodeIndexChanged(DataNode & modifiedNode, char op, uint32 index, const String & key)
{
   TCHECKPOINT;

   HashtableIterator<const String *, uint32> subIter = modifiedNode.GetSubscribers();
   const String * next = NULL;  // set just to shut the compiler up
   while(subIter.GetNextKey(next) == B_NO_ERROR) 
   {
      AbstractReflectSessionRef nRef = GetSession(*next);
      StorageReflectSession * s = dynamic_cast<StorageReflectSession *>(nRef());
      if (s) s->NodeIndexChanged(modifiedNode, op, index, key);
   }

   TCHECKPOINT;
}

void
StorageReflectSession ::
NotifySubscribersOfNewNode(DataNode & newNode)
{
   TCHECKPOINT;

   HashtableIterator<const String *, AbstractReflectSessionRef> sessions(GetSessions());
   AbstractReflectSessionRef * lRef;
   while((lRef = sessions.GetNextValue()) != NULL)
   {
      StorageReflectSession * next = dynamic_cast<StorageReflectSession *>(lRef->GetItemPointer());
      if (next) next->NodeCreated(newNode);  // always notify; !Self filtering will be done elsewhere
   }

   TCHECKPOINT;
}

void
StorageReflectSession ::
NodeCreated(DataNode & newNode)
{
   newNode.IncrementSubscriptionRefCount(GetSessionIDString(), _subscriptions.GetMatchCount(newNode, NULL, 0));
}

void
StorageReflectSession ::
NodeChanged(DataNode & modifiedNode, const MessageRef & oldData, bool isBeingRemoved)
{
   TCHECKPOINT;

   if (GetSubscriptionsEnabled())
   {
      if (_subscriptions.GetNumFilters() > 0)
      {
         // uh oh... we gotta determine whether the modified node's status wrt QueryFilters has changed!
         // Based on that, we will simulate the node's addition or removal at the appropriate times.
         if (isBeingRemoved)
         {
            if (_subscriptions.MatchesNode(modifiedNode, oldData(), 0) == false) return; // already didn't match, no remove-update is necessary
         }
         else if (oldData())
         {
            bool matches = _subscriptions.MatchesNode(modifiedNode, modifiedNode.GetData()(), 0);  // UPDATE
            if (matches == false)
            {
               if (_subscriptions.MatchesNode(modifiedNode, oldData(), 0)) isBeingRemoved = true;  // did but now doesn't?  Then remove it
                                                                      else return;                 // still doesn't?  Then do nothing
            }
         }
         else if (_subscriptions.MatchesNode(modifiedNode, modifiedNode.GetData()(), 0) == false) return;  // ADD: only if it matches!
      }
      NodeChangedAux(modifiedNode, isBeingRemoved);
   }
}

void
StorageReflectSession ::
NodeChangedAux(DataNode & modifiedNode, bool isBeingRemoved)
{
   TCHECKPOINT;

   if (_nextSubscriptionMessage() == NULL) _nextSubscriptionMessage = GetMessageFromPool(PR_RESULT_DATAITEMS);
   if (_nextSubscriptionMessage())
   {
      _sharedData->_subsDirty = true;
      String np;
      if (modifiedNode.GetNodePath(np) == B_NO_ERROR)
      {
         if (isBeingRemoved) 
         {
            if (_nextSubscriptionMessage()->HasName(np, B_MESSAGE_TYPE))
            {
               // Oops!  We can't specify a remove-then-add operation for a given node in a single Message,
               // because the removes and the adds are expressed via different mechanisms.
               // So in this case we have to force a flush of the current message now, and 
               // then add the new notification to the next one!
               PushSubscriptionMessages();
               NodeChangedAux(modifiedNode, isBeingRemoved);  // and then start again
            }
            else _nextSubscriptionMessage()->AddString(PR_NAME_REMOVED_DATAITEMS, np);
         }
         else _nextSubscriptionMessage()->AddMessage(np, modifiedNode.GetData());
      }
      if (_nextSubscriptionMessage()->GetNumNames() >= _maxSubscriptionMessageItems) PushSubscriptionMessages(); 
   }
   else WARN_OUT_OF_MEMORY;
}

void
StorageReflectSession ::
NodeIndexChanged(DataNode & modifiedNode, char op, uint32 index, const String & key)
{
   TCHECKPOINT;

   if (GetSubscriptionsEnabled())
   {
      if (_nextIndexSubscriptionMessage() == NULL) _nextIndexSubscriptionMessage = GetMessageFromPool(PR_RESULT_INDEXUPDATED);

      String np;
      if ((_nextIndexSubscriptionMessage())&&(modifiedNode.GetNodePath(np) == B_NO_ERROR))
      {
         _sharedData->_subsDirty = true;
         char temp[100];
         sprintf(temp, "%c"UINT32_FORMAT_SPEC":%s", op, index, key());
         _nextIndexSubscriptionMessage()->AddString(np, temp);
      }
      else WARN_OUT_OF_MEMORY;
      // don't push subscription messages here.... it will be done elsewhere
   }
}

status_t
StorageReflectSession ::
SetDataNode(const String & nodePath, const MessageRef & dataMsgRef, bool overwrite, bool create, bool quiet, bool addToIndex, const String * optInsertBefore)
{
   TCHECKPOINT;

   DataNode * node = _sessionDir();
   if (node == NULL) return B_ERROR;
 
   if ((nodePath.HasChars())&&(nodePath[0] != '/'))
   {
      int prevSlashPos = -1;
      int slashPos = 0;
      DataNodeRef childNodeRef;
      String nextClause;
      while(slashPos >= 0)
      {
         slashPos = nodePath.IndexOf('/', prevSlashPos+1);
         nextClause = nodePath.Substring(prevSlashPos+1, (slashPos >= 0) ? slashPos : nodePath.Length());
         DataNodeRef allocedNode;
         if (node->GetChild(nextClause, childNodeRef) != B_NO_ERROR)
         {
            if ((create)&&(_currentNodeCount < _maxNodeCount))
            {
               allocedNode = GetNewDataNode(nextClause, ((slashPos < 0)&&(addToIndex == false)) ? dataMsgRef : GetEmptyMessageRef());
               if (allocedNode())
               {
                  childNodeRef = allocedNode;
                  if ((slashPos < 0)&&(addToIndex))
                  {
                     if (node->InsertOrderedChild(dataMsgRef, optInsertBefore, (nextClause.HasChars())?&nextClause:NULL, this, quiet?NULL:this, NULL) == B_NO_ERROR)
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

         node = childNodeRef();
         if ((slashPos < 0)&&(addToIndex == false))
         {
            if ((node == NULL)||((overwrite == false)&&(node != allocedNode()))) return B_ERROR;
            node->SetData(dataMsgRef, quiet ? NULL : this, (node == allocedNode()));  // do this to trigger the changed-notification
         }
         prevSlashPos = slashPos;
      }
   }

   return B_NO_ERROR;
}

bool
StorageReflectSession ::
HasPrivilege(int priv) const
{
   uint32 privBits;
   return (_parameters.FindInt32(PR_NAME_PRIVILEGE_BITS, privBits) == B_NO_ERROR) ? ((privBits & (1L<<priv)) != 0L) : false;
}

void
StorageReflectSession ::
AfterMessageReceivedFromGateway(const MessageRef & msgRef, void * userData)
{
   AbstractReflectSession::AfterMessageReceivedFromGateway(msgRef, userData);
   PushSubscriptionMessages();
}

void 
StorageReflectSession :: 
MessageReceivedFromGateway(const MessageRef & msgRef, void * userData)
{
   TCHECKPOINT;

   Message * msgp = msgRef();
   if (msgp == NULL) return;

   Message & msg = *msgp;
   if (muscleInRange(msg.what, (uint32)BEGIN_PR_COMMANDS, (uint32)END_PR_COMMANDS))
   {
      switch(msg.what)
      {
         case PR_COMMAND_JETTISONDATATREES:
         {
            if (msg.HasName(PR_NAME_TREE_REQUEST_ID, B_STRING_TYPE)) 
            {
               const String * str;
               for (int32 i=0; msg.FindString(PR_NAME_TREE_REQUEST_ID, i, &str) == B_NO_ERROR; i++) JettisonOutgoingSubtrees(str);
            }
            else JettisonOutgoingSubtrees(NULL);
         }
         break;

         case PR_COMMAND_SETDATATREES:
            BounceMessage(PR_RESULT_ERRORUNIMPLEMENTED, msgRef);    // not implemented, for now
         break;

         case PR_COMMAND_GETDATATREES:
         {
            const String * id = NULL; (void) msg.FindString(PR_NAME_TREE_REQUEST_ID, &id);
            MessageRef reply = GetMessageFromPool(PR_RESULT_DATATREES);
            if ((reply())&&((id==NULL)||(reply()->AddString(PR_NAME_TREE_REQUEST_ID, *id) == B_NO_ERROR)))
            {
               if (msg.HasName(PR_NAME_KEYS, B_STRING_TYPE)) 
               {
                  int32 maxDepth = -1;  (void) msg.FindInt32(PR_NAME_MAXDEPTH, maxDepth);

                  NodePathMatcher matcher;
                  matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, msg, DEFAULT_PATH_PREFIX);

                  void * args[] = {reply(), (void *)((long)maxDepth)};
                  matcher.DoTraversal((PathMatchCallback)GetSubtreesCallbackFunc, this, GetGlobalRoot(), true, args);
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
            for (int i=0; msg.FindMessage(PR_NAME_KEYS, i, subRef) == B_NO_ERROR; i++) CallMessageReceivedFromGateway(subRef, userData);
         }
         break;

         case PR_COMMAND_KICK:
         {
            if (HasPrivilege(PR_PRIVILEGE_KICK))
            { 
               if (msg.HasName(PR_NAME_KEYS, B_STRING_TYPE)) 
               {
                  NodePathMatcher matcher;
                  matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, msg, DEFAULT_PATH_PREFIX);
                  matcher.DoTraversal((PathMatchCallback)KickClientCallbackFunc, this, GetGlobalRoot(), true, NULL);
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
            bool updateDefaultMessageRoute = false;
            bool subscribeQuietly = msg.HasName(PR_NAME_SUBSCRIBE_QUIETLY);
            MessageFieldNameIterator it = msg.GetFieldNameIterator();
            Message getMsg(PR_COMMAND_GETDATA);
            const String * fn;
            while((fn = it.GetNextFieldNameString()) != NULL)
            {
               bool copyField = true;
               if (strncmp(fn->Cstr(), "SUBSCRIBE:", 10) == 0)
               {
                  QueryFilterRef filter;
                  MessageRef filterMsgRef;
                  if (msg.FindMessage(*fn, filterMsgRef) == B_NO_ERROR) filter = GetGlobalQueryFilterFactory()()->CreateQueryFilter(*filterMsgRef());
                  
                  String path = fn->Substring(10);
                  String fixPath(path);
                  _subscriptions.AdjustStringPrefix(fixPath, DEFAULT_PATH_PREFIX);
                  PathMatcherEntry * e = _subscriptions.GetEntries().Get(fixPath);
                  if (e)
                  {
                     const QueryFilter * subscriptionFilter = e->GetFilter()();
                     if ((GetSubscriptionsEnabled())&&((filter() != NULL)||(subscriptionFilter != NULL)))
                     {
                        // If the filter is different, then we need to change our subscribed-set to
                        // report the addition of nodes that match the new filter but not the old, and 
                        // report the removal of the nodes that match the old filter but not the new.  Whew!
                        void * args[] = {(void *)subscriptionFilter, filter()};
                        NodePathMatcher temp;
                        if (temp.PutPathString(fixPath, QueryFilterRef()) == B_NO_ERROR) temp.DoTraversal((PathMatchCallback)ChangeQueryFilterCallbackFunc, this, GetGlobalRoot(), false, args);
                     }

                     // And now, set e's filter to the new filter.
                     e->SetFilter(filter);
                  }
                  else                  
                  {
                     // This marks any currently existing matching nodes so they know to notify us
                     // It must be done once per subscription path, as it uses per-sub ref-counting
                     NodePathMatcher temp;
                     if ((temp.PutPathString(fixPath, QueryFilterRef()) == B_NO_ERROR)&&(_subscriptions.PutPathString(fixPath, filter) == B_NO_ERROR)) temp.DoTraversal((PathMatchCallback)DoSubscribeRefCallbackFunc, this, GetGlobalRoot(), false, (void *)1L);
                  }
                  if ((subscribeQuietly == false)&&(getMsg.AddString(PR_NAME_KEYS, path) == B_NO_ERROR))
                  {
                     // We have to have a filter message to match each string, to prevent "bleed-down" of earlier
                     // filters matching later strings.  So add a dummy filter Message if we don't have an actual one.
                     if (filterMsgRef() == NULL) filterMsgRef.SetRef(const_cast<Message *>(&GetEmptyMessage()), false);
                     (void) getMsg.AddMessage(PR_NAME_FILTERS, filterMsgRef);
                  }
               }
               else if (*fn == PR_NAME_REFLECT_TO_SELF)
               {
                  SetReflectToSelf(true);
               }
               else if (*fn == PR_NAME_DISABLE_SUBSCRIPTIONS)
               {
                  SetSubscriptionsEnabled(false);
               }
               else if ((*fn == PR_NAME_KEYS)||(*fn == PR_NAME_FILTERS))
               {
                  msg.MoveName(*fn, _defaultMessageRouteMessage);
                  updateDefaultMessageRoute = true;
               }
               else if (*fn == PR_NAME_SUBSCRIBE_QUIETLY)
               {
                  // don't add this to the parameter set; it's just an "argument" for the SUBSCRIBE: fields.
                  copyField = false;
               }
               else if (*fn == PR_NAME_MAX_UPDATE_MESSAGE_ITEMS)
               {
                  (void) msg.FindInt32(PR_NAME_MAX_UPDATE_MESSAGE_ITEMS, _maxSubscriptionMessageItems);
               }
               else if (*fn == PR_NAME_PRIVILEGE_BITS)
               {
                  // don't add this to the parameter set; clients aren't allowed to change
                  // their privilege bits (maybe someday, for some clients, but not now)
                  copyField = false;
               }
               else if (*fn == PR_NAME_REPLY_ENCODING)
               {
                  int32 enc;
                  if (msg.FindInt32(PR_NAME_REPLY_ENCODING, enc) != B_NO_ERROR) enc = MUSCLE_MESSAGE_ENCODING_DEFAULT;
                  MessageIOGateway * gw = dynamic_cast<MessageIOGateway *>(GetGateway()());
                  if (gw) gw->SetOutgoingEncoding(enc);
               }

               if (copyField) msg.CopyName(*fn, _parameters);
            }
            if (updateDefaultMessageRoute) UpdateDefaultMessageRoute();
            if (getMsg.HasName(PR_NAME_KEYS)) DoGetData(getMsg);  // return any data that matches the subscription
         }
         break;

         case PR_COMMAND_GETPARAMETERS:
         {
            if (_sessionDir())
            {
               String np;
               MessageRef resultMessage = GetMessageFromPool(_parameters);
               if ((resultMessage())&&(_sessionDir()->GetNodePath(np) == B_NO_ERROR))
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
                  resultMessage()->AddInt64(PR_NAME_SERVER_UPTIME, GetServerUptime());

                  resultMessage()->RemoveName(PR_NAME_SERVER_CURRENTTIMEUTC);
                  resultMessage()->AddInt64(PR_NAME_SERVER_CURRENTTIMEUTC, GetCurrentTime64(MUSCLE_TIMEZONE_UTC));

                  resultMessage()->RemoveName(PR_NAME_SERVER_CURRENTTIMELOCAL);
                  resultMessage()->AddInt64(PR_NAME_SERVER_CURRENTTIMELOCAL, GetCurrentTime64(MUSCLE_TIMEZONE_LOCAL));

                  resultMessage()->RemoveName(PR_NAME_SERVER_RUNTIME);
                  resultMessage()->AddInt64(PR_NAME_SERVER_RUNTIME, GetRunTime64());

                  resultMessage()->RemoveName(PR_NAME_MAX_NODES_PER_SESSION);
                  resultMessage()->AddInt64(PR_NAME_MAX_NODES_PER_SESSION, _maxNodeCount);
        
                  resultMessage()->RemoveName(PR_NAME_SERVER_SESSION_ID);
                  resultMessage()->AddInt64(PR_NAME_SERVER_SESSION_ID, GetServerSessionID());

                  AddApplicationSpecificParametersToParametersResultMessage(*resultMessage());

                  MessageReceivedFromSession(*this, resultMessage, NULL);
               }
               else WARN_OUT_OF_MEMORY;
            }
         }
         break;

         case PR_COMMAND_REMOVEPARAMETERS:
         {
            bool updateDefaultMessageRoute = false;
            const String * nextName;
            for (int i=0; msg.FindString(PR_NAME_KEYS, i, &nextName) == B_NO_ERROR; i++) 
            {
               // Search the parameters message for all parameters that match (nextName)...
               StringMatcher matcher;
               if (matcher.SetPattern(*nextName) == B_NO_ERROR)
               {
                  if (matcher.IsPatternUnique()) (void) RemoveParameter(*nextName, updateDefaultMessageRoute);
                  else
                  {
                     MessageFieldNameIterator it = _parameters.GetFieldNameIterator();
                     const String * fn;
                     while((fn = it.GetNextFieldNameString()) != NULL) if (matcher.Match(fn->Cstr())) (void) RemoveParameter(*fn, updateDefaultMessageRoute);
                  }
               }
            }
            if (updateDefaultMessageRoute) UpdateDefaultMessageRoute();
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
               for (int32 i=0; msg.FindMessage(*nextName, i, dataMsgRef) == B_NO_ERROR; i++) SetDataNode(*nextName, dataMsgRef, true, true, quiet);
            }
         }
         break;

         case PR_COMMAND_INSERTORDEREDDATA:
            (void) InsertOrderedData(msgRef, NULL);
         break;

         case PR_COMMAND_REORDERDATA:
         {
            // Because REORDERDATA operates solely on pre-existing nodes, we can allow wildcards in our node paths.
            if (_sessionDir())
            {
               // Do each field as a separate operation (so they won't mess each other up)
               MessageFieldNameIterator iter = msg.GetFieldNameIterator(B_STRING_TYPE);
               const String * nextFieldName;
               while((nextFieldName = iter.GetNextFieldNameString()) != NULL)
               {
                  const String * value;
                  if (msg.FindString(*nextFieldName, &value) == B_NO_ERROR)
                  {
                     Message temp;
                     temp.AddString(PR_NAME_KEYS, *nextFieldName);

                     NodePathMatcher matcher;
                     matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, temp, NULL);
                     matcher.DoTraversal((PathMatchCallback)ReorderDataCallbackFunc, this, *_sessionDir(), true, (void *)value);
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
            matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, msg, NULL);
            DoRemoveData(matcher, msg.HasName(PR_NAME_REMOVE_QUIETLY));
         }
         break;

         case PR_RESULT_PARAMETERS:
            // fall-thru
         case PR_RESULT_DATAITEMS:
            LogTime(MUSCLE_LOG_WARNING, "Warning, client at [%s] sent me a PR_RESULT_* code.  Bad client!\n", GetHostName()());
         break;

         case PR_COMMAND_JETTISONRESULTS:
         {
            if (msg.HasName(PR_NAME_KEYS, B_STRING_TYPE))
            {
               NodePathMatcher matcher;
               matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, msg, DEFAULT_PATH_PREFIX);
               JettisonOutgoingResults(&matcher);
            }
            else JettisonOutgoingResults(NULL);
         }
         break;

         case PR_COMMAND_PING:
         {
            msg.what = PR_RESULT_PONG;                        // mark it as processed...
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
         matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, msg, DEFAULT_PATH_PREFIX);
         matcher.DoTraversal((PathMatchCallback)PassMessageCallbackFunc, this, GetGlobalRoot(), true, const_cast<MessageRef *>(&msgRef));
      }
      else if (_parameters.HasName(PR_NAME_KEYS, B_STRING_TYPE)) 
      {
         _defaultMessageRoute.DoTraversal((PathMatchCallback)PassMessageCallbackFunc, this, GetGlobalRoot(), true, const_cast<MessageRef *>(&msgRef));
      }
      else DumbReflectSession::MessageReceivedFromGateway(msgRef, userData);
   }

   TCHECKPOINT;
}

void StorageReflectSession :: UpdateDefaultMessageRoute()
{
   _defaultMessageRoute.Clear();
   _defaultMessageRoute.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, _defaultMessageRouteMessage, DEFAULT_PATH_PREFIX);
}

/** A little bitty class just to hold the find-sessions-traversal's results properly */
class FindMatchingSessionsData
{
public:
   FindMatchingSessionsData(Hashtable<const String *, AbstractReflectSessionRef> & results, uint32 maxResults) : _results(results), _ret(B_NO_ERROR), _maxResults(maxResults) 
   {
      // _results doesn't need to have SetKeyCompareFunction() called on it, since we know the pointers are unique anyway
   }

   Hashtable<const String *, AbstractReflectSessionRef> & _results;
   status_t _ret;
   uint32 _maxResults;
};

AbstractReflectSessionRef StorageReflectSession :: FindMatchingSession(const String & nodePath, const QueryFilterRef & filter, bool matchSelf) const
{
   AbstractReflectSessionRef ret;
   Hashtable<const String *, AbstractReflectSessionRef> results;
   if (FindMatchingSessions(nodePath, filter, results, matchSelf, 1) == B_NO_ERROR) (void) results.GetIterator().GetNextValue(ret);
   return ret;
}

status_t StorageReflectSession :: FindMatchingSessions(const String & nodePath, const QueryFilterRef & filter, Hashtable<const String *, AbstractReflectSessionRef> & retSessions, bool includeSelf, uint32 maxResults) const
{
   TCHECKPOINT;

   status_t ret = B_NO_ERROR;

   if (nodePath.HasChars())
   {
      const char * s;
      String temp;
      if (nodePath.StartsWith('/')) s = nodePath()+1;
      else
      {
         temp = nodePath.Prepend(DEFAULT_PATH_PREFIX "/");
         s = temp();
      }

      NodePathMatcher matcher;
      if (matcher.PutPathString(s, filter) == B_NO_ERROR)
      {
         FindMatchingSessionsData data(retSessions, maxResults);
         matcher.DoTraversal((PathMatchCallback)FindSessionsCallbackFunc, const_cast<StorageReflectSession*>(this), GetGlobalRoot(), true, &data);
         ret = data._ret;
      }
      else ret = B_ERROR;
   }
   else
   {
      HashtableIterator<const String *, AbstractReflectSessionRef> iter(GetSessions());
      const String * nextKey;
      AbstractReflectSessionRef nextValue;
      while(iter.GetNextKeyAndValue(nextKey, nextValue) == B_NO_ERROR) if (retSessions.Put(nextKey, nextValue) != B_NO_ERROR) ret = B_ERROR;
   }

   if (includeSelf == false) (void) retSessions.Remove(&GetSessionIDString());
   return ret;
}

status_t StorageReflectSession :: SendMessageToMatchingSessions(const MessageRef & msgRef, const String & nodePath, const QueryFilterRef & filter, bool includeSelf)
{
   TCHECKPOINT;

   if (nodePath.HasChars())
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
      if (matcher.PutPathString(s, filter) == B_NO_ERROR)
      {
         void * sendMessageData[] = {const_cast<MessageRef *>(&msgRef), &includeSelf}; // gotta include the includeSelf param too, alas
         matcher.DoTraversal((PathMatchCallback)SendMessageCallbackFunc, this, GetGlobalRoot(), true, sendMessageData);
         return B_NO_ERROR;
      }
      return B_ERROR;
   }
   else
   {
      BroadcastToAllSessions(msgRef, NULL, includeSelf);
      return B_NO_ERROR;
   }
}

int
StorageReflectSession ::
SendMessageCallback(DataNode & node, void * userData)
{
   void ** a = (void **) userData;
   return PassMessageCallbackAux(node, *((MessageRef *)a[0]), *((bool *)a[1]));
}

status_t StorageReflectSession :: InsertOrderedData(const MessageRef & msgRef, Hashtable<String, DataNodeRef> * optNewNodes)
{
   TCHECKPOINT;

   // Because INSERTORDEREDDATA operates solely on pre-existing nodes, we can allow wildcards in our node paths.
   if ((_sessionDir())&&(msgRef()))
   {
      void * args[2] = {msgRef(), optNewNodes};
      NodePathMatcher matcher;
      matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, *msgRef(), NULL);
      matcher.DoTraversal((PathMatchCallback)InsertOrderedDataCallbackFunc, this, *_sessionDir(), true, args);
      return B_NO_ERROR;
   }
   return B_ERROR;
}

void
StorageReflectSession :: 
BounceMessage(uint32 errorCode, const MessageRef & msgRef)
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
   TCHECKPOINT;

   NodePathMatcher matcher;
   matcher.PutPathsFromMessage(PR_NAME_KEYS, PR_NAME_FILTERS, msg, DEFAULT_PATH_PREFIX);

   MessageRef messageArray[2];  // first is the DATAITEMS message, second is the INDEXUPDATED message (both demand-allocated)
   matcher.DoTraversal((PathMatchCallback)GetDataCallbackFunc, this, GetGlobalRoot(), true, messageArray);

   // Send any still-pending "get" results...
   SendGetDataResults(messageArray[0]);
   SendGetDataResults(messageArray[1]);
}

void
StorageReflectSession ::
SendGetDataResults(MessageRef & replyMessage)
{
   TCHECKPOINT;

   if (replyMessage())
   {
      MessageReceivedFromSession(*this, replyMessage, NULL);
      replyMessage.Reset();  // force re-alloc later if need be
   }
}

void StorageReflectSession :: DoRemoveData(NodePathMatcher & matcher, bool quiet)
{
   TCHECKPOINT;

   if (_sessionDir())
   {
      Queue<DataNodeRef> removeSet;
      matcher.DoTraversal((PathMatchCallback)RemoveDataCallbackFunc, this, *_sessionDir(), true, &removeSet);
      for (int i=removeSet.GetNumItems()-1; i>=0; i--)
      {
         DataNode * next = removeSet[i]();
         DataNode * parent = next->GetParent();
         if ((next)&&(parent)) parent->RemoveChild(next->GetNodeName(), quiet ? NULL : this, true, &_currentNodeCount);
      }
   }
}

status_t StorageReflectSession :: RemoveDataNodes(const String & nodePath, const QueryFilterRef & filterRef, bool quiet)
{
   TCHECKPOINT;

   NodePathMatcher matcher;
   if (matcher.PutPathString(nodePath, filterRef) != B_NO_ERROR) return B_ERROR;
   DoRemoveData(matcher, quiet);
   return B_NO_ERROR;
}

void
StorageReflectSession ::
PushSubscriptionMessages()
{
   TCHECKPOINT;

   if (_sharedData->_subsDirty)
   {
      _sharedData->_subsDirty = false;

      // Send out any subscription results that were generated...
      HashtableIterator<const String *, AbstractReflectSessionRef> sessions(GetSessions());
      AbstractReflectSessionRef * lRef;
      while((lRef = sessions.GetNextValue()) != NULL)
      {
         StorageReflectSession * nextSession = dynamic_cast<StorageReflectSession *>(lRef->GetItemPointer());
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
   TCHECKPOINT;

   MessageRef oldRef = ref;  // (ref) is one of our _next*SubscriptionMessage members
   while(oldRef()) 
   {
      ref.Reset();  /* In case FromSession() wants to add more suscriptions... */
      MessageReceivedFromSession(*this, oldRef, NULL);
      oldRef = ref;
   }
}

int
StorageReflectSession ::
PassMessageCallback(DataNode & node, void * userData)
{
   return PassMessageCallbackAux(node, *((MessageRef *)userData), GetReflectToSelf());
}

int
StorageReflectSession ::
PassMessageCallbackAux(DataNode & node, const MessageRef & msgRef, bool includeSelfOkay)
{
   TCHECKPOINT;

   DataNode * n = &node;
   while(n->GetDepth() > 2) n = n->GetParent();  // go up to session level...
   StorageReflectSession * next = dynamic_cast<StorageReflectSession *>(GetSession(n->GetNodeName())());
   if ((next)&&((next != this)||(includeSelfOkay))) next->MessageReceivedFromSession(*this, msgRef, &node);
   return 2; // This causes the traversal to immediately skip to the next session
}

int
StorageReflectSession ::
FindSessionsCallback(DataNode & node, void * userData)
{
   TCHECKPOINT;

   DataNode * n = &node;
   while(n->GetDepth() > 2) n = n->GetParent();  // go up to session level...
   AbstractReflectSessionRef sref = GetSession(n->GetNodeName());

   StorageReflectSession * next = dynamic_cast<StorageReflectSession *>(sref());
   FindMatchingSessionsData * data = (FindMatchingSessionsData *) userData;
   if ((next)&&(data->_results.Put(&next->GetSessionIDString(), sref) != B_NO_ERROR))
   {
      data->_ret = B_ERROR;  // Oops, out of memory!
      return -1;  // abort now
   }
   else return (data->_results.GetNumItems() == data->_maxResults) ? -1 : 2; // This causes the traversal to immediately skip to the next session
}

int
StorageReflectSession ::
KickClientCallback(DataNode & node, void * /*userData*/)
{
   TCHECKPOINT;

   DataNode * n = &node;
   while(n->GetDepth() > 2) n = n->GetParent();  // go up to session level...
   AbstractReflectSessionRef sref = GetSession(n->GetNodeName());
   StorageReflectSession * next = dynamic_cast<StorageReflectSession *>(sref());
   if ((next)&&(next != this)) 
   {
      LogTime(MUSCLE_LOG_DEBUG, "Session [%s/%s] is kicking session [%s/%s] off the server.\n", GetHostName()(), GetSessionIDString()(), next->GetHostName()(), next->GetSessionIDString()());
      next->EndSession();  // die!!
   }
   return 2; // This causes the traversal to immediately skip to the next session
}

int
StorageReflectSession ::
GetSubtreesCallback(DataNode & node, void * ud)
{
   TCHECKPOINT;

   void ** args    = (void **)ud;
   Message * reply = (Message *) args[0];
   int32 maxDepth  = (int32) ((long)args[1]);

   bool inMyOwnSubtree = false;  // default:  actual value will only be calculated if it makes a difference
   bool reflectToSelf = GetReflectToSelf();
   if (reflectToSelf == false)
   {
      // Make sure (node) isn't part of our own tree!  If it is, move immediately to the next session
      const DataNode * n = &node;
      while(n->GetDepth() > 2) n = n->GetParent();
      if ((_indexingPresent == false)&&(GetSession(n->GetNodeName())() == this)) return 2;  // skip to next session node
   }
   // Don't send our own data to our own client; he already knows what we have, because he uploaded it!
   if ((inMyOwnSubtree == false)||(reflectToSelf))
   {
      MessageRef subMsg = GetMessageFromPool();
      String nodePath;
      if ((subMsg() == NULL)||(node.GetNodePath(nodePath) != B_NO_ERROR)||(reply->AddMessage(nodePath, subMsg) != B_NO_ERROR)||(SaveNodeTreeToMessage(*subMsg(), &node, "", true, (maxDepth>=0)?(uint32)maxDepth:MUSCLE_NO_LIMIT, NULL) != B_NO_ERROR)) return 0;
   }
   return node.GetDepth();  // continue traversal as usual
}

int
StorageReflectSession ::
ChangeQueryFilterCallback(DataNode & node, void * ud)
{
   void ** args = (void **) ud;
   const QueryFilter * oldFilter = (const QueryFilter *) args[0];
   const QueryFilter * newFilter = (const QueryFilter *) args[1];
   const Message * msg = node.GetData()();
   bool oldMatches = ((msg == NULL)||(oldFilter == NULL)||(oldFilter->Matches(*msg, &node)));
   bool newMatches = ((msg == NULL)||(newFilter == NULL)||(newFilter->Matches(*msg, &node)));
   if (oldMatches != newMatches) NodeChangedAux(node, oldMatches);
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
   TCHECKPOINT;

   MessageRef * messageArray = (MessageRef *) userData;

   bool inMyOwnSubtree = false;  // default:  actual value will only be calculated if it makes a difference
   bool reflectToSelf = GetReflectToSelf();
   if (reflectToSelf == false)
   {
      // Make sure (node) isn't part of our own tree!  If it is, move immediately to the next session
      const DataNode * n = &node;
      while(n->GetDepth() > 2) n = n->GetParent();
      if ((_indexingPresent == false)&&(GetSession(n->GetNodeName())() == this)) return 2;  // skip to next session node
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
         if (resultMsg()->GetNumNames() >= _maxSubscriptionMessageItems) SendGetDataResults(resultMsg);
      }
      else 
      {      
         WARN_OUT_OF_MEMORY;
         return 0;  // abort!
      }
   }

   // But indices we need to send to ourself no matter what, as they are generated on the server side.
   const Queue<const String *> * index = node.GetIndex();
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
               sprintf(temp, "%c"UINT32_FORMAT_SPEC":%s", INDEX_OP_ENTRYINSERTED, i, (*index)[i]->Cstr());
               (void) indexUpdateMsg()->AddString(np, temp);
            }
            if (indexUpdateMsg()->GetNumNames() >= _maxSubscriptionMessageItems) SendGetDataResults(messageArray[1]);
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
   TCHECKPOINT;

   if (node.GetDepth() > 2)  // ensure that we never remove host nodes or session nodes this way
   {
      DataNodeRef nodeRef;
      if (node.GetParent()->GetChild(node.GetNodeName(), nodeRef) == B_NO_ERROR) 
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
   TCHECKPOINT;

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
         if ((_currentNodeCount < _maxNodeCount)&&(node.InsertOrderedChild(nextRef, next, NULL, this, this, optRetResults) == B_NO_ERROR))
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
   if (indexNode) indexNode->ReorderChild(node, (const String *) userData, this);
   return node.GetDepth();
}

bool
StorageReflectSession :: NodePathMatcher ::
MatchesNode(DataNode & node, const Message * optData, int rootDepth) const
{
   HashtableIterator<String, PathMatcherEntry> iter(GetEntries());
   const PathMatcherEntry * next;
   while((next = iter.GetNextValue()) != NULL) if (PathMatches(node, optData, *next, rootDepth)) return true;
   return false;
}

uint32
StorageReflectSession :: NodePathMatcher ::
GetMatchCount(DataNode & node, const Message * optData, int rootDepth) const
{
   TCHECKPOINT;

   int matchCount = 0;
   HashtableIterator<String, PathMatcherEntry> iter(GetEntries());
   const PathMatcherEntry * next;
   while((next = iter.GetNextValue()) != NULL) if (PathMatches(node, optData, *next, rootDepth)) matchCount++;
   return matchCount;
}

bool
StorageReflectSession :: NodePathMatcher ::
PathMatches(DataNode & node, const Message * optData, const PathMatcherEntry & entry, int rootDepth) const
{
   TCHECKPOINT;

   const StringMatcherQueue * nextSubscription = entry.GetParser()();
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
      if (match) return entry.FilterMatches(optData, &node);
   }
   return false;
}

void
StorageReflectSession :: NodePathMatcher ::
DoTraversal(PathMatchCallback cb, StorageReflectSession * This, DataNode & node, bool useFilters, void * userData)
{
   (void) DoTraversalAux(TraversalContext(cb, This, useFilters, userData, node.GetDepth()), node);
}

int 
StorageReflectSession :: NodePathMatcher ::
DoTraversalAux(const TraversalContext & data, DataNode & node)
{
   TCHECKPOINT;

   int depth = node.GetDepth();
   bool parsersHaveWildcards = false;  // optimistic default
   {
      // If none of our parsers are using wildcarding at our current level, we can use direct hash lookups (faster)
      HashtableIterator<String, PathMatcherEntry> iter(GetEntries());
      const PathMatcherEntry * nextValue;
      while((nextValue = iter.GetNextValue()) != NULL)
      {
         const StringMatcherQueue * nextQueue = nextValue->GetParser()();
         if ((nextQueue)&&((int)nextQueue->GetNumItems() > depth-data._rootDepth))
         {
            StringMatcher * nextMatcher = nextQueue->GetItemAt(depth-data._rootDepth)->GetItemPointer();
            if ((nextMatcher == NULL)||(nextMatcher->IsPatternUnique() == false))
            {
               parsersHaveWildcards = true;  // Oops, there will be some pattern matching involved, gotta iterate
               break;
            }
         }
      }
   }

   if (parsersHaveWildcards)
   {
      // general case -- iterate over all children of our node and see if any match
      DataNodeRef nextChildRef;
      DataNodeRefIterator it = node.GetChildIterator();
      while(it.GetNextValue(nextChildRef) == B_NO_ERROR) if (CheckChildForTraversal(data, nextChildRef(), depth)) return depth;
   }
   else
   {
      // optimized case -- since our parsers are all node-specific, we can do a single lookup for each,
      // and avoid having to iterate over all the children of this node.
      Queue<DataNode *> alreadyDid;  // To make sure we don't do the same child twice (could happen if two matchers are the same)
      HashtableIterator<String, PathMatcherEntry> iter(GetEntries());
      const PathMatcherEntry * nextValue;
      while((nextValue = iter.GetNextValue()) != NULL)
      {
         const StringMatcherQueue * nextQueue = nextValue->GetParser()();
         if ((nextQueue)&&((int)nextQueue->GetNumItems() > depth-data._rootDepth))
         {
            const String & key = nextQueue->GetItemAt(depth-data._rootDepth)->GetItemPointer()->GetPattern();
            DataNodeRef nextChildRef;
            if ((node.GetChild(key, nextChildRef) == B_NO_ERROR)&&(alreadyDid.IndexOf(nextChildRef()) == -1))
            {
               if (CheckChildForTraversal(data, nextChildRef(), depth)) return depth;
               (void) alreadyDid.AddTail(nextChildRef());
            }
         }
      }
   }

   return node.GetDepth();
}

bool 
StorageReflectSession :: NodePathMatcher ::
CheckChildForTraversal(const TraversalContext & data, DataNode * nextChild, int & depth)
{
   TCHECKPOINT;

   if (nextChild)
   {
      const String & nextChildName = nextChild->GetNodeName();
      bool matched  = false;  // set if we have called the callback on this child already
      bool recursed = false;  // set if we have recursed to this child already

      // Try all parsers and see if any of them match at this level
      HashtableIterator<String, PathMatcherEntry> iter(GetEntries());
      const PathMatcherEntry * nextValue;
      while((nextValue = iter.GetNextValue()) != NULL)
      {
         const StringMatcherQueue * nextQueue = nextValue->GetParser()();
         if (nextQueue)
         {
            int numClausesInParser = nextQueue->GetNumItems();
            if (numClausesInParser > depth-data._rootDepth)
            {
               const StringMatcher * nextMatcher = nextQueue->GetItemAt(depth-data._rootDepth)->GetItemPointer();
               if ((nextMatcher == NULL)||(nextMatcher->Match(nextChildName())))
               {
                  // A match!  Now, depending on whether this match is the
                  // last clause in the path or not, we either do the callback or descend.
                  // But we make sure not to do either of these things more than once per node.
                  if (depth == data._rootDepth+numClausesInParser-1)
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
                        if (((GetEntries().GetNumItems() == 1)&&((data._useFilters == false)||(nextValue->GetFilter()() == NULL)))||
                             (MatchesNode(*nextChild, data._useFilters ? nextChild->GetData()() : NULL, data._rootDepth)))
                        {
                           int nextDepth = data._cb(data._This, *nextChild, data._userData);
                           if (nextDepth < ((int)nextChild->GetDepth())-1) 
                           {
                              depth = nextDepth;
                              return true;
                           }
                           matched = true;
                           if (recursed) break;  // done both possible actions, so be lazy
                        }
                     }
                  }
                  else
                  {
                     if (recursed == false)
                     {
                        // If we match a non-terminal clause in the path, recurse to the child.
                        int nextDepth = DoTraversalAux(data, *nextChild);
                        if (nextDepth < ((int)nextChild->GetDepth())-1) 
                        {
                           depth = nextDepth;
                           return true;
                        }
                        recursed = true;
                        if (matched) break;  // done both possible actions, so be lazy
                     }
                  }
               } 
            }
         }
      }
   }
   return false;
}

DataNodeRef
StorageReflectSession ::
GetNewDataNode(const String & name, const MessageRef & initialValue)
{
   DataNodeRef ret = _nodePool.ObtainObject();
   if (ret()) ret()->Init(name, initialValue);
   return ret;
}

void
StorageReflectSession ::
JettisonOutgoingSubtrees(const String * optMatchString)
{
   TCHECKPOINT;

   AbstractMessageIOGateway * gw = GetGateway()();
   if (gw)
   {
      StringMatcher sm;
      if ((optMatchString == NULL)||(sm.SetPattern(*optMatchString) == B_NO_ERROR))
      {
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
}

void
StorageReflectSession ::
JettisonOutgoingResults(const NodePathMatcher * matcher)
{
   TCHECKPOINT;

   AbstractMessageIOGateway * gw = GetGateway()();
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
               const String * rname;
               while(msg->FindString(PR_NAME_REMOVED_DATAITEMS, nextr, &rname) == B_NO_ERROR)
               {
                  if (matcher->MatchesPath(rname->Cstr(), NULL, NULL)) msg->RemoveData(PR_NAME_REMOVED_DATAITEMS, nextr);
                                                                  else nextr++;
               }
   
               // Remove all matching items from the Message.  (Yes, the iterator can handle this!  :^))
               const String * nextFieldName;
               MessageFieldNameIterator iter = msg->GetFieldNameIterator(B_MESSAGE_TYPE);
               while((nextFieldName = iter.GetNextFieldNameString()) != NULL)
               {
                  if (matcher->GetNumFilters() > 0)
                  {
                     MessageRef nextSubMsgRef;
                     for (uint32 j=0; msg->FindMessage(*nextFieldName, j, nextSubMsgRef) == B_NO_ERROR; /* empty */)
                     {
                        if (matcher->MatchesPath(nextFieldName->Cstr(), nextSubMsgRef(), NULL)) msg->RemoveData(*nextFieldName, 0);
                                                                                           else j++;
                     }
                  }
                  else if (matcher->MatchesPath(nextFieldName->Cstr(), NULL, NULL)) msg->RemoveName(*nextFieldName);
               }
            }
            else msg->Clear();

            if (msg->HasNames() == false) (void) oq.RemoveItemAt(i);
         }
      }
   }
}

status_t
StorageReflectSession :: CloneDataNodeSubtree(const DataNode & node, const String & destPath, bool allowOverwriteData, bool allowCreateNode, bool quiet, bool addToTargetIndex, const String * optInsertBefore, const ITraversalPruner * optPruner)
{
   TCHECKPOINT;

   {
      MessageRef payload = node.GetData();
      if ((optPruner)&&(optPruner->MatchPath(destPath, payload) == false)) return B_NO_ERROR;
      if ((payload() == NULL)||(SetDataNode(destPath, payload, allowOverwriteData, allowCreateNode, quiet, addToTargetIndex, optInsertBefore) != B_NO_ERROR)) return B_ERROR;
   }

   // Then clone all of his children
   DataNodeRefIterator iter = node.GetChildIterator();
   const String * nextChildName = NULL;  // set NULL just to shut the compiler up
   DataNodeRef nextChild; 
   while(iter.GetNextKeyAndValue(nextChildName, nextChild) == B_NO_ERROR)
   {
      // Note that we don't deal with the index-cloning here; we do it separately (below) instead, for efficiency
      const DataNode * child = nextChild();
      if ((child)&&(CloneDataNodeSubtree(*child, destPath+'/'+(*nextChildName), false, true, quiet, false, NULL, optPruner) != B_NO_ERROR)) return B_ERROR;
   }

   // Lastly, if he has an index, make sure the clone ends up with an equivalent index
   const Queue<const String *> * index = node.GetIndex();
   if (index)
   {
      DataNode * clone = GetDataNode(destPath);
      if (clone)
      {
         uint32 idxLen = index->GetNumItems();
         for (uint32 i=0; i<idxLen; i++) if (clone->InsertIndexEntryAt(i, this, *((*index)[i])) != B_NO_ERROR) return B_ERROR;
      }
      else return B_ERROR;
   }

   return B_NO_ERROR;
}

// Recursive helper function
status_t
StorageReflectSession :: SaveNodeTreeToMessage(Message & msg, const DataNode * node, const String & path, bool saveData, uint32 maxDepth, const ITraversalPruner * optPruner) const
{
   TCHECKPOINT;

   {
      MessageRef payload = node->GetData();
      if ((optPruner)&&(optPruner->MatchPath(path, payload) == false)) return B_NO_ERROR;
      if ((saveData)&&(msg.AddMessage(PR_NAME_NODEDATA, payload) != B_NO_ERROR)) return B_ERROR;
   }
   
   if ((node->HasChildren())&&(maxDepth > 0))
   {
      // Save the node-index, if there is one
      const Queue<const String *> * index = node->GetIndex();
      if (index)
      {
         uint32 indexSize = index->GetNumItems();
         if (indexSize > 0)
         {
            MessageRef indexMsgRef(GetMessageFromPool());
            if ((indexMsgRef() == NULL)||(msg.AddMessage(PR_NAME_NODEINDEX, indexMsgRef) != B_NO_ERROR)) return B_ERROR;
            Message * indexMsg = indexMsgRef();
            for (uint32 i=0; i<indexSize; i++) if (indexMsg->AddString(PR_NAME_KEYS, *((*index)[i])) != B_NO_ERROR) return B_ERROR;
         }
      }

      // Then save the children, recursing to each one as necessary
      {
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
               if (childPath.HasChars()) childPath += '/';
               childPath += child->GetNodeName();

               MessageRef childMsgRef(GetMessageFromPool());
               if ((childMsgRef() == NULL)||(childrenMsgRef()->AddMessage(child->GetNodeName(), childMsgRef) != B_NO_ERROR)||(SaveNodeTreeToMessage(*childMsgRef(), child, childPath, true, maxDepth-1, optPruner) != B_NO_ERROR)) return B_ERROR;
            }
         }
      }
   }
   return B_NO_ERROR;
}

status_t
StorageReflectSession :: RestoreNodeTreeFromMessage(const Message & msg, const String & path, bool loadData, bool appendToIndex, uint32 maxDepth, const ITraversalPruner * optPruner)
{
   TCHECKPOINT;

   if (loadData)
   {
      MessageRef payload;
      if (msg.FindMessage(PR_NAME_NODEDATA, payload) != B_NO_ERROR) return B_ERROR;
      if ((optPruner)&&(optPruner->MatchPath(path, payload) == false)) return B_NO_ERROR;
      if (SetDataNode(path, payload, true, true, false, appendToIndex) != B_NO_ERROR) return B_ERROR;
   }
   else 
   {
      MessageRef junk = GetEmptyMessageRef();
      if ((optPruner)&&(optPruner->MatchPath(path, junk) == false)) return B_NO_ERROR;
   }

   MessageRef childrenRef;
   if ((maxDepth > 0)&&(msg.FindMessage(PR_NAME_NODECHILDREN, childrenRef) == B_NO_ERROR)&&(childrenRef()))
   {
      // First recurse to the indexed nodes, adding them as indexed children
      Hashtable<const String *, uint32> indexLookup;
      indexLookup.SetKeyCompareFunction(StringCompareFunc);
      {
         MessageRef indexRef;
         if (msg.FindMessage(PR_NAME_NODEINDEX, indexRef) == B_NO_ERROR)
         {
            const String * nextFieldName;
            for (int i=0; indexRef()->FindString(PR_NAME_KEYS, i, &nextFieldName) == B_NO_ERROR; i++)
            {
               MessageRef nextChildRef;
               if (childrenRef()->FindMessage(*nextFieldName, nextChildRef) == B_NO_ERROR) 
               {
                  String childPath(path);
                  if (childPath.HasChars()) childPath += '/';
                  childPath += *nextFieldName;
                  if (RestoreNodeTreeFromMessage(*nextChildRef(), childPath, true, true, maxDepth-1, optPruner) != B_NO_ERROR) return B_ERROR;
                  if (indexLookup.Put(nextFieldName, i) != B_NO_ERROR) return B_ERROR;
               }
            }
         }
      }

      // Then recurse to the non-indexed child nodes 
      {
         MessageFieldNameIterator iter = childrenRef()->GetFieldNameIterator(B_MESSAGE_TYPE);
         const String * nextFieldName;
         while((nextFieldName = iter.GetNextFieldNameString()) != NULL)
         {
            if (indexLookup.ContainsKey(nextFieldName) == false)
            {
               MessageRef nextChildRef;
               if ((childrenRef()->FindMessage(*nextFieldName, nextChildRef) == B_NO_ERROR)&&(nextChildRef()))
               {
                  String childPath(path);
                  if (childPath.HasChars()) childPath += '/';
                  childPath += *nextFieldName;
                  if (RestoreNodeTreeFromMessage(*nextChildRef(), childPath, true, false, maxDepth-1, optPruner) != B_NO_ERROR) return B_ERROR;
               }
            }
         }
      }
   }
   return B_NO_ERROR;   
}

status_t StorageReflectSession :: RemoveParameter(const String & paramName, bool & retUpdateDefaultMessageRoute)
{
   if (_parameters.RemoveName(paramName) != B_NO_ERROR) return B_ERROR;

   if (paramName.StartsWith("SUBSCRIBE:"))
   {
      String str = paramName.Substring(10);
      _subscriptions.AdjustStringPrefix(str, DEFAULT_PATH_PREFIX);
      if (_subscriptions.RemovePathString(str) == B_NO_ERROR)
      {
         // Remove the references from this subscription from all nodes
         NodePathMatcher temp;
         temp.PutPathString(str, QueryFilterRef());
         temp.DoTraversal((PathMatchCallback)DoSubscribeRefCallbackFunc, this, GetGlobalRoot(), false, (void *)-1L);
      }
   }
   else if (paramName == PR_NAME_REFLECT_TO_SELF)          SetReflectToSelf(false);
   else if (paramName == PR_NAME_DISABLE_SUBSCRIPTIONS)    SetSubscriptionsEnabled(true);
   else if (paramName == PR_NAME_MAX_UPDATE_MESSAGE_ITEMS) _maxSubscriptionMessageItems = DEFAULT_MAX_SUBSCRIPTION_MESSAGE_SIZE;  // back to the default
   else if (paramName == PR_NAME_REPLY_ENCODING)
   {
      MessageIOGateway * gw = dynamic_cast<MessageIOGateway *>(GetGateway()());
      if (gw) gw->SetOutgoingEncoding(MUSCLE_MESSAGE_ENCODING_DEFAULT);
   }
   else if ((paramName == PR_NAME_KEYS)||(paramName == PR_NAME_FILTERS))
   {
      _defaultMessageRouteMessage.RemoveName(paramName);
      retUpdateDefaultMessageRoute = true;
   }
   return B_NO_ERROR;
}

void StorageReflectSession :: AddApplicationSpecificParametersToParametersResultMessage(Message & msg) const
{
   (void) msg;
}

}; // end namespace muscle

