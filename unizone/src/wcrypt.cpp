#include "wcrypt.h"
#include "titanic.h"

#if defined(WIN32)
#  include <windows.h>
#elif defined(LINUX) || defined(__LINUX__)
#  include <netinet/in.h>
#endif

const char mask[4] = {'5', '0', '3', '8'};

QString
wencrypt2(const QString &in)
{
	unsigned long len;
	QByteArray c = wencrypt(in, &len);
	QString out = TTPEncode(c, len);
	return out;
}

QByteArray 
wencrypt(const QString &in, unsigned long * outlen)
{
	unsigned long l,len;
	QCString tmp = in.utf8();
	l = tmp.length();
	len = htonl(l);
	QByteArray out(l + 4);
	
	int n = 4;
	memcpy(&out[0], &len, 4);
	for (unsigned int i = 0; i < tmp.length(); i += 4)
	{
		QCString buf = tmp.mid(i, 4);
		if (buf.length() == 4)
		{
			out.at(n++) = (QChar) (buf.at(3) ^ mask[0]);
			out.at(n++) = (QChar) (buf.at(1) ^ mask[2]);
			out.at(n++) = (QChar) (buf.at(2) ^ mask[1]);
			out.at(n++) = (QChar) (buf.at(0) ^ mask[3]);
		}
		if (buf.length() == 3)
		{
			out.at(n++) = (QChar) (buf.at(1) ^ mask[2]);
			out.at(n++) = (QChar) (buf.at(2) ^ mask[1]);
			out.at(n++) = (QChar) (buf.at(0) ^ mask[3]);
		}
		if (buf.length() == 2)
		{
			out.at(n++) = (QChar) (buf.at(1) ^ mask[1]);
			out.at(n++) = (QChar) (buf.at(0) ^ mask[3]);
		}
		if (buf.length() == 1)
		{
			out.at(n++) = (QChar) (buf.at(0) ^ mask[3]);
		}
	}
	
	if (outlen)
		*outlen = l + 4;
	return out;
}

QString 
wdecrypt2(const QString &in)
{
	unsigned long len;
	QByteArray tmp = TTPDecode(in, &len);
	QString out = wdecrypt(tmp, len);
	return out;
}

QString
wdecrypt(const QByteArray &in, unsigned long inlen)
{
	if (inlen < 5)
		return QString::null;
	unsigned long l,len;
	memcpy(&l, &in[0], 4);
	len = ntohl(l);
	QByteArray buf(len + 1);
	buf.at(len) = (QChar) 0;
	for (unsigned int i = 4; i <= inlen; i += 4)
	{
		int numleft = inlen - i;
		int l2 = (numleft > 4) ? 4 : numleft;
		if (l2 == 4)
		{
			buf.at(i-1) = (QChar) (in.at(i) ^ mask[0]);
			buf.at(i-2) = (QChar) (in.at(i + 2) ^ mask[1]);
			buf.at(i-3) = (QChar) (in.at(i + 1) ^ mask[2]);
			buf.at(i-4) = (QChar) (in.at(i + 3) ^ mask[3]);
		}
		if (l2 == 3)
		{
			buf.at(i-2) = (QChar) (in.at(i + 1) ^ mask[1]);
			buf.at(i-3) = (QChar) (in.at(i) ^ mask[2]);
			buf.at(i-4) = (QChar) (in.at(i + 2) ^ mask[3]);
		}
		if (l2 == 2)
		{
			buf.at(i-3) = (QChar) (in.at(i) ^ mask[1]);
			buf.at(i-4) = (QChar) (in.at(i + 1) ^ mask[3]);
		}
		if (l2 == 1)
		{
			buf.at(i-4) = (QChar) (in.at(i) ^ mask[3]);
		}
	}
	QString out = QString::fromUtf8(buf);
	return out;
}
