/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleObjectPool_h
#define MuscleObjectPool_h

#include "system/Mutex.h"

namespace muscle {

// Uncomment this #define to disable object pools (i.e. turn them into
// fancy new/delete operators).  This is helpful if you are trying
// to track down memory leaks.
//#define DISABLE_OBJECT_POOLING 1

#ifndef MUSCLE_POOL_SLAB_SIZE
# define MUSCLE_POOL_SLAB_SIZE (4*1024)  // let's have each slab fit nicely into a 4KB page
#endif

/** An interface that must be implemented by all ObjectPool classes.
  * Used to support polymorphism in pool management.
  */
class AbstractObjectGenerator
{
public:
   /** Default ctor */
   AbstractObjectGenerator() {/* empty */}

   /** Virtual dtor to keep C++ honest */
   virtual ~AbstractObjectGenerator() {/* empty */}

   /** Should be implemented to pass through to ObtainObject(). 
     * Useful for handling different types of ObjectPool interchangably.
     * Note that the caller is responsible for deleting or recycling
     * the returned object!
     */
   virtual void * ObtainObjectGeneric() = 0;
};

/** An interface that must be implemented by all ObjectPool classes.
  * Used to support polymorphism in our reference counting. 
  */
class AbstractObjectRecycler
{
public:
   /** Default constructor.  Registers us with the global recyclers list. */
   AbstractObjectRecycler();

   /** Default destructor.  Unregisters us from the global recyclers list. */
   virtual ~AbstractObjectRecycler();

   /** Should be implemented to downcast (obj) to the correct type,
     * and recycle it (typically by calling ReleaseObject().
     * @param obj Pointer to the object to recycle.  Must point to an object of the correct type.
     *            May be NULL, in which case this method is a no-op.
     */
   virtual void RecycleObject(void * obj) = 0;

   /** Should be implemented to destroy all objects we have cached and
     * return the number of objects that were destroyed.
     */
   virtual uint32 FlushCachedObjects() = 0;

