#include "mainwindowimpl.h"

#include <qapplication.h>
#include <qfile.h>
#include <qfiledialog.h>

#if !defined(QT_NO_STYLE_PLATINUM)
# include <qplatinumstyle.h>
#endif

int 
main( int argc, char** argv )
{
	QApplication app( argc, argv );

	// Load language file
	QTranslator qtr( 0 );
	QFile lang("isplitter.lng");
	QString lfile;
	if (!lang.exists())
	{
		lfile = QFileDialog::getOpenFileName( QString::null, "isplitter_*.qm", NULL );
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
		char plang[255];
		lang.readLine(plang, 255);
		lfile = QString::fromUtf8(plang);
		lang.close();
    }

	// Install translator ;)
	if (!lfile.isEmpty())
	{
		qtr.load(lfile);
		app.installTranslator( &qtr );
	}
	
#if !defined(QT_NO_STYLE_PLATINUM)
	// Set style
	app.setStyle(new QPlatinumStyle);
#endif

	ImageSplitter * window = new ImageSplitter(NULL);
	CHECK_PTR(window);

	app.setMainWidget(window);

	window->show();

	return app.exec();
}
