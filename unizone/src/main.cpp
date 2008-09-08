#include <qapplication.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <qmessagebox.h>
#include <qfont.h>
#include <Q3CString>
#include <QTranslator>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#if defined(WIN32) || defined(_WIN32)
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

QString gAppDir;
#ifdef _WIN32
QString gDataDir;
#endif

int64 GetStartTime()
{
	return fStartTime;
}

#ifndef _WIN32
void
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
			if (chdir(chd) != 0)
				PRINT("Setting working directory failed!\n");
			delete [] chd;
		}
	}
}
#else
QString
GetAppDirectory()
{
	// we have to use some windows api to get our path...
	wchar_t * name = new wchar_t[MAX_PATH];	// maximum size for Win32 filenames
	Q_CHECK_PTR(name);
	if (GetModuleFileName(NULL,				/* current apps module */
							name,			/* buffer */
							MAX_PATH		/* buffer length */
							) != 0)
	{
		qDebug("Module filename: %S", name);
		PathRemoveFileSpec(name);
		if (SetCurrentDirectory(name) == 0)
		{
			GetCurrentDirectory(MAX_PATH, name);
			qDebug("Current directory: %S", name);
		}
		else
			qDebug("Application directory: %S", name);
	}
	QString qname = QString::fromUcs2((const ushort *) name);
	delete [] name;
	name = NULL; // <postmaster@raasu.org> 20021027
	return qname;
}
#endif

int 
main( int argc, char** argv )
{
#ifdef _WIN32
	QString datadir = EnvironmentVariable("APPDATA");
	QDir(datadir).mkdir("Unizone");
	datadir = MakePath(datadir, "Unizone");
	gDataDir = datadir;
	gAppDir = GetAppDirectory();
	// Set our working directory
	QDir::setCurrent(gDataDir);
#endif
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

	WString wlangfile;
#ifndef _WIN32
	wlangfile = L"unizone.lng";
	// Set our working directory
	SetWorkingDirectory(argv[0]);
	gAppDir = QDir::currentDirPath();
#else
	QString datafile = MakePath(gDataDir, "unizone.lng");	
	wlangfile = datafile;
   
# ifdef _DEBUG
	WString wDataDir(gDataDir);
	PRINT("Data directory: %S\n", wDataDir.getBuffer());
# endif
#endif

	// Load language file
	WFile lang;
	QString lfile;
	if (!WFile::Exists(wlangfile))
	{
		lfile = Q3FileDialog::getOpenFileName( 
#ifdef _WIN32
			MakePath(gAppDir, "translations"),
#else
			gAppDir,
#endif
			"unizone_*.qm", NULL );
		if (!lfile.isEmpty())
		{
			// Save selected language's translator filename
			if ( lang.Open(wlangfile, 
#if defined(WIN32) || defined(_WIN32)
				O_WRONLY | O_CREAT | O_BINARY
#else
				O_WRONLY | O_CREAT
#endif 
			   ) )
			{
				Q3CString clang = lfile.utf8();
				lang.WriteBlock(clang, clang.length());
				lang.Close();
			}
		}
	}

	// (Re-)load translator filename
	if ( lang.Open(wlangfile,
#if defined(WIN32) || defined(_WIN32)
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
#ifdef _DEBUG
				WString wfile(QDir::convertSeparators(lfile));
				PRINT("Loaded translation: %S\n", wfile.getBuffer());
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
#ifdef _DEBUG
					WString wfile(QDir::convertSeparators(qt_lang));
					PRINT("Loaded translation: %S\n", wfile.getBuffer());
#endif
					app.installTranslator( &qtr2 );
				}
			}
		}
	}
	
	WinShareWindow * window = new WinShareWindow(NULL);
	Q_CHECK_PTR(window);

	app.setMainWidget(window);

	window->show();

	int ret = app.exec();

	CleanupDebug();

	return ret;
}