   /** Walks the linked list of all AbstractObjectRecyclers, calling
     * FlushCachedObjects() on each one, until all cached objects have been destroyed.
     * This method is called by the SetupSystem destructor, to ensure that
     * all objects have been deleted before main() returns, so that shutdown
     * ordering issues do not cause bad memory writes, etc.
     */
   static void GlobalFlushAllCachedObjects();

private:
   AbstractObjectRecycler * _prev;
   AbstractObjectRecycler * _next;
};

/** This class is just here to usefully tie together the object generating and
  * object recycling capabilities of its two superclasses into a single interface.
  */
class AbstractObjectManager : public AbstractObjectGenerator, public AbstractObjectRecycler
{
   // empty
};

/** A thread-safe templated object pooling class that helps reduce the number of 
 *  dynamic allocations and deallocations in your app.  Instead of calling 'new Object', 
 *  you call myObjectPool.ObtainObject(), and instead of calling 'delete Object', you call
 *  myObjectPool.ReleaseObject().  The advantage is that the ObjectPool will
 *  keep (up to) a certain number of "spare" Objects around, and recycle them back
 *  to you as needed. 
 */
template <class Object> class ObjectPool : public AbstractObjectManager
{
public:
   /** Callback function type for Init and Recycle callbacks */
   typedef void (*ObjectCallback)(Object * obj, void * userData);

   /**
    *  Constructor.
    *  @param maxPoolSize the maximum number of recycled objects that may be kept around for future reuse at any one time.
    *  @param recycleCallback optional callback function to be called whenever an object is returned to the pool.
    *  @param recycleData user value to be passed in to the recycleCallback function.
    *  @param initCallback optional callback function to be called whenever an object is taken out of the pool.
    *  @param initCallbackData user value to be passed in to the initCallback function.
    */      
   ObjectPool(uint32 maxPoolSize=100, 
              ObjectCallback recycleCallback = NULL, void * recycleData = NULL,
              ObjectCallback initCallback = NULL, void * initCallbackData = NULL) : 
      _initObjectFunc(initCallback), _initObjectUserData(initCallbackData),
      _recycleObjectFunc(recycleCallback), _recycleObjectUserData(recycleData),
      _curPoolSize(0), _maxPoolSize(maxPoolSize), _firstSlab(NULL), _lastSlab(NULL)
   {
      // empty
   }

   /** 
    *  Destructor.  Deletes all objects in the pool.
    */
   virtual ~ObjectPool()
   {
      while(_firstSlab)
      {
         if (_firstSlab->IsInUse()) 
         {
            LogTime(MUSCLE_LOG_CRITICALERROR, "~ObjectPool %p:  slab %p is still in use when we destroy it!\n", this, _firstSlab);
            MCRASH("ObjectPool destroyed while its objects were still in use (Is a CompleteSetupSystem object declared at the top of main()?)");
         }
         ObjectSlab * nextSlab = _firstSlab->GetNext();
         delete _firstSlab;
         _firstSlab = nextSlab;
      }
   }

   /** Returns a new Object for use (or NULL if no memory available).
    *  You are responsible for calling ReleaseObject() on this object
    *  when you are done with it.
    *  This method is thread-safe.
    *  @return a new Object, or NULL if out of memory.
    */
   Object * ObtainObject()
   {
#ifdef DISABLE_OBJECT_POOLING
      Object * ret = newnothrow Object;
      if (ret) InitObjectAux(ret);
          else WARN_OUT_OF_MEMORY;
      return ret;
#else
      Object * ret = NULL;
      if (_mutex.Lock() == B_NO_ERROR)
      {
         ret = ObtainObjectAux();
         _mutex.Unlock();
      }
      if (ret) InitObjectAux(ret);
          else WARN_OUT_OF_MEMORY;
      return ret;
#endif
   }

   /** Adds the given object to our "standby" object list to be recycled, or
    *  deletes it if the "standby" list is already at its maximum size.
    *  This method is thread-safe.
    *  @param obj An Object to recycle or delete.  May be NULL.  
    */
   void ReleaseObject(Object * obj)
   {
      if (obj)
      {
         MASSERT(obj->GetManager()==this, "ObjectPool::ReleaseObject was passed an object that it never allocated!");
         if (_recycleObjectFunc) _recycleObjectFunc(obj, _recycleObjectUserData);
         obj->SetManager(NULL);

#ifdef DISABLE_OBJECT_POOLING
         delete obj;
#else
         if (_mutex.Lock() == B_NO_ERROR)
         {
            ObjectSlab * slabToDelete = ReleaseObjectAux(obj);
            _mutex.Unlock();
            delete slabToDelete;  // do this outside the critical section, for better concurrency
         }
         else WARN_OUT_OF_MEMORY;  // critical error -- not really out of memory but still
#endif
      }
   }

   /** AbstractObjectGenerator API:  Useful for polymorphism */
   virtual void * ObtainObjectGeneric() {return ObtainObject();}

   /** AbstractObjectRecycler API:  Useful for polymorphism */
   virtual void RecycleObject(void * obj) {ReleaseObject((Object *)obj);}
    
  /**
   * With this call you can specify a callback function that
   * will be called whenever a new Object is created by this pool. 
   * If you need to do any initialization on the new Object, you can
   * do it here.
   * This method is thread safe.
   * @param cb The callback function that ObtainObject() will call.
   * @param userData User value that will be passed through to the callback function.
   * @returns B_NO_ERROR on success, B_ERROR if it couldn't lock its lock for some reason.
   */
   status_t SetInitObjectCallback(ObjectCallback cb, void * userData)
   {
      if (_mutex.Lock() == B_NO_ERROR)
      {
         _initObjectFunc = cb;
         _initObjectUserData = userData;
         (void) _mutex.Unlock();
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }

  /**
   * With this call you can specify a callback function that
   * will be called by ReleaseObject() whenever it is putting an
   * Object into its standby list.
   * If you wish to do any cleanup on the object, you can do it here.
   * @param cb The callback function that ReleaseObject() will call.
   * @param userData User value that will be passed through to the callback function.
   * @returns B_NO_ERROR on success, B_ERROR if it couldn't lock its lock for some reason.
   */
   void SetRecycleObjectCallback(ObjectCallback cb, void * userData)
   {
      if (_mutex.Lock() == B_NO_ERROR)
      {
         _recycleObjectFunc = cb;
         _recycleObjectUserData = userData;
         (void) _mutex.Unlock();
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }

   /** Implemented to call Drain() and return the number of objects drained. */
   virtual uint32 FlushCachedObjects() {uint32 ret = 0; (void) Drain(&ret); return ret;}

   /** Removes all "spare" objects from the pool and deletes them. 
     * This method is thread-safe.
     * @param optSetNumDrained If non-NULL, this value will be set to the number of objects destroyed.
     * @returns B_NO_ERROR on success, or B_ERROR if it couldn't lock the lock for some reason.
     */
   status_t Drain(uint32 * optSetNumDrained = NULL)
   {
      if (_mutex.Lock() == B_NO_ERROR)
      {
         // This will be our linked list of slabs to delete, later
         ObjectSlab * toDelete = NULL;

         // Pull out all the slabs that are not currently in use
         ObjectSlab * slab = _firstSlab;
         while(slab)
         {
            ObjectSlab * nextSlab = slab->GetNext();
            if (slab->IsInUse() == false)
            {
               slab->RemoveFromSlabList();
               slab->SetNext(toDelete);
               toDelete = slab;
            }
            slab = nextSlab;
         }
         (void) _mutex.Unlock();

         // Do the actual slab deletions outside of the critical section, for better concurrency
         uint32 numObjectsDeleted = 0;
         while(toDelete)
         {
            ObjectSlab * nextSlab = toDelete->GetNext();

            numObjectsDeleted += NUM_OBJECTS_PER_SLAB;
            _curPoolSize -= NUM_OBJECTS_PER_SLAB;

            delete toDelete;
            toDelete = nextSlab;
         }

         if (optSetNumDrained) *optSetNumDrained = numObjectsDeleted;
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }

   /** Returns the maximum number of "spare" objects that will be kept
     * in the pool, ready to be recycled.  This is the value that was
     * previously set either in the constructor or by SetMaxPoolSize().
     */
   uint32 GetMaxPoolSize() const {return _maxPoolSize;}

   /** Sets a new maximum size for this pool.  Note that changing this
     * value will not cause any object to be added or removed to the
     * pool immediately;  rather the new size will be enforced only
     * on future operations.
     */
   void SetMaxPoolSize(uint32 mps) {_maxPoolSize = mps;}

private:
   Mutex _mutex;

   ObjectCallback _initObjectFunc;
   void * _initObjectUserData;
   
   ObjectCallback _recycleObjectFunc;
   void * _recycleObjectUserData;

   class ObjectSlab;

   class ObjectNode : public Object
   {
   public:
      ObjectNode() : _slab(NULL), _next(NULL) {/* empty */}

      void SetSlab(ObjectSlab * slab) {_slab = slab;}
      ObjectSlab * GetSlab() const {return _slab;}

      void SetNext(ObjectNode * next) {_next = next;}
      ObjectNode * GetNext() const {return _next;}

   private:
      ObjectSlab * _slab;
      ObjectNode * _next;  // only used when we are in the free list
   };

   friend class ObjectSlab;  // for VC++ compatibility, this must be here

   // All the (int) casts are here so that it the user specifies a slab size of zero, we will get a negative
   // number and not a very large positive number that crashes the compiler!
   #define __POOL_OPS__ (((int)MUSCLE_POOL_SLAB_SIZE-(5*(int)sizeof(void *)))/(int)sizeof(ObjectNode))
   enum {NUM_OBJECTS_PER_SLAB = ((__POOL_OPS__>0)?__POOL_OPS__:1)};
   
   class ObjectSlab
   {
   public:
      // Note that _prev and _next are deliberately not set here... we don't use them until we are added to the list
      ObjectSlab(ObjectPool * pool) : _pool(pool), _firstFreeNode(NULL), _numNodesInUse(0)
      {
         for (int32 i=0; i<NUM_OBJECTS_PER_SLAB; i++)
         {
            ObjectNode * n = &_nodes[i];
            n->SetSlab(this);
            n->SetNext(_firstFreeNode);
            _firstFreeNode = n;
         }
      }

      /** Returns true iff there is at least one ObjectNode available in this slab. */
      bool HasAvailableNodes() const {return (_firstFreeNode != NULL);}

      /** Returns true iff there is at least one ObjectNode in use in this slab. */
      bool IsInUse() const {return (_numNodesInUse > 0);}

      // Note:  this method assumes a node is available!  Don't call it without
      //        calling HasAvailableNodes() first to check, or you will crash!
      ObjectNode * ObtainObjectNode()
      {
         ObjectNode * ret = _firstFreeNode;
         _firstFreeNode = ret->GetNext();
         ++_numNodesInUse;
         return ret;
      }

      /** Adds the specified node back to our free-nodes list. */
      void ReleaseNode(ObjectNode * node)
      {
         node->SetNext(_firstFreeNode);
         _firstFreeNode = node; 
         --_numNodesInUse;
      }

      void RemoveFromSlabList()
      {
         (_prev ? _prev->_next : _pool->_firstSlab) = _next;
         (_next ? _next->_prev : _pool->_lastSlab)  = _prev;
      }

      void AppendToSlabList()
      {
         _prev = _pool->_lastSlab;
         _next = NULL;
         (_prev ? _prev->_next : _pool->_firstSlab) = _pool->_lastSlab = this;
      }

      void PrependToSlabList()
      {
         _prev = NULL;
         _next = _pool->_firstSlab;
         (_next ? _next->_prev : _pool->_lastSlab) = _pool->_firstSlab = this;
      }

      void SetNext(ObjectSlab * next) {_next = next;}
      ObjectSlab * GetNext() const {return _next;}

   private:
      ObjectPool * _pool;
      ObjectSlab * _prev;
      ObjectSlab * _next;
      ObjectNode * _firstFreeNode;
      uint32 _numNodesInUse;
      ObjectNode _nodes[NUM_OBJECTS_PER_SLAB];
   };

   // Must be called with _mutex locked!   Returns either NULL, or a pointer to a
   // newly allocated Object.
   Object * ObtainObjectAux()
   {
      Object * ret = NULL;
      if ((_firstSlab)&&(_firstSlab->HasAvailableNodes()))
      {
         ret = _firstSlab->ObtainObjectNode();
         if ((_firstSlab->HasAvailableNodes() == false)&&(_firstSlab != _lastSlab))
         {
            // Move _firstSlab out of the way (to the end of the slab list) for next time
            ObjectSlab * tmp = _firstSlab;  // use temp var since _firstSlab will change
            tmp->RemoveFromSlabList();
            tmp->AppendToSlabList();
         }
      }
      else
      {
         // Hmm, we must have run out out of non-full slabs.  Create a new slab and use it.
         ObjectSlab * slab = newnothrow ObjectSlab(this);
         if (slab)
         {
            ret = slab->ObtainObjectNode();  // guaranteed not to fail, since slab is new
            if (slab->HasAvailableNodes()) slab->PrependToSlabList();
                                      else slab->AppendToSlabList();  // could happen, if NUM_OBJECTS_PER_SLAB==1
            _curPoolSize += NUM_OBJECTS_PER_SLAB;
         }
         // we'll do the WARN_OUT_OF_MEMORY below, outside the mutex lock
      }
      if (ret) --_curPoolSize;
      return ret;
   }

   // Must be called with _mutex locked!   Returns either NULL, or a pointer 
   // an ObjectSlab that should be deleted outside of the critical section.
   ObjectSlab * ReleaseObjectAux(Object * obj)
   {
      ObjectNode * objNode = static_cast<ObjectNode *>(obj);
      ObjectSlab * objSlab = objNode->GetSlab();

      objSlab->ReleaseNode(objNode);  // guaranteed to work, since we know (obj) is in use in (objSlab)

      if ((++_curPoolSize > (_maxPoolSize+NUM_OBJECTS_PER_SLAB))&&(objSlab->IsInUse() == false))
      {
         _curPoolSize -= NUM_OBJECTS_PER_SLAB;
         objSlab->RemoveFromSlabList();
         return objSlab;
      }
      else if (objSlab != _firstSlab) 
      {
         objSlab->RemoveFromSlabList();
         objSlab->PrependToSlabList();
      }
      return NULL;
   }

   void InitObjectAux(Object * o)
   {
      o->SetManager(this);
      if (_initObjectFunc) _initObjectFunc(o, _initObjectUserData);
   }

   uint32 _curPoolSize;  // tracks the current number of "available" objects
   uint32 _maxPoolSize;  // the maximum desired number of "available" objects
   ObjectSlab * _firstSlab;
   ObjectSlab * _lastSlab;
};

}; // end namespace muscle

#endif
