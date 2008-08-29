#ifndef TITANIC_H
#define TITANIC_H

enum {
	TTP_START_QUEUE = 'tTsQ',
	TTP_ADD_FILE
};

#include <qstring.h>
#include <q3cstring.h>

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
Q3CString chartohex(const char &);

#endif
