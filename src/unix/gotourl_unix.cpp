#include <qapplication.h>
#include <qstring.h>
#include <stdio.h>
#include <stdlib.h>

#include "wlaunchthread_unix.h"
#include "gotourl.h"
#include "wstring.h"
#include "debugimpl.h"
#include "winsharewindow.h"
#include "settings.h"
#include "wstring.h"
#include "global.h"
#include "util.h"					// for endsWith()

WLaunchThread * fLaunchThread = NULL;

void
GotoURL(const QString & url)
{
	PRINT("GotoURL() called\n");
	QString u = url;
	QString address;
	if (u.startsWith("server://", false))
	{
		address = url.mid(9);
		if (address.endsWith("/"))
		{
			address.truncate(address.length() - 1);
		}
		gWin->Connect(address);
		return;
	}
	else if (u.startsWith("mailto:", false))
	{
		address = url.mid( url.find(":") + 1 );
		if (address.endsWith("/"))
		{
			address.truncate(address.length() - 1);
		}
	}
	else
	{
		address = url;
	}

	QApplication::setOverrideCursor( Qt::waitCursor );
	if (fLaunchThread->IsInternalThreadRunning())
		fLaunchThread->WaitForInternalThreadToExit();
	fLaunchThread->SetURL(address);
	if (u.startsWith("http", false))	// also includes 'https'
	{
		fLaunchThread->SetLauncher(gWin->fSettings->GetHTTPLauncher());
	}
	else if (u.startsWith("ftp", false))
	{
		fLaunchThread->SetLauncher(gWin->fSettings->GetFTPLauncher());
	}
	else if (u.startsWith("mailto:", false))
	{
		fLaunchThread->SetLauncher(gWin->fSettings->GetMailLauncher());
	}
	else
	{
		// unknown? use default launcher...
		fLaunchThread->SetLauncher(gWin->fSettings->GetDefaultLauncher());
	}
	fLaunchThread->StartInternalThread();
	QApplication::restoreOverrideCursor();
}

void
RunCommand(const QString & command)
{
	PRINT("RunCommand() called\n");
	QApplication::setOverrideCursor( Qt::waitCursor );
	if (fLaunchThread->IsInternalThreadRunning())
		fLaunchThread->WaitForInternalThreadToExit();
	fLaunchThread->SetURL(command);
	fLaunchThread->SetLauncher(QString::null);
	fLaunchThread->StartInternalThread();
	QApplication::restoreOverrideCursor();
}

void
InitLaunchThread()
{
	fLaunchThread = new WLaunchThread();
	Q_CHECK_PTR(fLaunchThread);
}

void
DeinitLaunchThread()
{
	if (fLaunchThread->IsInternalThreadRunning())
		fLaunchThread->WaitForInternalThreadToExit();
}
