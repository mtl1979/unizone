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
	Init();
}

UFileInfo::UFileInfo(QString file)
{
	fInfo = new QFileInfo(file);
	CHECK_PTR(fInfo);
	Init();
}

UFileInfo::~UFileInfo()
{
	if (fInfo)
		delete fInfo;
}

void
UFileInfo::InitMIMEType()
{
	if (!fInfo)
	{
		fMIMEType = QString::null;
		return;
	}

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
	if (!fInfo)
	{
		fModificationTime = time(NULL);
		return;
	}

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
			fModificationTime = mtTime;
			return;
		}
		else
		{
			fModificationTime = time(NULL);
			return;
		}

#else
		fModificationTime = time(NULL);
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
	InitMIMEType();
	InitSize();
	InitModificationTime();
}

void
UFileInfo::InitPath()
{
	if (!fInfo)
	{
		fFilePath = QString::null;
	}
	else
	{
		fFilePath = fInfo->dirPath(true);
	}
	return;
}

QString
UFileInfo::getPath() const
{
	return fFilePath;
}

void
UFileInfo::InitName()
{
	if (!fInfo)
	{
		fFileName = QString::null;
	}
	else
	{
		fFileName = fInfo->fileName();
	}
	return;
}

QString
UFileInfo::getName() const
{
	return fFileName;
}

void
UFileInfo::InitSize()
{
	if (!fInfo)
	{
		fSize = 0;
	}
	else
	{
		fSize = fInfo->size();
	}
	return;
}

uint64
UFileInfo::getSize()
{
	return fSize;
}

bool
UFileInfo::isValid()
{
	if (!fInfo)				// no valid object
	{
		return false;
	}

	if (!fInfo->exists())	// non-existent file
	{
		return false;
	}
	return true;
}
