#include "wcrypt.h"
#include "titanic.h"

#if defined(LINUX) || defined(__LINUX__)
#include <netinet/in.h>
#endif

const char mask[4] = {'5', '0', '3', '8'};

QString
wencrypt2(const QString &in)
{
	uint32 len;
	char * c = wencrypt(in, &len);
	QString out = TTPEncode((uint8 *) c, len);
	delete [] c;
	return out;
}

char * 
wencrypt(const QString &in, uint32 * outlen)
{
	char *out;
	uint32 l,len;
	QCString tmp = in.utf8();
	l = tmp.length();
	len = htonl(l);
	out = new char[l + 4];
	if (out)
	{
		int n = 4;
		memcpy(&out[0], &len, 4);
		for (unsigned int i = 0; i < tmp.length(); i += 4)
		{
			QCString buf = tmp.mid(i, 4);
			if (buf.length() == 4)
			{
				out[n++] = (char) (buf.at(3) ^ mask[0]);
				out[n++] = (char) (buf.at(1) ^ mask[2]);
				out[n++] = (char) (buf.at(2) ^ mask[1]);
				out[n++] = (char) (buf.at(0) ^ mask[3]);
			}
			if (buf.length() == 3)
			{
				out[n++] = (char) (buf.at(1) ^ mask[2]);
				out[n++] = (char) (buf.at(2) ^ mask[1]);
				out[n++] = (char) (buf.at(0) ^ mask[3]);
			}
			if (buf.length() == 2)
			{
				out[n++] = (char) (buf.at(1) ^ mask[1]);
				out[n++] = (char) (buf.at(0) ^ mask[3]);
			}
			if (buf.length() == 1)
			{
				out[n++] = (char) (buf.at(0) ^ mask[3]);
			}
		}
	}
	if (outlen)
		*outlen = l + 4;
	return out;
}

QString 
wdecrypt2(const QString &in)
{
	uint32 len;
	char *tmp = TTPDecode(in, &len);
	QString out = wdecrypt(tmp, len);
	delete [] tmp;
	return out;
}

QString
wdecrypt(const char * in, uint32 inlen)
{
	if ((in == NULL) || (inlen < 5))
		return QString::null;
	uint32 l,len;
	memcpy(&l, &in[0], 4);
	len = ntohl(l);
	char * buf = new char[len + 1];
	buf[len] = 0;
	for (unsigned int i = 4; i <= inlen; i += 4)
	{
		int numleft = inlen - i;
		int l2 = (numleft > 4) ? 4 : numleft;
		if (l2 == 4)
		{
			buf[i-1] = (char) (in[i] ^ mask[0]);
			buf[i-2] = (char) (in[i + 2] ^ mask[1]);
			buf[i-3] = (char) (in[i + 1] ^ mask[2]);
			buf[i-4] = (char) (in[i + 3] ^ mask[3]);
		}
		if (l2 == 3)
		{
			buf[i-2] = (char) (in[i + 1] ^ mask[1]);
			buf[i-3] = (char) (in[i] ^ mask[2]);
			buf[i-4] = (char) (in[i + 2] ^ mask[3]);
		}
		if (l2 == 2)
		{
			buf[i-3] = (char) (in[i] ^ mask[1]);
			buf[i-4] = (char) (in[i + 1] ^ mask[3]);
		}
		if (l2 == 1)
		{
			buf[i-4] = (char) (in[i] ^ mask[3]);
		}
	}
	QString out = QString::fromUtf8(buf);
	delete [] buf;
	return out;
}
