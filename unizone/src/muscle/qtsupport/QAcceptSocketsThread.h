/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleQAcceptSocketsThread_h
#define MuscleQAcceptSocketsThread_h

#include <qobject.h>
#include <qthread.h>
#include "system/AcceptSocketsThread.h"
#include "util/SocketHolder.h"

namespace muscle {

/**
 *  This is a Qt-specified subclass of AcceptSocketsThread.
 *  It will listen on a port, and emit a ConnectionAccepted signal 
 *  whenever a new TCP connection is received on that port.  In all 
 *  other respects it works like an AcceptSocketsThread object.
 */
class QAcceptSocketsThread : public QObject, public AcceptSocketsThread
{
   Q_OBJECT

public:
   /** Constructor. */
   QAcceptSocketsThread();

   /** 
    *  Destructor.  You will generally want to call ShutdownInternalThread()
    *  before destroying this object.
    */
   virtual ~QAcceptSocketsThread();

signals:
   /** Emitted when a new TCP connection is accepted 
     * @param socketRef Reference to the newly accepted socket.  You may assume ownership of
     *                  the socket if you wish, or not.
     */
   void ConnectionAccepted(SocketHolderRef socketRef);

protected:
   /** Overridden to send a QEvent */
   virtual void SignalOwner();

private slots:
   virtual bool event(QEvent * event);
};

};  // end namespace muscle

#endif
