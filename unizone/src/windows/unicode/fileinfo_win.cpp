#include <windows.h>
#include <wchar.h>

#pragma warning(disable: 4786)

#include "fileinfo.h"
#include "debugimpl.h"
#include "wstring.h"

void
UFileInfo::InitMIMEType()
{
		// Read the mime-type
	HKEY hkey;
	DWORD type;
	wchar_t key[MAX_PATH];
	DWORD dsize = MAX_PATH * 2;
	QString mt = QString::null;

	QString ext(".");
	ext += getExtension();
	WString tExt(ext);
	if (RegOpenKey(HKEY_CLASSES_ROOT, tExt, &hkey) == ERROR_SUCCESS)
	{
		LONG ret;
		if ((ret = RegQueryValueEx(hkey, L"Content Type", NULL, &type, (LPBYTE)key, &dsize)) == ERROR_SUCCESS)
		{
			PRINT2("Read key: %S\n", key);
			mt = wideCharToQString(key);
		}
		else
		{
			PRINT("Error: %d [0x%08x]\n", ret, ret);
		}
		RegCloseKey(hkey);
	}
	fMIMEType = mt;
	return;
}

void
UFileInfo::InitModificationTime()
{
	if ( isValid() )
	{
		// Read the modification time
		WString tFilePath(fFullName);
		
		struct _stat fst;
		
		int ret = -1;
		wchar_t * wFilePath = tFilePath.getBuffer();

		if (wFilePath)
			ret = _wstat(wFilePath, &fst);
		
		if (ret == 0)
		{
			fModificationTime = fst.st_mtime;
		}
		else
		{
			fModificationTime = time(NULL);
		}
	}
	else
		fModificationTime = time(NULL);
	
}
