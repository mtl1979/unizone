/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#if defined(MUSCLE_USE_CLONE)
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
#endif

#include "system/Thread.h"
#include "util/NetworkUtilityFunctions.h"
#include "dataio/TCPSocketDataIO.h"  // to get the proper #includes for recv()'ing

#if defined(WIN32)
# include <process.h>  // for _beginthreadex()
#endif

#ifdef MUSCLE_SINGLE_THREAD_ONLY
# error "You're not allowed use the Thread class if you have the MUSCLE_SINGLE_THREAD_ONLY compiler constant is defined!"
#endif

BEGIN_NAMESPACE(muscle);

Thread :: Thread() : _messageSocketsAllocated(false), _threadRunning(false)
{
#if defined(MUSCLE_USE_CLONE)
   _cloneFlags     = CLONE_FILES|CLONE_VM;
   _cloneStackSize = 32768;
#elif defined(MUSCLE_USE_PTHREADS)
   // do nothing
#elif defined(WIN32)
   // do nothing
#elif defined(QT_THREAD_SUPPORT)
   _thread.SetOwner(this);
#endif
}

Thread :: ~Thread()
{
   MASSERT(IsInternalThreadRunning() == false, "You mustn't delete a Thread object while its internal thread is still running! (i.e. You must call thread.ShutdownInternalThread() or thread.WaitForThreadToExit() before deleting the Thread object)");
   CloseSockets();
}

int Thread :: GetInternalThreadWakeupSocket()
{
   return GetThreadWakeupSocketAux(_threadData[MESSAGE_THREAD_INTERNAL]);
}

int Thread :: GetOwnerWakeupSocket()
{
   return GetThreadWakeupSocketAux(_threadData[MESSAGE_THREAD_OWNER]);
}

int Thread :: GetThreadWakeupSocketAux(ThreadSpecificData & tsd)
{
   if ((_messageSocketsAllocated == false)&&(CreateConnectedSocketPair(_threadData[MESSAGE_THREAD_INTERNAL]._messageSocket, _threadData[MESSAGE_THREAD_OWNER]._messageSocket) != B_NO_ERROR)) return -1;  

   _messageSocketsAllocated = true;
   return tsd._messageSocket;
}

void Thread :: CloseSockets()
{
   for (uint32 i=0; i<NUM_MESSAGE_THREADS; i++) 
   {
      ThreadSpecificData & tsd = _threadData[i];
      if (tsd._closeMessageSocket) CloseSocket(tsd._messageSocket); 
      tsd._messageSocket      = -1;
      tsd._closeMessageSocket = true;
   }
   _messageSocketsAllocated = false;
}

