#ifndef ACCEPT_H
#define ACCEPT_H

#include "system/AcceptSocketsThread.h"
#include "util/SocketHolder.h"
using namespace muscle;

#include <qobject.h>
#include <qevent.h>

#define DEFAULT_LISTEN_PORT 7000
#define LISTEN_PORT_RANGE 100

class WAcceptThread : public AcceptSocketsThread
{
public:
	WAcceptThread(QObject * owner) : AcceptSocketsThread(), fOwner(owner) {}
	virtual ~WAcceptThread() {}

	class Event : public QCustomEvent
	{
	public:
		enum { Type = 'aTeT' };

		Event(SocketHolderRef ref) : QCustomEvent(Type) { fRef = ref; }
		virtual ~Event() {}

		SocketHolderRef Get() const { return fRef; }

	private:
		SocketHolderRef fRef;
	};

protected:
	virtual void SignalOwner();

private:
	QObject * fOwner;

};

#endif
