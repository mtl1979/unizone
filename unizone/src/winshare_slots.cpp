// An extension of the main WinShare class
// Contains all the slots for menu messages (except Connect() and Disconnect())
#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "winsharewindow.h"
#include "aboutdlgimpl.h"
#include "formatting.h"
#include "debugimpl.h"
#include "chattext.h"
#include "privatewindowimpl.h"
#include "prefsimpl.h"
#include "settings.h"
#include "search.h"
#include "gotourl.h"
#include "util/StringTokenizer.h"

#include <qapplication.h>

void
WinShareWindow::Exit()
{
	qApp->exit(0);
}

void
WinShareWindow::OpenDownloadsFolder()
{
	GotoURL("downloads");
}

void
WinShareWindow::OpenLogsFolder()
{
	GotoURL("logs");
}

void
WinShareWindow::OpenSharedFolder()
{
	GotoURL("shared");
}

void
WinShareWindow::ClearChatLog()
{
}

void
WinShareWindow::AboutWinShare()
{
	AboutDlg * win = new AboutDlg;
	CHECK_PTR(win);
	win->show();
}

// user slots
void
WinShareWindow::UserConnected(QString sid)
{
	if (fSettings->GetUserEvents())
	{
		QString system = WFormat::SystemText().arg(WColors::System).arg(fSettings->GetFontSize());
		system += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(WFormat::UserConnected().arg(sid));
		PrintText(system);
	}
}
void
WinShareWindow::UserDisconnected(QString sid, QString name)
{
	if (fSettings->GetUserEvents())
	{
		QString system;

		system = WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(WFormat::UserDisconnected().arg(sid).arg(FixStringStr(name)).arg(WColors::RemoteName)); // <postmaster@raasu.org> 20021112
		PrintSystem(system);
	}
}

void
WinShareWindow::UserNameChanged(QString sid, QString old, QString newname)
{
	if (fSettings->GetUserEvents())
	{
		QString system = WFormat::SystemText().arg(WColors::System).arg(fSettings->GetFontSize());

		if (old != "?" && old.length() > 0)
			system += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(WFormat::UserNameChanged().arg(sid).arg(FixStringStr(old)).arg(FixStringStr(newname)).arg(WColors::RemoteName).arg(WColors::RemoteName)); // <postmaster@raasu.org> 20021112 
		else
			system += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(WFormat::UserNameChangedNoOld().arg(sid).arg(FixStringStr(newname)).arg(WColors::RemoteName)); // <postmaster@raasu.org> 20021112
		PrintText(system);
	}
	WTextEvent * wte = new WTextEvent(newname, WTextEvent::ResumeType);
	if (wte)
		QApplication::postEvent(this, wte);
}

void
WinShareWindow::DisconnectedFromServer()
{
	fDisconnectCount++;
	
	if ((fDisconnectCount == 1) && (!fDisconnectFlag))
	{
		fDisconnect = true; // Premature disconnection detected!!!
	}
	
	if ((fDisconnectCount == 3) && (!fDisconnect))
	{
		fDisconnect = true;	// Premature disconnection detected!!!
	}

	Disconnect2();

	PRINT("DisconnectedFromServer()\n");

	if (fSettings->GetError() && !fDisconnect) // Don't flood the user ;)
		PrintError(tr("Disconnected from server."));

	if (fDLWindow)
		fDLWindow->KillLocalQueues();	// locally queued files will never connect to their peers when we get disconnected

	if (fDisconnect)
	{
		//Connect();			// Reconnect ;)
		QCustomEvent * recon = new QCustomEvent(WinShareWindow::ConnectRetry);
		if (recon)
			QApplication::postEvent(this, recon);
	}
}

void
WinShareWindow::UserStatusChanged(QString id, QString n, QString s)
{
	if (fSettings->GetUserEvents())
	{
		QString system = WFormat::SystemText().arg(WColors::System).arg(fSettings->GetFontSize());

		// <postmaster@raasu.org> 20020929,20030211,20030214

		TranslateStatus(s);

		if ((n == "?") || (n == "")) // Invalid user name?
		{
			// <postmaster@raasu.org> 20030214
			system += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(
				WFormat::UserStatusChanged2().arg(id).arg(FixStringStr(s)) 
				);	
		}
		else
		{
			// <postmaster@raasu.org> 20021112
			system += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(
				WFormat::UserStatusChanged().arg(id).arg(FixStringStr(n)).arg(FixStringStr(s)).arg(WColors::RemoteName) 
				); 
		}
		PrintText(system);
	}
}

void
WinShareWindow::TabPressed(QString str)
{
	PRINT("Wow, tab received\n");
	QString result;
	if (DoTabCompletion(str, result, NULL))
	{
		fInputText->setText(result);
		fInputText->setCursorPosition(9999, 9999);
		PRINT("Returned true\n");
	}
	PRINT("Tab completion result: %S\n", qStringToWideChar(result));
}

