#include "menubar.h"
#include "version.h"

#include <qapplication.h>
#include "lang.h"			// <postmaster@raasu.org> 20020924

MenuBar::MenuBar(QWidget * parent)
{
	fBar = new QMenuBar(parent);
	CHECK_PTR(fBar);

	/* create file menu */
	fFile = new QPopupMenu(parent);
	CHECK_PTR(fFile);
	fFile->insertItem(MSG_CONNECT, parent, SLOT(Connect()), MSG_ACONNECT);
	fFile->insertItem(MSG_DISCONNECT, parent, SLOT(Disconnect()), MSG_ADISCONNECT);
	fFile->insertSeparator();
	fFile->insertItem(MSG_OPEN_SHARED, parent, SLOT(OpenSharedFolder()), MSG_AOPEN_SHARED);
	fFile->insertItem(MSG_OPEN_DOWNLOAD, parent, SLOT(OpenDownloadsFolder()), MSG_AOPEN_DOWNLOAD);
	fFile->insertItem(MSG_OPEN_LOGFOLDER, parent, SLOT(OpenLogsFolder()), MSG_AOPEN_LOGFOLDER); 	
	fFile->insertSeparator();
	fFile->insertItem(MSG_CLEAR_CHATLOG, parent, SLOT(ClearChatLog()));
	fFile->insertSeparator();
	fFile->insertItem(MSG_SEARCH, parent, SLOT(SearchDialog()), MSG_ASEARCH);
	fFile->insertSeparator();

	char about[100];
	sprintf(about, MSG_ABOUT NAME " %s", WinShareVersionString());

	fFile->insertItem(about, parent, SLOT(AboutWinShare()), MSG_AABOUT);
	fFile->insertSeparator();
	fFile->insertItem(MSG_EXIT, parent, SLOT(Exit()), MSG_AEXIT);

	// style menu
	fEdit = new QPopupMenu(parent);
	CHECK_PTR(fEdit);

	fEdit->insertItem(MSG_PREFERENCES, parent, SLOT(Preferences()));
	/* Insert into menubar */
	fBar->insertItem(MSG_FILE, fFile);
	fBar->insertItem(MSG_EDIT, fEdit);
}

MenuBar::~MenuBar()
{
}

