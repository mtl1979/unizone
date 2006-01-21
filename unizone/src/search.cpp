#include <qpushbutton.h>

#include "search.h"
#include "netclient.h"
#include "debugimpl.h"
#include "wstring.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"
#include "util.h"
#include "downloadimpl.h"
#include "downloadqueue.h"


const int kListSizes[6] = { 200, 75, 100, 150, 150, 75 };

// quick inline method to generate a pair

inline
String 
MakeKey(const QString & a, const QString &b)
{
	String out;
	QString temp(a);
	temp += "@";
	temp += b;
	out = (const char *) temp.utf8();
	return out;
}

WSearch::WSearch(QWidget * parent, NetClient * fNet)
:	QDialog(parent, "WSearch", false, WStyle_SysMenu | WStyle_Minimize | WStyle_Maximize | WStyle_Title)
{
	setCaption(tr("Search"));

	// Initialize variables for Search Pane

	fCurrentSearchPattern = QString::null;
	fIsRunning = false;
	fNetClient = fNet;
	fQueryBytes = 0;
	fFileRegExpNeg = fUserRegExpNeg = false;
	fPending = false;
	
	// Create the Search Pane
	
	fSearchTab = new QGridLayout(this, 14, 10, 0, -1, "Search Tab");
	CHECK_PTR(fSearchTab);

	fSearchTab->addColSpacing(1, 20);
	fSearchTab->addColSpacing(3, 20);
	fSearchTab->addColSpacing(5, 20);
	fSearchTab->addColSpacing(7, 20);
	fSearchTab->addColSpacing(9, 20);

	fSearchTab->addRowSpacing(0, 5);
	fSearchTab->addRowSpacing(2, 20);
	fSearchTab->addRowSpacing(12, 20);

	fSearchTab->setRowStretch(0, 0);	// Least significant
	fSearchTab->setRowStretch(1, 1);
	fSearchTab->setRowStretch(2, 1);
	fSearchTab->setRowStretch(3, 2);	// 3 to 9, most significant
	fSearchTab->setRowStretch(4, 2);
	fSearchTab->setRowStretch(5, 2);
	fSearchTab->setRowStretch(6, 2);
	fSearchTab->setRowStretch(7, 2);
	fSearchTab->setRowStretch(8, 2);
	fSearchTab->setRowStretch(9, 2);
	fSearchTab->setRowStretch(10, 1);
	fSearchTab->setRowStretch(11, 1);
	fSearchTab->setRowStretch(12, 0);	// Least significant
	fSearchTab->setRowStretch(13, 1);

	// Results ListView
	
	fSearchList = new WUniListView(this);
	CHECK_PTR(fSearchList);

	fSearchList->addColumn(tr("File Name"));
	fSearchList->addColumn(tr("File Size"));
	fSearchList->addColumn(tr("File Type"));
	fSearchList->addColumn(tr("Modified"));
	fSearchList->addColumn(tr("Path"));
	fSearchList->addColumn(tr("User"));

	fSearchList->setColumnAlignment(WSearchListItem::FileSize, AlignRight); // <postmaster@raasu.org> 20021103
	fSearchList->setColumnAlignment(WSearchListItem::Modified, AlignRight);

	fSearchList->setShowSortIndicator(true);
	fSearchList->setAllColumnsShowFocus(true);

	int i;

	for (i = 0; i < 6; i++)
	{
		fSearchList->setColumnWidthMode(i, QListView::Manual);
		fSearchList->setColumnWidth(i, kListSizes[i]);
	}

	fSearchList->setSelectionMode(QListView::Extended);

	fSearchTab->addMultiCellWidget(fSearchList, 3, 9, 0, 9);

	// Status Bar

	fStatus = new WStatusBar(this);
	CHECK_PTR(fStatus);
	fStatus->setSizeGripEnabled(false);

	fSearchTab->addMultiCellWidget(fStatus, 13, 13, 0, 9);

	// Search Query Label

	fSearchLabel = new QLabel(this);
	CHECK_PTR(fSearchLabel);

	fSearchTab->addMultiCellWidget(fSearchLabel, 1, 1, 0, 3); 

	// Search Query Combo Box

	fSearchEdit = new WComboBox(this, this, "fSearchEdit");
	CHECK_PTR(fSearchEdit);

	fSearchEdit->setEditable(true);
	double dwidth = (double) this->width() * 0.75f;
	fSearchEdit->setMinimumWidth(lrint(dwidth));
	fSearchEdit->setDuplicatesEnabled(false);
	fSearchEdit->setAutoCompletion(true);

	fSearchLabel->setBuddy(fSearchEdit);
	fSearchLabel->setText(tr("Search:"));

	fSearchTab->addMultiCellWidget(fSearchEdit, 1, 1, 4, 9);
	
	// Download Button

	fDownload = new QPushButton(tr("Download"), this);
	CHECK_PTR(fDownload);

	fSearchTab->addMultiCellWidget(fDownload, 10, 10, 0, 2);

	// Download All Button

	fDownloadAll = new QPushButton(tr("Download All"), this);
	CHECK_PTR(fDownloadAll);

	fSearchTab->addMultiCellWidget(fDownloadAll, 11, 11, 0, 2);

	// Stop Button

	fStop = new QPushButton(tr("Stop"), this);
	CHECK_PTR(fStop);

	fSearchTab->addMultiCellWidget(fStop, 10, 10, 4, 6);

	// Clear Button

	fClear = new QPushButton(tr("Clear"), this);
	CHECK_PTR(fClear);

	fSearchTab->addMultiCellWidget(fClear, 10, 10, 8, 9);

	// Clear History Button

	fClearHistory = new QPushButton(tr("Clear History"), this);
	CHECK_PTR(fClearHistory);

	fSearchTab->addMultiCellWidget(fClearHistory, 11, 11, 8, 9);

	// connect up slots

	connect(fNetClient, SIGNAL(AddFile(const WUserRef &, const QString &, bool, MessageRef)), 
			this, SLOT(AddFile(const WUserRef &, const QString &, bool, MessageRef)));
	connect(fNetClient, SIGNAL(RemoveFile(const WUserRef &, const QString &)), 
			this, SLOT(RemoveFile(const WUserRef &, const QString &)));

	connect(fClear, SIGNAL(clicked()), this, SLOT(ClearList()));
	connect(fStop, SIGNAL(clicked()), this, SLOT(StopSearch()));
	connect(fDownload, SIGNAL(clicked()), this, SLOT(Download()));
	connect(fDownloadAll, SIGNAL(clicked()), this, SLOT(DownloadAll()));
	connect(fClearHistory, SIGNAL(clicked()), this, SLOT(ClearHistory()));

	SetSearchStatus(tr("Idle."));

	//
	// End of Search Pane
	//
}

