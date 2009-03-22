#ifndef GOTOURL_H
#define GOTOURL_H

#include <qstring.h>

void GotoURL(const QString & url);				// async launch
void RunCommand(const QString & command);		// shell command launcher

void InitLaunchThread();
void DeinitLaunchThread();
#endif
