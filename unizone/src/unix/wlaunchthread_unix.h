#ifndef WLAUNCHTHREAD_UNIX_H
#define WLAUNCHTHREAD_UNIX_H

#include <qstring.h>

#include "system/Thread.h"

using namespace muscle;

class WLaunchThread : public Thread
{
public:
	WLaunchThread(const QString & url) : Thread(), fURL(url) {}
	WLaunchThread() : Thread() {}

	void SetLauncher(const QString & launcher) { fLauncher = launcher; }
	void SetURL(const QString & url) { fURL = url; }

protected:
	virtual void InternalThreadEntry();

private:
	bool GotoURL(const QString & url, const QString & browser);

	QString fURL;
	QString fLauncher;
};

#endif
