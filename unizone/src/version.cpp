#include "version.h"

#include <qobject.h>

const char uzYears[] = "2003-2004";
const int kMajor = 1;
const int kMinor = 2;
const int kPatch = 2;
const int kBuild = 4;


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
