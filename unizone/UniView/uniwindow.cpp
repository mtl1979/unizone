#include <qapplication.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qaccel.h>
#include <qobject.h>
#include <qstring.h>
#include <qlabel.h>

#include "uniwindow.h"
#include "debugimpl.h"
#include "../src/combo.h"
#include "../src/textevent.h"

UniWindow::UniWindow( QWidget* parent, const char* name, WFlags f)
: QMainWindow( parent, name, f)
{
	if (!name)
		setName ( "UniWindow" );
	setCaption("UniView");
	
	resize(800, 600);
	
	QToolBar * tbMenu = new QToolBar(this);
	CHECK_PTR(tbMenu);
	
	addToolBar(tbMenu, "Menu Bar", Top, false);
	
	QMenuBar * mbMenu = new QMenuBar(tbMenu, "Menu Bar");
	CHECK_PTR(mbMenu);
	
	tbMenu->setStretchableWidget( mbMenu );
	setDockEnabled( tbMenu, Left, FALSE );
	setDockEnabled( tbMenu, Right, FALSE );
	
	
	QToolBar * tbURL = new QToolBar(this);
	CHECK_PTR(tbURL);
	
	addToolBar(tbURL, "URL Bar", Top, true);
	
	QLabel * urlLabel = new QLabel(tbURL);
	CHECK_PTR(urlLabel);
	
	urlLabel->setText("URL: ");
	
	cmbURL = new WComboBox(this, tbURL, "URL Edit");
	CHECK_PTR(cmbURL);
	
	cmbURL->setEditable(true);
	cmbURL->setDuplicatesEnabled(false);
	cmbURL->setAutoCompletion(true);
	
	tbURL->setStretchableWidget( cmbURL );
	setDockEnabled( tbURL, Left, FALSE );
	setDockEnabled( tbURL, Right, FALSE );
	
	fileMenu = new QPopupMenu(mbMenu, "File Menu");
	CHECK_PTR(fileMenu);
	
	fileMenu->insertItem("E&xit", this, SLOT(Exit()), QAccel::stringToKey(tr("ALT+X")));
	
	navMenu = new QPopupMenu(mbMenu, "Navigation");
	CHECK_PTR(navMenu);
	
	navMenu->insertItem("Back", this, SLOT(Back()), 0, ID_BACK);
	navMenu->insertItem("Forward", this, SLOT(Forward()), 0, ID_FORWARD);
	navMenu->insertItem("Home", this, SLOT(Home()), 0, ID_HOME);
	
	mbMenu->insertItem("&File", fileMenu);
	mbMenu->insertItem("&Navigation", navMenu);
	
	fView = new WHTMLView(this);
	CHECK_PTR(fView);
	
	setCentralWidget(fView);
	
	connect(fView, SIGNAL(backwardAvailable(bool)), this, SLOT(backwardAvailable(bool)));
	connect(fView, SIGNAL(forwardAvailable(bool)), this, SLOT(forwardAvailable(bool)));
	connect(fView, SIGNAL(URLClicked(const QString &)), this, SLOT(linkClicked(const QString &)));
#ifdef _DEBUG
	fView->mimeSourceFactory()->setFilePath( "../../doc" );
#else
	fView->mimeSourceFactory()->setFilePath( "./doc" );
#endif
	fView->setSource("index.html");
}

UniWindow::~UniWindow()
{
	CleanupDebug();
}

void
UniWindow::Exit()
{
	qApp->exit(0);
}

void
UniWindow::Back()
{
	fView->backward();
}

void
UniWindow::Forward()
{
	fView->forward();
}

void
UniWindow::Home()
{
	fView->home();
}

void 
UniWindow::backwardAvailable( bool available )
{
	navMenu->setItemEnabled(ID_BACK, available);
}

void 
UniWindow::forwardAvailable( bool available )
{
	navMenu->setItemEnabled(ID_FORWARD, available);
}

void 
UniWindow::linkClicked(const QString & link)
{
	PRINT("linkClicked: %s\n", link.latin1());
	if (cmbURL->currentText() != link) 
		cmbURL->setEditText(link);
}

void 
UniWindow::customEvent(QCustomEvent * event)
{
	switch (event->type())
	{
	case WTextEvent::ComboType:
		{
			WTextEvent * wte = dynamic_cast<WTextEvent *>(event);
			if (wte)
				PRINT("Source: %s\n", wte->Text().latin1());
			fView->setSource(wte->Text());
		}
	}
}