/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "reflector/DataNode.h"
#include "reflector/StorageReflectSession.h"

BEGIN_NAMESPACE(muscle);

DataNode :: DataNode() : _children(NULL), _orderedIndex(NULL), _orderedCounter(0L)
{
   _subscribers.SetKeyCompareFunction(CStringCompareFunc);
}

DataNode :: ~DataNode() 
{
   delete _children;
   delete _orderedIndex;
}

void DataNode :: Init(const char * name, const MessageRef & initData)
{
   _nodeName = name;
   _parent   = NULL;
   _depth    = 0;
   _data     = initData;
}

void DataNode :: Reset()
{
   TCHECKPOINT;

   if (_children) _children->Clear();
   if (_orderedIndex) _orderedIndex->Clear();
   _subscribers.Clear();
   _parent = NULL;
   _depth = 0;
   _data.Reset();
}

void DataNode :: IncrementSubscriptionRefCount(const char * sessionID, long delta)
{
   TCHECKPOINT;

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

status_t DataNode :: InsertOrderedChild(const MessageRef & data, const char * optInsertBefore, const char * optNodeName, StorageReflectSession * notifyWithOnSetParent, StorageReflectSession * optNotifyChangedData, Hashtable<String, DataNodeRef> * optRetAdded)
{
   TCHECKPOINT;

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
   if (optNodeName == NULL)
   {
      while(true)
      {
         sprintf(temp, "I%lu", _orderedCounter++);
         if (HasChild(temp) == false) break;
      }
   }

   DataNode * newNode = notifyWithOnSetParent->GetNewDataNode(optNodeName ? optNodeName : temp, data);
   if (newNode == NULL)
   {
      WARN_OUT_OF_MEMORY; 
      return B_ERROR;
   }
   DataNodeRef dref(newNode);

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

status_t DataNode :: RemoveIndexEntryAt(uint32 removeIndex, StorageReflectSession * optNotifyWith)
{
   TCHECKPOINT;

   if ((_orderedIndex)&&(removeIndex < _orderedIndex->GetNumItems()))
   {
      String holdKey = (*_orderedIndex)[removeIndex];      // gotta make a temp copy here, or it's dangling pointer time
      (void) _orderedIndex->RemoveItemAt(removeIndex);
      if (optNotifyWith) optNotifyWith->NotifySubscribersThatNodeIndexChanged(*this, INDEX_OP_ENTRYREMOVED, removeIndex, holdKey());
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t DataNode :: InsertIndexEntryAt(uint32 insertIndex, StorageReflectSession * notifyWithOnSetParent, const char * key)
{
   TCHECKPOINT;

   if (_children)
   {
      // Find a string matching (key) but that belongs to an actual child...
      HashtableIterator<const char *, DataNodeRef> iter(*_children);
      const char * childKey;
      if (_children->GetKey(key, childKey) == B_NO_ERROR)
      {
         if (_orderedIndex == NULL)
         {
            _orderedIndex = newnothrow Queue<const char *>;
            if (_orderedIndex == NULL) WARN_OUT_OF_MEMORY; 
         }
         if ((_orderedIndex)&&(_orderedIndex->InsertItemAt(insertIndex, childKey) == B_NO_ERROR))
         {
            // Notify anyone monitoring this node that the ordered-index has been updated
            notifyWithOnSetParent->NotifySubscribersThatNodeIndexChanged(*this, INDEX_OP_ENTRYINSERTED, insertIndex, childKey);
            return B_NO_ERROR;
         }
      }
   }
   return B_ERROR;
}

status_t DataNode :: ReorderChild(const DataNode & child, const char * moveToBeforeThis, StorageReflectSession * optNotifyWith)
{
   TCHECKPOINT;

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

status_t DataNode :: PutChild(DataNodeRef & node, StorageReflectSession * optNotifyWithOnSetParent, StorageReflectSession * optNotifyChangedData)
{
   TCHECKPOINT;

   status_t ret = B_ERROR;
   DataNode * child = node();
   if (child)
   {
      if (_children == NULL) 
      {
         _children = newnothrow Hashtable<const char *, DataNodeRef>;
         if (_children == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
         _children->SetKeyCompareFunction(CStringCompareFunc);
      }
      child->SetParent(this, optNotifyWithOnSetParent);
      DataNodeRef oldNode;
      ret = _children->Put(child->_nodeName(), node, oldNode);
      if ((ret == B_NO_ERROR)&&(optNotifyChangedData))
      {
         MessageRef oldData; if (oldNode()) oldData = oldNode()->GetData();
         optNotifyChangedData->NotifySubscribersThatNodeChanged(*child, oldData, false);
      }
   }
   return ret;
}

void DataNode :: SetParent(DataNode * parent, StorageReflectSession * optNotifyWith)
{
   TCHECKPOINT;

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

const char * DataNode :: GetPathClause(uint32 depth) const
{
   if (depth <= _depth)
   {
      const DataNode * node = this;
      for (uint32 i=depth; i<_depth; i++) if (node) node = node->_parent;
      if (node) return node->GetNodeName()();
   }
   return NULL;
}

status_t DataNode :: GetNodePath(String & retPath, uint32 startDepth) const
{
   TCHECKPOINT;

   // Calculate node path and node depth
   if (_parent)
   {
      // Calculate the total length that our node path string will be
      uint32 pathLen = 0;
      {
         uint32 d = _depth;
         const DataNode * node = this;
         while((d-- >= startDepth)&&(node->_parent))
         {
            pathLen += 1 + node->_nodeName.Length();  // the 1 is for the slash
            node = node->_parent;
         }
      }

      if ((pathLen > 0)&&(startDepth > 0)) pathLen--;  // for (startDepth>0), there will be no initial slash

      // Might as well make sure we have enough memory to return it, up front
      if (retPath.Prealloc(pathLen) != B_NO_ERROR) return B_ERROR;

      char * dynBuf = NULL;
      const uint32 stackAllocSize = 256;
      char stackBuf[stackAllocSize];      // try to do this without a dynamic allocation...
      if (pathLen >= stackAllocSize)  // but do a dynamic allocation if we have to (should be rare)
      {
         dynBuf = newnothrow_array(char, pathLen+1);
         if (dynBuf == NULL) 
         { 
            WARN_OUT_OF_MEMORY; 
            return B_ERROR;
         }
      }

      char * writeAt = (dynBuf ? dynBuf : stackBuf) + pathLen;  // points to last char in buffer
      *writeAt = '\0';  // terminate the string first (!)
      const DataNode * node = this;
      uint32 d = _depth;
      while((d >= startDepth)&&(node->_parent))
      {
         int len = node->_nodeName.Length();
         writeAt -= len;
         memcpy(writeAt, node->_nodeName(), len);
         if ((startDepth == 0)||(d > startDepth)) *(--writeAt) = '/';
         node = node->_parent;
         d--;
      }
 
      retPath = (dynBuf ? dynBuf : stackBuf);
      delete [] dynBuf;
   }
   else retPath = (startDepth == 0) ? "/" : "";

   return B_NO_ERROR;
}

status_t DataNode :: RemoveChild(const char * key, StorageReflectSession * optNotifyWith, bool recurse, uint32 * optCurrentNodeCount)
{
   TCHECKPOINT;

   DataNodeRef childRef;
   if ((_children)&&(_children->Get(key, childRef) == B_NO_ERROR))
   {
      DataNode * child = childRef.GetItemPointer();
      if (child)
      {
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

         (void) RemoveIndexEntry(key, optNotifyWith);
         if (optNotifyWith) optNotifyWith->NotifySubscribersThatNodeChanged(*child, child->GetData(), true);

         child->SetParent(NULL, optNotifyWith);
      }
      if (optCurrentNodeCount) (*optCurrentNodeCount)--;
      _children->Remove(key, childRef);
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t DataNode :: RemoveIndexEntry(const char * key, StorageReflectSession * optNotifyWith)
{
   TCHECKPOINT;

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

void DataNode ::   SetData(const MessageRef & data, StorageReflectSession * optNotifyWith, bool isBeingCreated)
{
   MessageRef oldData;
   if (isBeingCreated == false) oldData = _data;
   _data = data;
   if (optNotifyWith) optNotifyWith->NotifySubscribersThatNodeChanged(*this, oldData, false);
}

END_NAMESPACE(muscle);
