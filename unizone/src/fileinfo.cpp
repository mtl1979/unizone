#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#endif

#include <qfileinfo.h>
#ifndef WIN32
#include <qfile.h>
#endif
#include <qstring.h>

#include "fileinfo.h"

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


QString
UFileInfo::getMIMEType() const
{
	return fMIMEType;
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
