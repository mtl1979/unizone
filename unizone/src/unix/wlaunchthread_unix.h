#ifndef WLAUNCHTHREAD_UNIX_H
#define WLAUNCHTHREAD_UNIX_H

#include <qthread.h>
#include <qstring.h>

class WLaunchThread : public QThread
{
public:
	WLaunchThread(const QString & url) : QThread(), fURL(url) {}

	void SetLauncher(const QString & launcher) { fLauncher = launcher; }
	void SetURL(const QString & url) { fURL = url; }

protected:
	virtual void run();

private:
	bool GotoURL(const QString & url, const QString & browser);

	QString fURL;
	QString fLauncher;
};

#endif
