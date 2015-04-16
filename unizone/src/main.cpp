#include <qapplication.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <qmessagebox.h>
#include <qfont.h>
#include <QByteArray>
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

#ifdef MUSCLE_ENABLE_SSL
#  ifdef UNICODE
#    include "wchar.h"
#  endif
#include "dataio/FileDataIO.h"
#endif

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
	TCHAR * name = new TCHAR[MAX_PATH];	// maximum size for Win32 filenames
	Q_CHECK_PTR(name);
	if (GetModuleFileName(NULL,				/* current apps module */
							name,			/* buffer */
							MAX_PATH		/* buffer length */
							) != 0)
	{
#ifdef UNICODE
		qDebug("Module filename: %ls", name);
#else
		qDebug("Module filename; %s", name);
#endif
		PathRemoveFileSpec(name);
		if (SetCurrentDirectory(name) == 0)
		{
			GetCurrentDirectory(MAX_PATH, name);
#ifdef UNICODE
			qDebug("Current directory: %ls", name);
#else
			qDebug("Current directory: %s", name);
#endif
		}
		else
#ifdef UNICODE
			qDebug("Application directory: %ls", name);
#else
			qDebug("Application directory: %s", name);
#endif
	}
#ifdef UNICODE
	QString qname = QString::fromUtf16((const ushort *) name);
#else
	QString qname = QString::fromLocal8Bit(name);
#endif
	delete [] name;
	name = NULL; // <postmaster@raasu.org> 20021027
	return qname;
}
#endif

int
main( int argc, char** argv )
{
#ifdef _WIN32
#  ifdef MUSCLE_ENABLE_SSL
     TCHAR publicKeyFilePath[255];
#  endif

	QString datadir = EnvironmentVariable("APPDATA");
	QDir(datadir).mkdir("Unizone");
	datadir = MakePath(datadir, "Unizone");
	gDataDir = datadir;
	gAppDir = GetAppDirectory();
	// Set our working directory
	QDir::setCurrent(gDataDir);
#else
# ifdef MUSCLE_ENABLE_SSL
    char publicKeyFilePath[255];
# endif
#endif
	RedirectDebugOutput();
	muscle::CompleteSetupSystem fMuscle;

	fStartTime = GetCurrentTime64();

	QApplication app( argc, argv );
	QTranslator qtr( 0 );
	QTranslator qtr2( 0 );

	// Set alternative settings file if requested

	int a = 1;
	while (a < app.arguments().count())
	{
		if (app.arguments().at(a) == "--settings")
		{
			a++;
			SetSettingsFile(app.arguments().at(a));
		}
		else if (app.arguments().at(a) == "--font")
		{
			a++;
			int fs = app.arguments().at(a).toInt();
			QFont font = app.font();
			font.setPointSize(fs);
			app.setFont(font);
		}
#ifdef MUSCLE_ENABLE_SSL
		else if (app.arguments().at(a) == "--publickey")
		{
			a++;
#ifdef _WIN32
#  ifdef UNICODE
			wcscpy(publicKeyFilePath, app.arguments().at(a).utf16());
#  elif __STDC_WANT_SECURE_LIB__
			strcpy_s(publicKeyFilePath, app.arguments().at(a).local8Bit());
#  else
			strcpy(publicKeyFilePath, app.arguments().at(a).local8Bit());
#  endif
#else
			strcpy(publicKeyFilePath, app.arguments().at(a).utf8());
#endif
		}
#endif

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
				QByteArray clang = lfile.utf8();
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

	// Setup SSL
#ifdef MUSCLE_ENABLE_SSL
   ByteBufferRef optCryptoBuf;
#if defined(_WIN32)
#  if defined(UNICODE)
     FileDataIO fdio(_wfopen(publicKeyFilePath, L"rb"));
#  elif __STDC_WANT_SECURE_LIB__
     FILE *file;
     (void) fopen_s(&file, publicKeyFilePath, "rb");
     FileDataIO fdio(file);
#  else
     FileDataIO fdio(fopen(publicKeyFilePath, "rb"));
#  endif
#else
   FileDataIO fdio(fopen(publicKeyFilePath, "rb"));
#endif
   ByteBufferRef fileData = GetByteBufferFromPool((uint32)fdio.GetLength());
   if ((fdio.GetFile())&&(fileData())&&(fdio.ReadFully(fileData()->GetBuffer(), fileData()->GetNumBytes()) == fileData()->GetNumBytes()))
   {
#ifdef _DEBUG
# if defined(_WIN32) && defined(UNICODE)
      PRINT("Using public key file [%S] to authenticate with servers\n", publicKeyFilePath);
# else
      PRINT("Using public key file [%s] to authenticate with servers\n", publicKeyFilePath);
# endif
#endif
      window->SetSSLPublicKey(fileData);
   }
   else
   {
#ifdef _DEBUG
# if defined(_WIN32) && defined(UNICODE)
         PRINT("Couldn't load public key file [%S] (file not found?)\n", publicKeyFilePath);
# else
         PRINT("Couldn't load public key file [%s] (file not found?)\n", publicKeyFilePath);
# endif
#endif
   }
#endif


	window->show();

	int ret = app.exec();

	delete window;

	CleanupDebug();

	return ret;
}
