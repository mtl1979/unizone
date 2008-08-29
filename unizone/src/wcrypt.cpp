#include "wcrypt.h"
#include "titanic.h"

// for htonl() & ntohl()
#if defined(WIN32) || defined(_WIN32)
#  include <winsock2.h>
#else
#  include <netinet/in.h>
//Added by qt3to4:
#include <Q3CString>
#endif

const char mask[4] = {'5', '0', '3', '8'};

QString
wencrypt2(const QString &in)
{
	int len;
	QByteArray c = wencrypt(in, &len);
	QString out = TTPEncode(c, len);
	return out;
}

QByteArray
wencrypt(const QString &in, int * outlen)
{
	unsigned long l,len;
	Q3CString tmp = in.utf8();
	l = tmp.length();
	len = htonl(l);
	QByteArray out(l + 4);

	int n = 4;
	memcpy(out.data(), &len, 4);
	for (int i = 0; i < tmp.length(); i += 4)
	{
		Q3CString buf = tmp.mid(i, 4);
		switch (buf.length())
		{
		case 4:
			out[n++] = (buf.at(3) ^ mask[0]);
			out[n++] = (buf.at(1) ^ mask[2]);
			out[n++] = (buf.at(2) ^ mask[1]);
			out[n++] = (buf.at(0) ^ mask[3]);
			break;
		case 3:
			out[n++] = (buf.at(1) ^ mask[2]);
			out[n++] = (buf.at(2) ^ mask[1]);
			out[n++] = (buf.at(0) ^ mask[3]);
			break;
		case 2:
			out[n++] = (buf.at(1) ^ mask[1]);
			out[n++] = (buf.at(0) ^ mask[3]);
			break;
		case 1:
			out[n++] = (buf.at(0) ^ mask[3]);
			break;
		}
	}

	if (outlen)
		*outlen = l + 4;
	return out;
}

QString
wdecrypt2(const QString &in)
{
	int len;
	QByteArray tmp = TTPDecode(in, &len);
	QString out = wdecrypt(tmp, len);
	return out;
}

QString
wdecrypt(const QByteArray &in, int inlen)
{
	if (inlen < 5)
		return QString::null;
	unsigned int l,len;
	memcpy(&l, in.data(), 4);
	len = ntohl(l);
	QByteArray buf(len + 1);
	buf[len] = 0;
	for (int i = 4; i <= inlen; i += 4)
	{
		int numleft = inlen - i;
		int l2 = (numleft > 4) ? 4 : numleft;
		switch (l2)
		{
		case 4:
			buf[i-1] = (in.at(i) ^ mask[0]);
			buf[i-2] = (in.at(i + 2) ^ mask[1]);
			buf[i-3] = (in.at(i + 1) ^ mask[2]);
			buf[i-4] = (in.at(i + 3) ^ mask[3]);
			break;
		case 3:
			buf[i-2] = (in.at(i + 1) ^ mask[1]);
			buf[i-3] = (in.at(i) ^ mask[2]);
			buf[i-4] = (in.at(i + 2) ^ mask[3]);
			break;
		case 2:
			buf[i-3] = (in.at(i) ^ mask[1]);
			buf[i-4] = (in.at(i + 1) ^ mask[3]);
			break;
		case 1:
			buf[i-4] = (in.at(i) ^ mask[3]);
			break;
		}
	}
	QString out = QString::fromUtf8(buf);
	return out;
}
