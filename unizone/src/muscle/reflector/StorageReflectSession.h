/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleStorageReflectSession_h
#define MuscleStorageReflectSession_h

#include "reflector/DumbReflectSession.h"
#include "reflector/StorageReflectConstants.h"
#include "regex/PathMatcher.h"

namespace muscle {

/**
 *  This is a factory class that returns new StorageReflectSession objects.
 */
class StorageReflectSessionFactory : public ReflectSessionFactory
{
public:
   /** Default constructor.  The maximum incoming message size is set to "unlimited" by default.  */
   StorageReflectSessionFactory();

   /** Returns a new StorageReflectSession */
   virtual AbstractReflectSession * CreateSession(const String &);

   /** Sets the maximum-bytes-per-incoming-message limit that we will set on the StorageReflectSession
     * objects that we create.
     * @param maxIncomingMessageSize The maximum byte size of incoming flattened messages to allow.
     */
   void SetMaxIncomingMessageSize(uint32 maxBytes) {_maxIncomingMessageSize = maxBytes;}

   /** Returns our current setting for the maximum incoming message size for sessions we produce. */
   uint32 GetMaxIncomingMessageSize() const {return _maxIncomingMessageSize;}

protected:
   /** If we have a limited maximum size for incoming messages, then this method 
     * demand-allocate the session's gateway, and set its max incoming message size if possible.
     * @return B_NO_ERROR on success, or B_ERROR on failure (out of memory or the created gateway
     *         wasn't a MessageIOGateway)
     */
   status_t SetMaxIncomingMessageSizeFor(AbstractReflectSession * session) const;

private:
   uint32 _maxIncomingMessageSize;
};

/** An intelligent AbstractReflectSession that knows how to
 *  store data on the server, and filter using wildcards.  This class
 *  is used by the muscled server program to handle incoming connections.
 *  See StorageReflectConstants.h and/or "The Beginner's Guide.html"
 *  for details.
 */
class StorageReflectSession : public DumbReflectSession
{
public:
   /** Default constructor. */
   StorageReflectSession();

   /** Virtual Destructor. */
   virtual ~StorageReflectSession();

   /** Called after the constructor, when the session is ready to interact with the server.
    *  @return B_NO_ERROR if everything is okay to go, B_ERROR if there was a problem
    *          setting up, or if the IP address of our client has been banned. 
    */
   virtual status_t AttachedToServer();

   /** Implemented to remove our nodes from the server-side database and do misc cleanup. */
   virtual void AboutToDetachFromServer();

   /** Called when a new message is received from our IO gateway. */
   virtual void MessageReceivedFromGateway(MessageRef msg);

   /** Overridden to call PushSubscriptionMessages() */
   virtual void AfterMessageReceivedFromGateway(MessageRef msg);

   /** Drains the storage pools used by StorageReflectSessions, to recover memory */
   static void DrainPools();

protected:
   /** Returns a human-readable label for this session type:  "Session" */
   virtual const char * GetTypeName() const {return "Session";}

   class DataNode;

   /** Type for a Reference to a DataNode object */
   typedef Ref<DataNode> DataNodeRef;

   /** Iterator type for our child objects */
   typedef HashtableIterator<const char *, DataNodeRef> DataNodeRefIterator;

   /** Each object of this class represents one node in the server-side data-storage tree.  */
   class DataNode : public RefCountable
   {
   public:
      /**
       * Put a child without changing the ordering index
       * @param child Reference to the child to accept into our list of children
       * @param optNotifyWithOnSetParent If non-NULL, a StorageReflectSession to use to notify subscribers that the new node has been added
       * @param optNotifyWithOnChangedData If non-NULL, a StorageReflectSession to use to notify subscribers when the node's data has been alterered
       * @return B_NO_ERROR on success, B_ERROR if out of memory
       */
      status_t PutChild(DataNodeRef & child, StorageReflectSession * optNotifyWithOnSetParent, StorageReflectSession * optNotifyWithOnChangedData);
 
