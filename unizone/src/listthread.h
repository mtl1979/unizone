#ifndef LISTTHREAD_H
#define LISTTHREAD_H

#include <qapplication.h>
#include <qobject.h>

#include "system/Thread.h"

using namespace muscle;

class NetClient;
class WFileThread;

class WListThread : public QObject, public Thread
{
	Q_OBJECT
public:
	WListThread(NetClient * net, WFileThread * ft, QObject * owner, bool * optShutdownFlag = NULL);

	virtual ~WListThread();

	enum { ListDone = 'lTlD' };

protected:
	virtual void InternalThreadEntry();

	void Lock() { fLocker.Lock(); }
	void Unlock() { fLocker.Unlock(); }

private:
	NetClient * fNet;
	WFileThread * fFileScanThread;
	QObject * fOwner;
	bool * fShutdownFlag;

	mutable Mutex fLocker;

};

#endif
