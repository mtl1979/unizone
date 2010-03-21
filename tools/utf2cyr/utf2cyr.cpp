// utf2cyr.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <QByteArray>
#include <qstring.h>
#include <qfile.h>
#include <qtextcodec.h>
#include <qcoreapplication.h>

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	if (argc != 3)
	{
		printf("Usage: %s input output\n", argv[0]);
		return -1;
	}

	QFile fIn(argv[1]), fOut(argv[2]);
	if (fIn.open(QIODevice::ReadOnly) == false)
	{
		printf("Error opening input file %s!",argv[1]);
		return -1;
	}
	if (fOut.open(QIODevice::WriteOnly) == false)
	{
		printf("Error opening output file %s!",argv[2]);
		fIn.close();
		return -1;
	}
	
	char qTemp1[256];
	QString qTemp2("");
	QByteArray qTemp3("");
 	QTextCodec * ic = QTextCodec::codecForName( "UTF-8" );
	QTextCodec * oc = QTextCodec::codecForName( "CP1251" );
	do
	{
		int bytes = fIn.readLine(qTemp1, 255);
		if (bytes == -1)
			break;
		qTemp2 = ic->toUnicode(qTemp1);
		qTemp3 = oc->fromUnicode(qTemp2);
		fOut.write(qTemp3, qTemp3.length());
		fOut.write("\r\n", 2);
	} while (1);
	fOut.close();
	fIn.close();
	return 0;
}
