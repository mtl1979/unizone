#ifndef TITANIC_H
#define TITANIC_H

#include "util/String.h"

using namespace muscle;

enum {
	TTP_START_QUEUE = 'tTsQ',
	TTP_ADD_FILE
};

struct TTPInfo
{
	String bot;
	String file;
};

// Encodes text for transfer
String TTPEncode(const String &);
// Decodes returning text
String TTPDecode(const String &);

// convert hexadecimal code to character equivalent
char hextochar(const String &);
// convert character to hexadecimal code
String chartohex(const char);

#endif
