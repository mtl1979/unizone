/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   main.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "trwindow.h"

#include <qapplication.h>
#include <qtimer.h>

#if defined(_WS_X11_)
extern void qt_wait_for_window_manager( QWidget * );
#endif

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QApplication::setOverrideCursor( Qt::waitCursor );
    QTimer timer;
    QWidget *splash = 0;
    bool showSplash = TRUE;

    if ( showSplash ) {
	timer.start( 2000, TRUE );
	splash = TrWindow::splash();
    }

    TrWindow *tw = new TrWindow;
    app.setMainWidget( tw );

    if ( app.argc() > 1 )
	tw->openFile( QString(app.argv()[app.argc() - 1]) );

    while ( timer.isActive() ) // evil loop
	app.processEvents();
    tw->show();
#if defined(_WS_X11_)
    qt_wait_for_window_manager( tw );
#endif
    delete splash;

    QApplication::restoreOverrideCursor();
    return app.exec();
}
