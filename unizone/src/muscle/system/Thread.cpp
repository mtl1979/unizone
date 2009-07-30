/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "system/Thread.h"
#include "util/NetworkUtilityFunctions.h"
#include "dataio/TCPSocketDataIO.h"  // to get the proper #includes for recv()'ing

#if defined(MUSCLE_PREFER_WIN32_OVER_QT)
# include <process.h>  // for _beginthreadex()
#endif

#ifdef MUSCLE_SINGLE_THREAD_ONLY
# error "You're not allowed use the Thread class if you have the MUSCLE_SINGLE_THREAD_ONLY compiler constant defined!"
#endif

namespace muscle {

Thread :: Thread() : _messageSocketsAllocated(false), _threadRunning(false)
{
#if defined(MUSCLE_USE_QT_THREADS)
   _thread.SetOwner(this);
#endif
}

Thread :: ~Thread()
{
   MASSERT(IsInternalThreadRunning() == false, "You mustn't delete a Thread object while its internal thread is still running! (i.e. You must call thread.ShutdownInternalThread() or thread.WaitForThreadToExit() before deleting the Thread object)");
   CloseSockets();
}

const ConstSocketRef & Thread :: GetInternalThreadWakeupSocket()
{
   return GetThreadWakeupSocketAux(_threadData[MESSAGE_THREAD_INTERNAL]);
}

const ConstSocketRef & Thread :: GetOwnerWakeupSocket()
{
   return GetThreadWakeupSocketAux(_threadData[MESSAGE_THREAD_OWNER]);
}

const ConstSocketRef & Thread :: GetThreadWakeupSocketAux(ThreadSpecificData & tsd)
{
   if ((_messageSocketsAllocated == false)&&(CreateConnectedSocketPair(_threadData[MESSAGE_THREAD_INTERNAL]._messageSocket, _threadData[MESSAGE_THREAD_OWNER]._messageSocket) != B_NO_ERROR)) return GetNullSocket();

   _messageSocketsAllocated = true;
   return tsd._messageSocket;
}

void Thread :: CloseSockets()
{
   for (uint32 i=0; i<NUM_MESSAGE_THREADS; i++) _threadData[i]._messageSocket.Reset();
   _messageSocketsAllocated = false;
}

status_t Thread :: StartInternalThread()
{
   if (IsInternalThreadRunning() == false)
   {
      bool needsInitialSignal = (_threadData[MESSAGE_THREAD_INTERNAL]._messages.HasItems());
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
   if ((_messageSocketsAllocated)||(GetInternalThreadWakeupSocket()()))
   {
      _threadRunning = true;  // set this first, to avoid a race condition with the thread's startup...

#if defined(MUSCLE_USE_PTHREADS)
      if (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0) return B_NO_ERROR;
#elif defined(MUSCLE_PREFER_WIN32_OVER_QT)
      typedef unsigned (__stdcall *PTHREAD_START) (void *);
      if ((_thread = (::HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)InternalThreadEntryFunc, this, 0, (unsigned *)&_threadID)) != NULL) return B_NO_ERROR;
#elif defined(MUSCLE_USE_QT_THREADS)
# ifdef QT_HAS_THREAD_PRIORITIES
      _thread.start(GetInternalQThreadPriority());
# else
      _thread.start();
# endif
      return B_NO_ERROR;
#elif defined(__BEOS__) || defined(__HAIKU__)
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

void Thread :: ShutdownInternalThread(bool waitForThread)
{
   if (IsInternalThreadRunning())
   {
      SendMessageToInternalThread(MessageRef());  // a NULL message ref tells him to quit
      if (waitForThread) WaitForInternalThreadToExit();
   }
}

status_t Thread :: SendMessageToInternalThread(const MessageRef & ref) 
{
   return SendMessageAux(MESSAGE_THREAD_INTERNAL, ref);
}

status_t Thread :: SendMessageToOwner(const MessageRef & ref)
{
   return SendMessageAux(MESSAGE_THREAD_OWNER, ref);
}

status_t Thread :: SendMessageAux(int whichQueue, const MessageRef & replyRef)
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
      int fd = _threadData[whichSocket]._messageSocket.GetFileDescriptor();
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

      int msgfd;
      if ((ret < 0)&&((msgfd = tsd._messageSocket.GetFileDescriptor()) >= 0))  // no Message available?  then we'll have to wait until there is one!
      {
         uint64 now = GetRunTime64();
         if (wakeupTime < now) wakeupTime = now;

         // block until either 
         //   (a) a new-message-signal-byte wakes us, or 
         //   (b) we reach our wakeup/timeout time, or 
         //   (c) a user-specified socket in the socket set selects as ready-for-something
         struct timeval timeout;
         if (wakeupTime != MUSCLE_TIME_NEVER) Convert64ToTimeVal(wakeupTime-now, timeout);

         fd_set sets[NUM_SOCKET_SETS];
         fd_set * psets[NUM_SOCKET_SETS] = {NULL, NULL, NULL};
         int maxfd = msgfd;
         {
            for (uint32 i=0; i<ARRAYITEMS(sets); i++)
            {
               const Hashtable<ConstSocketRef, bool> & t = tsd._socketSets[i];
               if ((i == SOCKET_SET_READ)||(t.HasItems()))
               {
                  psets[i] = &sets[i];
                  FD_ZERO(psets[i]);
                  if (t.HasItems())
                  {
                     HashtableIterator<ConstSocketRef, bool> iter(t, HTIT_FLAG_NOREGISTER);
                     const ConstSocketRef * nextSocket;
                     while((nextSocket = iter.GetNextKey()) != NULL)
                     {
                        int nextFD = nextSocket->GetFileDescriptor();
                        if (nextFD >= 0)
                        {
                           FD_SET(nextFD, psets[i]);
                           maxfd = muscleMax(maxfd, nextFD);
                        }
                     }
                  }
               }
            }
         }
         FD_SET(msgfd, psets[SOCKET_SET_READ]);  // this pset is guaranteed to be non-NULL at this point

         if (select(maxfd+1, psets[SOCKET_SET_READ], psets[SOCKET_SET_WRITE], psets[SOCKET_SET_EXCEPTION], (wakeupTime == MUSCLE_TIME_NEVER)?NULL:&timeout) >= 0)
         {
            for (uint32 j=0; j<ARRAYITEMS(psets); j++)
            {
               Hashtable<ConstSocketRef, bool> & t = tsd._socketSets[j];
               if ((psets[j])&&(t.HasItems()))
               {
                  const ConstSocketRef * nextSocket;
                  bool * nextValue;
                  HashtableIterator<ConstSocketRef, bool> iter(t, HTIT_FLAG_NOREGISTER);
                  while(iter.GetNextKeyAndValue(nextSocket, nextValue) == B_NO_ERROR) 
                  {
                     int fd = nextSocket->GetFileDescriptor();  // keep separate to avoid warning from g++ 4.3.1 under Ubuntu/64
                     *nextValue = FD_ISSET(fd, psets[j]) ? true : false;  // ternary operator used to shut VC++ warnings up
                  }
               }
            }

            if (FD_ISSET(msgfd, psets[SOCKET_SET_READ]))  // any signals from the other thread?
            {
               uint8 bytes[256];
               if (ConvertReturnValueToMuscleSemantics(recv(msgfd, (char *)bytes, sizeof(bytes), 0), sizeof(bytes), false) > 0) ret = WaitForNextMessageAux(tsd, ref, wakeupTime);
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

status_t Thread :: MessageReceivedFromOwner(const MessageRef & ref, uint32)
{
   return ref() ? B_NO_ERROR : B_ERROR;
}

status_t Thread :: WaitForInternalThreadToExit()
{
   if (_threadRunning)
   {
#if defined(MUSCLE_USE_PTHREADS)
      (void) pthread_join(_thread, NULL);
#elif defined(MUSCLE_PREFER_WIN32_OVER_QT)
      (void) WaitForSingleObject(_thread, INFINITE);
      ::CloseHandle(_thread);  // Raymond Dahlberg's fix for handle-leak problem
#elif defined(MUSCLE_QT_HAS_THREADS)
      (void) _thread.wait();
#elif defined(__BEOS__) || defined(__HAIKU__)
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

Hashtable<Thread::muscle_thread_key, Thread *> Thread::_curThreads;
Mutex Thread::_curThreadsMutex;

Thread * Thread :: GetCurrentThread()
{
   muscle_thread_key key = GetCurrentThreadKey();

   Thread * ret = NULL; 
   if (_curThreadsMutex.Lock() == B_NO_ERROR)
   {
      (void) _curThreads.Get(key, ret);
      _curThreadsMutex.Unlock();
   }
   return ret;
}

// This method is here to 'wrap' the internal thread's virtual method call with some standard setup/tear-down code of our own
void Thread::InternalThreadEntryAux()
{
   muscle_thread_key curThreadKey = GetCurrentThreadKey();
   if (_curThreadsMutex.Lock() == B_NO_ERROR)
   {
      (void) _curThreads.Put(curThreadKey, this);
      _curThreadsMutex.Unlock();
   }

   if (_threadData[MESSAGE_THREAD_OWNER]._messages.HasItems()) SignalOwner();
   InternalThreadEntry();
   _threadData[MESSAGE_THREAD_INTERNAL]._messageSocket.Reset();  // this will wake up the owner thread with EOF on socket

   if (_curThreadsMutex.Lock() == B_NO_ERROR)
   {
      (void) _curThreads.Remove(curThreadKey);
      _curThreadsMutex.Unlock();
   }
}

Thread::muscle_thread_key Thread :: GetCurrentThreadKey()
{
#if defined(MUSCLE_USE_PTHREADS)
   return pthread_self();
#elif defined(MUSCLE_PREFER_WIN32_OVER_QT)
   return GetCurrentThreadId();
#elif defined(MUSCLE_QT_HAS_THREADS)
   return QThread::currentThread();
#elif defined(__BEOS__) || defined(__HAIKU__) || defined(__ATHEOS__)
   return find_thread(NULL);
#else
   #error "Thread::GetCurrentThreadKey():  Unsupported platform?"
#endif
}

bool Thread :: IsCallerInternalThread() const
{
   if (IsInternalThreadRunning() == false) return false;  // we can't be him if he doesn't exist!

#if defined(MUSCLE_USE_PTHREADS)
   return pthread_equal(pthread_self(), _thread);
#elif defined(MUSCLE_PREFER_WIN32_OVER_QT)
   return (_threadID == GetCurrentThreadId());
#elif defined(MUSCLE_QT_HAS_THREADS)
   return (QThread::currentThread() == static_cast<const QThread *>(&_thread));
#elif defined(__BEOS__) || defined(__HAIKU__)
   return (_thread == find_thread(NULL));
#elif defined(__ATHEOS__)
   return (_thread == find_thread(NULL));
#else
   #error "Thread::IsCallerInternalThread():  Unsupported platform?"
#endif
}

}; // end namespace muscle
