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
#include "platform.h"		// <postmaster@raasu.org> 20021023 -- Move Platform-dependant code to own files
#include "global.h"

class WLaunchThread : public QThread
{
public:
	WLaunchThread(QString url) : QThread(), fURL(url) {}

#ifdef __linux__	// set the launcher (linux only)
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

bool GotoURL(QString url, int showcmd)
    {
    /* if the ShellExecute() fails          */
    bool retflag = FALSE;

	wchar_t * tUrl = qStringToWideChar(url);
	PRINT("GotoURL: tUrl = %S\n",tUrl);
	// <postmaster@raasu.org> 20021103 -- Use NULL instead of L"open", L"open" doesn't seem to work on WinME
    if((long)ShellExecuteW(NULL, NULL, tUrl, NULL, NULL, showcmd) > 32)
        retflag = TRUE;
	else if ((long)ShellExecuteA(NULL, NULL, url.latin1(), NULL, NULL, showcmd) > 32)
		retflag = TRUE;
	delete [] tUrl;
	tUrl = NULL; // <postmaster@raasu.org> 20021027
    return retflag;
}

#else

bool
GotoURL(QString url, QString browser)
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
	PRINT("Launching %S\n", qStringToWideChar(launch));
	system(launch);
	return true;
}

#endif

void
GotoURL(QString url)
{
	PRINT("GotoURL() called\n");
	QString u = url.lower();
#ifdef WIN32
	if (u.startsWith("audio"))		// <postmaster@raasu.org> 20021116
	{
		url = "mms" + url.mid(5);
	}
#endif
	WLaunchThread * t = new WLaunchThread(url);
	CHECK_PTR(t);
#ifdef __linux__

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
		t->SetURL( u.mid( u.find(":") + 1 ) );
		t->fLauncher = gWin->fSettings->GetMailLauncher();
	}
	else
	{
		t->fLauncher = gWin->fSettings->GetHTTPLauncher();	// unknown? use HTTP launcher...
	}
#endif
	t->start();
}

void
RunCommand(QString command)
{
	PRINT("RunCommand() called\n");
	WLaunchThread * t = new WLaunchThread(command);
	CHECK_PTR(t);
#ifdef __linux__
	t->fLauncher = QString::null;
#endif
	t->start();
}
