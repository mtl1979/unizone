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
	PRINT("GotoURL: url = %S\n", GetBuffer(url));

    if ((long)ShellExecuteW(NULL, NULL, GetBuffer(url), NULL, NULL, showcmd) > 32)
        return true;

    return false;
}
