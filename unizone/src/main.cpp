#include <qapplication.h>
#include <qplatinumstyle.h>
#include "global.h"
#include "debugimpl.h"
#include "lang.h"				// <postmaster@raasu.org> 20020924
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qmessagebox.h>
#include <qfont.h>
#ifdef ALTCHARSET
#include <qtextcodec.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#endif
#undef POPUP
#define POPUP(X) \
{ \
	QMessageBox box(tr(NAME), X, QMessageBox::Information, QMessageBox::Ok | QMessageBox::Default, \
					QMessageBox::NoButton, QMessageBox::NoButton); \
	box.exec(); \
}

int 
main( int argc, char** argv )
{
	QApplication app( argc, argv );

#ifdef ALTCHARSET
	app.setDefaultCodec( QTextCodec::codecForName(ALTCHARSET) );
#endif

	// first set our working directory (linux only... Windows already does it for us :))

	if (argc > 1)
	{
		SetSettingsFile(argv[1]);
	}

#ifndef WIN32
	const char * wdir = strrchr(argv[0], '/');

	if (wdir)
	{
		char * chd = new char[wdir - argv[0] + 1]; // figure out length, and make a new string of that length
		if (chd)
		{
			strncpy(chd, argv[0], wdir - argv[0]);
			chd[wdir - argv[0]] = 0;
			printf("Setting working directory to: %s\n", chd);
			chdir(chd);
			delete [] chd;
		}
	}
#else
	// we have to use some windows api to get our path...
	// <postmaster@raasu.org> 20021022 -- use wchar_t instead of TCHAR to follow common typedef across source files
	wchar_t * name = new wchar_t[MAX_PATH];	// maximum size for Win32 filenames
	if (GetModuleFileName(NULL,				/* current apps module */
							name,			/* buffer */
							MAX_PATH		/* buffer length */
							) != 0)
	{
		PRINT("Module filename: %S\n", name);
		PathRemoveFileSpec(name);
		PRINT("Setting working directory to: %S\n", name);
		SetCurrentDirectory(name);
	}
	delete [] name;
	name = NULL; // <postmaster@raasu.org> 20021027
#endif
	app.setStyle(new QPlatinumStyle);

	WinShareWindow * window = new WinShareWindow(NULL, "Unizone");
	app.setMainWidget(window);

	window->show();

	return app.exec();
}
