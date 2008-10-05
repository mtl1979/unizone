#include "util.h"

#include <qdir.h>

QString MakePath(const QString &dir, const QString &file)
{
	QString ret = QDir::convertSeparators(dir);
	if (!ret.endsWith(QDir::separator()))
		ret += QDir::separator();

	ret += file;

	return ret;
}
