#include "platform.h"
// #include "debugimpl.h"

// #include <qstring.h>

#ifdef WIN32
#include <windows.h>

void
WFlashWindow(HWND fWinHandle)
{
#ifdef BUILD_WIN98	// flash our window
								FLASHWINFO finf;
								finf.cbSize = sizeof(finf);
								finf.hwnd = fWinHandle;
								finf.dwFlags = FLASHW_ALL;
								finf.uCount = 3;
								finf.dwTimeout = 400;
								FlashWindowEx(&finf);
#elif (WINVER < 0x0500)
								FlashWindow(fWinHandle,true);
#else
								FlashWindow(fWinHandle,FLASHW_ALL);
#endif // BUILD_WIN98
}

#endif
