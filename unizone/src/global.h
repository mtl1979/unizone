#ifndef GLOBAL_H
#define GLOBAL_H

#include "support/MuscleSupport.h"

class WinShareWindow;

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

extern WinShareWindow * gWin;	// global window
extern QString gAppDir; // global variable holding base directory of application

void SetSettingsFile(const char * sf);
const char * GetSettingsFile();

int64 GetStartTime();

#endif
