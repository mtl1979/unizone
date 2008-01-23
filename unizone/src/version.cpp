#include "version.h"

#include <qapplication.h>

const QString uzYears = "2002-2008";
const int kMajor = 1;
const int kMinor = 2;
const int kPatch = 3;
const int kBuild = 23;


QString
WinShareVersionString()
{
	static QString version = QString::null;
	if (version.isEmpty())
		version = qApp->translate("Version", "%1.%2.%3 build %4").arg(kMajor).arg(kMinor).arg(kPatch).arg(kBuild);
	return version;
}

const QString &
GetUnizoneYears()
{
	return uzYears;
}

const int &
UZ_MajorVersion()
{
	return kMajor;
}

const int &
UZ_MinorVersion()
{
	return kMinor;
}

const int &
UZ_Patch()
{
	return kPatch;
}

const int &
UZ_Build()
{
	return kBuild;
}
