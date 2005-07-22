#ifndef WLAUNCHTHREAD_WIN_H
#define WLAUNCHTHREAD_WIN_H

#include <qstring.h>
#include "system/Thread.h"

using namespace muscle;

class WLaunchThread : public Thread
{
public:
	WLaunchThread(const QString & url) : Thread(), fURL(url) {}
	WLaunchThread() : Thread() {}

	void SetURL(const QString & url) { fURL = url; }

protected:
	virtual void InternalThreadEntry();

private:
	bool GotoURL(const QString &url, int showcmd);
	QString fURL;
};

#endif
