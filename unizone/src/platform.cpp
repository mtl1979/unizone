#include "platform.h"
#include "debugimpl.h"

#include <qstring.h>

#ifdef WIN32
#include <windows.h>

long 
GetRegKey( HKEY key, wchar_t *subkey, wchar_t *retdata, wchar_t value )
{
    long retval;
    HKEY hkey;

    retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if(retval == ERROR_SUCCESS)
        {
        long datasize = MAX_PATH;
        wchar_t data[MAX_PATH];

        if (RegQueryValue(hkey, (wchar_t *)value,(wchar_t *)data,&datasize) == ERROR_SUCCESS)
	        lstrcpy(retdata,data);
    
		RegCloseKey(hkey);
        }

    return retval;
}

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

