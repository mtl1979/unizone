#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>

const int kMajor = 1;
const int kMinor = 2;
const int kPatch = 0;
const int kBuild = 10;

inline char *
WinShareVersionString()
{
	static char version[50];
	sprintf(version, "%d.%d.%d Build %d", kMajor, kMinor, kPatch, kBuild);
	return version;
}

#endif
