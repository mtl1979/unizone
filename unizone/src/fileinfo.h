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
	UFileInfo(QFileInfo & info);
	UFileInfo(QString file);
	virtual ~UFileInfo();
	
	uint32 getModificationTime();
	QString getMIMEType();
	QString getPath();
	QString getName();
	uint64 getSize();

	bool isValid();

private:
	QFileInfo *fInfo;				// Object initialized from?
};

#endif