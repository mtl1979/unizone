#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#endif

#include <qfileinfo.h>
#include <qstring.h>

#include "fileinfo.h"
#include "platform.h"
#include "debugimpl.h"

UFileInfo::UFileInfo(QFileInfo info)
{
	fInfo = new QFileInfo(info);
	CHECK_PTR(fInfo);
}

UFileInfo::UFileInfo(QString file)
{
	fInfo = new QFileInfo(file);
	CHECK_PTR(fInfo);
}

UFileInfo::~UFileInfo()
{
	if (fInfo)
		delete fInfo;
}

QString
UFileInfo::getMIMEType() const
{
	if (!fInfo)
		return QString::null;

#ifdef WIN32
	// Read the mime-type
	HKEY hkey;
	DWORD type;
	char key[MAX_PATH];
	DWORD dsize = MAX_PATH;
	QString mt = QString::null;

	QString ext = ".";
	ext += fInfo->extension();
	wchar_t * tExt = qStringToWideChar(ext);
	if (RegOpenKey(HKEY_CLASSES_ROOT, tExt, &hkey) == ERROR_SUCCESS)
	{
		LONG ret;
		// <postmaster@raasu.org> -- Don't use Unicode (wide-char) functions for char[] and char *
		if ((ret = RegQueryValueExA(hkey, "Content Type", NULL, &type, (LPBYTE)key, &dsize)) == ERROR_SUCCESS)
		{
			PRINT("Read key: %s\n", key);
			mt = key;
		}
		else
		{
			PRINT("Error: %d [0x%08x]\n", ret, ret);
		}
		RegCloseKey(hkey);
	}
	delete [] tExt;
	tExt = NULL; // <postmaster@raasu.org> 20021027
	return mt;
#else
	return QString::null;
#endif

}

uint32
UFileInfo::getModificationTime()
{
	if (!fInfo)
		return time(NULL);

#ifdef WIN32
			// Read the modification time
		// The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
		FILETIME ftime;
		// The time functions included in the C run-time use the time_t type to represent the number of seconds elapsed since midnight, January 1, 1970.
		uint32 mtTime; // <postmaster@raasu.org> 20021230
		uint64 ftTime; // 
		HANDLE fileHandle;
		wchar_t * tFilePath = qStringToWideChar( fInfo->filePath() );
		fileHandle = CreateFile(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		delete [] tFilePath;
		tFilePath = NULL; // <postmaster@raasu.org> 20021027
		if (fileHandle)
		{
			PRINT("File opened!\n");
			if (GetFileTime(fileHandle, NULL, NULL, &ftime))
			{
				ftTime = (uint64)
					(
					(((uint64)ftime.dwHighDateTime) << 32) + 
					((uint64)ftime.dwLowDateTime) 
					);
				ftTime -= 116444736000000000; 
				// = 11644473600 seconds
				// = 134774 days
				// = 369 years + 89 (leap) days
				mtTime = (uint32)(ftTime  / 10000000);	// converted to seconds
				PRINT("Got time: %d\n", mtTime);
			}
			else
				mtTime = time(NULL);

			CloseHandle(fileHandle);
			return mtTime;
		}
		else
			return time(NULL);

#else
		return time(NULL);
#endif

}

QString
UFileInfo::getPath() const
{
	if (!fInfo)
		return QString::null;
	return fInfo->dirPath(true);
}

QString
UFileInfo::getName() const
{
	if (!fInfo)
		return QString::null;
	return fInfo->fileName();
}

uint64
UFileInfo::getSize()
{
	if (!fInfo)
		return -1;
	return fInfo->size();
}

bool
UFileInfo::isValid()
{
	if (!fInfo)				// no valid object
		return false;
	if (!fInfo->exists())	// non-existent file
		return false;
	return true;
}
