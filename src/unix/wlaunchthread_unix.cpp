#include <qstring.h>
#include <stdio.h>
#include <stdlib.h>

#include "wlaunchthread_unix.h"
#include "wstring.h"
#include "debugimpl.h"

void
WLaunchThread::InternalThreadEntry()
{
	GotoURL(fURL, fLauncher);
}

bool
WLaunchThread::GotoURL(const QString & url, const QString & browser)
{
	QString launch;

	if (browser != QString::null)
	{
		launch = browser;
		launch += " ";
	}
	launch += "\"";
	launch += url;
	launch += "\"";

#ifdef _DEBUG
	WString wlaunch(launch);
	PRINT("Launching %S\n", wlaunch.getBuffer());
#endif

	int ret = system(launch);
	return (ret == 0) ? true : false;
}
