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
   /** Constructor. */
   QMessageTransceiverThread(QObject *parent);

   /** 
    *  Destructor.  You will generally want to call ShutdownInternalThread()
    *  before destroying this object.
    */
   virtual ~QMessageTransceiverThread();

signals:
   /** Emitted when MessageReceived() is about to be emitted one or more times. */
   void BeginMessageBatch();

   /** Emitted when a new Message has been received by one of the sessions being operated by our internal thread.
     * @param msg Reference to the Message that was received.
     * @param sessionID Session ID string of the session that received the message
     */
   void MessageReceived(MessageRef msg, const String & sessionID);

   /** Emitted when we are done emitting MessageReceived, for the time being. */
   void EndMessageBatch();

   /** Emitted when a new Session object is accepted by one of the factories being operated by our internal thread
     * @param sessionID Session ID string of the newly accepted Session object.
     * @param port Port number that the accepting factory was listening to
     */ 
   void SessionAccepted(const String & sessionID, uint16 port);

   /** Emitted when a session object is attached to the internal thread's ReflectServer */
   void SessionAttached(const String & sessionID);

   /** Emitted when a session object connects to its remote peer (only used by sessions that were
     * created using AddNewConnectSession())
     * @param sessionID Session ID string of the newly connected Session object.
     */
   void SessionConnected(const String & sessionID);

   /** Emitted when a session object is disconnected from its remote peer
     * @param sessionID Session ID string of the newly disconnected Session object.
     */
   void SessionDisconnected(const String & sessionID);

   /** Emitted when a session object is removed from the internal thread's ReflectServer 
     * @param sessionID Session ID string of the newly disconnected Session object.
     */
   void SessionDetached(const String & sessionID);

   /** Emitted when a factory object is attached to the internal thread's ReflectServer.
     * @param port Port ID that the factory object is attached on.
     */
   void FactoryAttached(uint16 port);

   /** Emitted when a factory object is removed from the internal thread's ReflectServer.
     * @param port Port ID that the factory object was removed from.
     */
   void FactoryDetached(uint16 port);

   /** Emitted when the thread's internal ReflectServer object exits. */
   void ServerExited();

   /** Signal emitted when a TCP connection is completed.  */
   void SessionConnected();

   /** Emitted when the output-queues of the sessions specified in a previous call to 
     * RequestOutputQueuesDrainedNotification() have drained.  Note that this signal only 
     * gets emitted once per call to RequestOutputQueuesDrainedNotification();
     * it is not emitted spontaneously.
     * @param ref MessageRef that you previously specified in RequestOutputQueuesDrainedNotification().
     */
   void OutputQueuesDrained(MessageRef ref);

   /** This signal is called for all events send by the internal thread.  You can use this
     * to catch custom events that don't have their own signal defined above, or if you want to
     * receive all thread events via a single slot.
     * @param code the MTT_EVENT_* code of the new event.
     * @param optMsg If a Message is relevant, this will contain it; else it's a NULL reference.
     * @param optFromSession If a session ID is relevant, this is the session ID; else it will be "".
     * @param optFromFactory If a factory is relevant, this will be the factory's port number; else it will be zero.
     */
   void InternalThreadEvent(uint32 code, MessageRef optMsg, const String & optFromSession, uint16 optFromfactory);

public slots:
   /**
    * This method is the same as the MessageTransceiverThread::SendMessageToSessions();
    * it's reimplemented here as a pass-through merely so it can be a slot.
    * Enqueues the given message for output by one or more of our attached sessions.
    * @param msgRef a reference to the Message to send out.
    * @return B_NO_ERROR on success, B_ERROR if out of memory.
    */
   status_t SendMessageToSessions(MessageRef msgRef, const char * optDistPath = NULL);

protected:
   /** Overridden to send a QEvent */
   virtual void SignalOwner();

private slots:
   virtual bool event(QEvent * event);
};

};  // end namespace muscle

#endif
