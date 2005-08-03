#include <windows.h>
#include <shellapi.h>

#include <qstring.h>

#include "windows/wlaunchthread_win.h"
#include "wstring.h"
#include "debugimpl.h"

void
WLaunchThread::InternalThreadEntry()
{
	GotoURL(fURL, SW_SHOW);
}

bool 
WLaunchThread::GotoURL(const QString & url, int showcmd)
{
#ifdef _DEBUG
	PRINT("GotoURL: tUrl = %S\n", GetBuffer(url));
#endif

	if ((long)ShellExecuteA(NULL, NULL, (const char *) url.local8Bit(), NULL, NULL, showcmd) > 32)
		return true;
    return false;
}