      /**
       * Create and add a new child node for (data), and put it into the ordering index
       * @param data Reference to a message to create a new child node for.
       * @param optInsertBefore if non-NULL, the name of the child to put the new child before in our index.  If NULL, (or the specified child cannot be found) the new child will be appended to the end of the index.
       * @param optNotifyWithOnSetParent If non-NULL, a StorageReflectSession to use to notify subscribers that the new node has been added
       * @param optNotifyWithOnChangedData If non-NULL, a StorageReflectSession to use to notify subscribers when the node's data has been alterered
       * @param optAddNewChildren If non-NULL, any newly formed nodes will be added to this hashtable, keyed on their absolute node path.
       * @return B_NO_ERROR on success, B_ERROR if out of memory
       */
      status_t InsertOrderedChild(MessageRef data, const char * optInsertBefore, StorageReflectSession * optNotifyWithOnSetParent, StorageReflectSession * optNotifyWithOnChangedData, Hashtable<String, DataNodeRef> * optAddNewChildren);
 
      /** 
       * Moves the given node (which must be a child of ours) to be just before the node named
       * (moveToBeforeThis) in our index.  If (moveToBeforeThis) is not a node in our index,
       * then (child) will be moved back to the end of the index. 
       * @param child A child node of ours, to be moved in the node ordering index.
       * @param moveToBeforeThis name of another child node of ours.  If this name is NULL or
       *                         not found in our index, we'll move (child) to the end of the index.
       * @param optNotifyWith If non-NULL, this will be used to sent INDEXUPDATE message to the
       *                      interested clients, notifying them of the change.
       * @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
       */
      status_t ReorderChild(const DataNode & child, const char * moveToBeforeThis, StorageReflectSession * optNotifyWith);

      /** Returns true iff we have a child with the given name */
      bool HasChild(const char * key) const {return ((_children)&&(_children->ContainsKey(key)));}

      /** Retrieves the child with the given name.
       *  @param key The name of the child we wish to retrieve
       *  @param returnChild On success, a reference to the retrieved child is written into this object.
       *  @return B_NO_ERROR if a child node was successfully retrieved, or B_ERROR if it was not found.
       */
      status_t GetChild(const char * key, DataNodeRef & returnChild) const {return ((_children)&&(_children->Get(key, returnChild) == B_NO_ERROR)) ? B_NO_ERROR : B_ERROR;}

      /** Removes the child with the given name.
       *  @param key The name of the child we wish to remove.
       *  @param optNotifyWith If non-NULL, the StorageReflectSession that should be used to notify subscribers that the given node has been removed
       *  @param recurse if true, the removed child's children will be removed from it, and so on, and so on...
       *  @param optCounter if non-NULL, this value will be incremented whenever a child is removed (recursively)
       *  @return B_NO_ERROR if the given child is found and removed, or B_ERROR if it could not be found.
       */
      status_t RemoveChild(const char * key, StorageReflectSession * optNotifyWith, bool recurse, uint32 * optCounter);

      /** Returns an iterator that can be used for iterating over our set of children. */
      DataNodeRefIterator GetChildIterator() const {return _children ? _children->GetIterator() : DataNodeRefIterator();}

      /** Returns the number of child nodes this node contains. */
      uint32 CountChildren() const {return _children ? _children->GetNumItems() : 0;}

      /** Returns the ASCII name of this node (e.g. "joe") */
      const String & GetNodeName() const {return _nodeName;}

      /** Generates and returns the full node path of this node (e.g. "/12.18.240.15/1234/beshare/files/joe").
        * @param retPath On success, this String will contain this node's absolute path.
        * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
        */
      status_t GetNodePath(String & retPath) const;

      /** Returns the name of the node in our path at the (depth) level.
        * @param depth The node name we are interested in.  For example, 0 will return the name of the
        *              root node, 1 would return the name of the IP address node, etc.  If this number
        *              is greater than (depth), this method will return NULL.
        */
      const char * GetPathClause(uint32 depth) const;

