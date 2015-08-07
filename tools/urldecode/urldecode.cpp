// urldecode.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <qstring.h>
#include <qfile.h>
#include <qiodevice.h>
#include <qcoreapplication.h>

char
hextochar(const QByteArray &orig)
{
	if (orig.length() != 2)
		return (char) 0;
	const char *buf = orig.data();
	long l = strtol(buf, NULL, 16);
	return (char) (l & 0xFF);
}

wchar_t
hextouchar(const QByteArray &orig)
{
	if (orig.length() != 4)
		return (wchar_t) 0;
	const char *buf = orig.data();
	long l = strtol(buf, NULL, 16);
	return (wchar_t) (l & 0xFFFF);
}

QString
ConvertFileName(const QByteArray &in)
{
	QByteArray out = "";
	QString out2 = QString::null;
	int slash = 0;
	for (int x = 0; x < in.length(); x++)
	{
		if ((in.at(x) == '\\') || (in.at(x) == '/'))
			slash = x;
		if ((in.at(x) == '%') || (in.at(x) == '@'))
		{
			if (in.at(x+1) == 'u') // Unicode
			{
				wchar_t temp = hextouchar(in.mid(x+2, 4));
				if (temp != (wchar_t) 0)
				{
					if (out.length() > 0) // Flush transcode buffer
					{
						out2 += QString::fromLocal8Bit(out);
						out = "";
					}
					out2 += QChar(temp);
					x += 5;
					continue;
				}
			}
			else
			{
				char temp = hextochar(in.mid(x+1, 2));
				if (temp != (char) 0)
				{
					if ((temp == '\\') || (temp == '/'))
						out.truncate(slash);
					else
						out += temp;
					x += 2;
					continue;
				}
			}
		}
		else if (in.at(x) == '+')
		{
			out += " ";
			continue;
		}
		out += in.at(x);
	}
	if (out.length() > 0)
	{
		out2 += QString::fromLocal8Bit(out);
	}
	return out2;
}

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	if (argc != 2)
	{
		printf("Usage: %s <input file>\n", argv[0]);
		return -1;
	}
	else
	{
		QString out = ConvertFileName(argv[1]);
		QString in = QString::fromLocal8Bit(argv[1]);
		uint sz = 0;
		char buf[1024];
		if (in != out)
		{
			QFile ifile(in);
			QFile ofile(out);
			
			if (ifile.open(QIODevice::ReadOnly) == false)
			{
				printf("Error opening input file!");
				return -1;
			}
			
			sz = ifile.size();
			
			if (ofile.open(QIODevice::WriteOnly) == false)
			{
				printf("Error opening output file!");
				ifile.close();
				return -1;
			}
			int r;
			qint64 numbytes = 0;
			while ((r = ifile.read(buf, 1024)) > 0)
			{
				ofile.write(buf, r);
				numbytes += r;
			}
			printf("Read %u bytes.\n", sz);
#ifdef WIN32
			printf("Wrote %I64i bytes.\n", numbytes);
#else
			printf("Wrote %Li bytes.\n", numbytes);
#endif
			ifile.close();
			ofile.close();
			if (sz == numbytes)
			{
				ifile.remove();
			}
		}
	}
	return 0;
}
