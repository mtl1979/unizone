#ifdef WIN32
#pragma warning(disable: 4786)
#endif
#include <qapplication.h>

#include "search.h"
#include "debugimpl.h"
#include "global.h"
#include "settings.h"
#include "downloadimpl.h"
#include "searchitem.h"
#include "lang.h"			// <postmaster@raasu.org> 20020924
#include "platform.h"		// <postmaster@raasu.org> 20021114
#include "combo.h"			// <postmaster@raasu.org> 20030218

#include <time.h>

struct WFileInfo
{
	WUserRef fiUser;
	QString fiFilename;
	MessageRef fiRef;
	bool fiFirewalled;
	WSearchListItem * fiListItem;	// the list view item this file is tied to
};
const int kListSizes[6] = { 200, 75, 100, 150, 150, 75 };

WSearch::WSearch(NetClient * net, QWidget * parent)
	: QDialog(parent, "WSearch" /* name */, false /* modal */, QDialog::WDestructiveClose| QWidget::WStyle_Minimize | 
			  QWidget::WStyle_Maximize | QWidget::WStyle_Title | QWidget::WStyle_SysMenu /* flags */)
{
	fCurrentSearchPattern = "";
	fIsRunning = false;

	setCaption(tr(MSG_SEARCH));
	// initialize GUI
	fMainBox = new QVGroupBox(this);
	CHECK_PTR(fMainBox);

	fSearchBox = new QVGroupBox(fMainBox);
	CHECK_PTR(fSearchBox);
	
	fButtonsBox = new QHGroupBox(fMainBox);
	CHECK_PTR(fButtonsBox);

	fEntryBox = new QHGroupBox(fSearchBox);
	CHECK_PTR(fEntryBox);
	fEntryBox->setFrameStyle(QFrame::NoFrame);

	fSearchList = new QListView(fSearchBox);
	CHECK_PTR(fSearchList);
	fSearchList->addColumn(tr(MSG_SW_FILENAME));
	fSearchList->addColumn(tr(MSG_SW_FILESIZE));
	fSearchList->addColumn(tr(MSG_SW_FILETYPE));
	fSearchList->addColumn(tr(MSG_SW_MODIFIED));
	fSearchList->addColumn(tr(MSG_SW_PATH));
	fSearchList->addColumn(tr(MSG_SW_USER));

	fSearchList->setColumnAlignment(1, AlignRight); // <postmaster@raasu.org> 20021103

	fSearchList->setShowSortIndicator(true);
	fSearchList->setAllColumnsShowFocus(true);

	for (int i = 0; i < 6; i++)
	{
		fSearchList->setColumnWidthMode(i, QListView::Manual);
		fSearchList->setColumnWidth(i, kListSizes[i]);
	}

	fSearchList->setSelectionMode(QListView::SelectionMode::Extended);

	fSearchLabel = new QLabel(fEntryBox);
	CHECK_PTR(fSearchLabel);

	// fSearchEdit = new QLineEdit(fEntryBox);
	fSearchEdit = new WComboBox(this, fEntryBox, "fSearchEdit");
	CHECK_PTR(fSearchEdit);
	fSearchEdit->setEditable(true);
	fSearchEdit->setMinimumWidth(this->width()*0.75);
	fSearchLabel->setBuddy(fSearchEdit);
	fSearchLabel->setText(tr(MSG_SW_CSEARCH));
	
	fDownload = new QPushButton(fButtonsBox);
	CHECK_PTR(fDownload);
	fClose = new QPushButton(fButtonsBox);
	CHECK_PTR(fClose);
	fClear = new QPushButton(fButtonsBox);
	CHECK_PTR(fClear);
	fStop = new QPushButton(fButtonsBox);
	CHECK_PTR(fStop);

	fDownload->setText(tr(MSG_SW_DOWNLOAD));
	fClose->setText(tr(MSG_SW_CLOSE));
	fClear->setText(tr(MSG_SW_CLEAR));
	fStop->setText(tr(MSG_SW_STOP));

	fStatus = new QStatusBar(fMainBox);
	CHECK_PTR(fStatus);
	fStatus->setSizeGripEnabled(false);

	resize(760, 500);

	fNet = net;

	// load query history
	QString str;
	for (i = 0; (str = gWin->fSettings->GetQueryItem(i)) != QString::null; i++)
		fSearchEdit->insertItem(str, i);


	// connect up slots
	connect(fClose, SIGNAL(clicked()), this, SLOT(Close()));
	// connect(fSearchEdit, SIGNAL(returnPressed()), this, SLOT(GoSearch()));
	connect(fNet, SIGNAL(AddFile(const QString, const QString, bool, MessageRef)), this,
			SLOT(AddFile(const QString, const QString, bool, MessageRef)));
	connect(fNet, SIGNAL(RemoveFile(const QString, const QString)), this, 
			SLOT(RemoveFile(const QString, const QString)));
	connect(fClear, SIGNAL(clicked()), this, SLOT(ClearList()));
	connect(fStop, SIGNAL(clicked()), this, SLOT(StopSearch()));
	connect(fDownload, SIGNAL(clicked()), this, SLOT(Download()));
	connect(fNet, SIGNAL(DisconnectedFromServer()), this, SLOT(DisconnectedFromServer()));

	fQueue = new Message();

	SetStatus(tr(MSG_IDLE));

	fSearchEdit->setFocus();
}

