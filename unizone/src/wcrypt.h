#ifndef WCRYPT_H
#define WCRYPT_H

#include <qstring.h>

QByteArray wencrypt(const QString &, unsigned long * = NULL);
QString wdecrypt(const QByteArray &, unsigned long);

// Armoured versions
QString wencrypt2(const QString &);
QString wdecrypt2(const QString &);
#endif
