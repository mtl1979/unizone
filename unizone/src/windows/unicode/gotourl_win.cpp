#include <qstring.h>

#include "windows/wlaunchthread_win.h"
#include "gotourl.h"
#include "debugimpl.h"
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
	else if (u.startsWith("audio"))		// <postmaster@raasu.org> 20021116
	{
		address = "mms" + url.mid(5);
	}
	else
	{
		address = url;
	}

	WLaunchThread * t = new WLaunchThread(address);
	CHECK_PTR(t);
	t->start();
}

void
RunCommand(const QString & command)
{
	PRINT("RunCommand() called\n");
	WLaunchThread * t = new WLaunchThread(command);
	CHECK_PTR(t);
	t->start();
}
