/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "system/Thread.h"
#include "util/NetworkUtilityFunctions.h"
#include "dataio/TCPSocketDataIO.h"  // to get the proper #includes for recv()'ing

namespace muscle {

Thread :: Thread() : _messageSocketsAllocated(false), _threadRunning(false)
{
#if defined(MUSCLE_USE_PTHREADS)
   // do nothing
//#elif defined(QT_THREAD_SUPPORT)
//   _thread.SetOwner(this);
#endif

   for (uint32 i=0; i<NUM_MESSAGE_THREADS; i++) 
   {
      _messageSockets[i]      = -1;
      _closeMessageSockets[i] = true;
   }
}

Thread :: ~Thread()
{
   MASSERT(IsInternalThreadRunning() == false, "You mustn't delete a Thread object while its internal thread is still running! (i.e. You must call thread.WaitForThreadToExit() before deleting the Thread)");
   CloseSockets();
}

int Thread :: GetInternalThreadWakeupSocket()
{
   return GetThreadWakeupSocketAux(MESSAGE_THREAD_INTERNAL);
}

int Thread :: GetOwnerWakeupSocket()
{
   return GetThreadWakeupSocketAux(MESSAGE_THREAD_OWNER);
}

int Thread :: GetThreadWakeupSocketAux(int whichSocket)
{
   if ((_messageSocketsAllocated == false)&&(CreateConnectedSocketPair(_messageSockets[MESSAGE_THREAD_INTERNAL], _messageSockets[MESSAGE_THREAD_OWNER]) != B_NO_ERROR)) return -1;  
   _messageSocketsAllocated = true;
   return _messageSockets[whichSocket];
}

void Thread :: CloseSockets()
{
   for (uint32 i=0; i<NUM_MESSAGE_THREADS; i++) 
   {
      if (_closeMessageSockets[i]) CloseSocket(_messageSockets[i]); 
      _messageSockets[i]      = -1;
      _closeMessageSockets[i] = true;
   }
   _messageSocketsAllocated = false;
}

status_t Thread :: StartInternalThread()
{
   if (IsInternalThreadRunning() == false)
   {
      bool needsInitialSignal = (_messages[MESSAGE_THREAD_INTERNAL].GetNumItems() > 0);
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

#if defined(MUSCLE_USE_PTHREADS)
      if (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0) return B_NO_ERROR;
#elif defined(QT_THREAD_SUPPORT)
      start();
      return B_NO_ERROR;
#elif defined(__BEOS__)
      if ((_thread = spawn_thread(InternalThreadEntryFunc, "MUSCLE Thread", B_NORMAL_PRIORITY, this)) >= 0)
      {
         if (resume_thread(_thread) == B_NO_ERROR) return B_NO_ERROR;
                                              else kill_thread(_thread);
      }
#elif defined(WIN32)
      DWORD junkThreadID;
      if ((_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InternalThreadEntryFunc, this, 0, &junkThreadID)) != NULL) return B_NO_ERROR;
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
   if (_queueLocks[whichQueue].Lock() == B_NO_ERROR)
   {
      if (_messages[whichQueue].AddTail(replyRef) == B_NO_ERROR) ret = B_NO_ERROR;
      bool sendNotification = (_messages[whichQueue].GetNumItems() == 1);
      (void) _queueLocks[whichQueue].Unlock();
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
      int fd = _messageSockets[whichSocket];
      if (fd >= 0) 
      {
         char junk = 'S';
         (void) send(fd, &junk, sizeof(junk), 0);
      }
   }
}

int32 Thread :: GetNextReplyFromInternalThread(MessageRef & ref, uint64 wakeupTime)
{
   return WaitForNextMessageAux(MESSAGE_THREAD_OWNER, ref, wakeupTime);
}

int32 Thread :: WaitForNextMessageFromOwner(MessageRef & ref, uint64 wakeupTime)
{
   return WaitForNextMessageAux(MESSAGE_THREAD_INTERNAL, ref, wakeupTime);
}

int32 Thread :: WaitForNextMessageAux(int whichThread, MessageRef & ref, uint64 wakeupTime)
{
   int32 ret = -1;  // pessimistic default
   if (_queueLocks[whichThread].Lock() == B_NO_ERROR)
   {
      if (_messages[whichThread].RemoveHead(ref) == B_NO_ERROR) ret = _messages[whichThread].GetNumItems();
      (void) _queueLocks[whichThread].Unlock();
      if (ret < 0)
      {
         int fd = _messageSockets[whichThread];
         if (fd >= 0)
         {
            uint64 now = GetCurrentTime64();
            if (wakeupTime > now)
            {
               // block until either a new-message-signal-byte wakes us, or we reach our wakeup time
               struct timeval timeout;
               if (wakeupTime != MUSCLE_TIME_NEVER) Convert64ToTimeVal(wakeupTime-now, timeout);
               fd_set readSet;
               FD_ZERO(&readSet);
               FD_SET(fd, &readSet);
               if ((select(fd+1, &readSet, NULL, NULL, (wakeupTime == MUSCLE_TIME_NEVER)?NULL:&timeout) >= 0)&&(FD_ISSET(fd, &readSet)))
               {
                  uint8 bytes[256];
                  (void) recv(fd, (char *)bytes, sizeof(bytes), 0);  // just clear them all out, we only need to wake up once...
                  ret = WaitForNextMessageAux(whichThread, ref, wakeupTime); // recurse to get the message
               }
            }
         }
      }
   }
   return ret;
}

void Thread :: InternalThreadEntry()
{
   MessageRef msgRef;
   int32 numLeft;
   while((numLeft = WaitForNextMessageFromOwner(msgRef)) >= 0)
   {
      if (MessageReceivedFromOwner(msgRef, numLeft) == B_NO_ERROR) msgRef.Reset();  // free up the memory now, in case we wait for a long time
                                                              else break;
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
#if defined(MUSCLE_USE_PTHREADS)
      (void) pthread_join(_thread, NULL);
#elif defined(QT_THREAD_SUPPORT)
      (void) wait();
#elif defined(__BEOS__)
      status_t junk;
      (void) wait_for_thread(_thread, &junk);
#elif defined(WIN32)
      (void) WaitForSingleObject(_thread, INFINITE);
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
   return (_queueLocks[MESSAGE_THREAD_INTERNAL].Lock() == B_NO_ERROR) ? &_messages[MESSAGE_THREAD_INTERNAL] : NULL;
}

status_t Thread :: UnlockMessageQueue()
{
   return _queueLocks[MESSAGE_THREAD_INTERNAL].Unlock();
}

Queue<MessageRef> * Thread :: LockAndReturnReplyQueue()
{
   return (_queueLocks[MESSAGE_THREAD_OWNER].Lock() == B_NO_ERROR) ? &_messages[MESSAGE_THREAD_OWNER] : NULL;
}

status_t Thread :: UnlockReplyQueue()
{
   return _queueLocks[MESSAGE_THREAD_OWNER].Unlock();
}

// This method is here to 'wrap' the internal thread's virtual method call with some standard setup/tear-down code of our own
void Thread::InternalThreadEntryAux()
{
   if (_messages[MESSAGE_THREAD_OWNER].GetNumItems() > 0) SignalOwner();
   InternalThreadEntry();
}

void Thread :: SetOkayToCloseOwnerWakeupSocket(bool okayToClose) 
{
   _closeMessageSockets[MESSAGE_THREAD_OWNER] = okayToClose;
}

void Thread :: SetOkayToCloseInternalThreadWakeupSocket(bool okayToClose)
{
   _closeMessageSockets[MESSAGE_THREAD_INTERNAL] = okayToClose;
}


};  // end namespace muscle