      /** Replaces this node's payload message with that of (data).
       *  @data the new Message to associate with this node.
       *  @param optNotifyWith if non-NULL, this StorageReflectSession will be used to notify subscribers that this node's data has changed.
       */
      void SetData(MessageRef data, StorageReflectSession * optNotifyWith);

      /** Returns a reference to this node's Message payload. */
      MessageRef GetData() const {return _data;}

      /** Returns our node's parent, or NULL if this node doesn't have a parent node. */
      DataNode * GetParent() const {return _parent;}

      /** Returns this node's depth in the tree (e.g. zero if we are the root node, 1 if we are its child, etc) */
      uint32 GetDepth() const {return _depth;}

      /** Returns us to our virgin, pre-Init() state, by clearing all our children, subscribers, parent, etc.  */
      void Reset();  

      /**
       * Modifies the refcount for the given sessionID.
       * Any sessionID's with (refCount > 0) will be in the GetSubscribers() list.
       * @param sessionID the sessionID whose reference count is to be modified
       * @param delta the amount to add to the reference count.
       */
      void IncrementSubscriptionRefCount(const char * sessionID, long delta);

      /** Returns an iterator that can be used to iterate over our list of active subscribers */
      HashtableIterator<const char *, uint32> GetSubscribers() const;

      /** Returns a pointer to our ordered-child index */
      const Queue<const char *> * GetIndex() const {return _orderedIndex;}

      /** Default Constructor.  Don't use this, use StorageReflectSession::GetNewDataNode() instead!  */
      DataNode();

      /** Destructor.   Don't use this, use StorageReflectSession::ReleaseDataNode() instead!  */
      ~DataNode();

   private:
      friend class StorageReflectSession;

      void Init(const char * nodeName, MessageRef initialValue);
      void SetParent(DataNode * _parent, StorageReflectSession * optNotifyWith);
      status_t RemoveIndexEntry(const char * key, StorageReflectSession * optNotifyWith);

      DataNode * _parent;
      MessageRef _data;
      Hashtable<const char *, DataNodeRef> * _children;  // lazy-allocated
      Queue<const char *> * _orderedIndex;  // only used when tracking the ordering of our children (lazy-allocated)
      uint32 _orderedCounter;
      String _nodeName;
      uint32 _depth;  // number of ancestors our node has (e.g. root's _depth is zero)

      Hashtable<const char *, uint32> _subscribers; 
   };

   /**
    * Create or Set the value of a data node.
    * @param nodePath Should be the path relative to the home dir (e.g. "MyNode/Child1/Grandchild2")
    * @param dataMsgRef The value to set the node to
    * @param allowOverwriteData Indicates whether existing node-data may be overwritten.  If false, the method will fail if the specified node already exists.
    * @param allowCreateNode indicates whether new nodes may be created.  (If false, the method will fail if any node in the specified node path doesn't already exist)
    * @param quiet If set to true, subscribers won't be updated regarding this change to the database.
    * @param appendToIndex If set to true, this node will be inserted under its parent as a new indexed node, rather than doing the regular add/replace bit.
    * @return B_NO_ERROR on success, or B_ERROR on failure.
    */
   status_t SetDataNode(const String & nodePath, MessageRef dataMsgRef, bool allowOverwriteData=true, bool allowCreateNode=true, bool quiet=false, bool appendToIndex=false);

   /** Remove all nodes that match (nodePath).
    *  @param nodePath A relative path indicating node(s) to remove.  Wildcarding is okay.
    *  @param quiet If set to true, subscriber's won't be updated regarding this change to the database
    *  @return B_NO_ERROR on success, or B_ERROR on failure.
    */
   status_t RemoveDataNodes(const String & nodePath, bool quiet = false);

