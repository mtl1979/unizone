#ifndef ACCEPT_H
#define ACCEPT_H

#include "system/AcceptSocketsThread.h"
#include "util/SocketHolder.h"
using namespace muscle;

#include <qobject.h>
#include <qevent.h>

#define DEFAULT_LISTEN_PORT 7000
#define LISTEN_PORT_RANGE 100

class WAcceptThreadEvent : public QCustomEvent
{
public:
	enum { Type = 'aTeT' };
	
	WAcceptThreadEvent(SocketHolderRef ref) : QCustomEvent(Type) { fRef = ref; }
	virtual ~WAcceptThreadEvent() {}
	
	SocketHolderRef Get() const { return fRef; }
	
private:
	SocketHolderRef fRef;
};

class WAcceptThread : public AcceptSocketsThread
{
public:
	WAcceptThread(QObject * owner) : AcceptSocketsThread(), fOwner(owner) {}
	virtual ~WAcceptThread() {}
	
protected:
	virtual void SignalOwner();
	
private:
	QObject * fOwner;
	
};

#endif