void
WinShareWindow::URLClicked()
{
	PRINT("URL Clicked\n");
	if (fCurURL != QString::null)
	{
		// <postmaster@raasu.org> 20021021
		//
		// added .lower() to all comparisons
		//
		QString surl;	// for LaunchSearch() and LaunchPrivate() 
		if (fCurURL.lower().startsWith("beshare:") || fCurURL.lower().startsWith("share:"))
		{
			surl = fCurURL.mid(fCurURL.find(":") + 1);
			LaunchSearch(surl);
		}
		else if (fCurURL.lower().startsWith("priv:")) // <postmaster@raasu.org> 20021013
		{
			surl = fCurURL.mid(fCurURL.find(":") + 1);
			LaunchPrivate(surl);
		}
		else
			GotoURL(fCurURL);
	}
}

void
WinShareWindow::URLSelected(const QString & href)
{
	PRINT("URL Selected %S\n", qStringToWideChar(href));
	fCurURL = href;
}

void
WinShareWindow::RightButtonClicked(QListViewItem * i, const QPoint & p, int c)
{
	// empty menu
	while (fPrivate->count() > 0)
		fPrivate->removeItemAt(0);
	if (i)
	{
		QString uid = i->text(1).stripWhiteSpace();		// session ID
		WUserIter it = fNetClient->Users().begin();
		while (it != fNetClient->Users().end())
		{
			if ((*it).second()->GetUserID() == uid)
			{
				// found user...
				// <postmaster@raasu.org> 20020927 -- Added internationalization
				QString txt = tr("Private Chat With %1").arg(StripURL((*it).second()->GetUserName()));
				// <postmaster@raasu.org> 20020924 -- Added ',1'
				fPrivate->insertItem(txt, 1);
				// <postmaster@raasu.org> 20020924 -- Added id 2
				fPrivate->insertItem(tr("List All Files"), 2);
				// <postmaster@raasu.org> 20020926 -- Added id 3
				fPrivate->insertItem(tr("Get IP Address"), 3);
				// <postmaster@raasu.org> 20030307 -- Inserted new item as id 4, moved old as id 5
				fPrivate->insertItem(tr("Get Address Info"), 4);
				txt = tr("Ping %1").arg(StripURL((*it).second()->GetUserName()));
				fPrivate->insertItem(txt, 5);

				fPopupUser = uid;
				fPrivate->popup(p);
			}
			it++;
		}
	}
}

void
WinShareWindow::PopupActivated(int id)
{
	// <postmaster@raasu.org> 20020924 -- Add id detection
	WUserIter it = fNetClient->Users().find(fPopupUser);
	if (it != fNetClient->Users().end())
	{
		if (id == 1) {
			WPrivateWindow * window = new WPrivateWindow(this, fNetClient, this);
			CHECK_PTR(window);
			window->AddUser((*it).second);
			window->show();
			// it's a map... but so far, there is no need for a key
			// as i just iterate through the list
			WPrivPair p = MakePair(window);
			pLock.lock();
			fPrivateWindows.insert(p);
			pLock.unlock();
		} 
		else if (id == 2) 
		{
			QString qPattern = "*@";
			qPattern += (*it).second()->GetUserID();
			WinShareWindow::LaunchSearch(qPattern);
		} 
		else if (id == 3) 
		{
			QString qTemp = tr("<font color=\"%3\">%1</font>'s IP address is %2.").arg(FixStringStr((*it).second()->GetUserName())).arg((*it).second()->GetUserHostName()).arg(WColors::RemoteName); // <postmaster@raasu.org> 20021112
			PrintSystem(qTemp,false);
		}
		else if (id == 4)
		{
			GetAddressInfo((*it).second()->GetUserID());
		}
		else if (id == 5)
		{
			QString pingMsg = tr("/ping %1").arg((*it).second()->GetUserID());
			SendPingOrMsg(pingMsg, true);
		}

	}
}