   /**
    * Recursively saves a given subtree of the node database into the given Message object, for safe-keeping.
    * (It's a bit more efficient than it looks, since all data Messages are reference counted rather than copied)
    * @param msg the Message to save the subtree into.  This object can later be provided to RestoreNodeTreeFromMessage() to restore the subtree.
    * @param node The node to begin recursion from (i.e. the root of the subtree)
    * @param path The path to prepend to the paths of children of the node.  Used in the recursion; you typically want to pass in "" here.
    * @param saveData Whether or not the payload Message of (node) should be saved.  The payload Messages of (node)'s children will always be saved no matter what.
    * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
    */
   status_t SaveNodeTreeToMessage(Message & msg, const DataNode * node, const String & path, bool saveData) const;

   /**
    * Recursively creates or updates a subtree of the node database from the given Message object.
    * (It's a bit more efficient than it looks, since all data Messages are reference counted rather than copied)
    * @param msg the Message to restore the subtree into.  This Message is typically one that was created earlier by SaveNodeTreeToMessage().
    * @param path The relative path of the root node, e.g. "" is your home session node.
    * @param loadData Whether or not the payload Message of (node) should be restored.  The payload Messages of (node)'s children will always be restored no matter what.
    * @param appendToIndex Used in the recursion to handle restoring indexed nodes.  You will usually want to Leave it as false when you call this method.
    * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
    */
   status_t RestoreNodeTreeFromMessage(const Message & msg, const String & path, bool loadData, bool appendToIndex = false);

   /** 
     * Create and insert a new node into one or more ordered child indices in the node tree.
     * This method is similar to calling MessageReceivedFromGateway() with a PR_COMMAND_INSERTORDEREDDATA 
     * Message, only it gives more information back about what happened.
     * @param inserMsg a PR_COMMAND_INSERTORDEREDDATA Message specifying what insertions should be done.
     * @param optRetNewNodes If non-NULL, any newly-created DataNodes will be adde to this table for your inspection.
     * @returns B_NO_ERROR on success, or B_ERROR on failure.
     */
   status_t InsertOrderedData(MessageRef insertMsg, Hashtable<String, DataNodeRef> * optRetNewNodes);

   /**
    * This typedef represents the proper signature of a node-tree traversal callback function.
    * Functions with this signature may be used with the NodePathMatcher::DoTraversal() method.
    * @param This The StorageReflectSession doing the traveral, as specified in the DoTraversal() call
    * @param node The DataNode that was matched the criteria of the traversal
    * @param userData the same value that was passed in to the DoTraversal() method.
    * @return The callback function should return the depth at which the traversal should continue after
    *         the callback is done.  So to allow the traversal to continue normally, return (node.GetDepth()),
    *         or to terminate the traversal immediately, return 0, or to resume the search at the next
    *         session, return 2, or etc.
    */
   typedef int (*PathMatchCallback)(StorageReflectSession * This, DataNode & node, void * userData);

   /** A slightly extended version of PathMatcher that knows how to handle DataNodes directly. */
   class NodePathMatcher : public PathMatcher
   {
   public:
      /** Default constructor. */
      NodePathMatcher()  {/* empty */}

      /** Destructor. */
      ~NodePathMatcher() {/* empty */}

      /**
       * Returns true iff the given node matches our query.
       * @param node the node to check to see if it matches
       * @param rootDepth the depth at which the traversal started (i.e. 0 if started at root)
       */
      bool MatchesNode(DataNode & node, int rootDepth) const;

      /**
       * Does a depth-first traversal of the node tree, starting with (node) as the root.
       * @param cb The callback function to call whenever a node is encountered in the traversal
       *           that matches at least one of our path strings.
       * @param node The node to begin the traversal at.
       * @param userData Any value you wish; it will be passed along to the callback method.
       */
      void DoTraversal(PathMatchCallback cb, StorageReflectSession * This, DataNode & node, void * userData);
 
      /**
       * Returns the number of path-strings that we contain that match (node).
       * Note this is a bit more expensive than MatchesNode(), as we can't use short-circuit boolean logic here!
       * @param node A node to check against our set of path-matcher strings.
       * @param rootDepth the depth at which the traversal started (i.e. 0 if started at root)
       */
      int GetMatchCount(DataNode & node, int nodeDepth) const;

