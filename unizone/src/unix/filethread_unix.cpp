#include <qstring.h>
#include <qfileinfo.h>

#include "filethread.h"

QString
WFileThread::ResolveLink(const QString & lnk)
{
	QFileInfo inf(lnk);
	if (inf.isSymLink())
		return inf.readLink();
	else
		return lnk; 	// oops
}
