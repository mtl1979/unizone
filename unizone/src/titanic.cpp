#include "titanic.h"
#include <stdio.h>

QString
TTPEncode(const QByteArray & orig, unsigned long len)
{
	QCString temp("");
	QCString c;
	for (unsigned int x = 0; x < len; x++)
	{
		c = chartohex(orig.at(x));
		temp += c;
	}
	return QString::fromUtf8(temp);
}

QString
TTPEncode(const QString &orig)
{
	QCString temp = orig.utf8();
	QCString out("");
	QCString c;
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
		return QString("");
	QCString temp("");
	for (unsigned int x = 0; x < orig.length(); x += 2)
	{
		QString tmp = orig.mid(x,2);
		QChar c = hextochar(tmp);
		temp += c;
	}
	return QString::fromUtf8(temp);
}

QByteArray
TTPDecode(const QString &orig, unsigned long * len)
{
	if ((orig.length() % 2) != 0)
		return NULL;
	QByteArray temp(orig.length());
	int n = 0;
	for (unsigned int x = 0; x < orig.length(); x += 2)
	{
		QString tmp = orig.mid(x, 2);
		QChar c = hextochar(tmp);
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

QCString
chartohex(const QChar &c)
{
	if (c.unicode() < 256)
	{
		char h[3];	// Include NULL terminator ;)
		sprintf(h,"%02x", (c.unicode() & 0xFF));
		return QCString(h);
	}
	else
	{
		char h[5];
		sprintf(h,"%04x", c.unicode());
		return QCString(h);
	}
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

QChar
hextochar(const QString &orig)
{
	if (orig.length() != 2) 
		return (QChar) 0;
	
	unsigned int l = hfind(orig.at(0)) * 16 + hfind(orig.at(1));
	return (QChar) (l & 0xFF);
}

