#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>

const int kMajor = 1;
const int kMinor = 2;
const int kPatch = 0;

#include "build.h"		// just a trick to increment builds everytime it is built :)

inline char *
WinShareVersionString()
{
	static char version[50];
	sprintf(version, "%d.%d.%d Build %d", kMajor, kMinor, kPatch, kBuild);
	return version;
}

#ifdef _DEBUG
inline void 
IncrementBuild()	// trick from above :)
{
#ifdef WIN32
	FILE * fp = fopen("..\\src\\build.h", "w");
#else
	FILE * fp = fopen("src/build.h", "w");
#endif
	if (fp)
	{
		fprintf(fp, "const int kBuild = %d;\n", kBuild + 1);
		fclose(fp);
	}
}
#else
#define IncrementBuild()
#endif

#endif
