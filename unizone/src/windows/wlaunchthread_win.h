#ifndef WLAUNCHTHREAD_WIN_H
#define WLAUNCHTHREAD_WIN_H

#include <qthread.h>
#include <qstring.h>

class WLaunchThread : public QThread
{
public:
	WLaunchThread(const QString & url) : QThread(), fURL(url) {}
	WLaunchThread() : QThread() {}

	void SetURL(const QString & url) { fURL = url; }

protected:
	virtual void run();

private:
	bool GotoURL(const QString &url, int showcmd);
	QString fURL;
};

#endif