WSearch::~WSearch()
{
	Cleanup();
}

void
WSearch::Cleanup()
{
	StopSearch();
	ClearList();

	fIsRunning = false;
}

void
WSearch::AddFile(const WUserRef &user, const QString &filename, bool firewalled, MessageRef file)
{
	PRINT("ADDFILE called\n");
	
	QString sid = user()->GetUserID();

	RemoveFile(user, filename);

#ifdef _DEBUG
	WString wfile(filename);
	WString wsid(sid);
	PRINT("ADDFILE: filename=%S (%s) [%S]\n", wfile.getBuffer(), firewalled ? "firewalled" : "hackable", wsid.getBuffer());
#endif

	// Workaround for StringMatcher bug (for now!)
	if (fCurrentSearchPattern.isEmpty())
		return;

	fSearchLock.Lock();
	// see if the filename matches our file regex
	// AND that the session ID matches our session ID regex
	if ((Match(filename, fFileRegExp) >= 0) != fFileRegExpNeg)
	{
		PRINT("ADDFILE: file Regex ok\n");
		if ((Match(sid, fUserRegExp) >= 0) != fUserRegExpNeg)
		{
			// yes!
			
			/*
			 *
			 * a) Both aren't firewalled or
			 * b) Only one is firewalled or
			 * c) Both are firewalled but other party supports tunneling
			 *
			 */
			if (!firewalled || !gWin->fSettings->GetFirewalled() || user()->GetTunneling())
			{
				String path, kind;
				uint64 size = 0;
				int32 mod = 0;
				
				file()->FindString("beshare:Kind", kind);
				file()->FindString("beshare:Path", path);
				file()->FindInt32("beshare:Modification Time", (int32 *)&mod);
				file()->FindInt64("beshare:File Size", (int64 *)&size);
				
				WFileInfo * info = new WFileInfo;
				CHECK_PTR(info);
				info->fiUser = user;
				info->fiFilename = filename;
				info->fiSize = size;
				info->fiRef = file;
				info->fiFirewalled = firewalled;
				
				fQueryBytes += size;
				
				// name, size, type, modified, path, user
				QString qkind	= QString::fromUtf8(kind.Cstr());
				QString qsize	= fromULongLong(size); 
				QString qmod	= QString::number(mod); // <postmaster@raasu.org> 20021126
				QString qpath	= QString::fromUtf8(path.Cstr());
				QString quser	= user()->GetUserName();
				
				info->fiListItem = new WSearchListItem(fSearchList, filename, qsize, qkind, qmod, qpath, quser);
				CHECK_PTR(info->fiListItem);
				
#ifdef WIN32
				PRINT("Setting key to %I64i\n", size);
#else
				PRINT("Setting key to %lli\n", size);
#endif
				
				fFileList.Put(MakeKey(filename, sid), info);
			}
		}		
	}
	fSearchLock.Unlock();
	SetResultsMessage();
}

