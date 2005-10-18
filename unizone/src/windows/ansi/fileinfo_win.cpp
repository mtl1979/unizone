#include <windows.h>
#include <sys/stat.h>
#pragma warning(disable: 4786)

#include "fileinfo.h"
#include "debugimpl.h"

void
UFileInfo::InitMIMEType()
{
	// Read the mime-type
	HKEY hkey;
	DWORD type;
	char key[MAX_PATH];
	DWORD dsize = MAX_PATH;
	QString mt = QString::null;

	QString ext(".");
	ext += getExtension();
	QCString qcext = ext.local8Bit();
	const char * cext = (const char *) qcext;
	if (RegOpenKeyA(HKEY_CLASSES_ROOT, cext, &hkey) == ERROR_SUCCESS)
	{
		LONG ret;
		if ((ret = RegQueryValueExA(hkey, "Content Type", NULL, &type, (LPBYTE)key, &dsize)) == ERROR_SUCCESS)
		{
			PRINT2("Read key: %s\n", key);
			mt = QString::fromLocal8Bit(key);
		}
		else
		{
			PRINT("Error: %d [0x%08x]\n", ret, ret);
		}
		RegCloseKey(hkey);
	}
	if (mt == QString::null)
	{
		HKEY hk1, hk2, hk3, hk4;
		if (RegOpenKeyA(HKEY_CLASSES_ROOT, "MIME", &hk1) == ERROR_SUCCESS)
		{
			if (RegOpenKeyA(hk1, "Database", &hk2) == ERROR_SUCCESS)
			{
				if (RegOpenKeyA(hk2, "Content Type", &hk3) == ERROR_SUCCESS)
				{
					DWORD numkeys;
					if (RegQueryInfoKeyA(hk3, NULL, NULL, NULL, &numkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
					{
						for (DWORD keys = 0; keys < numkeys; keys++)
						{
							bool found = false;
							char lname[MAX_PATH];
							dsize = MAX_PATH;
							if (RegEnumKeyA(hk3, keys, lname, dsize) == ERROR_SUCCESS)
							{
								if (RegOpenKeyA(hk3, lname, &hk4) == ERROR_SUCCESS)
								{
									char lext[MAX_PATH];
									dsize = MAX_PATH;
									if (RegQueryValueExA(hk4, "Extension", NULL, &type, (LPBYTE) lext, &dsize) == ERROR_SUCCESS)
									{
										if (strcmp(lext, cext) == 0)
										{
											PRINT("Read key: %s\n", lname);
											mt = QString::fromLocal8Bit(lname);
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
		QCString qcFilePath = QFile::encodeName(fFullName);
		const char * cFilePath = (const char *) qcFilePath;
		
		struct _stat fst;
		
		int ret = _stat(cFilePath, &fst);
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

