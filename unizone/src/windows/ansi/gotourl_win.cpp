#include <qstring.h>
#include <qapplication.h>

#include "windows/wlaunchthread_win.h"
#include "gotourl.h"
#include "debugimpl.h"
#include "winsharewindow.h"
#include "settings.h"
#include "wstring.h"
#include "global.h"

WLaunchThread * fLaunchThread = NULL;

void
GotoURL(const QString & url)
{
	PRINT("GotoURL() called\n");
	QString u = url;
	QString address;
	if (startsWith(u, "server://", false))
	{
		address = url.mid(9);
		if (endsWith(address, "/"))
		{
			address.truncate(address.length() - 1);
		}
		gWin->Connect(address);
		return;
	}
	else if (startsWith(u, "audio", false))		// <postmaster@raasu.org> 20021116
	{
		address = "mms" + url.mid(5);
	}
	else
	{
		address = url;
	}

	RunCommand(address);
}

void
RunCommand(const QString & command)
{
	PRINT("RunCommand() called\n");
	QApplication::setOverrideCursor( Qt::waitCursor );
	if (fLaunchThread->IsInternalThreadRunning())
		fLaunchThread->WaitForInternalThreadToExit();
	fLaunchThread->SetURL(command);
	fLaunchThread->StartInternalThread();
	QApplication::restoreOverrideCursor();
}

void
InitLaunchThread()
{
	fLaunchThread = new WLaunchThread();
	CHECK_PTR(fLaunchThread);
}

void
DeinitLaunchThread()
{
	if (fLaunchThread->IsInternalThreadRunning())
		fLaunchThread->WaitForInternalThreadToExit();
}
