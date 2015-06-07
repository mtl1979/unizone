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
		QTextCodec *uc = QTextCodec::codecForName("UTF-8");
		QString out = uc->toUnicode(argv[1], strlen(argv[1]));
		QString in = QString::fromLocal8Bit(argv[1]);
		QByteArray test = uc->fromUnicode(out);
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