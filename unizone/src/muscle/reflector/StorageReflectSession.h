/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

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

#define DECLARE_MUSCLE_TRAVERSAL_CALLBACK(sessionClass, funcName) \
 int funcName(DataNode & node, void * userData); \
 static int funcName##Func(StorageReflectSession * This, DataNode & node, void * userData) {return ((sessionClass *)This)->funcName(node, userData);}

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
   virtual void MessageReceivedFromGateway(MessageRef msg, void * userData);

   /** Overridden to call PushSubscriptionMessages() */
   virtual void AfterMessageReceivedFromGateway(MessageRef msg, void * userData);

   /** Drains the storage pools used by StorageReflectSessions, to recover memory */
   static void DrainPools();

   /** Returns a human-readable label for this session type:  "Session" */
   virtual const char * GetTypeName() const {return "Session";}

protected:
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
       * @param optNodeName If non-NULL, the inserted node will have the specified name.  Otherwise, a name will be generated for the node.
       * @param optNotifyWithOnSetParent If non-NULL, a StorageReflectSession to use to notify subscribers that the new node has been added
       * @param optNotifyWithOnChangedData If non-NULL, a StorageReflectSession to use to notify subscribers when the node's data has been alterered
       * @param optAddNewChildren If non-NULL, any newly formed nodes will be added to this hashtable, keyed on their absolute node path.
       * @return B_NO_ERROR on success, B_ERROR if out of memory
       */
      status_t InsertOrderedChild(MessageRef data, const char * optInsertBefore, const char * optNodeName, StorageReflectSession * optNotifyWithOnSetParent, StorageReflectSession * optNotifyWithOnChangedData, Hashtable<String, DataNodeRef> * optAddNewChildren);
 
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
        * @param startDepth The depth at which the path should start.  Defaults to zero, meaning the full path.
        *                   Values greater than zero will return a partial path (e.g. a startDepth of 1 in the
        *                   above example would return "12.18.240.15/1234/beshare/files/joe", and a startDepth
        *                   of 2 would return "1234/beshare/files/joe")
        * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
        */
      status_t GetNodePath(String & retPath, uint32 startDepth = 0) const;

      /** Returns the name of the node in our path at the (depth) level.
        * @param depth The node name we are interested in.  For example, 0 will return the name of the
        *              root node, 1 would return the name of the IP address node, etc.  If this number
        *              is greater than (depth), this method will return NULL.
        */
      const char * GetPathClause(uint32 depth) const;

      /** Replaces this node's payload message with that of (data).
       *  @data the new Message to associate with this node.
       *  @param optNotifyWith if non-NULL, this StorageReflectSession will be used to notify subscribers that this node's data has changed.
       *  @param isBeingCreated Should be set true only if this is the first time SetData() was called on this node after its creation.
       *                        Which is to say, this should almost always be false.
       */
      void SetData(MessageRef data, StorageReflectSession * optNotifyWith, bool isBeingCreated);

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

      /** Insert a new entry into our ordered-child list at the (nth) entry position.
       *  Don't call this function unless you really know what you are doing!
       *  @param insertIndex Offset into the array to insert at
       *  @param optNotifyWith Session to use to inform everybody that the index has been changed.
       *  @param key Name of an existing child of this node that should be associated with the given entry.
       *             This child must not already be in the ordered-entry index!
       *  @return B_NO_ERROR on success, or B_ERROR on failure (bad index or unknown child).
       */
      status_t InsertIndexEntryAt(uint32 insertIndex, StorageReflectSession * optNotifyWith, const char * key);

      /** Removes the (nth) entry from our ordered-child index, if possible.
       *  Don't call this function unless you really know what you are doing!
       *  @param removeIndex Offset into the array to remove
       *  @param optNotifyWith Session to use to inform everybody that the index has been changed.
       *  @return B_NO_ERROR on success, or B_ERROR on failure.
       */
      status_t RemoveIndexEntryAt(uint32 removeIndex, StorageReflectSession * optNotifyWith);

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
    * @param addToIndex If set to true, this node will be inserted under its parent as a new indexed node, rather than doing the regular add/replace bit.
    * @param optInsertBefore If (addToIndex) is true, this may be the name of the node to insert this new node before in the index.
    *                        If NULL, the new node will be appended to the end of the index.  If (addToIndex) is false, this argument is ignored.
    * @return B_NO_ERROR on success, or B_ERROR on failure.
    */
   status_t SetDataNode(const String & nodePath, MessageRef dataMsgRef, bool allowOverwriteData=true, bool allowCreateNode=true, bool quiet=false, bool addToIndex=false, const char *optInsertBefore=NULL);

   /** Remove all nodes that match (nodePath).
    *  @param nodePath A relative path indicating node(s) to remove.  Wildcarding is okay.
    *  @param filterRef If non-NULL, we'll use the given QueryFilter object to filter out our result set.
    *                   Only nodes whose Messages match the QueryFilter will be removed.  Default is a NULL reference.
    *  @param quiet If set to true, subscriber's won't be updated regarding this change to the database
    *  @return B_NO_ERROR on success, or B_ERROR on failure.
    */
   status_t RemoveDataNodes(const String & nodePath, QueryFilterRef filterRef = QueryFilterRef(), bool quiet = false);

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
       * @param optData Message to use for QueryFilter filtering, or NULL to disable filtering.
       * @param rootDepth the depth at which the traversal started (i.e. 0 if started at root)
       */
      bool MatchesNode(DataNode & node, const Message * optData, int rootDepth) const;

      /**
       * Does a depth-first traversal of the node tree, starting with (node) as the root.
       * @param cb The callback function to call whenever a node is encountered in the traversal
       *           that matches at least one of our path strings.
       * @param node The node to begin the traversal at.
       * @param useFilters If true, we will only call (cb) on nodes whose Messages match our filter; otherwise
       *                   we'll call (cb) on any node whose path matches, regardless of filtering status.
       * @param userData Any value you wish; it will be passed along to the callback method.
       */
      void DoTraversal(PathMatchCallback cb, StorageReflectSession * This, DataNode & node, bool useFilters, void * userData);
 
      /**
       * Returns the number of path-strings that we contain that match (node).
       * Note this is a bit more expensive than MatchesNode(), as we can't use short-circuit boolean logic here!
       * @param node A node to check against our set of path-matcher strings.
       * @param optData If non-NULL, any QueryFilters will use this Message to filter against.
       * @param rootDepth the depth at which the traversal started (i.e. 0 if started at root)
       */
      uint32 GetMatchCount(DataNode & node, const Message * optData, int nodeDepth) const;

   private:
      // This little structy class is used to pass context data around more efficiently
      class TraversalContext
      {
      public:
         TraversalContext(PathMatchCallback cb, StorageReflectSession * This, bool useFilters, void * userData, int rootDepth) : _cb(cb), _This(This), _useFilters(useFilters), _userData(userData), _rootDepth(rootDepth) {/* empty */}

         PathMatchCallback _cb;
         StorageReflectSession * _This;
         bool _useFilters;
         void * _userData;
         int _rootDepth;
      };

      int DoTraversalAux(const TraversalContext & data, DataNode & node);
      bool PathMatches(DataNode & node, const Message * optData, const PathMatcherEntry & entry, int rootDepth) const;
      bool CheckChildForTraversal(const TraversalContext & data, DataNode * nextChild, int & depth);
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

   /** Convenience method:  Adds sessions that contain nodes that match the given pattern to the passed-in Hashtable.
    *  @param nodePath the node path to match against.  May be absolute (e.g. "/0/1234/frc*") or relative (e.g. "blah")
    *                  If the nodePath is a zero-length String, all sessions will match.
    *  @param filter If non-NULL, only nodes whose data Messages match this filter will have their sessions added 
    *                to the (retSessions) table.
    *  @param retSessions A table that will on return contain the set of matching sessions, keyed by their session ID strings.
    *                     Make sure you have called SetKeyCompareFunction(CStringCompareFunc) on this table!
    *  @param matchSelf If true, we will include as a candidate for pattern matching.  Otherwise we won't.
    *  @return B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
    */
    status_t FindMatchingSessions(const String & nodePath, QueryFilterRef filter, Hashtable<const char *, AbstractReflectSessionRef> & retSessions, bool matchSelf) const;

   /** Convenience method:  Passes the given Message on to the sessions who match the given nodePath.
    *  (that is, any sessions who have nodes that match (nodePath) will have their MessageReceivedFromSession()
    *  method called with the given Message)
    *  @param msgRef the Message to pass on.
    *  @param nodePath the node path to match against.  May be absolute (e.g. "/0/1234/frc*") or relative (e.g. "blah")
    *                  If the nodePath is a zero-length String, all sessions will match.
    *  @param filter If non-NULL, only nodes whose data Messages match this filter will receive the Message.
    *  @param matchSelf If true, we will include as a candidate for pattern matching.  Otherwise we won't.
    *  @return B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
    */
    status_t SendMessageToMatchingSessions(MessageRef msgRef, const String & nodePath, QueryFilterRef filter, bool matchSelf);

    /** This type is sometimes used when calling CloneDataNodeSubtree(). */
    typedef MessageRef (*MessageReplaceFunc)(MessageRef oldRef, void *);

   /** Convenience method (used by some customized daemons) -- Given a source node and a destination path,
     * Make (path) a deep, recursive clone of (node).
     * @param sourceNode Reference to a DataNode to clone.
     * @param destPath Path of where the newly created node subtree will appear.  Should be relative to our home node.
     * @param allowOverwriteData If true, we will clobber any previously existing node at the destination path.
     *                           Otherwise, the existence of a pre-existing node there will cause us to fail.
     * @param allowCreateNode If true, we will create a node at the destination path if necessary.
     *                        Otherwise, the non-existence of a pre-existing node there will cause us to fail.
     * @param quiet If false, no subscribers will be notified of the changes we make.
     * @param addToTargetIndex If true, the newly created subtree will be added to the target node using InsertOrderedChild().
     *                         If false, it will be added using PutChild().
     * @param optInsertBefore If (addToTargetIndex) is true, this argument will be passed on to InsertOrderedChild().
     *                        Otherwise, this argument is ignored.
     * @param optFunc If specified as non-NULL, this callback function will be called as a hook to adjust the data payloads of
     *                the newly created node(s).  Note that this function should NOT modify any Message in place; rather it should
     *                return a MessageRef to be used in the newly created nodes, which may be the passed-in MessageRef of the existing
     *                node, or a newly created one.
     * @param optFuncArg This argument is passed to (optFunc) as its second argument.
     * @return B_NO_ERROR on success, or B_ERROR on failure (may leave a partially cloned subtree on failure)
     */
   status_t CloneDataNodeSubtree(const StorageReflectSession::DataNode & sourceNode, const String & destPath, bool allowOverwriteData=true, bool allowCreateNode=true, bool quiet=false, bool addToTargetIndex=false, const char * optInsertBefore = NULL, MessageReplaceFunc optFunc = NULL, void * optFuncArg = NULL);

   /** Tells other sessions that we have modified (node) in our node subtree.
    *  @param node The node that has been modfied.
    *  @param oldData If the node is being modified, this argument contains the node's previously
    *                 held data.  If it is being created, this is a NULL reference.  If the node
    *                 is being destroyed, this will contain the node's current data.
    *  @param isBeingRemoved If true, this node is about to go away.
   */
   virtual void NotifySubscribersThatNodeChanged(DataNode & node, MessageRef oldData, bool isBeingRemoved);

   /** Tells other sessions that we have changed the index of (node) in our node subtree.
    *  @param node The node whose index was changed.
    *  @param op The INDEX_OP_* opcode of the change.  (Note that currently INDEX_OP_CLEARED is never used here)
    *  @param index The index at which the operation took place (not defined for clear operations)
    *  @param key The key of the operation (aka the name of the associated node)
    */
   virtual void NotifySubscribersThatNodeIndexChanged(DataNode & node, char op, uint32 index, const char * key);

   /** Called by NotifySubscribersThatNodeChanged(), to tell us that (node) has been 
    *  created, modified, or is about to be destroyed.
    *  @param node The node that was modified, created, or is about to be destroyed.
    *  @param oldData If the node is being modified, this argument contains the node's previously
    *                 held data.  If it is being created, this is a NULL reference.  If the node
    *                 is being destroyed, this will contain the node's current data.
    *  @param isBeingRemoved True iff this node is about to be destroyed.
    */
   virtual void NodeChanged(DataNode & node, MessageRef oldData, bool isBeingRemoved);

   /** Called by NotifySubscribersThatIndexChanged() to tell us how (node)'s index has been modified.  
    *  @param node The node whose index was changed.
    *  @param op The INDEX_OP_* opcode of the change.  (Note that currently INDEX_OP_CLEARED is never used here)
    *  @param index The index at which the operation took place (not defined for clear operations)
    *  @param key The key of the operation (aka the name of the associated node)
    */
   virtual void NodeIndexChanged(DataNode & node, char op, uint32 index, const char * key);

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

   /** Returns a reference to an empty Message.
    *  Note that it is an error to modify the Message contained in this reference!
    */
   MessageRef GetBlankMessage() const;

   /** object-pool/recycling callback */
   static void ResetNodeFunc(StorageReflectSession::DataNode * node, void * /*userData*/) {node->Reset();}

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

   /** Returns a reference to the global root node of the database */
   DataNode & GetGlobalRoot() const {return *(_sharedData->_root);}

   /** Macro to declare the proper callback declarations for the message-passing callback.  */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, PassMessageCallback);    /** Matching nodes are sent the given message.  */

