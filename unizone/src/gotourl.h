#ifndef GOTOURL_H
#define GOTOURL_H

#include <qstring.h>

void GotoURL(QString url);		// async launch
void RunCommand(QString command);

#ifdef WIN32
bool GotoURL(QString url, int showcmd );	// sync launch, used internally by the asnyc version

#else
bool GotoURL(QString url, QString browser);
#endif

#endif
