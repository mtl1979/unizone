#ifndef RESOLVERTHREAD_H
#define RESOLVERTHREAD_H

#include <qapplication.h>
#include <qobject.h>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

#include "user.h"

#include "util/NetworkUtilityFunctions.h"
#include "util/Queue.h"

using namespace muscle;

class QString;

struct ResolverEntry
{
	QString user;
	bool verbose;
};

class ResolverThread : public QThread
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
#ifdef MUSCLE_AVOID_IPV6
	bool PrintAddressInfo(uint32 address, bool verbose);
#else;
	bool PrintAddressInfo(muscle::ip_address address, bool verbose);
#endif

	mutable Mutex fQueueLock;
	mutable QMutex fWaitLock;
	Queue<ResolverEntry> fQueue;
	bool *fShutdownFlag;
	QWaitCondition cond;
};

#endif
