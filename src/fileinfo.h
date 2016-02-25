#ifndef UFILEINFO_H
#define UFILEINFO_H

#include <qfileinfo.h>

#include "support/MuscleSupport.h"

class QString;

class UFileInfo
{
public:
	UFileInfo(const QFileInfo & info);
	UFileInfo(const QString & file);
	virtual ~UFileInfo();

	time_t getModificationTime() const;
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
	time_t fModificationTime;
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
