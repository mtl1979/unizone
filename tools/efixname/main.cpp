#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qapplication.h>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	if (argc != 2)
	{
		printf("Usage: %s <input file>\n", argv[0]);
		return -1;
	}
	else
	{
		QTextCodec *ec = QTextCodec::codecForName("EUC-KR");
		QString out = ec->toUnicode(argv[1], strlen(argv[1]));
		QCString test = ec->fromUnicode(out);
		if (test != argv[1])
		{
			printf("Decoder error!\n");
			return -1;
		}
		QString in = argv[1];
		uint sz = 0;
		char buf[1024];
		if (in != out)
		{
			QFile ifile(in);
			QFile ofile(out);
		

		if (ifile.open(IO_ReadOnly) == false)
		{
			printf("Error opening input file!");
			return -1;
		}
		
		sz = ifile.size();

		if (ofile.open(IO_WriteOnly) == false)
		{
			printf("Error opening output file!");
			ifile.close();
			return -1;
		}
		int r;
		__int64 numbytes = 0;
		while ((r = ifile.readBlock(buf, 1024)) > 0)
		{
			ofile.writeBlock(buf, r);
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
