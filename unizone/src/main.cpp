#include <qapplication.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfont.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>
#endif

#include "global.h"
#include "debugimpl.h"
#include "winsharewindow.h"

#include "util/TimeUtilityFunctions.h"
#include "system/SetupSystem.h"

int64 fStartTime;

int64 GetStartTime()
{
	return fStartTime;
}

void
#ifndef WIN32
SetWorkingDirectory(const char *app)
{
	const char * wdir = strrchr(app, '/');

	if (wdir)
	{
		char * chd = new char[wdir - app + 1]; // figure out length, and make a new string of that length
		if (chd)
		{
			strncpy(chd, app, wdir - app);
			chd[wdir - app] = 0;
			PRINT("Setting working directory to: %s\n", chd);
			chdir(chd);
			delete [] chd;
		}
	}
}
#else
SetWorkingDirectory()
{
	// we have to use some windows api to get our path...
	TCHAR * name = new TCHAR[MAX_PATH];	// maximum size for Win32 filenames
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
}
#endif

int 
main( int argc, char** argv )
{
	RedirectDebugOutput();
	muscle::CompleteSetupSystem fMuscle;

	fStartTime = GetCurrentTime64();

	QApplication app( argc, argv );

	// Set alternative settings file if requested

	int a = 1;
	while (a < argc)
	{
		if (strcmp(argv[a], "--settings") == 0)
		{
			a++;
			SetSettingsFile(argv[a]);
		}
		else if (strcmp(argv[a], "--font") == 0)
		{
			a++;
			int fs = atoi(argv[a]);
			QFont font = app.font();
			font.setPointSize(fs);
			app.setFont(font);
		}

		a++;
	}

	// Set our working directory

#ifndef WIN32
	SetWorkingDirectory(argv[0]);
#else
	SetWorkingDirectory();
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
		QByteArray plang(256);
		lang.readLine(plang.data(), 255);
		lfile = QString::fromUtf8(plang);
		lang.close();
    }

	// Install translator ;)
	if (!lfile.isEmpty())
	{
		qtr.load(lfile);
		app.installTranslator( &qtr );
	}
	
	WinShareWindow * window = new WinShareWindow(NULL);
	CHECK_PTR(window);

	app.setMainWidget(window);

	window->show();

	int ret = app.exec();

	CleanupDebug();

	return ret;
}
