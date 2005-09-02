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

