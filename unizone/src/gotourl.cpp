#ifdef WIN32
# include <windows.h>
# include <shellapi.h>
#else
# include <stdio.h>
# include <stdlib.h>
#endif

#include <string.h>
#include "gotourl.h"

#if !defined(WIN32)
#   define MAX_PATH 260
#endif

#include <qthread.h>

#include "debugimpl.h"
#include "util/String.h"
#include "winsharewindow.h"
#include "settings.h"
#include "wstring.h"
#include "global.h"

class WLaunchThread : public QThread
{
public:
	WLaunchThread(const QString & url) : QThread(), fURL(url) {}

// set the launcher (Linux and FreeBSD only)
#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	|| defined(__QNX__)
	QString fLauncher;
#endif
	void SetURL(const QString & url) { fURL = url; }

protected:
	virtual void run();

private:
	QString fURL;
};

void
WLaunchThread::run()
	{
#ifdef WIN32
		GotoURL(fURL, SW_SHOW);
#else
		GotoURL(fURL, fLauncher);
#endif
	}

#ifdef WIN32

bool GotoURL(const QString & url, int showcmd)
    {
    /* if the ShellExecute() fails          */
    bool retflag = FALSE;

	WString tUrl(url);
	PRINT("GotoURL: tUrl = %S\n",tUrl.getBuffer());

	// <postmaster@raasu.org> 20021103 -- Use NULL instead of L"open", L"open" doesn't seem to work on WinME
    if((long)ShellExecuteW(NULL, NULL, tUrl, NULL, NULL, showcmd) > 32)
        retflag = TRUE;
	else if ((long)ShellExecuteA(NULL, NULL, (const char *) url.local8Bit(), NULL, NULL, showcmd) > 32)
		retflag = TRUE;
    return retflag;
}

#else

bool
GotoURL(const QString & url, const QString & browser)
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
	WString wLaunch(launch);
	PRINT("Launching %S\n", wLaunch.getBuffer());
#endif

	system(launch);
	return true;
}

#endif

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
#ifdef WIN32
	if (u.startsWith("audio"))		// <postmaster@raasu.org> 20021116
	{
		address = "mms" + url.mid(5);
	}
#else // !WIN32
	if (u.startsWith("mailto:"))
	{
		address = url.mid( url.find(":") + 1 );
		if (address.right(1) == "/")
		{
			address.truncate(address.length() - 1);
		}
	}
#endif
	else
	{
		address = url;
	}

	WLaunchThread * t = new WLaunchThread(address);
	CHECK_PTR(t);
#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	|| defined(__QNX__)

	if (u.startsWith("http"))	// also includes 'https'
	{
		t->fLauncher = gWin->fSettings->GetHTTPLauncher();
	}
	else if (u.startsWith("ftp"))
	{
		t->fLauncher = gWin->fSettings->GetFTPLauncher();
	}
	else if (u.startsWith("mailto:"))
	{
		t->fLauncher = gWin->fSettings->GetMailLauncher();
	}
	else
	{
		// unknown? use default launcher...
		t->fLauncher = gWin->fSettings->GetDefaultLauncher();	
	}
#endif
	t->start();
}

void
RunCommand(const QString & command)
{
	PRINT("RunCommand() called\n");
	WLaunchThread * t = new WLaunchThread(command);
	CHECK_PTR(t);
#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	|| defined(__QNX__)
	t->fLauncher = QString::null;
#endif
	t->start();
}
