#ifndef PARSER_H
#define PARSER_H

#include <qstring.h>

// Parse text for showing on WHTMLView
QString ParseForShown(const QString & txt);
QString ParseForShownAux(const QString &txt);
int ParseBufferSize();
#endif
