#include "titanic.h"

String
TTPEncode(const String &orig)
{
	String temp("");
	for (unsigned int x = 0; x < orig.Length(); x++)
	{
		String h = chartohex(orig.CharAt(x));
		temp += h;
	}
	return temp;
}

String
TTPDecode(const String &orig)
{
	if ((orig.Length() % 2) != 0)
		return String("");
	String temp("");
	for (unsigned int x = 0; x < orig.Length(); x += 2)
	{
		char c = hextochar(orig.Substring(x, x+2));
		temp += c;
	}
	return temp;
}

#ifdef QT_DLL
QString
TTPDecode(const QString &orig)
{
	if ((orig.length() % 2) != 0)
		return QString("");
	QString temp("");
	for (unsigned int x = 0; x < orig.length(); x += 2)
	{
		QChar c = hextochar(orig.mid(x, 2));
		temp += c;
	}
	return temp;
}
#endif

String
chartohex(const char c)
{
	char h[3];	// Include NULL terminator ;)
	sprintf(h,"%02x", (c & 0xFF));
	return String(h);
}

char
hextochar(const String &orig)
{
	if (orig.Length() != 2) 
		return (char) 0;
	const char *buf = orig.Cstr();
	long l = strtol(buf, NULL, 16);
	return (char) (l & 0xFF);
}

#ifdef QT_DLL
QChar
hextochar(const QString &orig)
{
	if (orig.length() != 2) 
		return (QChar) 0;
	const char *buf = orig.local8Bit();
	long l = strtol(buf, NULL, 16);
	return (QChar) (l & 0xFF);
}
#endif
