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
	if (argc != 2)
	{
		printf("Usage: %s <input directory>\n", argv[0]);
		return -1;
	}
	else
	{
		QTextCodec *ec = QTextCodec::codecForName("EUC-KR");
		QString out = ec->toUnicode(argv[1], (int) strlen(argv[1]));
		QByteArray test = ec->fromUnicode(out);
		if (test != argv[1])
		{
			printf("Decoder error!\n");
			return -1;
		}
		QString in = argv[1];
		if (in != out)
		{
			if (QDir::current().exists(out))
			{
				printf("Output directory already exists!");
				return -1;
			}
			if (QDir::current().rename(in, out) == false)
			{
				printf("Error renaming!");
				return -1;
			}
		}
	}
	return 0;
}
