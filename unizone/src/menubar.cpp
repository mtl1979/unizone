#include "menubar.h"
#include "version.h"

#include <qapplication.h>
#include <q3accel.h>
#include <Q3PopupMenu>

const char *appleItems[] = {
	QT_TRANSLATE_NOOP("QMenuBar", "Preference"),
        QT_TRANSLATE_NOOP("QMenuBar", "About"),
        NULL
};

MenuBar::MenuBar(QWidget * parent) : QMenuBar(parent)
{
	// create search... menu

	Q3PopupMenu *fSearch = new Q3PopupMenu(this);
	Q_CHECK_PTR(fSearch);

	fSearch->insertItem(tr("Music"), parent, SLOT(SearchMusic()));
	fSearch->insertItem(tr("Videos"), parent, SLOT(SearchVideos()));
	fSearch->insertItem(tr("Pictures"), parent, SLOT(SearchPictures()));
	fSearch->insertItem(tr("Disk Images"), parent, SLOT(SearchImages()));

	// create file menu

	fFile = new Q3PopupMenu(this);
	Q_CHECK_PTR(fFile);

	fFile->insertItem(tr("&Connect"), parent, SLOT(Connect()), Q3Accel::stringToKey(tr("CTRL+SHIFT+C")));
	fFile->insertItem(tr("&Disconnect"), parent, SLOT(Disconnect()), Q3Accel::stringToKey(tr("CTRL+SHIFT+D")));
	fFile->insertSeparator();
	fFile->insertItem(tr("Open &Shared Folder"), parent, SLOT(OpenSharedFolder()), Q3Accel::stringToKey(tr("CTRL+S")));
	fFile->insertItem(tr("Open &Downloads Folder"), parent, SLOT(OpenDownloadsFolder()), Q3Accel::stringToKey(tr("CTRL+D")));
	fFile->insertItem(tr("Open &Logs Folder"), parent, SLOT(OpenLogsFolder()), Q3Accel::stringToKey(tr("CTRL+L")));
	fFile->insertSeparator();
	fFile->insertItem(tr("&Search"), parent, SLOT(OpenSearch()), Q3Accel::stringToKey(tr("ALT+S")));
	fFile->insertItem(tr("Search..."), fSearch);
	fFile->insertSeparator();
	fFile->insertItem(tr("E&xit"), parent, SLOT(Exit()), Q3Accel::stringToKey(tr("ALT+X")));

	// edit menu
	fEdit = new Q3PopupMenu(this);
	Q_CHECK_PTR(fEdit);

	fEdit->insertItem(tr("Cl&ear Chat Log"), parent, SLOT(ClearChatLog()), Q3Accel::stringToKey(tr("CTRL+E")));
	fEdit->insertSeparator();
	fEdit->insertItem(tr("&Preferences"), parent, SLOT(Preferences()), Q3Accel::stringToKey(tr("CTRL+P")));

	// windows menu

	fWindows = new Q3PopupMenu(this);
	Q_CHECK_PTR(fWindows);

	fWindows->insertItem(tr("Picture Viewer"), parent, SLOT(OpenViewer()), Q3Accel::stringToKey(tr("F9")));
	fWindows->insertItem(tr("C&hannels"), parent, SLOT(OpenChannels()), Q3Accel::stringToKey(tr("F10")));
	fWindows->insertItem(tr("&Downloads"), parent, SLOT(OpenDownloads()), Q3Accel::stringToKey(tr("F11")));
	fWindows->insertItem(tr("&Uploads"), parent, SLOT(OpenUploads()), Q3Accel::stringToKey(tr("Shift+F11")));
	// help menu
	fHelp = new Q3PopupMenu(this);
	Q_CHECK_PTR(fHelp);

	QString about = tr( "&About Unizone (English) %1" ).arg(WinShareVersionString());

	fHelp->insertItem(about, parent, SLOT(AboutWinShare()), Q3Accel::stringToKey(tr("F12")));

	/* Insert into menubar */
	insertItem(tr("&File"), fFile);
	insertItem(tr("&Edit"), fEdit);
	insertItem(tr("&Window"), fWindows);
	insertItem(tr("&Help"), fHelp);
}

MenuBar::~MenuBar()
{
}

