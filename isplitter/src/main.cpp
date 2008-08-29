#include "uenv.h"
#include "mainwindowimpl.h"
#include "debugimpl.h"

#include <qapplication.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3CString>

#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#endif

#include <QTranslator>

#ifdef WIN32
# if !defined(QT_NO_STYLE_WINDOWSXP)
#  include <qwindowsxpstyle.h>
# endif
#endif

bool
endsWith(const QString &str1, const QString &str2, bool cs = true)
{
	return str1.endsWith(str2, cs ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

QString MakePath(const QString &dir, const QString &file)
{
	QString ret = QDir::convertSeparators(dir);
	if (!endsWith(ret, QChar(QDir::separator())))
		ret += QDir::separator();

	ret += file;

	return ret;
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
	Q_CHECK_PTR(name);
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
	QApplication app( argc, argv );
	QTranslator qtr( 0 );
	QTranslator qtr2( 0 );

	// Set our working directory

#ifndef WIN32
	SetWorkingDirectory(argv[0]);
#else
	SetWorkingDirectory();
#endif

	// Load language file
	QFile lang("isplitter.lng");
	QString lfile;
	if (!lang.exists())
	{
		lfile = Q3FileDialog::getOpenFileName(QString::null, "isplitter_*.qm");
		if (!lfile.isEmpty())
		{
			// Save selected language's translator filename
			if ( lang.open(QIODevice::WriteOnly) )
			{
				Q3CString clang = lfile.utf8();
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

#ifdef WIN32
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
