#ifndef TITANIC_H
#define TITANIC_H

enum {
	TTP_START_QUEUE = 'tTsQ',
	TTP_ADD_FILE
};

#include <qstring.h>

struct TTPInfo
{
	QString bot;
	QString file;
};

// Encodes text for transfer
QString TTPEncode(const QByteArray &, unsigned long);
QString TTPEncode(const QString &);

// Decodes returning text
QString TTPDecode(const QString &);
QByteArray TTPDecode(const QString &, unsigned long *);

// convert hexadecimal code to character equivalent
QChar hextochar(const QString &);

// convert character to hexadecimal code
QCString chartohex(const QChar &);

#endif
