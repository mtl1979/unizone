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
	fFile->insertItem(QObject::tr(MSG_CONNECT), parent, SLOT(Connect()), MSG_ACONNECT);
	fFile->insertItem(QObject::tr(MSG_DISCONNECT), parent, SLOT(Disconnect()), MSG_ADISCONNECT);
	fFile->insertSeparator();
	fFile->insertItem(QObject::tr(MSG_OPEN_SHARED), parent, SLOT(OpenSharedFolder()), MSG_AOPEN_SHARED);
	fFile->insertItem(QObject::tr(MSG_OPEN_DOWNLOAD), parent, SLOT(OpenDownloadsFolder()), MSG_AOPEN_DOWNLOAD);
	fFile->insertItem(QObject::tr(MSG_OPEN_LOGFOLDER), parent, SLOT(OpenLogsFolder()), MSG_AOPEN_LOGFOLDER); 	
	fFile->insertSeparator();
	fFile->insertItem(QObject::tr(MSG_CLEAR_CHATLOG), parent, SLOT(ClearChatLog()), MSG_ACLEAR_CHATLOG);
	fFile->insertSeparator();
	fFile->insertItem(QObject::tr(MSG_SEARCH), parent, SLOT(SearchDialog()), MSG_ASEARCH);
	fFile->insertSeparator();

	fFile->insertItem(QObject::tr(MSG_EXIT), parent, SLOT(Exit()), MSG_AEXIT);

	// edit menu
	fEdit = new QPopupMenu(parent);
	CHECK_PTR(fEdit);

	fEdit->insertItem(QObject::tr(MSG_PREFERENCES), parent, SLOT(Preferences()), MSG_APREFERENCES);

	// help menu
	fHelp = new QPopupMenu(parent);
	CHECK_PTR(fHelp);

	char about[100];
	sprintf(about, MSG_ABOUT NAME " %s", WinShareVersionString());

	fHelp->insertItem(QObject::tr(about), parent, SLOT(AboutWinShare()), MSG_AABOUT);

	/* Insert into menubar */
	fBar->insertItem(QObject::tr(MSG_FILE), fFile);
	fBar->insertItem(QObject::tr(MSG_EDIT), fEdit);
	fBar->insertItem(QObject::tr(MSG_HELP), fHelp);
}

MenuBar::~MenuBar()
{
}

