/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleObjectPool_h
#define MuscleObjectPool_h

#include "util/Queue.h"
#include "system/Mutex.h"

BEGIN_NAMESPACE(muscle);

// Uncomment this #define to disable object pools (i.e. turn them into
// fancy new/delete operators).  This is helpful if you are trying
// to track down memory leaks.
//#define DISABLE_OBJECT_POOLING 1

/** An interface that must be implemented by all ObjectPool classes.
  * Used to support polymorphism in pool management.
  */
class AbstractObjectGenerator
{
public:
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
   /** Should be implemented to downcast (obj) to the correct type,
     * and recycle it (typically by calling ReleaseObject().
     * @param obj Pointer to the object to recycle.  Must point to an object of the correct type.
     *            May be NULL, in which case this method is a no-op.
     */
   virtual void RecycleObject(void * obj) = 0;
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
      _maxPoolSize(maxPoolSize)
   {
      // empty
   }

   /** 
    *  Destructor.
    *  Deletes all held "spare" objects in the pool.
    */
   virtual ~ObjectPool()
   {
      Drain();
   }

   /** Returns a new Object for use (or NULL if no memory available).
    *  You are responsible for calling ReleaseObject() on this object
    *  when you are done with it.  This method can be used interchangably
    *  with 'new (nothrow) Object()'.
    *  This method is thread-safe.
    *  @return a new Object, or NULL if out of memory.
    */
   Object * ObtainObject()
   {
#ifdef DISABLE_OBJECT_POOLING
      Object * ret = newnothrow Object;
      if (ret == NULL) WARN_OUT_OF_MEMORY;
      return ret;
#else
      Object * ret = NULL;

      if (_mutex.Lock() == B_NO_ERROR)
      {
         (void) _pool.RemoveTail(ret);
         _mutex.Unlock();
      }

      if (ret == NULL) ret = newnothrow Object;
      if (ret)
      {
         if (_initObjectFunc) _initObjectFunc(ret, _initObjectUserData);
      }
      else WARN_OUT_OF_MEMORY;
      return ret;
#endif
   }

   /** Adds the given object to our "standby" object list to be recycled, or
    *  deletes it if the "standby" list is already at its maximum size.
    *  This method is thread-safe.
    *  @param obj An Object to recycle or delete.  May be NULL.  
    *             This method can be used interchangable with 'delete Object'.
    */
   void ReleaseObject(Object * obj)
   {
#ifdef DISABLE_OBJECT_POOLING
      delete obj;
#else
      if (obj)
      {
         if (_recycleObjectFunc) _recycleObjectFunc(obj, _recycleObjectUserData);
         bool deleteObject = true;
         if (_mutex.Lock() == B_NO_ERROR)
         {
            if ((_pool.GetNumItems() < _maxPoolSize)&&(_pool.AddTail(obj) == B_NO_ERROR)) deleteObject = false;
            _mutex.Unlock();
         }
         if (deleteObject) delete obj;
      }
#endif
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

   /** Removes all "spare" objects from the pool and deletes them. 
     * This method is thread-safe.
     * @returns B_NO_ERROR on success, or B_ERROR if it couldn't lock it's lock for some reason.
     */
   status_t Drain()
   {
      if (_mutex.Lock() == B_NO_ERROR)
      {
         for (int i=_pool.GetNumItems()-1; i>=0; i--) delete _pool[i];
         _pool.Clear();
         (void) _mutex.Unlock();
         return B_NO_ERROR;
      } 
      else return B_ERROR;
   }

private:
   Mutex _mutex;

   ObjectCallback _initObjectFunc;
   void * _initObjectUserData;
   
   ObjectCallback _recycleObjectFunc;
   void * _recycleObjectUserData;
   
   Queue<Object *> _pool;
   uint32 _maxPoolSize;
};

END_NAMESPACE(muscle);

#endif