void
WSearch::RemoveFile(const WUserRef &user, const QString &filename)
{
	fSearchLock.Lock();
	PRINT("WSearch::RemoveFile\n");
	// go through our multi map and find the item that matches the sid and filename
	WFileInfo * info;

	QString sid = user()->GetUserID();

#ifdef _DEBUG
	WString wfile(filename);
	WString wsid(sid);
	PRINT("Sid = %S, filename = %S\n", wsid.getBuffer(), wfile.getBuffer());
#endif

	String key = MakeKey(filename, sid);
	if (fFileList.GetValue(key, info) == B_OK)
	{
		fQueryBytes -= info->fiSize;
		delete info->fiListItem;	// remove it from the list view
		delete info;
		info = NULL; // <postmaster@raasu.org> 20021027

		fFileList.Remove(key);
	}
	fSearchLock.Unlock();
	SetResultsMessage();
}

void
WSearch::StopSearch()
{
	fSearchLock.Lock();
	if (!fCurrentSearchPattern.isEmpty())	// we actually have an old search pattern,
	{
		// cancel it
		// fCurrentSearchPattern contains a fully formatted subscription string
		fNetClient->RemoveSubscription(fCurrentSearchPattern);
		// also dump pending results
		MessageRef cancel(GetMessageFromPool(PR_COMMAND_JETTISONRESULTS));
		if (cancel())
		{
			// /*/*/beshare/fi*es/*
			//      ^---- cancel PR_NAME_KEYS

			// <postmaster@raasu.org> 20021023 -- Use temporary variable to help debugging
			{
				QString tmp;
				int a = fCurrentSearchPattern.find("/");
				int b = fCurrentSearchPattern.find("/", a + 1);
				int c = fCurrentSearchPattern.find("/", b + 1);
				tmp = fCurrentSearchPattern.mid(c + 1);
				cancel()->AddString(PR_NAME_KEYS, (const char *) tmp.utf8());
			}
			fNetClient->SendMessageToSessions(cancel);
		}

		fCurrentSearchPattern = QString::null;
		fFileRegExp.setPattern("");
		fUserRegExp.setPattern("");
	}
	fSearchLock.Unlock();
	SetSearchStatus(tr("Idle."));
	SetSearchStatus("", 2);
}

