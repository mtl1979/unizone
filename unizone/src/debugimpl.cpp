#include "debugimpl.h"

#if !defined(_DEBUG) && !defined(BETA)
#error "Only include for debug build!"
#endif

#include <qapplication.h>
#include <qmultilineedit.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <qfile.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

FILE * nfp = NULL;

#ifdef _DEBUG
void PRINT(const char *fmt, ...)
{
	if (nfp)
	{
		va_list a;
		va_start(a, fmt);
		vfprintf(nfp, fmt, a);
	}
}

void
RedirectDebugOutput()
{
	nfp = fopen("stdout.txt", "w");
	setbuf(nfp, NULL);
}

void
CleanupDebug()
{
	if (nfp)
		fclose(nfp);
}
#endif

void WPopup(const QString &msg)
{
	(void) QMessageBox::information(NULL, qApp->translate( "Debug", "Unizone (English)" ), msg, QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton); 
}

void WPopup(const char *msg)
{ 
	WPopup(qApp->translate("Debug", msg));
}

void
WAssert(bool test, const char *message, int line, const char *file, const char *date)
{
	if (!test) 
	{ 
		QString out = qApp->translate("Debug", "Send this message to postmaster@raasu.org! This message has also been dumped to 'assert.txt'." 
			"\n\n%1\n\nLine %2\nFile %3\nDate: %4").arg(qApp->translate("Debug", message)).arg(line).arg(file).arg(date); 
		QFile f("assert.txt");
		if (f.open(IO_WriteOnly)) 
		{ 
			QCString tmp = out.local8Bit();
			f.writeBlock(tmp, tmp.length()); 
			f.close(); 
		} 
		WPopup(out);
	}
}
