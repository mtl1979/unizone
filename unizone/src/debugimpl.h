#ifndef DEBUGIMPL_H
#define DEBUGIMPL_H


#ifdef _DEBUG

#include <qmultilineedit.h>
#include <qthread.h>
#include <qapp.h>

#include <stdio.h>
#include <stdlib.h>

#define PRINT printf

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

#endif


#if defined(_DEBUG) || defined(BETA)

#include <qmessagebox.h>
#define WASSERT(X, Y) \
		if (!(X)) \
		{ \
			QString out = QObject::tr("Send this message to postmaster@raasu.org! This message has also been dumped to 'assert.txt'." \
										"\n\n%1\n\nLine %2\nFile %3\nDate: %4").arg(Y).arg(__LINE__).arg(__FILE__).arg(__DATE__); \
			QFile f("assert.txt"); \
			if (f.open(IO_WriteOnly)) \
			{ \
				f.writeBlock(out.latin1(), out.length()); \
				f.close(); \
			} \
			QMessageBox box(tr(NAME), out.latin1(),QMessageBox::Information, QMessageBox::Ok | QMessageBox::Default, \
					QMessageBox::NoButton, QMessageBox::NoButton); \
			box.exec(); \
		}
#else
#define WASSERT(X, Y)
#endif

#endif
