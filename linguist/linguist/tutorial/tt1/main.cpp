/****************************************************************
**
** Translation tutorial 1
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qtranslator.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QTranslator tor( 0 );
    tor.load( "tt1_la", "." );
    a.installTranslator( &tor );

    QPushButton hello( QPushButton::tr("Hello world!"), 0 );

    a.setMainWidget( &hello );
    hello.show();
    return a.exec();
}
