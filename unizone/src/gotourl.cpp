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
#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	
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

	WString tUrl = url;
	PRINT("GotoURL: tUrl = %S\n",tUrl.getBuffer());
	// <postmaster@raasu.org> 20021103 -- Use NULL instead of L"open", L"open" doesn't seem to work on WinME
    if((long)ShellExecuteW(NULL, NULL, tUrl, NULL, NULL, showcmd) > 32)
        retflag = TRUE;
	else if ((long)ShellExecuteA(NULL, NULL, url.latin1(), NULL, NULL, showcmd) > 32)
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

	WString wLaunch = launch;
	PRINT("Launching %S\n", wLaunch.getBuffer());

	system(launch);
	return true;
}

#endif

void
GotoURL(const QString & url)
{
	PRINT("GotoURL() called\n");
	QString u = url.lower();
	if (u.startsWith("server://"))
	{
		u = u.mid(9);
		if (u.right(1) == "/")
		{
			u.truncate(u.length() - 1);
		}
		gWin->Connect(u);
		return;
	}
#ifdef WIN32
	if (u.startsWith("audio"))		// <postmaster@raasu.org> 20021116
	{
		u = "mms" + url.mid(5);
	}
#endif
	WLaunchThread * t = new WLaunchThread(u);
	CHECK_PTR(t);
#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	

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
		u = u.mid( u.find(":") + 1 );
		if (u.right(1) == "/")
		{
			u.truncate(u.length() - 1);
		}
		t->SetURL( u );
		t->fLauncher = gWin->fSettings->GetMailLauncher();
	}
	else
	{
		t->fLauncher = gWin->fSettings->GetDefaultLauncher();	// unknown? use default launcher...
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
#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	
	t->fLauncher = QString::null;
#endif
	t->start();
}
