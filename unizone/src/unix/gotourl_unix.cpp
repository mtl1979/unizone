#include <qstring.h>
#include <qthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "gotourl.h"
#include "wstring.h"
#include "debugimpl.h"
#include "winsharewindow.h"
#include "settings.h"
#include "wstring.h"
#include "global.h"

void
GotoURL(const QString & url)
{
	PRINT("GotoURL() called\n");
	QString u = url.lower();
	QString address;
	if (u.startsWith("server://"))
	{
		address = url.mid(9);
		if (address.right(1) == "/")
		{
			address.truncate(address.length() - 1);
		}
		gWin->Connect(address);
		return;
	}
	else if (u.startsWith("mailto:"))
	{
		address = url.mid( url.find(":") + 1 );
		if (address.right(1) == "/")
		{
			address.truncate(address.length() - 1);
		}
	}
	else
	{
		address = url;
	}

	WLaunchThread * t = new WLaunchThread(address);
	CHECK_PTR(t);
	if (u.startsWith("http"))	// also includes 'https'
	{
		t->SetLauncher(gWin->fSettings->GetHTTPLauncher());
	}
	else if (u.startsWith("ftp"))
	{
		t->SetLauncher(gWin->fSettings->GetFTPLauncher());
	}
	else if (u.startsWith("mailto:"))
	{
		t->SetLauncher(gWin->fSettings->GetMailLauncher());
	}
	else
	{
		// unknown? use default launcher...
		t->SetLauncher(gWin->fSettings->GetDefaultLauncher());	
	}
	t->start();
}

void
RunCommand(const QString & command)
{
	PRINT("RunCommand() called\n");
	WLaunchThread * t = new WLaunchThread(command);
	CHECK_PTR(t);
	t->SetLauncher(QString::null);
	t->start();
}