   private:
      int DoTraversalAux(PathMatchCallback cb, StorageReflectSession * This, DataNode & node, void * userData, int rootDepth);
      bool PathMatches(DataNode & node, int whichPath, int rootDepth) const;
   };

   friend class StorageReflectSession::DataNode;

   /**
    * Returns true iff we have the given PR_PRIVILEGE_* privilege.
    * @return Default implementation looks at the PR_NAME_PRIVILEGE_BITS parameter.
    */
   virtual bool HasPrivilege(int whichPriv) const;

   /**
    * Returns the given Message to our client, inside an error message
    * with the given error code.
    */
   void BounceMessage(uint32 errorCode, MessageRef msgRef);

   /** Removes our nodes from the tree and removes our subscriptions from our neighbors.  Called by the destructor.
     */
   void Cleanup();

   /** Convenience method:  Passes the given Message on to the sessions who match the given nodePath.
    *  (that is, any sessions who have nodes that match (nodePath) will have their MessageReceivedFromSession()
    *  method called with the given Message)
    *  @param msgRef the Message to pass on.
    *  @param nodePath the node path to match against.  May be absolute (e.g. "/0/1234/frc*") or relative (e.g. "blah")
    *  @return B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
    */
    status_t SendMessageToMatchingSessions(MessageRef msgRef, const String & nodePath);

   /** Convenience method (used by some customized daemons) -- Given a source node and a destination path,
     * Make (path) a deep, recursive clone of (node).  (note that node data messages are not cloned, however)
     * @param sourceNode Reference to a DataNode to clone.
     * @param destPath Path of where the newly created node subtree will appear.  Should be relative to our home node.
     * @return B_NO_ERROR on success, or B_ERROR on failure (may leave a partially cloned subtree on failure)
     */
   status_t CloneDataNodeSubtree(const StorageReflectSession::DataNode & sourceNode, const String & destPath);

   /** Tells other sessions that we have modified (node).  */
   virtual void NotifySubscribersThatNodeChanged(DataNode & node, bool isBeingRemoved);

   /** Tells other sessions that we have a new (node) available.  */
   virtual void NotifySubscribersOfNewNode(DataNode & newNode);

   /** Tells other sessions that we have changed the index of (node).  */
   virtual void NotifySubscribersThatNodeIndexChanged(DataNode & node, char op, uint32 index, const char * key);

   /** Called by NotifySubscribersOfNewNode to tell us that (node) has been modified.  */
   virtual void NodeChanged(DataNode & node, bool isBeingRemoved);

   /** Called by NotifySubscribersThatIndexChanged to tell us how (node)'s index has been modified.  */
   virtual void NodeIndexChanged(DataNode & node, char op, uint32 index, const char * key);

   /**
    * Called by SetParent() to tell us that (node) has been created at a given location.
    * We then respond by letting any matching subscriptions add their mark to the node.
    * @param node the new Node that has been added to the database.
    */
   virtual void NodeCreated(DataNode & node);

   /**
    * Takes any messages that were created in the NodeChanged() callbacks and 
    * sends them to their owner's MessageReceivedFromSession() method for
    * processing and eventual forwarding to the client.
    */
   void PushSubscriptionMessages();

   /** Auxilliary helper method for PushSubscriptionMessages() */
   void PushSubscriptionMessage(MessageRef & msgRef); 

   /**
    * Executes a data-gathering tree traversal based on the PR_NAME_KEYS specified in the given message.
    * @param getMsg a Message whose PR_NAME_KEYS field specifies which nodes we are interested in.
    */
   void DoGetData(const Message & getMsg);

   /**
    * Executes a node-removal traversal using the given NodePathMatcher.
    * Note that you may find it easier to call RemoveDataNodes() than to call this method directly.
    * @param matcher Reference to the NodePathMatcher object to use to guide the node-removal traversal.
    * @param quiet If set to true, subscriber's won't be updated regarding this change to the database
    */
   void DoRemoveData(NodePathMatcher & matcher, bool quiet = false);