// This method locks the list, so be sure the mutex is unlocked before calling it!
void
WSearch::ClearList()
{
	fSearchLock.Lock();
	// go through and empty the list
	WFileInfo * info;
	String key;
	HashtableIterator<String, WFileInfo *> iter = fFileList.GetIterator(HTIT_FLAG_BACKWARDS);
	while ((iter.GetNextKey(key) == B_OK) && (iter.GetNextValue(info) == B_OK))
	{
		// don't delete the list items here
		delete info;
		info = NULL; // <postmaster@raasu.org> 20021027
		fFileList.Remove(key);
	}
	// delete them NOW
	fSearchList->clear();
	fQueryBytes = 0;
	fSearchLock.Unlock();
	SetSearchStatus("", 1);
}

int
WSearch::SplitQuery(const QString &fileExp)
{
	// Test if all but one @ is escaped (don't change string length)
	{
		QString temp(fileExp);
		temp.replace(QRegExp("\\\\@"), "??");
		if (temp.contains("@") == 1)
		{
			return temp.find("@");
		}
		else if ((temp.contains("@") == 0) &&
			(fileExp.contains("\\@") > 0)) // Don't try to split if only escaped @'s
		{
			return -1;
		}
	}
	//
	if (startsWith(fileExp, "*@"))
	{
		return fileExp.find("@");
	}

	if (endsWith(fileExp, "@*"))
	{
		return fileExp.findRev("@");
	}

	{
		WUserMap &users = fNetClient->Users();
		WUserIter it = users.GetIterator(HTIT_FLAG_NOREGISTER);
		
		while (it.HasMoreValues())
		{
			WUserRef uref;
			it.GetNextValue(uref);
			// User ID?
			QString user(uref()->GetUserID());
			user.prepend("@");
			if (endsWith(fileExp, user))
			{
				return fileExp.findRev(user);
			}
			// User Name?
			QString name = uref()->GetUserName().lower();
			name = StripURL(name).stripWhiteSpace();
			name.prepend("@");
			
			// Compare end of fileExp against stripped user name
			
			if (endsWith(fileExp, name, false))
			{
				return fileExp.length() - name.length();
			}
		}
	}
	// No exact match?
	return fileExp.findRev("@");
}

void
WSearch::GoSearch()
{
	// cancel current search (if there is one)
	StopSearch();	// these methods lock
	ClearList();

	QString pattern = fSearchEdit->currentText().stripWhiteSpace();
	if (pattern.isEmpty())	// no search string
		return;

	if (fNetClient->IsConnected() == false)
	{
		fStatus->message(tr("Not connected."));
		return;
	}

	// here we go with the new search pattern

	// now we lock
	fSearchLock.Lock();

	QString fileExp(pattern);
	QString userExp;

	fileExp = fileExp.stripWhiteSpace();

	// parse the string for the '@' if it exists
	int32 atIndex = SplitQuery(fileExp);

	if (atIndex == 0)
	{
		atIndex++;
		fileExp.prepend("*");
	}

	if (atIndex > 0)
	{
		if ((uint32)atIndex < fileExp.length())
		{
			userExp = fileExp.mid(atIndex + 1);

			if (!HasRegexTokens(userExp))
			{
				bool nonNumericFound = false;
				unsigned int x = 0;
				while (x < userExp.length())
				{
					if ((userExp[x] != ',') && !muscleInRange((QChar) userExp[x], (QChar) '0', (QChar) '9'))
					{
						nonNumericFound = true;
						break;
					}
					x++;
				}
				if (nonNumericFound)
					userExp.prepend("*").append("*");
			}
			ConvertToRegex(userExp);
			QRegExp qr(userExp, false);
			WUserMap & users = fNetClient->Users();
			WUserIter it = users.GetIterator(HTIT_FLAG_NOREGISTER);
			QString ulist;
			while (it.HasMoreValues())
			{
				WUserRef user;
				it.GetNextValue(user);
				QString username = StripURL(user()->GetUserName());
				QString userid = user()->GetUserID();
				if (username.find(qr) >= 0)
				{
					AddToList(ulist, userid);
				}
				else if (userid.find(qr) >= 0)
				{
					AddToList(ulist, userid);
				}
			}
			if (ulist.isEmpty())
			{
				SetSearchStatus(tr("User(s) not found!"));
				fSearchLock.Unlock();
				return;
			}
			userExp	= ulist;
		}

		fileExp = fileExp.left(atIndex);
	}

	if (!fileExp.isEmpty())
	{
		if (!HasRegexTokens(fileExp))
			fileExp.prepend("*").append("*");

		fileExp.replace(QRegExp("/"), "?");

		ConvertToRegex(fileExp);	
	}

	if (!userExp.isEmpty())
	{
		userExp.replace(QRegExp("/"), "?");
	}

	fSearchLock.Unlock();	// unlock before StartQuery();

	StartQuery(userExp.isEmpty() ? ".*" : userExp, fileExp.isEmpty() ? ".*" : fileExp);
}

