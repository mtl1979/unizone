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

private:
	QFileInfo *fInfo;				// Object initialized from?
};

#endif