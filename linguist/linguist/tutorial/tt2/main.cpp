/****************************************************************
**
** Translation tutorial 2
**
****************************************************************/

#include "mainwindow.h"

#include <qapplication.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QTranslator tor( 0 );
    tor.load( QString("tt2_") + QTextCodec::locale(), "." );
    a.installTranslator( &tor );

    MainWindow *mw = new MainWindow;

    a.setMainWidget( mw );
    mw->show();
    return a.exec();
}
