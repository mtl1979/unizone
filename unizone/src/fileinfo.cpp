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
	WString tFilePath = fFullName;

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
