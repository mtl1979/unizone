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

char *
TTPDecode(const String &orig, uint32 *len)
{
	if ((orig.Length() % 2) != 0)
		return NULL;
	char * temp = new char[orig.Length() / 2];
	int n = 0;
	for (unsigned int x = 0; x < orig.Length(); x += 2)
	{
		char c = hextochar(orig.Substring(x, x+2));
		temp[n++] = c;
	}
	if (len)
		*len = orig.Length() / 2;
	return temp;
}

#if !defined(__BEOS__)

QString
TTPEncode(uint8 * orig, uint32 len)
{
	QString temp("");
	for (unsigned int x = 0; x < len; x++)
	{
		QString h = chartohex(QChar(orig[x]));
		temp += h;
	}
	return temp;
}

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

char *
TTPDecode(const QString &orig, uint32 * len)
{
	if ((orig.length() % 2) != 0)
		return NULL;
	char * temp = new char[orig.length() / 2];
	int n = 0;
	for (unsigned int x = 0; x < orig.length(); x += 2)
	{
		QString tmp = orig.mid(x, 2);
		QChar c = hextochar(tmp);
		temp[n++] = c.unicode() % 0xFF;
	}
	if (len)
		*len = orig.length() / 2;
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

#if !defined(__BEOS__)
QString
chartohex(const QChar c)
{
	if (c.unicode() < 65536)
	{
		char h[3];	// Include NULL terminator ;)
		sprintf(h,"%02x", (c & 0xFF));
		return QString(h);
	}
	else
	{
		char h[5];
		sprintf(h,"%05x", c.unicode());
		return QString(h);
	}
}

const char values[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f'
};

int
hfind(const QChar &c)
{
	for (int i = 0; i < 16; i++)
	{
		if (c.lower() == values[i])
			return i;
	}
	return -1;
}

QChar
hextochar(const QString &orig)
{
	if (orig.length() != 2) 
		return (QChar) 0;
	
	unsigned int l = hfind(orig.at(0)) * 16 + hfind(orig.at(1));
	return (QChar) (l & 0xFF);
}
#endif
