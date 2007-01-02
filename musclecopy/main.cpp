#include <qapplication.h>
#include <qplatinumstyle.h>
#include <qfiledialog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qmessagebox.h>
#include <qfont.h>

#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#endif
#include "statusimpl.h"

int 
main( int argc, char** argv )
{
	QApplication app( argc, argv );

	// Set our working directory

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
	CHECK_PTR(name);
	if (GetModuleFileName(NULL,				/* current apps module */
							name,			/* buffer */
							MAX_PATH		/* buffer length */
							) != 0)
	{
		PathRemoveFileSpec(name);
		SetCurrentDirectory(name);
	}
	delete [] name;
	name = NULL; // <postmaster@raasu.org> 20021027
#endif

	Status * status = new Status(NULL);
	CHECK_PTR(status);

	app.setMainWidget(status);

	status->show();

	return app.exec();
}
