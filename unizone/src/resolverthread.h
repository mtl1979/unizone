#ifndef RESOLVERTHREAD_H
#define RESOLVERTHREAD_H

#include <qapplication.h>
#include <qstring.h>
#include <qobject.h>
#include <qthread.h>

#include "user.h"

#include "util/Queue.h"

using namespace muscle;

struct ResolverEntry
{
	QString user;
	bool verbose;
};

class ResolverThread : public QObject, public QThread 
{
	Q_OBJECT;
public:
	ResolverThread(bool * shutdownflag);
	~ResolverThread();
	void Query(const QString &user, bool verbose);
	void wakeup();
private:
	void run();
	void QueryEntry(const QString &user, bool verbose);
	void PrintAddressInfo(const WUserRef &user, bool verbose);
	bool PrintAddressInfo(uint32 address, bool verbose);

	mutable Mutex fQueueLock;
	Queue<ResolverEntry> fQueue;
	bool *fShutdownFlag;
	QWaitCondition cond;
};

#endif
