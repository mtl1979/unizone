#include <qapplication.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfont.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#endif

#include "global.h"
#include "debugimpl.h"
#include "winsharewindow.h"
#include "wfile.h"
#include "wstring.h"
#include "util.h"
#include "uenv.h"

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
#elif defined(UNICODE)
SetWorkingDirectory()
{
	// we have to use some windows api to get our path...
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
}
#else
SetWorkingDirectory()
{
	// we have to use some windows api to get our path...
	char * name = new char[MAX_PATH];	// maximum size for Win32 filenames
	CHECK_PTR(name);
	if (GetModuleFileName(NULL,				/* current apps module */
							name,			/* buffer */
							MAX_PATH		/* buffer length */
							) != 0)
	{
		PRINT("Module filename: %s\n", name);
		PathRemoveFileSpec(name);
		PRINT("Setting working directory to: %s\n", name);
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
	QTranslator qtr( 0 );
	QTranslator qtr2( 0 );

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
	WFile lang;
	QString lfile;
	if (!WFile::Exists(L"unizone.lng"))
	{
		lfile = QFileDialog::getOpenFileName( QString::null, "unizone_*.qm", NULL );
		if (!lfile.isEmpty())
		{
			// Save selected language's translator filename
			if ( lang.Open(L"unizone.lng", 
#ifdef WIN32
				O_WRONLY | O_CREAT | O_BINARY
#else
				O_WRONLY | O_CREAT
#endif 
			   ) )
			{
				QCString clang = lfile.utf8();
				lang.WriteBlock(clang, clang.length());
				lang.Close();
			}
		}
	}

	// (Re-)load translator filename
	if ( lang.Open(L"unizone.lng",
#if defined(WIN32)
		O_RDONLY | O_BINARY
#else
		O_RDONLY
#endif
		) )
	{    
		// file opened successfully
		QByteArray plang(256);
		lang.ReadLine(plang.data(), 255);
		lfile = QString::fromUtf8(plang.data());
		lang.Close();
    }

	// Install translator ;)
	if (!lfile.isEmpty())
	{
		if (WFile::Exists(lfile))
		{
			if (qtr.load(lfile))
			{
#ifdef DEBUG
				WString wfile(QDir::convertSeparators(lfile));
				PRINT("Loaded translation %S\n", wfile.getBuffer());
#endif
				app.installTranslator( &qtr );
			}
		}
		// Qt's own translator file
		QFileInfo qfi(lfile);
		QString langfile = qfi.fileName().replace(QRegExp("unizone"), "qt");
		QString qt_lang = QString::null;
		QString qtdir = EnvironmentVariable("QTDIR");
		if (qtdir != QString::null)
		{
			QString tr_dir = MakePath(qtdir, "translations");
			qt_lang = MakePath(tr_dir, langfile);
			if (!WFile::Exists(qt_lang))
				qt_lang = QString::null;
		}
		if (qt_lang == QString::null)
		{
			// Try using same directory as Unizones translations
			qt_lang = MakePath(qfi.dirPath(true), langfile);
		}

		if (!qt_lang.isEmpty())
		{
			if (WFile::Exists(qt_lang))
			{
				if (qtr2.load(qt_lang))
				{
#ifdef DEBUG
					WString wfile(QDir::convertSeparators(qt_lang));
					PRINT("Loaded translation %S\n", wfile.getBuffer());
#endif
					app.installTranslator( &qtr2 );
				}
			}
		}
	}
	
	WinShareWindow * window = new WinShareWindow(NULL);
	CHECK_PTR(window);

	app.setMainWidget(window);

	window->show();

	int ret = app.exec();

	CleanupDebug();

	return ret;
}
