#include "titanic.h"
#include <stdio.h>
#include <Q3CString>

QString
TTPEncode(const QByteArray & orig, int len)
{
	Q3CString temp;
	Q3CString c;
	for (int x = 0; x < len; x++)
	{
		c = chartohex(orig.at(x));
		temp += c;
	}
	return QString::fromUtf8(temp);
}

QString
TTPEncode(const QString &orig)
{
	Q3CString temp = orig.utf8();
	Q3CString out;
	Q3CString c;
	for (unsigned int x = 0; x < strlen(temp); x++)
	{
		c = chartohex(temp.at(x));
		out += c;
	}
	return QString::fromUtf8(out);
}

QString
TTPDecode(const QString &orig)
{
	if ((orig.length() % 2) != 0)
		return QString::null;
	Q3CString temp;
	for (int x = 0; x < orig.length(); x += 2)
	{
		QString tmp = orig.mid(x,2);
		QChar c = hextochar(tmp);
		temp += c;
	}
	return QString::fromUtf8(temp);
}

QByteArray
TTPDecode(const QString &orig, int * len)
{
	if ((orig.length() % 2) != 0)
	{
		if (len)
			*len = 0;
		return QByteArray();
	}
	QByteArray temp(orig.length());
	int n = 0;
	for (int x = 0; x < orig.length(); x += 2)
	{
		QString tmp = orig.mid(x, 2);
		char c = hextochar(tmp);
		temp[n++] = c;
	}
	if (len)
		*len = orig.length() / 2;
	return temp;
}

const char values[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f'
};

Q3CString
chartohex(const char &c)
{
	char h[3];	// Include NULL terminator ;)
	sprintf(h,"%02x", (c & 0xFF));
	return Q3CString(h);
}

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

char
hextochar(const QString &orig)
{
	if (orig.length() != 2) 
		return 0;
	
	unsigned int l = hfind(orig.at(0)) * 16 + hfind(orig.at(1));
	return (char) (l & 0xFF);
}
