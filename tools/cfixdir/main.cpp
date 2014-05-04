#include <stdio.h>
#include <stdlib.h>

#include <QByteArray>
#include <qstring.h>
#include <qtextcodec.h>
#include <qdir.h>
#include <qcoreapplication.h>

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	if (argc != 3)
	{
		printf("Usage: %s <input directory> <source encoding>\n", argv[0]);
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

		if (in != out)
		{
			if (QDir::current().rename(in, out) == false)
			{
				printf("Error renaming!");
				return -1;
			}
		}
	}
	return 0;
}
