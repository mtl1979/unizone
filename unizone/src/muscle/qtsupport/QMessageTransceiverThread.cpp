/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "qtsupport/QMessageTransceiverThread.h"

#include <qapplication.h>

namespace muscle {

static const uint32 QMTT_SIGNAL_EVENT = 8360447;  // why yes, this is a completely arbitrary number

QMessageTransceiverThread :: QMessageTransceiverThread(QObject * parent, const char * name) : QObject(parent, name)
{
   if (!name) setName( "QMessageTransceiverThread" );
}

QMessageTransceiverThread :: ~QMessageTransceiverThread()
{
//   ShutdownInternalThread();  // just in case (note this assumes the user isn't going to subclass this class!)
}

status_t QMessageTransceiverThread :: SendMessageToSessions(MessageRef msgRef, const char * optDistPath)       
{
   return MessageTransceiverThread :: SendMessageToSessions(msgRef, optDistPath);
}

void QMessageTransceiverThread :: SignalOwner()
{
   QCustomEvent * evt = newnothrow QCustomEvent(QMTT_SIGNAL_EVENT);
   if (evt) QApplication::postEvent(this, evt);
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
                  BeginMessageBatch();
               }
               MessageReceived(next, sessionID); 
            break;
            case MTT_EVENT_SESSION_ACCEPTED:      SessionAccepted(sessionID, port); break;
            case MTT_EVENT_SESSION_ATTACHED:      SessionAttached(sessionID);       break;
            case MTT_EVENT_SESSION_CONNECTED:     SessionConnected(sessionID);      break;
            case MTT_EVENT_SESSION_DISCONNECTED:  SessionDisconnected(sessionID);   break;
            case MTT_EVENT_SESSION_DETACHED:      SessionDetached(sessionID);       break;
            case MTT_EVENT_FACTORY_ATTACHED:      FactoryAttached(port);            break;
            case MTT_EVENT_FACTORY_DETACHED:      FactoryDetached(port);            break;
            case MTT_EVENT_OUTPUT_QUEUES_DRAINED: OutputQueuesDrained(next);        break;
            case MTT_EVENT_SERVER_EXITED:         ServerExited();                   break;
         }
         InternalThreadEvent(code, next, sessionID, port);  // these get emitted for any event
      }
      if (seenIncomingMessage) EndMessageBatch();
      return true;
   }
   else return QObject::event(event);
}

};  // end namespace muscle