private:
   void NodeChangedAux(DataNode & modifiedNode, bool isBeingRemoved);
   void UpdateDefaultMessageRoute();
   int PassMessageCallbackAux(DataNode & node, MessageRef msgRef, bool matchSelfOkay);

   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, KickClientCallback);     /** Sessions of matching nodes are EndSession()'d  */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, InsertOrderedDataCallback); /** Matching nodes have ordered data inserted into them as child nodes */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, ReorderDataCallback);    /** Matching nodes area reordered in their parent's index */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, GetSubtreesCallback);    /** Matching nodes are added in to the user Message as archived subtrees */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, GetDataCallback);        /** Matching nodes are added to the (userData) Message */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, RemoveDataCallback);     /** Matching nodes are placed in a list (userData) for later removal. */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, DoSubscribeRefCallback); /** Matching nodes are ref'd or unref'd with subscribed session IDs */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, ChangeQueryFilterCallback); /** Matching nodes are ref'd or unref'd depending on the QueryFilter change */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, FindSessionsCallback);   /** Sessions of matching nodes are added to the given Hashtable */
   DECLARE_MUSCLE_TRAVERSAL_CALLBACK(StorageReflectSession, SendMessageCallback);    /** Similar to PassMessageCallback except matchSelf is an argument */

   /**
    * Called by SetParent() to tell us that (node) has been created at a given location.
    * We then respond by letting any matching subscriptions add their mark to the node.
    * Private because subclasses should override NodeChanged(), not this.
    * @param node the new Node that has been added to the database.
    */
   void NodeCreated(DataNode & node);

   /** Tells other sessions that we have a new node available.  
    *  Private because subclasses should override NotifySubscriberThatNodeChanged(), not this.
    *  @param newNode The newly available node.
    */
   void NotifySubscribersOfNewNode(DataNode & newNode);

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
   Message _defaultMessageRouteMessage;

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