void
WinShareWindow::Preferences()
{
	WPrefs * prefs = new WPrefs(NULL, NULL, true);
	CHECK_PTR(prefs);
	bool oldSharing = fSettings->GetSharingEnabled();
	bool oldLogging = fSettings->GetLogging();

	if (prefs->exec() == QDialog::Accepted)	// only do the below code if the dialog was ACCEPTED!
	{
		SaveSettings();	// in case we crash :)
		if (fNetClient->IsInternalThreadRunning())
			fNetClient->SetConnection(fSettings->GetConnection());
		SetAutoAwayTimer();

		if (fNetClient->IsInternalThreadRunning())	// are we still connected?
		{
			if (oldSharing && !fSettings->GetSharingEnabled())	// if we were previously sharing and are not now.. remove old data
			{
				CancelShares();
				fNetClient->SetFileCount(0);
				fFileScanThread->EmptyList();
				StopAcceptThread();
				fNetClient->SetLoad(0, 0);
			}
			if (fSettings->GetSharingEnabled())		// this will also handle changes in firewall settings
			{
				if (QMessageBox::information(this, "File Scan", "Scan your shared files now?", "Yes", "No") == 0)
				{
					ScanShares();
					if (!fAccept)	// accept thread not running yet?
						StartAcceptThread();
				}
				fNetClient->SetLoad(0, fSettings->GetMaxUploads());
			}
			if (fDLWindow)
			{
				fDLWindow->DequeueDLSessions();
				fDLWindow->DequeueULSessions();
			}

			
		}

		if (oldLogging && !fSettings->GetLogging())
		{
			if (fSettings->GetInfo())
				PrintSystem(tr("Logging disabled."));
			StopLogging();

			pLock.lock();
			for (WPrivIter privIter = fPrivateWindows.begin(); privIter != fPrivateWindows.end(); privIter++)
				(*privIter).first->StopLogging();
			pLock.unlock();
		}
		else if (!oldLogging && fSettings->GetLogging())
		{
			if (fSettings->GetInfo())
				PrintSystem(tr("Logging enabled."));
			StartLogging();
			pLock.lock();
			for (WPrivIter privIter = fPrivateWindows.begin(); privIter != fPrivateWindows.end(); privIter++)
				(*privIter).first->StartLogging();
			pLock.unlock();
		}
	}
}

void
WinShareWindow::AutoAwayTimer()
{
	SetStatus(fAwayMsg);
	fAway = true;
}

void
WinShareWindow::ReconnectTimer()
{
	Connect();
}

void
WinShareWindow::SearchDialog()
{
	LaunchSearch("");
}

void
WinShareWindow::GotShown(const QString & txt)
{
#ifndef WIN32
	fChatText->setText(ParseForShown(txt));
	UpdateTextView();
#endif
}

void
WinShareWindow::AboutToQuit()
{
	WaitOnFileThread();
	SaveSettings();
	QApplication::exit(0);
}

void
WinShareWindow::SearchWindowClosed()
{
	PRINT("Search window closed!\n");
	fSearchWindow = NULL;
}

void
WinShareWindow::ChannelsWindowClosed()
{
	PRINT("Channels window closed!\n");
	fChannels = NULL;
}

void
WinShareWindow::DownloadWindowClosed()
{
	PRINT("Download window closed!\n");
	fDLWindow = NULL;
}

// Insert failed download to resume list
//

void
WinShareWindow::FileFailed(QString file, QString user)
{
	FileInterrupted(file, user);

	WTextEvent * wte = new WTextEvent(user, WTextEvent::ResumeType);
	if (wte)
	{
		QApplication::postEvent(this, wte);
	}
}

// Insert interrupted download to resume list
//

void
WinShareWindow::FileInterrupted(QString file, QString user)
{
	WResumePair wrp = MakePair(file, user);
	rLock.lock();
	if (!fResumeMap.empty())
	{
		// Check if file is already in the resume list
		for (WResumeIter iter = fResumeMap.begin(); iter != fResumeMap.end(); iter++)
		{
			if (
				((*iter).first == file) &&
				((*iter).second == user)
				)
			{
				rLock.unlock();
				return;
			}
		}
	}
	fResumeMap.insert(wrp);
	rLock.unlock();
}

// Check username against resume list
//

void 
WinShareWindow::CheckResumes(QString user)
{
	// No need to check if empty!
	PRINT("CheckResumes: user   = %S\n", qStringToWideChar( StripURL(user) ));

	if (fResumeMap.empty()) 
		return;

	WUserRef u = FindUser(user);
	
	if (u() == NULL)
		return;

	rLock.lock();

	Queue<QString> fFiles;
	
	WResumeIter it = fResumeMap.begin();
	while (it != fResumeMap.end())
	{
		PRINT("CheckResumes: second = %S\n", qStringToWideChar( StripURL((*it).second) ));
		if (StripURL((*it).second) == StripURL(user))
		{
			// User name matches

			PrintSystem( tr("Trying to resume file %1 from user %2").arg((*it).first).arg(user), false);
			fFiles.AddTail((*it).first);
			fResumeMap.erase(it);
			it = fResumeMap.begin(); // start from beginning
		}
		else
			it++;
	}
	rLock.unlock();
	if (fFiles.GetNumItems() > 0)
	{
		// Make sure File Transfers window is open

		OpenDownload(); 

		fDLWindow->AddDownloadList(fFiles, u());
	}
}