QString
Simplify(const QString &str)
{
	QString ret;
	unsigned int x = 0;
	while (x < str.length())
	{
		if (str[x] == "\\")
		{
			ret += str.mid(x, 2);
			x += 2;
		}
		else if (str.mid(x, 2) == ".*")
		{
			ret += "*";
			x += 2;
		}
		else if (str[x] == ".")
		{
			ret += "?";
			x++;
		}
		else if (str[x] == "|")
		{
			ret += ",";
			x++;
		}
		else if (str[x].upper() != str[x].lower())
		{
			if ((str[x].lower() < (QChar) 128) && (str.lower() < (QChar) 128))
			{
				ret += "[";
				ret += str[x].lower();
				ret += str[x].upper();
				ret += "]";
			}
			else
			{
				// Characters above 0x7F will be encoded using multiple bytes, so we use special trick.
				ret += "(";
				ret += str[x].lower();
				ret += "|";
				ret += str[x].upper();
				ret += ")";
			}
			x++;
		}
		else
		{
			ret += str[x];
			x++;
		}
	}
	return ret;
}

void
WSearch::StartQuery(const QString & sidRegExp, const QString & fileRegExp)
{
	fSearchLock.Lock();
	QString tmp("SUBSCRIBE:/*/");
	tmp += Simplify(sidRegExp);
	tmp += "/beshare/fi?es/";
	tmp += Simplify(fileRegExp);
	fCurrentSearchPattern = tmp;
	// <postmaster@raasu.org> 20021023 -- Fixed typo

#ifdef _DEBUG
	WString wpattern(fCurrentSearchPattern);
	WString wsid(sidRegExp);
	WString wfile(fileRegExp);
	PRINT("Current Search Pattern = %S, fUserRegExp = %S, fFileRegExp = %S\n", wpattern.getBuffer(), wsid.getBuffer(), wfile.getBuffer());
#endif

	fUserRegExpNeg = sidRegExp[0] == "~";
	fUserRegExpStr = fUserRegExpNeg ? sidRegExp.mid(1) : sidRegExp;
	fUserRegExpStr.replace(QRegExp(","), "|");
	fUserRegExp.setPattern(fUserRegExpStr);
	fUserRegExp.setCaseSensitive(false);
	fFileRegExpNeg = fileRegExp[0] == "~";
	fFileRegExpStr = fFileRegExpNeg ? fileRegExp.mid(1) : fileRegExp;
	fFileRegExp.setPattern(fFileRegExpStr);
	fFileRegExp.setCaseSensitive(false);

	if (!fGotResults)
	{
		SetSearchStatus(tr("Initializing..."));
		fPending = true;
		return;
	}

	fGotResults = false;
	fPending = false;

	fNetClient->AddSubscription(tmp); // <postmaster@raasu.org> 20021026
	// Test when initial results have been returned
	fNetClient->SendMessageToSessions(GetMessageFromPool(PR_COMMAND_PING)); 

	fSearchLock.Unlock();

	SetSearchStatus(tr("Searching for: \"%1\".").arg(Simplify(fileRegExp)));
	SetSearchStatus(tr("active"), 2);
}

