#include <windows.h>
#include <shellapi.h>

#include <qstring.h>

#include "windows/wlaunchthread_win.h"
#include "wstring.h"
#include "debugimpl.h"

void
WLaunchThread::run()
{
	GotoURL(fURL, SW_SHOW);
}

bool 
WLaunchThread::GotoURL(const QString & url, int showcmd)
{
	WString tUrl(url);
	PRINT("GotoURL: tUrl = %S\n",tUrl.getBuffer());

    if ((long)ShellExecuteW(NULL, NULL, tUrl, NULL, NULL, showcmd) > 32)
        return true;

    return false;
}
