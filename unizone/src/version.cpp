#include "version.h"

#include <qobject.h>

const char uzYears[] = "2003-2004";

QString
WinShareVersionString()
{
	QString version = QObject::tr("%1.%2.%3 build %4").arg(kMajor).arg(kMinor).arg(kPatch).arg(kBuild);
	return version;
}

QString
GetUnizoneYears()
{
	return uzYears;
}