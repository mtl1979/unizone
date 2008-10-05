#include "uenv.h"
#include "mainwindowimpl.h"
#include "debugimpl.h"
#include "util.h"

#include <qapplication.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qregexp.h>
#include <QByteArray>
#include <QTranslator>


#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
# if !defined(QT_NO_STYLE_WINDOWSXP)
#  include <qwindowsxpstyle.h>
# endif
#endif

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
			chdir(chd);
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
	QApplication app( argc, argv );
	QTranslator qtr( 0 );
	QTranslator qtr2( 0 );

	// Set our working directory

#ifndef _WIN32
	SetWorkingDirectory(argv[0]);
#else
	QString appdir = GetAppDirectory();
	QString datadir = EnvironmentVariable("APPDATA");
	QDir dir(datadir);
	dir.mkdir("Image Splitter");
	datadir = MakePath(datadir, "Image Splitter");
	QDir::setCurrent(datadir);
#endif

	// Load language file
	QString langfile;
#ifdef _WIN32
	langfile = MakePath(datadir, "isplitter.lng");
#else
	langfile = "isplitter.lng");
#endif
	QFile lang(langfile);
	QString lfile;
	if (!lang.exists())
	{
		lfile = QFileDialog::getOpenFileName(NULL, app.translate("main", "Open translation file..."), 
#ifdef _WIN32
			MakePath(appdir, "translations"),
#else
			QString::null,
#endif		
			"isplitter_*.qm");
		if (!lfile.isEmpty())
		{
			// Save selected language's translator filename
			if ( lang.open(QIODevice::WriteOnly) )
			{
				QByteArray clang = lfile.utf8();
				lang.writeBlock(clang, clang.length());
				lang.close();
			}
		}
	}

	// (Re-)load translator filename
	if ( lang.open(QIODevice::ReadOnly) )
	{
		// file opened successfully
		char plang[255];
		lang.readLine(plang, 255);
		lfile = QString::fromUtf8(plang);
		lang.close();
    }


	// Install translator ;)
	if (!lfile.isEmpty())
	{
		if (QFile::exists(lfile))
		{
			if (qtr.load(lfile))
			{
				app.installTranslator( &qtr );
			}
		}
		// Qt's own translator file
		QFileInfo qfi(lfile);
		QString langfile = qfi.fileName().replace(QRegExp("isplitter"), "qt");
		QString qt_lang = QString::null;
		QString qtdir = EnvironmentVariable("QTDIR");
		if (qtdir != QString::null)
		{
			QString tr_dir = MakePath(qtdir, "translations");
			qt_lang = MakePath(tr_dir, langfile);
			if (!QFile::exists(qt_lang))
				qt_lang = QString::null;
		}
		if (qt_lang == QString::null)
		{
			// Try using same directory as Image Splitters translations
			qt_lang = MakePath(qfi.dirPath(true), langfile);
		}

		if (QFile::exists(qt_lang))
		{
			if (qtr2.load(qt_lang))
			{
				app.installTranslator( &qtr2 );
			}
		}
	}

#ifdef _WIN32
# if !defined(QT_NO_STYLE_WINDOWSXP)
	// Set style
	app.setStyle(new QWindowsXPStyle);
# endif
#endif
	ImageSplitter * window = new ImageSplitter(NULL);
	Q_CHECK_PTR(window);

	app.setMainWidget(window);

	window->show();

	if (argc > 0)
		window->Load(QString::fromLocal8Bit(argv[1]));

	return app.exec();
}
