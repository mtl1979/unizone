#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>

const int kMajor = 1;
const int kMinor = 2;
const int kPatch = 0;
const int kBuild = 11;

inline QString
WinShareVersionString()
{
	static char version[50];
	const char * format = QT_TRANSLATE_NOOP( "WinShare", "%d.%d.%d build %d");
	sprintf(version, format, kMajor, kMinor, kPatch, kBuild);
	return QObject::tr(version);
}

#endif
