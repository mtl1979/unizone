#ifndef TITANIC_H
#define TITANIC_H

#include "util/String.h"

using namespace muscle;

enum {
	TTP_START_QUEUE = 'tTsQ',
	TTP_ADD_FILE
};

#include <qstring.h>

#if !defined(__BEOS__)
struct TTPInfo
{
	QString bot;
	QString file;
};
#else
struct TTPInfo
{
	String bot;
	String file;
};
#endif

// Encodes text for transfer
String TTPEncode(const String &);
// Decodes returning text
String TTPDecode(const String &);
char * TTPDecode(const String &, uint32 *len);

#if !defined(__BEOS__)
QString TTPEncode(uint8 * orig, uint32 len);

QString TTPDecode(const QString &);
char * TTPDecode(const QString &, uint32 *len);
#endif

// convert hexadecimal code to character equivalent
char hextochar(const String &);
#if !defined(__BEOS__)
QChar hextochar(const QString &);
#endif
// convert character to hexadecimal code
String chartohex(const char);
#if !defined(__BEOS__)
QString chartohex(const QChar);
#endif

#endif
