#include "menubar.h"
#include "version.h"

#include <qapplication.h>
#include <qaccel.h>

MenuBar::MenuBar(QWidget * parent) : QMenuBar(parent)
{
	//fBar = new QMenuBar(parent);
	//CHECK_PTR(fBar);

	/* create file menu */
	fFile = new QPopupMenu(this);
	CHECK_PTR(fFile);
	fFile->insertItem(tr("&Connect"), parent, SLOT(Connect()), QAccel::stringToKey(tr("CTRL+SHIFT+C")));
	fFile->insertItem(tr("&Disconnect"), parent, SLOT(Disconnect()), QAccel::stringToKey(tr("CTRL+SHIFT+D")));
	fFile->insertSeparator();
	fFile->insertItem(tr("Open &Shared Folder"), parent, SLOT(OpenSharedFolder()), QAccel::stringToKey(tr("CTRL+S")));
	fFile->insertItem(tr("Open &Downloads Folder"), parent, SLOT(OpenDownloadsFolder()), QAccel::stringToKey(tr("CTRL+D")));
	fFile->insertItem(tr("Open &Logs Folder"), parent, SLOT(OpenLogsFolder()), QAccel::stringToKey(tr("CTRL+L")));
	fFile->insertSeparator();
	fFile->insertItem(tr("Cl&ear Chat Log"), parent, SLOT(ClearChatLog()), QAccel::stringToKey(tr("CTRL+E")));
	fFile->insertSeparator();
	fFile->insertItem(tr("Search"), parent, SLOT(SearchDialog()), QAccel::stringToKey(tr("ALT+S")));
	fFile->insertSeparator();

	fFile->insertItem(tr("E&xit"), parent, SLOT(Exit()), QAccel::stringToKey(tr("ALT+X")));

	// edit menu
	fEdit = new QPopupMenu(this);
	CHECK_PTR(fEdit);

	fEdit->insertItem(tr("&Preferences"), parent, SLOT(Preferences()), QAccel::stringToKey(tr("CTRL+P")));


	// windows menu

	fWindows = new QPopupMenu(this);
	CHECK_PTR(fWindows);

	fWindows->insertItem(tr("C&hannels"), parent, SLOT(OpenChannels()), QAccel::stringToKey(tr("F10")));
	fWindows->insertItem(tr("&Downloads"), parent, SLOT(OpenDownloads()), QAccel::stringToKey(tr("F11")));

	// help menu
	fHelp = new QPopupMenu(this);
	CHECK_PTR(fHelp);

	QString about = tr( "&About Unizone (English) %1" ).arg(WinShareVersionString());

	fHelp->insertItem(about, parent, SLOT(AboutWinShare()), QAccel::stringToKey(tr("F12")));

	/* Insert into menubar */
	insertItem(tr("&File"), fFile);
	insertItem(tr("&Edit"), fEdit);
	insertItem(tr("&Window"), fWindows);
	insertItem(tr("&Help"), fHelp);
}

MenuBar::~MenuBar()
{
}