WSearch::~WSearch()
{
	StopSearch();
	ClearList();

	gWin->fSettings->EmptyQueryList();

	// save query history
	int i;
	for (i = 0; i < fSearchEdit->count(); i++)
	{
		gWin->fSettings->AddQueryItem(fSearchEdit->text(i));
		PRINT("Saved query %s\n", fSearchEdit->text(i).latin1());
	}

	fIsRunning = false;

	if (fQueue)
		delete fQueue;

	emit Closed();
}

void
WSearch::resizeEvent(QResizeEvent * r)
{
	fMainBox->resize(r->size());
}

void
WSearch::Close()
{
	accept();
}

void
WSearch::AddFile(const QString sid, const QString filename, bool firewalled, MessageRef file)
{
	PRINT("ADDFILE called\n");
	if (firewalled && gWin->fSettings->GetFirewalled())
		return;	// we don't need to show this file if we are firewalled
	PRINT("ADDFILE: filename=%s (%s) [%s]\n", filename.latin1(), firewalled ? "firewalled" : "hackable", sid.latin1());
	Lock();
	// see if the filename matches our file regex
	// AND that the session ID matches our session ID regex
	if (fFileRegExp.Match((const char *) filename.utf8()))
	{
		PRINT("ADDFILE: file Regex ok\n");
		if (fUserRegExp.Match((const char *) sid.utf8()))
		{
			// yes!
			WUserIter uit = fNet->Users().find(sid);
			if (uit != fNet->Users().end())	// found our user
			{
				WUserRef user = (*uit).second;
				
				WFileInfo * info = new WFileInfo;
				info->fiUser = user;
				info->fiFilename = filename;
				info->fiRef = file;
				info->fiFirewalled = firewalled;
				
				String path, kind;
				int64 size = 0;
				int32 mod = 0;
				
				file()->FindString("beshare:Kind", kind);
				file()->FindString("beshare:Path", path);
				file()->FindInt32("beshare:Modification Time", (int32 *)&mod);
				file()->FindInt64("beshare:File Size", (int64 *)&size);
				
				// name, size, type, modified, path, user
				QString qkind = QString::fromUtf8(kind.Cstr());
				QString qsize = tr("%1").arg((int)size); 
				QString qmod = tr("%1").arg(mod); // <postmaster@raasu.org> 20021126
				QString qpath = QString::fromUtf8(path.Cstr());
				QString quser = user()->GetUserName();
				
				info->fiListItem = new WSearchListItem(fSearchList, filename, qsize, qkind, qmod, qpath, quser);
				PRINT("Setting key to %d\n", (long)size);
				
				// The map is based on _filename's_, not session ID's.
				// And as filename's can be duplicate, we use a multimap
				WFIPair pair = MakePair(filename, info);
				fFileList.insert(fFileList.end(), pair);
			}
		}
		
	}
	Unlock();
	SetResultsMessage();
}

void
WSearch::RemoveFile(const QString sid, const QString filename)
{
	Lock();
	PRINT("WSearch::RemoveFile\n");
	// go through our multi map and find the item that matches the sid and filename
	WFIIter iter = fFileList.begin();
	WFileInfo * info;

	PRINT("Sid = %s, filename = %s\n", sid.latin1(), filename.latin1());

	while (iter != fFileList.end())
	{
		info = (*iter).second;
		if (info->fiFilename == filename && info->fiUser()->GetUserID() == sid)	// we found it
		{
			delete info->fiListItem;	// remove it from the list view
			delete info;
			info = NULL; // <postmaster@raasu.org> 20021027

			fFileList.erase(iter);
			break;	// break from the loop
		}
		// not found, continue looking
		iter++;
	}
	Unlock();
	SetResultsMessage();
}