   /** Auxilliary helper function. */
   void SendGetDataResults(MessageRef & msg);

   /**
    * If set false, we won't receive subscription updates.
    * @param e Whether or not we wish to get update messages from our subscriptions.
    */
   void SetSubscriptionsEnabled(bool e) {_subscriptionsEnabled = e;}

   /** Returns true iff our "subscriptions enabled" flag is set.  Default state is of this flag is true.  */
   bool GetSubscriptionsEnabled() const {return _subscriptionsEnabled;}

   /**
    * Convenience method:  Uses the given path to lookup a single node in the node tree
    * and return it.  (Note that wildcards are not supported by this method!)
    * If (path) begins with a '/', the search will begin with the root
    * node of the tree; if not, it will begin with this session's node.  Returns NULL on failure.
    * @param path The fully specified path to a single node in the database.
    * @return A pointer to the specified DataNode, or NULL if the node wasn't found.
    */
   DataNode * GetDataNode(const String & path) const;

   /** Traversal callback:  matching nodes are ref'd/unref'd with subscribed session IDs */
   int DoSubscribeRefCallback(DataNode & node, void * userData);

   /** object-pool/recycling callback */
   static void ResetMatcherQueueFunc(StringMatcherQueue * q, void * /*userData*/) {q->Clear();}

   /** object-pool/recycling callback */
   static void ResetNodeFunc(StorageReflectSession::DataNode * node, void * /*userData*/) {node->Reset();}

   /** callback stub */
   static int DoSubscribeRefCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->DoSubscribeRefCallback(node, userData);}

   /** Traversal callback:  matching nodes are sent the given message (userData).  */
   int PassMessageCallback(DataNode & node, void * userData);