status_t Thread :: StartInternalThread()
{
   if (IsInternalThreadRunning() == false)
   {
      bool needsInitialSignal = (_threadData[MESSAGE_THREAD_INTERNAL]._messages.GetNumItems() > 0);
      status_t ret = StartInternalThreadAux();
      if (ret == B_NO_ERROR)
      {
         if (needsInitialSignal) SignalInternalThread();  // make sure he gets his already-queued messages!
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t Thread :: StartInternalThreadAux()
{
   if ((_messageSocketsAllocated)||(GetInternalThreadWakeupSocket() >= 0))
   {
      _threadRunning = true;  // set this first, to avoid a race condition with the thread's startup...

#if defined(MUSCLE_USE_CLONE)
      if (_cloneStack.SetNumBytes(_cloneStackSize, false) == B_NO_ERROR)
      {
         typedef int (*cloneFunc)(void *);
         _clonePID = clone((cloneFunc)InternalThreadEntryFunc, _cloneStack()+_cloneStackSize, _cloneFlags, this); 
         if (_clonePID >= 0) return B_NO_ERROR;
      }
#elif defined(MUSCLE_USE_PTHREADS)
      if (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0) return B_NO_ERROR;
#elif defined(WIN32)
      typedef unsigned (__stdcall *PTHREAD_START) (void *);
      if ((_thread = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)InternalThreadEntryFunc, this, 0, (unsigned *)&_threadID)) != NULL) return B_NO_ERROR;
#elif defined(QT_THREAD_SUPPORT)
      QWaitCondition waitCondition;
      QMutex mutex; mutex.lock();
      _waitForHandleSet = &waitCondition;  // used as a temporary parameter only
      _waitForHandleMutex = &mutex;  // used as a temporary parameter only
      _thread.start(GetInternalQThreadPriority());
      waitCondition.wait(&mutex);  // wait until the internal thread signal us that it's okay to continue
      mutex.unlock();
      return B_NO_ERROR;
#elif defined(__BEOS__)
      if ((_thread = spawn_thread(InternalThreadEntryFunc, "MUSCLE Thread", B_NORMAL_PRIORITY, this)) >= 0)
      {
         if (resume_thread(_thread) == B_NO_ERROR) return B_NO_ERROR;
                                              else kill_thread(_thread);
      }
#elif defined(__ATHEOS__)
      if ((_thread = spawn_thread("MUSCLE Thread", InternalThreadEntryFunc, NORMAL_PRIORITY, 32767, this)) >= 0)
      {
         if (resume_thread(_thread) == B_NO_ERROR) return B_NO_ERROR;
      }
#endif

      _threadRunning = false;  // oops, nevermind, thread spawn failed
   }
   return B_ERROR;
}

bool Thread :: IsCallerInternalThread() const
{
   if (IsInternalThreadRunning() == false) return false;  // we can't be him if he doesn't exist!

#if defined(MUSCLE_USE_CLONE)
   return (_clonePID == getpid());
#elif defined(MUSCLE_USE_PTHREADS)
   return pthread_equal(pthread_self(), _thread);
#elif defined(WIN32)
   return (_threadID == GetCurrentThreadId());
#elif defined(QT_THREAD_SUPPORT)
   return (QThread::currentThread() == _internalThreadHandle);
#elif defined(__BEOS__)
   return (_thread == find_thread(NULL));
#elif defined(__ATHEOS__)
   return (_thread == find_thread(NULL));
#else
   return false;  // should never get here, but just in case
#endif
}

void Thread :: ShutdownInternalThread(bool waitForThread)
{
   if (IsInternalThreadRunning())
   {
      SendMessageToInternalThread(MessageRef());  // a NULL message ref tells him to quit
      if (waitForThread) WaitForInternalThreadToExit();
   }
}

status_t Thread :: SendMessageToInternalThread(MessageRef ref) 
{
   return SendMessageAux(MESSAGE_THREAD_INTERNAL, ref);
}

status_t Thread :: SendMessageToOwner(MessageRef ref)
{
   return SendMessageAux(MESSAGE_THREAD_OWNER, ref);
}

status_t Thread :: SendMessageAux(int whichQueue, MessageRef replyRef)
{
   status_t ret = B_ERROR;
   ThreadSpecificData & tsd = _threadData[whichQueue];
   if (tsd._queueLock.Lock() == B_NO_ERROR)
   {
      if (tsd._messages.AddTail(replyRef) == B_NO_ERROR) ret = B_NO_ERROR;
      bool sendNotification = (tsd._messages.GetNumItems() == 1);
      (void) tsd._queueLock.Unlock();
      if ((sendNotification)&&(_signalLock.Lock() == B_NO_ERROR))
      {
         switch(whichQueue)
         {
            case MESSAGE_THREAD_INTERNAL: SignalInternalThread(); break;
            case MESSAGE_THREAD_OWNER:    SignalOwner();          break;
         }
         _signalLock.Unlock();
      }
   }
   return ret;
}

void Thread :: SignalInternalThread() 
{
   SignalAux(MESSAGE_THREAD_OWNER);  // we send a byte on the owner's socket and the byte comes out on the internal socket
}

void Thread :: SignalOwner() 
{
   SignalAux(MESSAGE_THREAD_INTERNAL);  // we send a byte on the internal socket and the byte comes out on the owner's socket
}

void Thread :: SignalAux(int whichSocket)
{
   if (_messageSocketsAllocated)
   {
      int fd = _threadData[whichSocket]._messageSocket;
      if (fd >= 0) 
      {
         char junk = 'S';
         (void) send(fd, &junk, sizeof(junk), 0);
      }
   }
}

int32 Thread :: GetNextReplyFromInternalThread(MessageRef & ref, uint64 wakeupTime)
{
   return WaitForNextMessageAux(_threadData[MESSAGE_THREAD_OWNER], ref, wakeupTime);
}

int32 Thread :: WaitForNextMessageFromOwner(MessageRef & ref, uint64 wakeupTime)
{
   return WaitForNextMessageAux(_threadData[MESSAGE_THREAD_INTERNAL], ref, wakeupTime);
}

int32 Thread :: WaitForNextMessageAux(ThreadSpecificData & tsd, MessageRef & ref, uint64 wakeupTime)
{
   int32 ret = -1;  // pessimistic default
   if (tsd._queueLock.Lock() == B_NO_ERROR)
   {
      if (tsd._messages.RemoveHead(ref) == B_NO_ERROR) ret = tsd._messages.GetNumItems();
      (void) tsd._queueLock.Unlock();

      if ((ret < 0)&&(tsd._messageSocket >= 0))  // no Message available?  then we'll have to wait until there is one!
      {
         uint64 now = GetRunTime64();
         if (wakeupTime > now)
         {
            // block until either 
            //   (a) a new-message-signal-byte wakes us, or 
            //   (b) we reach our wakeup/timeout time, or 
            //   (c) a user-specified socket in the socket set selects as ready-for-something
            struct timeval timeout;
            if (wakeupTime != MUSCLE_TIME_NEVER) Convert64ToTimeVal(wakeupTime-now, timeout);

            fd_set sets[NUM_SOCKET_SETS];
            fd_set * psets[NUM_SOCKET_SETS] = {NULL, NULL, NULL};
            int maxfd = tsd._messageSocket;
            {
               for (uint32 i=0; i<ARRAYITEMS(sets); i++)
               {
                  const Hashtable<int, bool> & t = tsd._socketSets[i];
                  if ((i == SOCKET_SET_READ)||(t.GetNumItems() > 0))
                  {
                     psets[i] = &sets[i];
                     FD_ZERO(psets[i]);
                     if (t.GetNumItems() > 0)
                     {
                        HashtableIterator<int, bool> iter = t.GetIterator();
                        int nextSocket;
                        while(iter.GetNextKey(nextSocket) == B_NO_ERROR) 
                        {
                           FD_SET(nextSocket, psets[i]);
                           maxfd = muscleMax(maxfd, nextSocket);
                        }
                     }
                  }
               }
            }
            FD_SET(tsd._messageSocket, psets[SOCKET_SET_READ]);  // this pset is guaranteed to be non-NULL at this point

            if (select(maxfd+1, psets[SOCKET_SET_READ], psets[SOCKET_SET_WRITE], psets[SOCKET_SET_EXCEPTION], (wakeupTime == MUSCLE_TIME_NEVER)?NULL:&timeout) >= 0)
            {
               for (uint32 j=0; j<ARRAYITEMS(psets); j++)
               {
                  Hashtable<int, bool> & t = tsd._socketSets[j];
                  if ((psets[j])&&(t.GetNumItems() > 0))
                  {
                     int nextSocket;
                     bool * nextValue;
                     HashtableIterator<int, bool> iter = t.GetIterator();
                     while(iter.GetNextKeyAndValue(nextSocket, nextValue) == B_NO_ERROR) *nextValue = FD_ISSET(nextSocket, psets[j]) ? true : false;  // ternary operator used to shut VC++ warnings up
                  }
               }

               if (FD_ISSET(tsd._messageSocket, psets[SOCKET_SET_READ]))  // any signals from the other thread?
               {
                  uint8 bytes[256];
                  (void) recv(tsd._messageSocket, (char *)bytes, sizeof(bytes), 0);  // just clear them all out, we only need to wake up once...
                  ret = WaitForNextMessageAux(tsd, ref, wakeupTime); // then recurse to get the message
               }
            }
         }
      }
   }
   return ret;
}

void Thread :: InternalThreadEntry()
{
   while(true)
   {
      MessageRef msgRef;
      int32 numLeft = WaitForNextMessageFromOwner(msgRef);
      if ((numLeft >= 0)&&(MessageReceivedFromOwner(msgRef, numLeft) != B_NO_ERROR)) break;
   } 
}

status_t Thread :: MessageReceivedFromOwner(MessageRef ref, uint32)
{
   return ref() ? B_NO_ERROR : B_ERROR;
}

status_t Thread :: WaitForInternalThreadToExit()
{
   if (_threadRunning)
   {
#if defined(MUSCLE_USE_CLONE)
      (void) waitpid(_clonePID, NULL, __WCLONE); 
#elif defined(MUSCLE_USE_PTHREADS)
      (void) pthread_join(_thread, NULL);
#elif defined(WIN32)
      (void) WaitForSingleObject(_thread, INFINITE);
#elif defined(QT_THREAD_SUPPORT)
      (void) _thread.wait();
#elif defined(__BEOS__)
      status_t junk;
      (void) wait_for_thread(_thread, &junk);
#elif defined(__ATHEOS__)
      (void) wait_for_thread(_thread);
#endif
      _threadRunning = false;
      CloseSockets();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

Queue<MessageRef> * Thread :: LockAndReturnMessageQueue()
{
   ThreadSpecificData & tsd = _threadData[MESSAGE_THREAD_INTERNAL];
   return (tsd._queueLock.Lock() == B_NO_ERROR) ? &tsd._messages : NULL;
}

status_t Thread :: UnlockMessageQueue()
{
   return _threadData[MESSAGE_THREAD_INTERNAL]._queueLock.Unlock();
}

Queue<MessageRef> * Thread :: LockAndReturnReplyQueue()
{
   ThreadSpecificData & tsd = _threadData[MESSAGE_THREAD_OWNER];
   return (tsd._queueLock.Lock() == B_NO_ERROR) ? &tsd._messages : NULL;
}

status_t Thread :: UnlockReplyQueue()
{
   return _threadData[MESSAGE_THREAD_OWNER]._queueLock.Unlock();
}

// This method is here to 'wrap' the internal thread's virtual method call with some standard setup/tear-down code of our own
void Thread::InternalThreadEntryAux()
{
   if (_threadData[MESSAGE_THREAD_OWNER]._messages.GetNumItems() > 0) SignalOwner();
   InternalThreadEntry();
}

void Thread :: SetOkayToCloseOwnerWakeupSocket(bool okayToClose) 
{
   _threadData[MESSAGE_THREAD_OWNER]._closeMessageSocket = okayToClose;
}

void Thread :: SetOkayToCloseInternalThreadWakeupSocket(bool okayToClose)
{
   _threadData[MESSAGE_THREAD_INTERNAL]._closeMessageSocket = okayToClose;
}

END_NAMESPACE(muscle);
