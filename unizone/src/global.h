#ifndef GLOBAL_H
#define GLOBAL_H
#include "winsharewindow.h"

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

extern WinShareWindow * gWin;	// global window
void SetSettingsFile(const char * sf);
const char * GetSettingsFile();
#endif
