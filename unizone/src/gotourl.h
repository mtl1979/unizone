#ifndef GOTOURL_H
#define GOTOURL_H

#include <qstring.h>

void GotoURL(const QString & url);				// async launch
void RunCommand(const QString & command);		// shell command launcher

#ifdef WIN32
bool GotoURL(const QString & url, int showcmd);	// sync launch, used internally by the async version
#else
bool GotoURL(const QString & url, const QString & browser);
#endif

#endif
