#include <stdio.h>
#include <stdlib.h>

#include <QByteArray>
#include <qstring.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qcoreapplication.h>

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	if (argc != 3)
	{
		printf("Usage: %s <input file> <source encoding>\n", argv[0]);
		printf("\n");
		printf("Encodings:\n");
		printf("----------\n");
		printf("5 - Big-5\n");
		printf("G - GBK\n");
		return -1;
	}
	else
	{
        QTextCodec *cc;
		switch (argv[2][0])
		{
			case '5': 
				cc = QTextCodec::codecForName("Big-5");
				break;
			case 'G': 
				cc = QTextCodec::codecForName("GBK");
				break;
			default: return -2;
		}
		QString out = cc->toUnicode(argv[1], strlen(argv[1]));
		QString in = argv[1];
		QByteArray test = cc->fromUnicode(out);
		if (test != argv[1])
		{
			printf("Decoder error!\n");
			return -1;
		}

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
		__int64 numbytes = 0;
		while ((r = ifile.read(buf, 1024)) > 0)
		{
			ofile.write(buf, r);
			numbytes += r;
		}
		printf("Read %u bytes.\n", sz); 
		printf("Wrote %Li bytes.\n", numbytes);
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
