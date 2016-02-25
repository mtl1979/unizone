#ifndef GLOBAL_H
#define GLOBAL_H

#include "support/MuscleSupport.h"

class WinShareWindow;

extern WinShareWindow * gWin;	// global window
extern QString gAppDir; // global variable holding base directory of application

void SetSettingsFile(const QString &sf);
const QString & GetSettingsFile();

int64 GetStartTime();

#endif
