#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <qfileinfo.h>
#ifndef WIN32
#include <qfile.h>
#else
#include <wchar.h>
#endif
#include <qstring.h>

#include "fileinfo.h"
#include "wstring.h"
#include "debugimpl.h"

UFileInfo::UFileInfo(const QFileInfo & info)
{
	fFileInfo = new QFileInfo(info);
	Init();
}

UFileInfo::UFileInfo(const QString & file)
{
	fFileInfo = new QFileInfo(file);
	Init();
}

UFileInfo::~UFileInfo()
{
	if (fFileInfo)
		delete fFileInfo;
}

void
UFileInfo::InitMIMEType()
{
#ifdef WIN32
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
#ifdef DEBUG2
			PRINT("Read key: %S\n", key);
#endif
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
	if ( isValid() )
	{
#ifdef WIN32
		// Read the modification time
		WString tFilePath(fFullName);
		
		struct _stat fst;
		
		int ret = _wstat(tFilePath.getBuffer(), &fst);
		if (ret == 0)
		{
			fModificationTime = fst.st_mtime;
		}
		else
		{
			fModificationTime = time(NULL);
		}
		
#else
		struct stat fst;
		// Encode the Unicode filename to local file system character set
		const char * fname = (const char *) QFile::encodeName(fFullName); 
		int ret = stat(fname, &fst);
		if (ret == 0)
		{
#ifdef __USE_MISC
			fModificationTime = fst.st_mtim.tv_sec;
#else
			fModificationTime = fst.st_mtime;
#endif
		}
		else
		{
			fModificationTime = time(NULL);
		}
		return;
#endif
	}
	else
		fModificationTime = time(NULL);
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
	if ( isValid() )
	{
		fFilePath = fFileInfo->dirPath(true);
		fAbsPath = fFileInfo->absFilePath();
	}
	else
	{
		fFilePath = QString::null;
		fAbsPath = QString::null;
	}
}

QString
UFileInfo::getPath() const
{
	return fFilePath;
}

QString
UFileInfo::getAbsPath() const
{
	return fAbsPath;
}

void
UFileInfo::InitName()
{
	if ( isValid() )
	{
		fFileName = fFileInfo->fileName();
		fFullName = fFileInfo->filePath();
	}
	else
	{
		fFileName = QString::null;
		fFullName = QString::null;
	}
}

QString
UFileInfo::getName() const
{
	return fFileName;
}

QString
UFileInfo::getFullName() const
{
	return fFullName;
}

void
UFileInfo::InitExtension()
{
	if ( isValid() )
		fExtension = fFileInfo->extension( false );
	else
		fExtension = QString::null;
}

QString
UFileInfo::getExtension() const
{
	return fExtension;
}

void
UFileInfo::InitSize()
{
	if ( isValid() )
		fSize = fFileInfo->size();
	else
		fSize = 0;
}

uint64
UFileInfo::getSize()
{
	return fSize;
}

bool
UFileInfo::isValid()
{
	if (fFileInfo)
		return fFileInfo->exists();	// non-existent file?
	else
		return false;
}

void
UFileInfo::setName(const QString & n)
{
	if (fFileInfo)
	{
		delete fFileInfo;
		fFileInfo = NULL;
	}

	fFileInfo = new QFileInfo(n);

	Init();
}

bool
UFileInfo::isDir()
{
	if ( isValid() )
		return fFileInfo->isDir();
	else
		return false;
}
