/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleQMessageTransceiverThread_h
#define MuscleQMessageTransceiverThread_h

#include <qobject.h>
#include <qthread.h>
#include "system/MessageTransceiverThread.h"

namespace muscle {

/**
 *  This is a Qt-specific subclass of MessageTransceiverThread.
 *  It hooks all the standard MessageTransceiverThread events
 *  up to Qt signals, so you can just connect() your QMessageTransceiverThread
 *  to the various slots in your application, instead of having to worry
 *  about event loops and such.  In all other ways it works the
 *  same as any MessageTransceiverThread object.
 */
class QMessageTransceiverThread : public QObject, public MessageTransceiverThread
{
   Q_OBJECT

public:
   /** Constructor.
     * @param parent Passed on to the QObject constructor
     * @param name Passed on to the QObject constructor
     */
   QMessageTransceiverThread(QObject * parent = NULL, const char * name = NULL);

   /** 
    *  Destructor.  This constructor will call ShutdownInternalThread() itself,
    *  so you don't need to call ShutdownInternalThread() explicitly UNLESS you
    *  have subclassed this class and overridden virtual methods that can get
    *  called from the internal thread -- in that case you should call 
    *  ShutdownInternalThread() yourself to avoid potential race conditions between
    *  the internal thread and your own destructor method.
    */
   virtual ~QMessageTransceiverThread();

protected:
   /** Emitted when MessageReceived() is about to be emitted one or more times. */
   virtual void BeginMessageBatch() {}

   /** Emitted when a new Message has been received by one of the sessions being operated by our internal thread.
     * @param msg Reference to the Message that was received.
     * @param sessionID Session ID string of the session that received the message
     */
   virtual void MessageReceived(MessageRef msg, const String & sessionID) {}

   /** Emitted when we are done emitting MessageReceived, for the time being. */
   virtual void EndMessageBatch() {}

   /** Emitted when a new Session object is accepted by one of the factories being operated by our internal thread
     * @param sessionID Session ID string of the newly accepted Session object.
     * @param port Port number that the accepting factory was listening to
     */ 
   virtual void SessionAccepted(const String & sessionID, uint16 port) {}

   /** Emitted when a session object is attached to the internal thread's ReflectServer */
   virtual void SessionAttached(const String & sessionID) {}

   /** Emitted when a session object connects to its remote peer (only used by sessions that were
     * created using AddNewConnectSession())
     * @param sessionID Session ID string of the newly connected Session object.
     */
   virtual void SessionConnected(const String & sessionID) {}

   /** Emitted when a session object is disconnected from its remote peer
     * @param sessionID Session ID string of the newly disconnected Session object.
     */
   virtual void SessionDisconnected(const String & sessionID) {}

   /** Emitted when a session object is removed from the internal thread's ReflectServer 
     * @param sessionID Session ID string of the newly disconnected Session object.
     */
   virtual void SessionDetached(const String & sessionID) {}

   /** Emitted when a factory object is attached to the internal thread's ReflectServer.
     * @param port Port ID that the factory object is attached on.
     */
   virtual void FactoryAttached(uint16 port) {}

   /** Emitted when a factory object is removed from the internal thread's ReflectServer.
     * @param port Port ID that the factory object was removed from.
     */
   virtual void FactoryDetached(uint16 port) {}

   /** Emitted when the thread's internal ReflectServer object exits. */
   virtual void ServerExited() {}

   /** Signal emitted when a TCP connection is completed.  */
   virtual void SessionConnected() {}

   /** Emitted when the output-queues of the sessions specified in a previous call to 
     * RequestOutputQueuesDrainedNotification() have drained.  Note that this signal only 
     * gets emitted once per call to RequestOutputQueuesDrainedNotification();
     * it is not emitted spontaneously.
     * @param ref MessageRef that you previously specified in RequestOutputQueuesDrainedNotification().
     */
   virtual void OutputQueuesDrained(MessageRef ref) {}

   /** This signal is called for all events send by the internal thread.  You can use this
     * to catch custom events that don't have their own signal defined above, or if you want to
     * receive all thread events via a single slot.
     * @param code the MTT_EVENT_* code of the new event.
     * @param optMsg If a Message is relevant, this will contain it; else it's a NULL reference.
     * @param optFromSession If a session ID is relevant, this is the session ID; else it will be "".
     * @param optFromFactory If a factory is relevant, this will be the factory's port number; else it will be zero.
     */
   virtual void InternalThreadEvent(uint32 code, MessageRef optMsg, const String & optFromSession, uint16 optFromfactory) {}

public:
   /**
    * This method is the same as the MessageTransceiverThread::SendMessageToSessions();
    * Enqueues the given message for output by one or more of our attached sessions.
    * @param msgRef a reference to the Message to send out.
    * @return B_NO_ERROR on success, B_ERROR if out of memory.
    */
   status_t SendMessageToSessions(MessageRef msgRef, const char * optDistPath = NULL);

protected:
   /** Overridden to send a QEvent */
   virtual void SignalOwner();

   virtual bool event(QEvent * event);
};

};  // end namespace muscle

#endif
