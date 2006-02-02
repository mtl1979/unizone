#include <windows.h>
#include <wchar.h>

#pragma warning(disable: 4786)

#include "fileinfo.h"
#include "debugimpl.h"
#include "wstring.h"
#include "wutil.h"

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
			PRINT2("\tRead key: %S\n", key);
			mt = wideCharToQString(key);
		}
		else
		{
			PRINT("\tFailed reading key: %S\\Content Type\n", tExt);
			PRINT("\t\tError: %d [0x%08x]\n", ret, ret);
		}
		RegCloseKey(hkey);
	}
	if (mt == QString::null)
	{
		HKEY hk1, hk2, hk3, hk4;
		if (RegOpenKey(HKEY_CLASSES_ROOT, L"MIME", &hk1) == ERROR_SUCCESS)
		{
			if (RegOpenKey(hk1, L"Database", &hk2) == ERROR_SUCCESS)
			{
				if (RegOpenKey(hk2, L"Content Type", &hk3) == ERROR_SUCCESS)
				{
					DWORD numkeys;
					if (RegQueryInfoKey(hk3, NULL, NULL, NULL, &numkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
					{
						for (DWORD keys = 0; keys < numkeys; keys++)
						{
							bool found = false;
							wchar_t lname[MAX_PATH];
							dsize = MAX_PATH * 2;
							if (RegEnumKey(hk3, keys, lname, dsize) == ERROR_SUCCESS)
							{
								if (RegOpenKey(hk3, lname, &hk4) == ERROR_SUCCESS)
								{
									wchar_t lext[MAX_PATH];
									dsize = MAX_PATH * 2;
									if (RegQueryValueEx(hk4, L"Extension", NULL, &type, (LPBYTE) lext, &dsize) == ERROR_SUCCESS)
									{
										if (wcscmp(lext, tExt) == 0)
										{
											PRINT2("\tRead key: %S\n", lname);
											mt = wideCharToQString(lname);
											found = true;
										}
									}
									RegCloseKey(hk4);
								}
							}
							if (found)
								break;
						}
					}
					RegCloseKey(hk3);
				}
				RegCloseKey(hk2);
			}
			RegCloseKey(hk1);
		}
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
