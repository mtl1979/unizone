#ifndef TITANIC_H
#define TITANIC_H

enum {
	TTP_START_QUEUE = 1951691601, // tTsQ
	TTP_ADD_FILE
};

#include <qstring.h>
#include <qbytearray.h>

struct TTPInfo
{
	QString bot;
	QString file;
};

// Encodes text for transfer
QString TTPEncode(const QByteArray &, int);
QString TTPEncode(const QString &);

// Decodes returning text
QString TTPDecode(const QString &);
QByteArray TTPDecode(const QString &, int *);

// convert hexadecimal code to character equivalent
char hextochar(const QString &);

// convert character to hexadecimal code
QString chartohex(const char &);

#endif
