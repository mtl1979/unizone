#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <qfileinfo.h>
#include <qstring.h>

#include "fileinfo.h"
#include "wstring.h"
#include "debugimpl.h"

UFileInfo::UFileInfo(QFileInfo info) : QFileInfo(info)
{
}

UFileInfo::UFileInfo(QString file) : QFileInfo(file)
{
}

UFileInfo::~UFileInfo()
{
}

void
UFileInfo::InitMIMEType()
{
#ifdef WIN32
	// Read the mime-type
	HKEY hkey;
	DWORD type;
	char key[MAX_PATH];
	DWORD dsize = MAX_PATH;
	QString mt = QString::null;

	QString ext = ".";
	ext += getExtension();
	WString tExt = ext;
	if (RegOpenKey(HKEY_CLASSES_ROOT, tExt, &hkey) == ERROR_SUCCESS)
	{
		LONG ret;
		// <postmaster@raasu.org> -- Don't use Unicode (wide-char) functions for char[] and char *
		if ((ret = RegQueryValueExA(hkey, "Content Type", NULL, &type, (LPBYTE)key, &dsize)) == ERROR_SUCCESS)
		{
#ifdef DEBUG2
			PRINT("Read key: %s\n", key);
#endif
			mt = key;
		}
		else
		{
			PRINT("Error: %d [0x%08x]\n", ret, ret);
		}
		RegCloseKey(hkey);
	}
	fMIMEType = mt;
	return;
#else
	fMIMEType = QString::null;
	return;
#endif

}

QString
UFileInfo::getMIMEType() const
{
	return fMIMEType;
}

void
UFileInfo::InitModificationTime()
{
#ifdef WIN32
		// Read the modification time
		// The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
		FILETIME ftime;
		// The time functions included in the C run-time use the time_t type to represent the number of seconds elapsed since midnight, January 1, 1970.
		uint32 mtTime; // <postmaster@raasu.org> 20021230
		uint64 ftTime; // 
		HANDLE fileHandle;
		WString tFilePath = fFullName;
		fileHandle = CreateFile(tFilePath.getBuffer(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileHandle)
		{
#ifdef DEBUG2
			PRINT("File opened!\n");
#endif
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
			fModificationTime = mtTime;
			return;
		}
		else
		{
			fModificationTime = time(NULL);
			return;
		}

#else
		struct stat fst;
		// assume UTF-8 encoded file system
		int ret = stat((const char *) fFullName.utf8(), &fst);
		if (ret == 0)
		{
			fModificationTime = fst.st_mtim.tv_sec;
		}
		else
		{
			fModificationTime = time(NULL);
		}
		return;
#endif

}

uint32
UFileInfo::getModificationTime()
{
	return fModificationTime;
}

void
UFileInfo::Init()
{
	InitPath();
	InitName();
	InitExtension();
	InitMIMEType();
	InitSize();
	InitModificationTime();
}

void
UFileInfo::InitPath()
{
	fFilePath = dirPath(true);
}

QString
UFileInfo::getPath() const
{
	return fFilePath;
}

void
UFileInfo::InitName()
{
	fFileName = fileName();
	fFullName = filePath();
}

QString
UFileInfo::getName() const
{
	return fFileName;
}

void
UFileInfo::InitExtension()
{
	fExtension = extension( false );
}

QString
UFileInfo::getExtension() const
{
	return fExtension;
}

void
UFileInfo::InitSize()
{
	fSize = size();
}

uint64
UFileInfo::getSize()
{
	return fSize;
}

bool
UFileInfo::isValid()
{
	return exists();	// non-existent file?
}
