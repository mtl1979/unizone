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
	WString wurl(url);
	PRINT("GotoURL: url = %S\n", wurl.getBuffer());

    if ((uintptr_t)ShellExecuteW(NULL, NULL, wurl.getBuffer(), NULL, NULL, showcmd) > 32)
        return true;

    return false;
}
