#include "menubar.h"
#include "version.h"

#include <qapplication.h>
#include <qaccel.h>

const char *appleItems[] = {
	QT_TRANSLATE_NOOP("QMenuBar", "Preference"),
        QT_TRANSLATE_NOOP("QMenuBar", "About"),
        NULL
};

MenuBar::MenuBar(QWidget * handler, QWidget * parent) : QMenuBar(parent)
{
	// create search... menu

	QPopupMenu *fSearch = new QPopupMenu(this);
	CHECK_PTR(fSearch);

	fSearch->insertItem(tr("Music"), handler, SLOT(SearchMusic()));
	fSearch->insertItem(tr("Videos"), handler, SLOT(SearchVideos()));
	fSearch->insertItem(tr("Pictures"), handler, SLOT(SearchPictures()));
	fSearch->insertItem(tr("Disk Images"), handler, SLOT(SearchImages()));

	// create file menu

	fFile = new QPopupMenu(this);
	CHECK_PTR(fFile);

	fFile->insertItem(tr("&Connect"), handler, SLOT(Connect()), QAccel::stringToKey(tr("CTRL+SHIFT+C")));
	fFile->insertItem(tr("&Disconnect"), handler, SLOT(Disconnect()), QAccel::stringToKey(tr("CTRL+SHIFT+D")));
	fFile->insertSeparator();
	fFile->insertItem(tr("Open &Shared Folder"), handler, SLOT(OpenSharedFolder()), QAccel::stringToKey(tr("CTRL+S")));
	fFile->insertItem(tr("Open &Downloads Folder"), handler, SLOT(OpenDownloadsFolder()), QAccel::stringToKey(tr("CTRL+D")));
	fFile->insertItem(tr("Open &Logs Folder"), handler, SLOT(OpenLogsFolder()), QAccel::stringToKey(tr("CTRL+L")));
	fFile->insertSeparator();
	fFile->insertItem(tr("&Search"), handler, SLOT(OpenSearch()), QAccel::stringToKey(tr("ALT+S")));
	fFile->insertItem(tr("Search..."), fSearch);
	fFile->insertSeparator();
	fFile->insertItem(tr("E&xit"), handler, SLOT(Exit()), QAccel::stringToKey(tr("ALT+X")));

	// edit menu
	fEdit = new QPopupMenu(this);
	CHECK_PTR(fEdit);

	fEdit->insertItem(tr("Cl&ear Chat Log"), handler, SLOT(ClearChatLog()), QAccel::stringToKey(tr("CTRL+E")));
	fEdit->insertSeparator();
	fEdit->insertItem(tr("&Preferences"), handler, SLOT(Preferences()), QAccel::stringToKey(tr("CTRL+P")));

	// windows menu

	fWindows = new QPopupMenu(this);
	CHECK_PTR(fWindows);

	fWindows->insertItem(tr("Picture Viewer"), handler, SLOT(OpenViewer()), QAccel::stringToKey(tr("F9")));
	fWindows->insertItem(tr("C&hannels"), handler, SLOT(OpenChannels()), QAccel::stringToKey(tr("F10")));
	fWindows->insertItem(tr("&Downloads"), handler, SLOT(OpenDownloads()), QAccel::stringToKey(tr("F11")));

	// help menu
	fHelp = new QPopupMenu(this);
	CHECK_PTR(fHelp);

	QString about = tr( "&About Unizone (English) %1" ).arg(WinShareVersionString());

	fHelp->insertItem(about, handler, SLOT(AboutWinShare()), QAccel::stringToKey(tr("F12")));

	/* Insert into menubar */
	insertItem(tr("&File"), fFile);
	insertItem(tr("&Edit"), fEdit);
	insertItem(tr("&Window"), fWindows);
	insertItem(tr("&Help"), fHelp);
}

MenuBar::~MenuBar()
{
}

