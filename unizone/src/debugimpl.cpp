#include "debugimpl.h"

#if !defined(_DEBUG) && !defined(BETA)
#error "Only include for debug build!"
#endif

#include <qapplication.h>
#include <q3multilineedit.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <QByteArray>
#include "util.h"
#include "wfile.h"
#include "wstring.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _DEBUG

FILE * nfp = NULL;

void PRINT(const char *fmt, ...)
{
	if (nfp)
	{
		va_list a;
		va_start(a, fmt);
		vfprintf(nfp, fmt, a);
	}
}

#ifdef _WIN32
#include <tchar.h>
extern QString gDataDir; // main.cpp
#endif

void
RedirectDebugOutput()
{
#ifdef _WIN32
	WString logfile = MakePath(gDataDir, "stdout.txt");
	nfp = _tfsopen(logfile, _T("w"), _SH_DENYWR);
#else // _WIN32
	nfp = fopen("stdout.txt", "w");
#endif
	if (nfp)
#if __STDC_WANT_SECURE_LIB__
		setvbuf(nfp, NULL, _IOLBF, 1024);
#else
		setbuf(nfp, NULL);
#endif
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
	(void) QMessageBox::information(NULL, qApp->translate( "Debug", "Unizone (English)" ),
	                                msg, QMessageBox::Ok | QMessageBox::Default,
	                                QMessageBox::NoButton, QMessageBox::NoButton);
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
		QString out = qApp->translate("Debug",
			"Send this message to postmaster@raasu.org! "
		    "This message has also been dumped to 'assert.txt'."
			"\n\n%1\n\nLine %2\nFile %3\nDate: %4").arg(qApp->translate("Debug",
				message)).arg(line).arg(file).arg(date);
		WFile f;
		if (f.Open(L"assert.txt", QIODevice::WriteOnly))
		{
			QByteArray tmp = out.utf8();
			f.WriteBlock(tmp, tmp.length());
			f.Close();
		}
		WPopup(out);
	}
}

#ifdef _DEBUG

void
CheckSize(int64 size)
{
	if (size > LONG_MAX)
	{
		WPopup( qApp->translate( "Debug",
			"Attempt to read file larger than maximum allocatable memory amount!") );
	}
}

#endif