void
WSearch::StopSearch()
{
	Lock();
	if (fCurrentSearchPattern != "")	// we actually have an old search pattern,
	{
		// cancel it
		// fCurrentSearchPattern contains a fully formatted subscription string
		fNet->RemoveSubscription(fCurrentSearchPattern);
		// also dump pending results
		MessageRef cancel(new Message(PR_COMMAND_JETTISONRESULTS), NULL);
		if (cancel())
		{
			// since "fCurrentSearchPattern" is in a /*/*/beshare/fi*es/*
			// pattern, if I grab the third '/' and add one, that is my
			// cancel PR_NAME_KEYS
			// <postmaster@raasu.org> 20021023 -- Use temporary variable to help debugging
			String CurrentSearchPattern = (const char *) fCurrentSearchPattern.utf8();
			const char * cancelStr = strchr(CurrentSearchPattern.Cstr(), '/') + 1;
			cancelStr = strchr(cancelStr, '/') + 1;
			cancelStr = strchr(cancelStr, '/') + 1;
			cancel()->AddString(PR_NAME_KEYS, cancelStr);
			fNet->SendMessageToSessions(cancel);
		}

		fCurrentSearchPattern = "";
		fFileRegExp.SetPattern("");
		fUserRegExp.SetPattern("");
	}
	Unlock();
	SetStatus(tr(MSG_IDLE));
}

// This method locks the list, so be sure the mutex is unlocked before calling it!
void
WSearch::ClearList()
{
	Lock();
	// go through and empty the list
	WFIIter it = fFileList.begin();
	while (it != fFileList.end())
	{
		WFileInfo * info = (*it).second;
		// don't delete the list items here
		delete info;
		info = NULL; // <postmaster@raasu.org> 20021027
		fFileList.erase(it);
		it = fFileList.begin();
	}
	// delete them NOW
	fSearchList->clear();
	Unlock();
}

void
WSearch::GoSearch()
{
	// cancel current search (if there is one)
	StopSearch();	// these methods lock
	ClearList();


	// here we go with the new search pattern
	if (fSearchEdit->currentText().stripWhiteSpace() == "")	// no search string
		return;

	if (gWin->fNetClient->IsInternalThreadRunning() == false)
	{
		fStatus->message(tr(MSG_NOTCONNECTED));
		return;
	}

	// now we lock
	Lock();

	// parse the string for the '@' if it exists
	String fileExp(fSearchEdit->currentText().utf8());
	String userExp;

	fileExp = fileExp.Trim();
	int32 atIndex = fileExp.IndexOf('@');
	if (atIndex >= 0)
	{
		if ((uint32)atIndex < fileExp.Length())
		{
			userExp = fileExp.Substring(atIndex + 1);

			if (!HasRegexTokens(userExp.Cstr()))
			{
				bool nonNumericFound = false;
				const char * check = userExp.Cstr();
				while (*check)
				{
					if ((*check != ',') && ((*check < '0') || (*check > '9')))
					{
						nonNumericFound = true;
						break;
					}
					check++;
				}
				if (nonNumericFound)
					userExp = userExp.Prepend("*").Append("*");
			}

			MakeRegexCaseInsensitive(userExp);
			StringMatcher match(userExp.Cstr());
			WUserMap & users = fNet->Users();
			WUserIter it = users.begin();
			WUserRef user;
			while (it != users.end())
			{
				user = (*it).second;
				if (match.Match((const char *) user()->GetUserName().utf8()))
				{
					userExp += ",";
					userExp += (const char *) user()->GetUserID().utf8();
				}
				it++;
			}
			userExp	= userExp.Substring(userExp.IndexOf(",") + 1);
		}

		if (atIndex > 0)
			fileExp = fileExp.Substring(0, atIndex);
		else
			fileExp = "";
	}

	if (!HasRegexTokens(fileExp.Cstr()) && (userExp.Length() > 0 || fileExp.Length() > 0))
		fileExp = fileExp.Prepend("*").Append("*");

	fileExp.Replace('/', '?');
	userExp.Replace('/', '?');

	MakeRegexCaseInsensitive(fileExp);
	
	Unlock();	// unlock before StartQuery();

	StartQuery(userExp.Length() > 0 ? QString::fromUtf8(userExp.Cstr()) : "*", QString::fromUtf8(fileExp.Cstr()));
}

