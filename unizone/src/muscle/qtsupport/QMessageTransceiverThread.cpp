/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "qtsupport/QMessageTransceiverThread.h"

namespace muscle {

static const uint32 QMTT_SIGNAL_EVENT = QEvent::MaxUser;

QMessageTransceiverThread :: QMessageTransceiverThread(QObject *parent)
	: QObject(parent)
{
   // empty
}

QMessageTransceiverThread :: ~QMessageTransceiverThread()
{
   // empty
}

status_t QMessageTransceiverThread :: SendMessageToSessions(MessageRef msgRef, const char * optDistPath)       
{
   return MessageTransceiverThread :: SendMessageToSessions(msgRef, optDistPath);
}

void QMessageTransceiverThread :: SignalOwner()
{
   QCustomEvent * evt = newnothrow QCustomEvent(QMTT_SIGNAL_EVENT);
   if (evt) QThread::postEvent(this, evt);
       else WARN_OUT_OF_MEMORY;
}

bool QMessageTransceiverThread :: event(QEvent * event)
{
   if (event->type() == QMTT_SIGNAL_EVENT)
   {
      uint32 code;
      MessageRef next;
      String sessionID;
      uint16 port;
      bool seenIncomingMessage = false;

      // Check for any new messages from our internal thread
      while(GetNextEventFromInternalThread(code, &next, &sessionID, &port) >= 0)
      {
         switch(code)
         {
            case MTT_EVENT_INCOMING_MESSAGE:      
               if (seenIncomingMessage == false)
               {
                  seenIncomingMessage = true;
                  emit BeginMessageBatch();
               }
               emit MessageReceived(next, sessionID); 
            break;
            case MTT_EVENT_SESSION_ACCEPTED:      emit SessionAccepted(sessionID, port); break;
            case MTT_EVENT_SESSION_ATTACHED:      emit SessionAttached(sessionID);       break;
            case MTT_EVENT_SESSION_CONNECTED:     emit SessionConnected(sessionID);      break;
            case MTT_EVENT_SESSION_DISCONNECTED:  emit SessionDisconnected(sessionID);   break;
            case MTT_EVENT_SESSION_DETACHED:      emit SessionDetached(sessionID);       break;
            case MTT_EVENT_FACTORY_ATTACHED:      emit FactoryAttached(port);            break;
            case MTT_EVENT_FACTORY_DETACHED:      emit FactoryDetached(port);            break;
            case MTT_EVENT_OUTPUT_QUEUES_DRAINED: emit OutputQueuesDrained(next);        break;
            case MTT_EVENT_SERVER_EXITED:         emit ServerExited();                   break;
         }
         emit InternalThreadEvent(code, next, sessionID, port);  // these get emitted for any event
      }
      if (seenIncomingMessage) emit EndMessageBatch();
      return true;
   }
   else return QObject::event(event);
}

};  // end namespace muscle
