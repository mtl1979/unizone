#include <qstring.h>
#include <qfileinfo.h>

#include "filethread.h"

QString
WFileThread::ResolveLink(const QString & lnk) const
{
	QFileInfo * info = new QFileInfo(lnk);
	if (inf.isSymLink())
	{
		QString ret = info->readLink();
		delete info;
		return ResolveLink(ret);
	}
	delete info;
	return lnk; 	// oops
}
