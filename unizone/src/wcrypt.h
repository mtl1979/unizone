#ifndef WCRYPT_H
#define WCRYPT_H

#include <qstring.h>
#include "util/string.h"

using namespace muscle;

char * wencrypt(const QString &, uint32 * len = NULL);
QString wdecrypt(const char *, uint32);

// Armoured versions
QString wencrypt2(const QString &);
QString wdecrypt2(const QString &);
#endif