void
WSearch::StartQuery(QString sidRegExp, QString fileRegExp)
{
	Lock();
	QString tmp = "SUBSCRIBE:/*/";
	tmp += sidRegExp;
	tmp += "/beshare/";
	tmp += gWin->fSettings->GetFirewalled() ? "files/" : "fi*/";		// if we're firewalled, we can only get files from non-firewalled users
	tmp += fileRegExp;
	fCurrentSearchPattern = tmp;
	// <postmaster@raasu.org> 20021023 -- Fixed typo
	PRINT("Current Search Pattern = %s, fUserRegExp = %s, fFileRegExp = %s\n", fCurrentSearchPattern.latin1(), sidRegExp, fileRegExp);
	fUserRegExp.SetPattern((const char *) sidRegExp.utf8());
	fUserRegExpStr = sidRegExp;
	fFileRegExp.SetPattern((const char *) fileRegExp.utf8());
	fFileRegExpStr = fileRegExp;
	fNet->AddSubscription(tmp); // <postmaster@raasu.org> 20021026
	Unlock();

	SetStatus(tr(MSG_SEARCHING).arg(fileRegExp));
}

void
WSearch::Download()
{
	Lock();
	fDownload->setEnabled(false);
	if (!fFileList.empty())
	{
		WFIIter it = fFileList.begin();
		while (it != fFileList.end())
		{
			WFileInfo * fi = (*it).second;
			PRINT("Checking: %s, %s\n", fi->fiListItem->text(0).latin1(), fi->fiListItem->text(5).latin1());
			if (fi->fiListItem->isSelected())
			{
				PRINT("DOWNLOAD: Found item\n");
				WUserRef user = fi->fiUser;
				
				QueueDownload(fi->fiFilename, user());
			}					
			it++;
		}
	}
	EmptyQueues();
	fDownload->setEnabled(true);
	Unlock();
}

void
WSearch::DisconnectedFromServer()
{
	this->close();
}

void
WSearch::SetResultsMessage()
{
	fStatus->message(tr(MSG_WF_RESULTS).arg(fFileList.size()));
}

void
WSearch::SetStatus(const QString & status)
{
	fStatus->message(status);
}

void
WSearch::SetSearch(QString pattern)
{
	//fSearchEdit->setText(pattern);
	// Is already on history?
	for (int i = 0; i < fSearchEdit->count(); i++)
	{
		if (fSearchEdit->text(i) == pattern)
		{
			fSearchEdit->setCurrentItem(i);
			GoSearch();
			return;
		}
	}
	fSearchEdit->insertItem(pattern, 0);
	fSearchEdit->setCurrentItem(0);
	GoSearch();
}

void
WSearch::customEvent(QCustomEvent * event)
{
	switch (event->type())
	{
				case WTextEvent::ComboType:
					HandleComboEvent(dynamic_cast<WTextEvent *>(event));
					return;
				default:
					break;
	}
}

void
WSearch::HandleComboEvent(WTextEvent * e)
{
	if (e)
	{
		WComboBox * sender = (WComboBox *)e->data();
		// see who sent this 
		if (sender == fSearchEdit)
			GoSearch();
	}
}

void
WSearch::QueueDownload(QString file, WUser * user)
{
	int32 i = 0;
	String mUser = (const char *) user->GetUserID().utf8();
	if (fQueue->FindInt32(mUser, &i) == B_OK)
		fQueue->RemoveName(mUser);
	fQueue->AddInt32(mUser, ++i);
	mUser = mUser.Prepend("_");
	fQueue->AddString(mUser, (const char *) file.utf8());
}

void
WSearch::EmptyQueues()
{
	String mUser, mFile;
	QString user;
	QString * files;
	WUserRef u;
	int32 numItems;
	MessageFieldNameIterator iter = fQueue->GetFieldNameIterator(B_INT32_TYPE);
	while (iter.GetNextFieldName(mUser) == B_OK)
	{
		user = QString::fromUtf8(mUser.Cstr());
		u = gWin->FindUser(user);
		if (u() != NULL)
		{
			fQueue->FindInt32(mUser, &numItems);
			files = new QString[numItems];
			mUser = mUser.Prepend("_");
			for (int32 i = 0; i < numItems; i++)
			{
				fQueue->FindString(mUser, i, mFile);
				files[i] = QString::fromUtf8(mFile.Cstr());
			}
			gWin->OpenDownload();
			gWin->fDLWindow->AddDownload(files, numItems, u()->GetUserID(), u()->GetPort(), u()->GetUserHostName(), u()->GetInstallID(), u()->GetFirewalled(), u()->GetPartial());
		}
	}
	delete fQueue;
	fQueue = new Message();
}


	