   /** callback stub */
   static int PassMessageCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->PassMessageCallback(node, userData);}

   /** Traversal callback:  owners of matching nodes are EndSession()'d (i.e. kicked off the server) */
   int KickClientCallback(DataNode & node, void * userData);

   /** callback stub */
   static int KickClientCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->KickClientCallback(node, userData);}

   /** Traversal callback:  matching nodes are placed into the given message (userData).  */
   int GetDataCallback(DataNode & node, void * userData);

   /** callback stub */
   static int GetDataCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->GetDataCallback(node, userData);}

   /** Traversal callback:  matching nodes are placed in a list (userData) to be removed from the node tree. */
   int RemoveDataCallback(DataNode & node, void * userData);

   /** callback stub */
   static int RemoveDataCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->RemoveDataCallback(node, userData);}

   /** Traversal callback:  matching nodes have ordered data inserted into them as children.  */
   int InsertOrderedDataCallback(DataNode & node, void * userData);

   /** callback stub */
   static int InsertOrderedDataCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->InsertOrderedDataCallback(node, userData);}

   /** Traversal callback:  matching nodes are reordered in their parent's index */
   int ReorderDataCallback(DataNode & node, void * userData);

   /** callback stub */
   static int ReorderDataCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->ReorderDataCallback(node, userData);}

   /** Traversal callback:  matching nodes are added into the user message as archived subtrees */
   int GetSubtreesCallback(DataNode & node, void * userData);

   /** callback stub */
   static int GetSubtreesCallbackFunc(StorageReflectSession * This, DataNode & node, void * userData) {return This->GetSubtreesCallback(node, userData);}

   /**
    * Call this to get a new DataNode, instead of using the DataNode ctor directly.
    * @param nodeName The name to be given to the new DataNode that will be created.
    * @param initialValue The Message payload to be given to the new DataNode that will be created.
    * @return A pointer to the new DataNode (caller assumes responsibility for it), or NULL if out of memory.
    */
   DataNode * GetNewDataNode(const char * nodeName, MessageRef initialValue);

   /**
    * Call this when you are done with a DataNode, instead of the DataNode destructor.
    * @param node The DataNode to delete or recycle.
    */
   void ReleaseDataNode(DataNode * node);

   /**
    * This method goes through the outgoing-messages list looking for PR_RESULT_DATAITEMS
    * messages.  For each such message that it finds, it will remove all result items
    * (both update items and items in PR_NAME_REMOVED_DATAITEMS) that match the expressions
    * in the given NodePathMatcher.  If the NodePathMatcher argument is NULL, all results
    * will be removed.  Any PR_RESULT_DATAITEMS messages that become empty will be removed
    * completely from the queue.
    * @param matcher Specifies which data items to delete from messages in the outgoing message queue.
    */
   void JettisonOutgoingResults(const NodePathMatcher * matcher);

   /**
    * This method goes through the outgoing-messages list looking for PR_RESULT_SUBTREE
    * messages.  For each such message that it finds, it will see if the message's PR_NAME_REQUEST_TREE_ID
    * field matches the pattern specified by (optMatchString), and if it does, it will remove
    * that Message.  If (optMatchString) is NULL, on the other hand, all PR_RESULT_SUBTREE
    * Messages that have no PR_NAME_REQUEST_TREE_ID field will be removed.
    * @param optMatchString Optional pattern to match PR_NAME_REQUEST_TREE_ID strings against.  If
    *                       NULL, all PR_RESULT_SUBTREE Messages with no PR_NAME_TREE_REQUEST_ID field will match.
    */
   void JettisonOutgoingSubtrees(const char * optMatchString);

   /** Returns a reference to our session node */
   DataNodeRef GetSessionNode() const {return _sessionDir;}

   /** Returns a reference to our parameters message */
   Message & GetParameters() {return _parameters;}

   /** Our pool of saved database nodes */
   static DataNodeRef::ItemPool _nodePool;   

   /** Our pool of saved StringMatchers */
   static StringMatcherRef::ItemPool _stringMatcherPool;   

   /** Our pool of saved StringMatcherQueues */
   static StringMatcherQueueRef::ItemPool _stringMatcherQueuePool;    

   /** Returns a reference to the global root node of the database */
   DataNode & GetGlobalRoot() const {return *(_sharedData->_root);}

private:
   /** This class holds data that needs to be shared by all attached instances
     * of the StorageReflectSession class.  An instance of this class is stored
     * on demand in the central-state Message.
     */
   class StorageReflectSessionSharedData
   {
   public:
      StorageReflectSessionSharedData(DataNode * root) : _root(root), _subsDirty(false) {/* empty */}

      DataNode * _root;
      bool _subsDirty;
   };

   /** Sets up the global root and other shared data */
   StorageReflectSessionSharedData * InitSharedData();

   /** our current parameter set */
   Message _parameters;  

   /** cached to be sent when a subscription triggers */
   MessageRef _nextSubscriptionMessage;  

   /** cached to be sent when an index subscription triggers */
   MessageRef _nextIndexSubscriptionMessage;  

   /** Points to shared data object; this object is the same for all StorageReflectSessions */
   StorageReflectSessionSharedData * _sharedData;       

   /** this session's subdir (grandchild of _globalRoot) */
   DataNodeRef _sessionDir;      

   /** Our session's set of active subscriptions */
   NodePathMatcher _subscriptions;  

   /** Where user messages get sent if no PR_NAME_KEYS field is present */
   NodePathMatcher _defaultMessageRoute;  

   /** Whether or not we set to report subscription updates or not */
   bool _subscriptionsEnabled;            

   /** Maximum number of subscription update fields per PR_RESULT message */
   uint32 _maxSubscriptionMessageItems;    

   /** Optimization flag:  set true the first time we index a node */
   bool _indexingPresent;                 

   /** The number of database nodes we currently have created */
   uint32 _currentNodeCount;              

   /** The maximum number of database nodes we are allowed to create */
   uint32 _maxNodeCount;                  

   friend class StorageReflectSession :: NodePathMatcher;
};

};  // end namespace muscle

#endif