void
WSearch::Download()
{
	fSearchLock.Lock();
	if (!fFileList.IsEmpty())
	{
		fDownload->setEnabled(false);
		DownloadQueue fQueue;

		WFileInfo * fi;

		HashtableIterator<String, WFileInfo *> iter = fFileList.GetIterator(HTIT_FLAG_NOREGISTER);
		
		while (iter.GetNextValue(fi) == B_OK)
		{
#ifdef _DEBUG
			WString w0(fi->fiListItem->text(0));
			WString w5(fi->fiListItem->text(5));
			PRINT("Checking: %S, %S\n", w0.getBuffer(), w5.getBuffer());
#endif

			if (fi->fiListItem->isSelected())
			{
				PRINT("DOWNLOAD: Found item\n");
				
				fQueue.addItem(fi->fiFilename, fi->fiUser);
			}					
		}

		fQueue.run();
		fDownload->setEnabled(true);
	}
	fSearchLock.Unlock();
}

void
WSearch::DownloadAll()
{
	fSearchLock.Lock();
	if (!fFileList.IsEmpty())
	{
		fDownload->setEnabled(false);
		DownloadQueue fQueue;

		WFileInfo * fi;
		HashtableIterator<String, WFileInfo *> iter = fFileList.GetIterator(HTIT_FLAG_NOREGISTER);
		
		while (iter.GetNextValue(fi) == B_OK)
		{
#ifdef _DEBUG
			WString w0(fi->fiListItem->text(0));
			WString w5(fi->fiListItem->text(5));
			PRINT("Checking: %S, %S\n", w0.getBuffer(), w5.getBuffer());
#endif

			fQueue.addItem(fi->fiFilename, fi->fiUser);

		}

		fQueue.run();
		fDownload->setEnabled(true);
	}
	fSearchLock.Unlock();
}

void
WSearch::SetResultsMessage()
{
	QString qmsg = tr("Results: %1").arg(fFileList.GetNumItems());
	qmsg += " (";
	qmsg += MakeSizeString(fQueryBytes);
	qmsg += ")";
	SetSearchStatus(qmsg, 1);
}

void
WSearch::SetSearchStatus(const QString & status, int index)
{
	fStatus->setText(status, index);
}

void
WSearch::SetSearch(const QString & pattern)
{
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
WSearch::ClearHistory()
{
	fSearchEdit->clear();
}

void
WSearch::HandleComboEvent(WTextEvent * e)
{
	if (e)
	{
	WComboBox * sender = (WComboBox *)e->data();
	if (sender == fSearchEdit)
	{
		PRINT("Received text change event from Search combo\n");
		GoSearch();
	}
	}
}

void
WSearch::customEvent(QCustomEvent *qce)
{
	if (fNetClient)		// do this to avoid bad crash
	{
		switch ((int) qce->type())
		{
		case WTextEvent::ComboType:
			{
				PRINT("\tWTextEvent::ComboType\n");
				HandleComboEvent(dynamic_cast<WTextEvent *>(qce));
				return;
			}
		}
	}
}

void
WSearch::LoadSettings()
{
	int i;
	QString str;

	ClearHistory();

	// load query history
	for (i = 0; (str = gWin->fSettings->GetQueryItem(i)) != QString::null; i++)
		fSearchEdit->insertItem(str, i);
	i = gWin->fSettings->GetCurrentQueryItem();
	if (i < fSearchEdit->count())
		fSearchEdit->setCurrentItem(i);

	fSearchList->setSorting(gWin->fSettings->GetSearchListSortColumn(), gWin->fSettings->GetSearchListSortAscending());

}

void
WSearch::SaveSettings()
{
	gWin->fSettings->EmptyQueryList();

	for (int i = 0; i < fSearchEdit->count(); i++)
	{
		QString qQuery = fSearchEdit->text(i).stripWhiteSpace();
		gWin->fSettings->AddQueryItem(qQuery);

#ifdef _DEBUG
		WString wquery(qQuery);
		PRINT("Saved query %S\n", wquery.getBuffer());
#endif
	}
	gWin->fSettings->SetCurrentQueryItem(fSearchEdit->currentItem());

	// search query list sort settings

	gWin->fSettings->SetSearchListSortColumn(fSearchList->sortColumn());
	gWin->fSettings->SetSearchListSortAscending(fSearchList->sortAscending());
}

void
WSearch::SetSearchPassive()
{
	SetSearchStatus(tr("passive"), 2);
	SetGotResults(true);
	if (fPending)
	{
		fPending = false;
		GoSearch();
	}
}
