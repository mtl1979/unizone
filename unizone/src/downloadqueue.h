#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <qapplication.h>

#include "user.h"
#include "message/Message.h"
// #include "system/Mutex.h"

using namespace muscle;

class muscle::Mutex;

class QString;

class DownloadQueue
{
public:
	DownloadQueue();
	~DownloadQueue();

	void addItem(const QString & file, const QString & path, const WUserRef & user);
	void run();
private:
	MessageRef fQueue;
	mutable Mutex fLock;
};
#endif
