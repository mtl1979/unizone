#ifndef UFILEINFO_H
#define UFILEINFO_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qfileinfo.h>
#include <qstring.h>

#include "support/MuscleSupport.h"

class UFileInfo
{
public:
	UFileInfo(const QFileInfo & info);
	UFileInfo(const QString & file);
	virtual ~UFileInfo();
	
	uint32 getModificationTime() const;
	QString getMIMEType() const;
	QString getPath() const;
	QString getAbsPath() const;
	QString getName() const;
	void setName(const QString & n);
	QString getFullName() const;
	QString getExtension() const;
	uint64 getSize();

	bool isValid() const;
	bool isDir() const;

protected:

	void Init();

private:
	QString fFileName, fFullName;
	QString fExtension;
	QString fFilePath;
	QString fAbsPath;
	QString fMIMEType;
	uint32 fModificationTime;
	uint64 fSize;

	void InitMIMEType();
	void InitModificationTime();
	void InitPath();
	void InitName();
	void InitExtension();
	void InitSize();

	QFileInfo *fFileInfo;
};

#endif
