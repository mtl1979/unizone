#include "winsharewindow.h"
#include "combo.h"
#include "debugimpl.h"
#include "downloadimpl.h"
#include "settings.h"
#include "wstatusbar.h"
#include "platform.h"
#include "wstring.h"
#include "searchitem.h"

void
WinShareWindow::AddFile(const QString sid, const QString filename, bool firewalled, MessageRef file)
{
	PRINT("ADDFILE called\n");
	if (firewalled && fSettings->GetFirewalled())
		return;	// we don't need to show this file if we are firewalled
	
	WString wFileName = filename;
	WString wSID = sid;
	PRINT("ADDFILE: filename=%S (%s) [%S]\n", wFileName.getBuffer(), firewalled ? "firewalled" : "hackable", wSID.getBuffer());

	// Workaround for StringMatcher bug (for now!)
	if (fCurrentSearchPattern.isEmpty())
		return;

	Lock();
	// see if the filename matches our file regex
	// AND that the session ID matches our session ID regex
	if (fFileRegExp.Match((const char *) filename.utf8()))
	{
		PRINT("ADDFILE: file Regex ok\n");
		if (fUserRegExp.Match((const char *) sid.utf8()))
		{
			// yes!
			WUserIter uit = fNetClient->Users().find(sid);
			if (uit != fNetClient->Users().end())	// found our user
			{
				WUserRef user = (*uit).second;

				// We need to check file properties before allocating new WFileInfo, saves time and one unnecessary
				// delete command.

				String path, kind;
				int64 size = 0;
				int32 mod = 0;
				
				file()->FindString("beshare:Kind", kind);
				file()->FindString("beshare:Path", path);
				file()->FindInt32("beshare:Modification Time", (int32 *)&mod);
				file()->FindInt64("beshare:File Size", (int64 *)&size);
/*
				if (size == 0)	// Can't download files of size 0
				{
					Unlock();	// Don't forget to unlock ;)
					return;
				}
*/
				WFileInfo * info = new WFileInfo;
				CHECK_PTR(info);
				info->fiUser = user;
				info->fiFilename = filename;
				info->fiRef = file;
				info->fiFirewalled = firewalled;
								
				// name, size, type, modified, path, user
				QString qkind	= QString::fromUtf8(kind.Cstr());
				QString qsize	= QString::number((int)size); 
				QString qmod	= QString::number(mod); // <postmaster@raasu.org> 20021126
				QString qpath	= QString::fromUtf8(path.Cstr());
				QString quser	= user()->GetUserName();
				
				info->fiListItem = new WSearchListItem(fSearchList, filename, qsize, qkind, qmod, qpath, quser);
				CHECK_PTR(info->fiListItem);

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
WinShareWindow::RemoveFile(const QString sid, const QString filename)
{
	Lock();
	PRINT("WSearch::RemoveFile\n");
	// go through our multi map and find the item that matches the sid and filename
	WFIIter iter = fFileList.begin();
	WFileInfo * info;

	WString wSID = sid;
	WString wFilename = filename;
	PRINT("Sid = %S, filename = %S\n", wSID.getBuffer(), wFilename.getBuffer());

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
WinShareWindow::StopSearch()
{
	Lock();
	if (fCurrentSearchPattern != "")	// we actually have an old search pattern,
	{
		// cancel it
		// fCurrentSearchPattern contains a fully formatted subscription string
		fNetClient->RemoveSubscription(fCurrentSearchPattern);
		// also dump pending results
		MessageRef cancel(GetMessageFromPool(PR_COMMAND_JETTISONRESULTS));
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
			fNetClient->SendMessageToSessions(cancel);
		}

		fCurrentSearchPattern = "";
		fFileRegExp.SetPattern("");
		fUserRegExp.SetPattern("");
	}
	Unlock();
	SetSearchStatus(tr("Idle."));
	SetSearchStatus("", 2);
}

// This method locks the list, so be sure the mutex is unlocked before calling it!
void
WinShareWindow::ClearList()
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
	SetSearchStatus("", 1);
}

void
WinShareWindow::GoSearch()
{
	// cancel current search (if there is one)
	StopSearch();	// these methods lock
	ClearList();

	if (fSearchEdit->currentText().stripWhiteSpace().isEmpty())	// no search string
		return;

	if (fNetClient->IsConnected() == false)
	{
		fStatus->message(tr("Not connected."));
		return;
	}

	// here we go with the new search pattern

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
			ConvertToRegex(userExp);
			MakeRegexCaseInsensitive(userExp);
			StringMatcher match(userExp.Cstr());
			WUserMap & users = fNetClient->Users();
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

	ConvertToRegex(fileExp);
	MakeRegexCaseInsensitive(fileExp);
	
	Unlock();	// unlock before StartQuery();

	StartQuery(userExp.Length() > 0 ? QString::fromUtf8(userExp.Cstr()) : "*", QString::fromUtf8(fileExp.Cstr()));
}

void
WinShareWindow::StartQuery(QString sidRegExp, QString fileRegExp)
{
	Lock();
	QString tmp = "SUBSCRIBE:/*/";
	tmp += sidRegExp;
	tmp += "/beshare/";
	tmp += fSettings->GetFirewalled() ? "files/" : "fi*/";		// if we're firewalled, we can only get files from non-firewalled users
	tmp += fileRegExp;
	fCurrentSearchPattern = tmp;
	// <postmaster@raasu.org> 20021023 -- Fixed typo

	WString wCurrentSearchPattern = fCurrentSearchPattern;
	WString wSIDRegExp = sidRegExp;
	WString wFileRegExp = fileRegExp;
	PRINT("Current Search Pattern = %S, fUserRegExp = %S, fFileRegExp = %S\n", wCurrentSearchPattern.getBuffer(), wSIDRegExp.getBuffer(), wFileRegExp.getBuffer());

	fUserRegExp.SetPattern((const char *) sidRegExp.utf8());
	fUserRegExpStr = sidRegExp;
	fFileRegExp.SetPattern((const char *) fileRegExp.utf8());
	fFileRegExpStr = fileRegExp;

	fGotResults = false;

	fNetClient->AddSubscription(tmp); // <postmaster@raasu.org> 20021026
	// Test when initial results have been returned
	fNetClient->SendMessageToSessions(GetMessageFromPool(PR_COMMAND_PING)); 

	Unlock();

	SetSearchStatus(tr("Searching for: \"%1\".").arg(fileRegExp));
	SetSearchStatus(tr("active"), 2);
}

void
WinShareWindow::Download()
{
	Lock();
	fDownload->setEnabled(false);
	if (!fFileList.empty())
	{
		WFIIter it = fFileList.begin();
		while (it != fFileList.end())
		{
			WFileInfo * fi = (*it).second;

			WString wFile = fi->fiListItem->text(0);
			WString wUser = fi->fiListItem->text(5);
			PRINT("Checking: %S, %S\n", wFile.getBuffer(), wUser.getBuffer());

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
WinShareWindow::SetResultsMessage()
{
	fStatus->setText(tr("Results: %1").arg(fFileList.size()), 1);
}

void
WinShareWindow::SetSearchStatus(const QString & status, int index)
{
	fStatus->setText(status, index);
}

void
WinShareWindow::SetSearch(QString pattern)
{
	fTabs->showPage(fSearchWidget);
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
WinShareWindow::QueueDownload(QString file, WUser * user)
{
	int32 i = 0;
	String mUser = (const char *) user->GetUserID().utf8();
	if (fQueue()->FindInt32(mUser, &i) == B_OK)
		fQueue()->RemoveName(mUser);
	fQueue()->AddInt32(mUser, ++i);
	mUser = mUser.Prepend("_");
	fQueue()->AddString(mUser, (const char *) file.utf8());
}

void
WinShareWindow::EmptyQueues()
{
	String mUser, mFile;
	QString user;
	QString * files;
	WUserRef u;
	int32 numItems;
	MessageFieldNameIterator iter = fQueue()->GetFieldNameIterator(B_INT32_TYPE);
	while (iter.GetNextFieldName(mUser) == B_OK)
	{
		user = QString::fromUtf8(mUser.Cstr());
		u = FindUser(user);
		if (u() != NULL)
		{
			fQueue()->FindInt32(mUser, &numItems);
			files = new QString[numItems];
			CHECK_PTR(files);
			mUser = mUser.Prepend("_");
			for (int32 i = 0; i < numItems; i++)
			{
				fQueue()->FindString(mUser, i, mFile);
				files[i] = QString::fromUtf8(mFile.Cstr());
			}
			OpenDownload();
			fDLWindow->AddDownload(files, NULL, numItems, u()->GetUserID(), u()->GetPort(), u()->GetUserHostName(), u()->GetInstallID(), u()->GetFirewalled(), u()->GetPartial());
		}
	}
	fQueue()->Clear();
}


void
WinShareWindow::ClearHistory()
{
	fSearchEdit->clear();
}
