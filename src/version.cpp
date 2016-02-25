#include "version.h"

#include <qapplication.h>

const QString uzYears = "2002-2015";
const int kMajor = 1;
const int kMinor = 3;
const int kPatch = 0;
const int kBuild = 2;


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
