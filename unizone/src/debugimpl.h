#ifndef DEBUGIMPL_H
#define DEBUGIMPL_H

// Make sure _DEBUG is defined if DEBUG2 is
#ifdef DEBUG2
# ifndef _DEBUG
#  define _DEBUG
# endif
#endif

#ifdef _DEBUG

#include <qmultilineedit.h>
#include <qthread.h>
#include <qapp.h>
#include <qmessagebox.h>

#include <stdio.h>
#include <stdlib.h>

#define PRINT printf

#define POPUP(X) \
{ \
	QMessageBox box(QObject::tr( "Unizone (English)" ), tr(X), QMessageBox::Information, QMessageBox::Ok | QMessageBox::Default, \
					QMessageBox::NoButton, QMessageBox::NoButton); \
	box.exec(); \
}

# ifdef WIN32
inline void
RedirectDebugOutput()
{
	FILE * nfp = NULL;
	nfp = freopen("stdout.txt", "w", stdout);
	if (!nfp)
	{
#if !defined(stdout)
		stdout = fopen("stdout.txt", "w");
#else
		nfp = fopen("stdout.txt", "w");
		if (nfp)
			*stdout = *nfp;
#endif
	}
	setbuf(nfp, NULL);
}

inline void
CleanupDebug()
{
	fclose(stdout);
}

# else
#  define RedirectDebugOutput()
#  define CleanupDebug()
# endif

#else

// Empty inline to eliminate debug output
inline void PRINT(const char *, ...)
{
}
# define RedirectDebugOutput()
# define CleanupDebug()
# define POPUP(X)

#endif


#if defined(_DEBUG) || defined(BETA)

#include <qmessagebox.h>
#include <qstring.h>
#include <qfile.h>

#define WASSERT(X, Y) \
		if (!(X)) \
		{ \
			QString out = QObject::tr("Send this message to postmaster@raasu.org! This message has also been dumped to 'assert.txt'." \
			"\n\n%1\n\nLine %2\nFile %3\nDate: %4").arg(QObject::tr(Y)).arg(__LINE__).arg(__FILE__).arg(__DATE__); \
			QFile f("assert.txt"); \
			if (f.open(IO_WriteOnly)) \
			{ \
				f.writeBlock(out.local8Bit(), out.length()); \
				f.close(); \
			} \
			QMessageBox box(QObject::tr( "Unizone (English)" ), out,QMessageBox::Information, QMessageBox::Ok | QMessageBox::Default, \
					QMessageBox::NoButton, QMessageBox::NoButton); \
			box.exec(); \
		}
#else
#define WASSERT(X, Y)
#endif

#endif
