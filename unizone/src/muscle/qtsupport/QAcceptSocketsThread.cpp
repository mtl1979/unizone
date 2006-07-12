/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#include <qapplication.h>
#include "qtsupport/QAcceptSocketsThread.h"

BEGIN_NAMESPACE(muscle);

static const uint32 QMTT_SIGNAL_EVENT = QEvent::User+14836;  // why yes, this is a completely arbitrary number

QAcceptSocketsThread :: QAcceptSocketsThread(QObject * parent, const char * name) : QObject(parent, name)
{
   if (!name) setName("QAcceptSocketsThread");
}

QAcceptSocketsThread :: ~QAcceptSocketsThread()
{
   ShutdownInternalThread();  // just in case (note this assumes the user isn't going to subclass this class!)
}

void QAcceptSocketsThread :: SignalOwner()
{
   QCustomEvent * evt = newnothrow QCustomEvent(QMTT_SIGNAL_EVENT);
   if (evt) QApplication::postEvent(this, evt);
       else WARN_OUT_OF_MEMORY;
}

bool QAcceptSocketsThread :: event(QEvent * event)
{
   if (event->type() == QMTT_SIGNAL_EVENT)
   {
      MessageRef next;

      // Check for any new messages from our HTTP thread
      while(GetNextReplyFromInternalThread(next) >= 0)
      {
         switch(next()->what)
         {
            case AST_EVENT_NEW_SOCKET_ACCEPTED:      
            {
               GenericRef tag;
               if (next()->FindTag(AST_NAME_SOCKET, tag) == B_NO_ERROR)
               {
                  SocketHolderRef sref(tag, false);
                  if (sref()) emit ConnectionAccepted(sref);
               }
            }
            break;
         }
      }
      return true;
   }
   else return QObject::event(event);
}

END_NAMESPACE(muscle);
