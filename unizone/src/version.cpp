#include "version.h"

#include <qapplication.h>

const char uzYears[] = "2002-2005";
const int kMajor = 1;
const int kMinor = 2;
const int kPatch = 3;
const int kBuild = 10;


QString
WinShareVersionString()
{
	QString version = qApp->translate("Version", "%1.%2.%3 build %4").arg(kMajor).arg(kMinor).arg(kPatch).arg(kBuild);
	return version;
}

QString
GetUnizoneYears()
{
	return uzYears;
}

int UZ_MajorVersion()
{
	return kMajor;
}

int UZ_MinorVersion()
{
	return kMinor;
}

int UZ_Patch()
{
	return kPatch;
}

int UZ_Build()
{
	return kBuild;
}
