#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include "user.h"
#include "message/Message.h"
#include "system/Mutex.h"

using namespace muscle;

class QString;

class DownloadQueue
{
public:
	DownloadQueue();
	~DownloadQueue();

	void addItem(const QString & file, const WUserRef & user);
	void run();
private:
	MessageRef fQueue;
	Mutex fLock;
};
#endif
