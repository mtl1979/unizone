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
	UFileInfo(QFileInfo info);
	UFileInfo(QString file);
	virtual ~UFileInfo();
	
	uint32 getModificationTime();
	QString getMIMEType() const;
	QString getPath() const;
	QString getName() const;
	uint64 getSize();

	bool isValid();
	void Init();

private:
	QFileInfo *fInfo;				// Object initialized from?
	QString fFileName;
	QString fFilePath;
	QString fMIMEType;
	uint32 fModificationTime;
	uint64 fSize;

	void InitMIMEType();
	void InitPath();
	void InitName();
	void InitSize();
	void InitModificationTime();
};

#endif
