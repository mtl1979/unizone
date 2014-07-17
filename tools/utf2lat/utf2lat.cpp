// utf2lat.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <QByteArray>
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qtextcodec.h>
#include <qcoreapplication.h>

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	if (app.arguments().count() != 3)
	{
		printf("Usage: %S input output\n", app.arguments().at(0).utf16());
		return -1;
	}

	QFile fIn(app.arguments().at(1)), fOut(app.arguments().at(2));
	if (fIn.open(QIODevice::ReadOnly) == false)
	{
		printf("Error opening input file %S!",app.arguments().at(1).utf16());
		return -1;
	}
	if (fOut.open(QIODevice::WriteOnly) == false)
	{
		printf("Error opening output file %S!",app.arguments().at(2).utf16());
		fIn.close();
		return -1;
	}
	
	char qTemp1[256];
	QString qTemp2("");
	QByteArray qTemp3("");
 	QTextCodec * ic = QTextCodec::codecForName( "UTF-8" );
	QTextCodec * oc = QTextCodec::codecForName( "ISO-8859-1" );
	do
	{
		int bytes = fIn.readLine(qTemp1, 255);
		if (bytes == -1)
			break;
		qTemp2 = ic->toUnicode(qTemp1);
		qTemp3 = oc->fromUnicode(qTemp2);
		fOut.write(qTemp3, qTemp3.length());
	} while (1);
	fOut.close();
	fIn.close();
	return 0;
}
