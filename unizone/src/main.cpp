#include "winsharewindow.h"
#include "util/TimeUtilityFunctions.h"

#include <qapplication.h>
#include <qplatinumstyle.h>
#include <qfile.h>
#include <qfiledialog.h>
#include "global.h"
#include "debugimpl.h"
#include "version.h"
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

int64 fStartTime;

int64 GetStartTime()
{
	return fStartTime;
}


QString
WinShareVersionString()
{
	QString version = QObject::tr("%1.%2.%3 build %4").arg(kMajor).arg(kMinor).arg(kPatch).arg(kBuild);
	return version;
}

int 
main( int argc, char** argv )
{
	fStartTime = GetCurrentTime64();
	QApplication app( argc, argv );

	RedirectDebugOutput();

	// Set alternative settings file if requested

	if (argc > 1)
	{
		SetSettingsFile(argv[1]);
	}

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
			PRINT("Setting working directory to: %s\n", chd);
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
		PRINT("Module filename: %S\n", name);
		PathRemoveFileSpec(name);
		PRINT("Setting working directory to: %S\n", name);
		SetCurrentDirectory(name);
	}
	delete [] name;
	name = NULL; // <postmaster@raasu.org> 20021027
#endif
	// Load language file
	QTranslator qtr( 0 );
	QFile lang("unizone.lng");
	QString lfile;
	if (!lang.exists())
	{
		lfile = QFileDialog::getOpenFileName( QString::null, "unizone_*.qm", NULL );
		if (!lfile.isEmpty())
		{
			// Save selected language's translator filename
			if ( lang.open(IO_WriteOnly) )
			{
				QCString clang = lfile.utf8();
				lang.writeBlock(clang, clang.length());
				lang.close();
			}
		}
	}
	// (Re-)load translator filename
	if ( lang.open(IO_ReadOnly) ) 
	{    
		// file opened successfully
		char * plang = (char *) malloc(255);
		if (plang)
		{
			lang.readLine(plang, 255);
			lfile = QString::fromUtf8(plang);
			delete plang;
		}
		lang.close();
    }
	// Install translator ;)
	if (!lfile.isEmpty())
	{
		qtr.load(lfile);
		app.installTranslator( &qtr );
	}
	
	// Set style
	app.setStyle(new QPlatinumStyle);

	WinShareWindow * window = new WinShareWindow(NULL);
	CHECK_PTR(window);

	app.setMainWidget(window);

	window->show();

	return app.exec();
}
