/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#ifndef MuscleThread_h
#define MuscleThread_h

#if defined(MUSCLE_USE_PTHREADS)
# include <pthread.h>
#elif defined(QT_THREAD_SUPPORT)
# include <qthread.h>
   class MuscleQThread : public QThread
   {
   public:
      MuscleQThread()  {/* empty */}  // _owner not set here, for VC++6 compatibility
      virtual void run() {InternalThreadEntryAux();}
   protected:
	  virtual void InternalThreadEntryAux() = 0;
   };
#elif defined(__BEOS__)
# include <kernel/OS.h>
#elif defined(WIN32)
# include <windows.h>
#elif defined(__ATHEOS__)
# include <atheos/threads.h>
#else
# error "Thread:  threading support not implemented for this platform.  You'll need to add support for your platform to the MUSCLE Lock and Thread classes for your OS before you can use the Thread class here."
#endif

#include "system/Mutex.h"
#include "message/Message.h"
#include "util/Queue.h"

namespace muscle {

/** This class is an platform-independent class that creates an internally held thread and executes it.
  * You will want to subclass Thread in order to specify the behaviour of the internally held thread...
  * The default thread implementation doesn't do anything very useful.
  * It also includes support for sending Messages to the thread, receiving reply Messages from the thread,
  * and for waiting for the thread to exit.
  */
class Thread
#ifdef QT_THREAD_SUPPORT
: public MuscleQThread
#endif
{
public:
   /** Constructor.  Does very little (in particular, the internal thread is not started here...
     * that happens when you call StartInternalThread())
     */
   Thread();

   /** Destructor.  You must have made sure that the internal thread is no longer running before
     * deleting the Thread object, or an assertion failure will occur.  (You should make sure the
     * internal thread is gone by calling WaitForInternalThreadToExit() before deleting this Thread object)
     */
   virtual ~Thread();

   /** Start the internal thread running
     * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory, or thread is already running)
     */
   virtual status_t StartInternalThread();

   /** Returns true iff the thread is considered to be running.
     * (Note that the thread is considered running from the time StartInternalThread() returns B_NO_ERROR
     * until the time WaitForInternalThreadToExit() is called and returns B_NO_ERROR.  Even if the thread
     * terminates itself before then, it is still considered to be 'running' as far as we're concerned)
     */
   bool IsInternalThreadRunning() const {return _threadRunning;}

   /** Tells the internal thread to quit by sending it a NULL MessageRef, and then optionally 
     * waits for it to go away by calling WaitForInternalThreadToExit().  
     * If the internal thread isn't running, this method is a no-op.
     * You must call this before deleting the MessageTransceiverThread object!
     * @param waitForThread if true, this method won't return until the thread is gone.  Defaults to true.
     *                      (if you set this to false, you'll need to also call WaitForThreadToExit() before deleting this object)
     */
   virtual void ShutdownInternalThread(bool waitForThread = true);

   /** Blocks and won't return until after the internal thread exits.  If you have called
     * StartInternalThread(), you'll need to call this method before deleting this Thread object
     * or calling StartInternalThread() again--even if your thread has already terminated itself!
     * @returns B_NO_ERROR on success, or B_ERROR if the internal thread wasn't running.
     */
   status_t WaitForInternalThreadToExit();

   /** Puts the given message into a message queue for the internal thread to pick up,
     * and then calls SignalInternalThread() (if necessary) to signal the internal thread that a
     * new message is ready.  If the internal thread isn't currently running, then the 
     * MessageRef will be queued up and available to the internal thread to process when it is started.
     * @param msg Reference to the message that is to be given to the internal thread. 
     * @return B_NO_ERROR on success, or B_ERROR on failure (out of memory)
     */
   virtual status_t SendMessageToInternalThread(MessageRef msg);

   /** This method attempts to retrieve the next reply message that has been
     * sent back to the main thread by the internal thread (via SendMessageToOwner()).
     * @param ref On success, (ref) will be a reference to the new reply message.
     * @param wakeupTime Time at which this method should stop blocking and return,
     *                   even if there is no new reply message ready.  If this value is
     *                   0 (the default) or otherwise less than the current time 
     *                   (as returned by GetRunTime64()), then this method does a 
     *                   non-blocking poll of the reply queue.
     *                   If (wakeuptime) is set to MUSCLE_TIME_NEVER, then this method 
     *                   will block indefinitely, until a new reply is ready.
     * @returns The number of Messages left in the reply queue on success, or -1 on failure
     *          (The call timed out without any replies ever showing up)
     */
   virtual int32 GetNextReplyFromInternalThread(MessageRef & ref, uint64 wakeupTime = 0);

   /** Locks the internal thread's message queue and returns a pointer to it.  
     * Since the queue is locked, you may examine or modify the queue safely.
     * Once this method has returned successfully, you are responsible for unlocking the
     * message queue again by calling UnlockMessageQueue().  If you don't, the Thread will
     * remain locked and stuck!
     * @returns a pointer to our internal Message queue, on success, or NULL on failure (couldn't lock)
     */
   Queue<MessageRef> * LockAndReturnMessageQueue();

   /** Unlocks our internal message queue, so that the internal thread can again pop messages off of it.
     * Should be called exactly once after each successful call to LockAndReturnMessageQueue().
     * After this call returns, it is no longer safe to use the pointer that was
     * previously returned by LockAndReturnMessageQueue().
     * @returns B_NO_ERROR on success, or B_ERROR if the unlock call failed (perhaps it wasn't locked?)
     */
   status_t UnlockMessageQueue();

   /** Locks this Thread's reply queue and returns a pointer to it.  Since the queue is
     * locked, you may examine or modify the queue safely.
     * Once this method has returned successfully, you are responsible for unlocking the
     * message queue again by calling UnlockReplyQueue().  If you don't, the Thread will
     * remain locked and stuck!
     * @returns a pointer to our internal reply queue on success, or NULL on failure (couldn't lock)
     */
   Queue<MessageRef> * LockAndReturnReplyQueue();

   /** Unlocks the reply message queue, so that the internal thread can again append messages to it.
     * Should be called exactly once after each successful call to LockAndReturnReplyQueue().
     * After this call returns, it is no longer safe to use the pointer that was
     * previously returned by LockAndReturnReplyQueue().
     * @returns B_NO_ERROR on success, or B_ERROR if the unlock call failed (perhaps it wasn't locked?)
     */
   status_t UnlockReplyQueue();

   /** Returns the socket that the main thread may select() for read on for wakeup-notification bytes. 
     * This Thread object's thread-signalling sockets will be allocated by this method if they aren't already allocated.
     */
   int GetOwnerWakeupSocket();

   /** Call this to tell the Thread object whether it should call CloseSocket() on its owner-wakeup-socket
     * when it is done with it.  Default state is that yes, it will call CloseSocket().
     * The okay-to-close state will be automatically set back to true after the next call to CloseSockets().
     * @param okayToClose New state for whether the Thread should call CloseSocket() to clean up the owner-wakeup-socket.
     */
   void SetOkayToCloseOwnerWakeupSocket(bool okayToClose);

protected:
   /** If you are using the default implementation of InternalThreadEntry(), then this
     * method will be called whenever a new MessageRef is received by the internal thread.
     * Default implementation does nothing, and returns B_NO_ERROR if (msgRef) is valid,
     * or B_ERROR if (msgRef) is a NULL reference.
     * @param msgRef Reference to the just-received Message object.
     * @param numLeft Number of Messages still left in the owner's message queue.
     * @return B_NO_ERROR if you wish to continue processing, or B_ERROR if you wish to
     *                    terminate the internal thread and go away.
     */
   virtual status_t MessageReceivedFromOwner(MessageRef msgRef, uint32 numLeft);

   /** May be called by the internal thread to send a Message back to the owning thread.
     * Puts the given MessageRef into the replies queue, and then calls SignalOwner()
     * (if necessary) to notify the main thread that replies are pending.
     * @param replyRef MessageRef to send back to the owning thread.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory?)
     */
   status_t SendMessageToOwner(MessageRef replyRef);

   /** You may override this method to be your Thread's execution entry point.  
     * Default implementation runs in a loop calling WaitForNextMessageFromOwner() and
     * then MessageReceivedFromOwner().  In many cases, that is all you need, so you may
     * not need to override this method.
     */
   virtual void InternalThreadEntry();
 
   /** This method is meant to be called by the internally held thread.
     * It will attempt retrieve the next message that has been sent to the 
     * thread via SendMessageToThread().
     * @param ref On success, (ref) will be set to be a reference to the retrieved Message.
     * @param wakeupTime Time at which this method should stop blocking and return,
     *                   even if there is no new message ready.  If this value is
     *                   0 or otherwise less than the current time (as returned by GetRunTime64()),
     *                   then this method does a non-blocking poll of the queue.
     *                   If (wakeuptime) is set to MUSCLE_TIME_NEVER (the default value),
     *                   then this method will block indefinitely, until a Message is ready.
     * @returns The number of Messages left in the message queue on success, or -1 on failure
     *          (call timed out with no Messages ever showing up)
     */
   virtual int32 WaitForNextMessageFromOwner(MessageRef & ref, uint64 wakeupTime = MUSCLE_TIME_NEVER);

   /** Called by SendMessageToThread() whenever there is a need to wake up the internal
     * thread so that it will look at its reply queue.
     * Default implementation sends a byte on a socket to implement this,
     * but you can override this method to do it a different way if you need to.
     */
   virtual void SignalInternalThread();

   /** Called by SendMessageToOwner() whenever there is a need to wake up the owning
     * thread so that it will look at its reply queue.  Default implementation sends
     * a byte to the main-thread-listen socket, but you can override this method to
     * do it different way if you need to.
     */
   virtual void SignalOwner();

   /** Returns the socket that the internal thread may select() for read on for wakeup-notification bytes.
     * This Thread object's thread-signalling sockets will be allocated by this method if they aren't already allocated.
     * @returns The socket fd that the thread is to listen on, or -1 on error.
     */
   int GetInternalThreadWakeupSocket();

   /** Call this to tell the Thread object whether it should call CloseSocket() on its internal-thread-wakeup-socket
     * when it is done with it.  Default state is that yes, it will call CloseSocket().
     * The okay-to-close state will be automatically set back to true after the next call to CloseSockets().
     * @param okayToClose New state for whether the Thread should call CloseSocket() to clean up the internal-thread-wakeup-socket.
     */
   void SetOkayToCloseInternalThreadWakeupSocket(bool okayToClose);

   /** Locks the lock we use to serialize calls to SignalInternalThread() and
     * SignalOwner().  Be sure to call UnlockSignallingLock() when you are done with the lock.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (couldn't lock)
     */
   status_t LockSignalling() {return _signalLock.Lock();}

   /** Unlocks the lock we use to serialize calls to SignalInternalThread() and SignalOwner().  
     * @returns B_NO_ERROR on success, or B_ERROR on failure (couldn't unlock)
     */
   status_t UnlockSignalling() {return _signalLock.Unlock();}

   /** Closes both threading sockets, if they are open. 
     * If SetOkayToClose*WakeupSocket(false) was previously called on one or both of the sockets,
     * those sockets will not be closed; merely forgotten.  However, calling this method causes
     * the okay-to-close states to return to true for both sockets types.
     */
   void CloseSockets();

private:
   status_t StartInternalThreadAux();
   int GetThreadWakeupSocketAux(int whichSocket);
   int32 WaitForNextMessageAux(int whichQueue, MessageRef & ref, uint64 wakeupTime = MUSCLE_TIME_NEVER);
   status_t SendMessageAux(int whichQueue, MessageRef ref);
   void SignalAux(int whichSocket);
   void InternalThreadEntryAux();

   enum {
      MESSAGE_THREAD_INTERNAL = 0,  // internal thread's (input queue, socket to block on)
      MESSAGE_THREAD_OWNER,         // main thread's (input queue, socket to block on)
      NUM_MESSAGE_THREADS
   };

   bool _messageSocketsAllocated;
   Mutex _queueLocks[NUM_MESSAGE_THREADS];
   int _messageSockets[NUM_MESSAGE_THREADS];
   bool _closeMessageSockets[NUM_MESSAGE_THREADS];
   Queue<MessageRef> _messages[NUM_MESSAGE_THREADS];
   bool _threadRunning;
   Mutex _signalLock;

#if defined(MUSCLE_USE_PTHREADS)
   pthread_t _thread;
   static void * InternalThreadEntryFunc(void * This) {((Thread *)This)->InternalThreadEntryAux(); return NULL;}
#elif defined(__BEOS__)
   thread_id _thread;
   static int32 InternalThreadEntryFunc(void * This) {((Thread *)This)->InternalThreadEntryAux(); return 0;}
#elif defined(QT_THREAD_SUPPORT)
   // empty
#elif defined(WIN32)
   HANDLE _thread;
   static DWORD WINAPI InternalThreadEntryFunc(LPVOID This) {((Thread*)This)->InternalThreadEntryAux(); return 0;}
#elif defined(__ATHEOS__)
   thread_id _thread;
   static void InternalThreadEntryFunc(void * This) {((Thread *)This)->InternalThreadEntryAux();}
#endif
};

};  // end namespace muscle

#endif
