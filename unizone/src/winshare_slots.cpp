// An extension of the main WinShare class
// Contains all the slots for menu messages (except Connect() and Disconnect())
#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qapplication.h>

#include "winsharewindow.h"
#include "aboutdlgimpl.h"
#include "formatting.h"
#include "colors.h"
#include "debugimpl.h"
#include "chattext.h"
#include "privatewindowimpl.h"
#include "prefsimpl.h"
#include "settings.h"
#include "gotourl.h"
#include "util/StringTokenizer.h"
#include "textevent.h"
#include "downloadimpl.h"
#include "util.h"
#include "wstring.h"
#include "filethread.h"
#include "netclient.h"
#include "wstatusbar.h"
#include "wsystemevent.h"
#include "textevent.h"

#include "events.h"

void
WinShareWindow::Exit()
{
	Cleanup();
	QApplication::exit(0);
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
WinShareWindow::UserConnected(const QString &sid)
{
	if (fSettings->GetUserEvents())
	{
		QString text = WFormat::UserConnected(sid);
		QString system = WFormat::Text(text);
		SendSystemEvent(system);
	}
	WUserRef uref = fNetClient->FindUser(sid);
	if (uref())
		uref()->AddToListView(fUsers);
	UpdateUserCount();
}
	
void
WinShareWindow::UserDisconnected(const QString &sid, const QString &name)
{
	if (fSettings->GetUserEvents())
	{
		QString uname = FixStringStr(name);
		QString msg;
		if (uname.isEmpty())
		{
			msg = WFormat::UserDisconnected2(sid);
		}
		else
		{
			// <postmaster@raasu.org> 20021112
			msg = WFormat::UserDisconnected(sid, uname); 
		}
		QString parse = WFormat::Text(msg);
		SystemEvent(this, parse);
	}
	UpdateUserCount();
}

void
WinShareWindow::UserNameChanged(const QString &sid, const QString &old, const QString &newname)
{
	if (fSettings->GetUserEvents())
	{
		QString system;

		// <postmaster@raasu.org> 20030622
		QString nameformat;
		if (old != "?" && !old.isEmpty())
		{
			// <postmaster@raasu.org> 20021112, 20030622
			nameformat = WFormat::UserNameChanged(sid, FixStringStr(old), FixStringStr(newname));  
		}
		else if (newname == "?" || newname.isEmpty())
		{
			// <postmaster@raasu.org> 20030819
			nameformat = WFormat::UserNameChangedNoNew(sid);  
		}
		else
		{
			// <postmaster@raasu.org> 20021112, 20030622
			nameformat = WFormat::UserNameChangedNoOld(sid, FixStringStr(newname)); 
		}
		system = WFormat::Text(nameformat);
		SystemEvent(this, system);
	}
	TextEvent(this, newname, WTextEvent::ResumeType);
}

void
WinShareWindow::DisconnectedFromServer()
{
	StopSearch();
	fDisconnectCount++;

	setStatus(tr( "Not connected." ), 0);
	setStatus( "", 1);
	
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
		SendErrorEvent(tr("Disconnected from server."));

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
WinShareWindow::UserStatusChanged(const QString &id, const QString &n, const QString &s)
{
	if (fSettings->GetUserEvents())
	{
		QString system;
		QString nameformat;

		// <postmaster@raasu.org> 20020929,20030211,20030214

		QString status = s;
		TranslateStatus(status);

		if (status.isEmpty())
			return;

		if ((n == "?") || (n.isEmpty())) // Invalid user name?
		{
			// <postmaster@raasu.org> 20030214
			nameformat = WFormat::UserStatusChanged2(id, FixStringStr(status)); 
		}
		else
		{
			// <postmaster@raasu.org> 20021112
			nameformat = WFormat::UserStatusChanged(id, FixStringStr(n), FixStringStr(status)); 
		}
		system = WFormat::Text(nameformat);
		SystemEvent(this, system);
	}
}

void
WinShareWindow::UserHostName(const QString &sid, const QString &host)
{
	if (fSettings->GetIPAddresses())
	{
		QString ip = WFormat::UserIPAddress2(sid, host);
		SystemEvent(this, ip);
	}
}

void
WinShareWindow::TabPressed(const QString &str)
{
	PRINT("Wow, tab received\n");
	QString result;
	if (DoTabCompletion(str, result, NULL))
	{
		fInputText->setText(result);
		fInputText->gotoEnd();
		PRINT("Returned true\n");
	}
#ifdef _DEBUG
	WString wResult(result);
	PRINT("Tab completion result: %S\n", wResult.getBuffer());
#endif
}

void
WinShareWindow::URLClicked(const QString & url)
{
	PRINT("URL Clicked\n");
	if (url != QString::null)
	{
		// <postmaster@raasu.org> 20021021
		//
		// added .lower() to all comparisons
		//
		QString surl;	// for LaunchSearch() and LaunchPrivate() 
		if (url.lower().startsWith("beshare:") || url.lower().startsWith("share:"))
		{
			surl = url.mid(url.find(":") + 1);
			LaunchSearch(surl);
		}
		else if (url.lower().startsWith("priv:"))	// <postmaster@raasu.org> 20021013
		{
			surl = url.mid(url.find(":") + 1);
			LaunchPrivate(surl);
		}
		else if (url.lower().startsWith("ttp://"))	// <postmaster@raasu.org> 20030911
		{
			surl = url.mid(url.find(":") + 3);		// skip ://
			QueueFile(surl);
		}
		else
			GotoURL(url);
	}
}

void
WinShareWindow::DoubleClicked(QListViewItem * i)
{
	QString uid = i->text(1).stripWhiteSpace();
	LaunchPrivate(uid);
}

void
WinShareWindow::RightButtonClicked(QListViewItem * i, const QPoint & p, int /* c */)
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
			WPrivateWindow * window = new WPrivateWindow(this, fNetClient, NULL);
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
			QString qTemp = WFormat::UserIPAddress(FixStringStr((*it).second()->GetUserName()), (*it).second()->GetUserHostName()); // <postmaster@raasu.org> 20021112
			SystemEvent(this, qTemp);
		}
		else if (id == 4)
		{
			GetAddressInfo((*it).second()->GetUserID());
		}
		else if (id == 5)
		{
			QString pingMsg("/ping ");
			pingMsg += (*it).second()->GetUserID();
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
	bool oldFirewalled = fSettings->GetFirewalled();

	if (prefs->exec() == QDialog::Accepted)	// only do the below code if the dialog was ACCEPTED!
	{
		SaveSettings();	// in case we crash :)

		if (fNetClient->IsConnected())
			fNetClient->SetConnection(fSettings->GetConnection());

		SetAutoAwayTimer();

		if (oldSharing && !fSettings->GetSharingEnabled())	// if we were previously sharing and are not now.. remove old data
		{
			fFileScanThread->EmptyList();
			fFilesScanned = false;
			StopAcceptThread();
			if (fNetClient->IsConnected())
			{
				CancelShares();
				fNetClient->SetFileCount(0);
				fNetClient->SetLoad(0, 0);
			}
			fScanning = false;
		}

		// this will also handle changes in firewall settings
		if (fSettings->GetSharingEnabled())		
		{
			if ((oldSharing == false) || (oldFirewalled != fSettings->GetFirewalled()))
			{
					StartAcceptThread();
					if (!fFilesScanned)
					{
						if (QMessageBox::information(this, tr( "File Scan" ), tr( "Scan your shared files now?" ), tr( "Yes" ), tr( "No" )) == 0)
							ScanShares();
					}
					else if (fNetClient->IsConnected())
						UpdateShares();
			}
			if (fNetClient->IsConnected())
				fNetClient->SetLoad(0, fSettings->GetMaxUploads());
		}

		if (fDLWindow)
		{
			SignalDownload(WDownload::DequeueDownloads);
			SignalDownload(WDownload::DequeueUploads);
		}			

		if (oldLogging && !fSettings->GetLogging())
		{
			if (fSettings->GetInfo())
				SystemEvent(this, tr("Logging disabled."));
			StopLogging();

			pLock.lock();
			for (WPrivIter privIter = fPrivateWindows.begin(); privIter != fPrivateWindows.end(); privIter++)
				(*privIter).first->StopLogging();
			pLock.unlock();
		}
		else if (!oldLogging && fSettings->GetLogging())
		{
			if (fSettings->GetInfo())
				SystemEvent(this, tr("Logging enabled."));
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
	PRINT("WinShareWindow::ReconnectTimer()\n");
	Connect();
}

void
WinShareWindow::ConnectTimer()
{
	PRINT("WinShareWindow::ConnectTimer()\n");

	fNetClient->Disconnect();

	if (fSettings->GetInfo())
		SystemEvent(this, tr("Connection to server failed!"));

	// Schedule reconnect attempt
	QCustomEvent * recon = new QCustomEvent(WinShareWindow::ConnectRetry);
	if (recon)
		QApplication::postEvent(this, recon);
}

void
WinShareWindow::AboutToQuit()
{
	Cleanup();

	QApplication::exit(0);
}

void
WinShareWindow::DownloadWindowClosed()
{
	PRINT("Download window closed!\n");
	fDLWindow = NULL;
}

/*
 * Insert failed download to resume list
 * -------------------------------------
 *
 * Failed downloads gets auto-resumed immediately
 *
 */

void
WinShareWindow::FileFailed(const QString &file, const QString &lfile, const QString &user)
{
	FileInterrupted(file, lfile, user);

	TextEvent(this, user, WTextEvent::ResumeType);
}

// Insert interrupted download to resume list
//

void
WinShareWindow::FileInterrupted(const QString &file, const QString &lfile, const QString &user)
{
	WResumeInfo wri;
	wri.fRemoteName = file;
	wri.fLocalName = lfile;

	WResumePair wrp = MakePair(user, wri);
	rLock.lock();
	if (!fResumeMap.empty())
	{
		// Check if file is already in the resume list
		for (WResumeIter iter = fResumeMap.begin(); iter != fResumeMap.end(); iter++)
		{
			if (
				((*iter).second.fRemoteName == file) &&
				((*iter).second.fLocalName == lfile) &&
				((*iter).first == user)
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
WinShareWindow::CheckResumes(const QString &user)
{
#ifdef _DEBUG
	WString wUser(StripURL(user));
	PRINT("CheckResumes: user   = %S\n", wUser.getBuffer());
#endif

	// No need to check if empty!
	if (fResumeMap.empty()) 
		return;

	WUserRef u = FindUser(user);
	
	if (u() == NULL)
		return;

	rLock.lock();

	Queue<QString> fFiles;
	Queue<QString> fLFiles;
	QString out;
	
	WResumeIter it = fResumeMap.begin();
	while (it != fResumeMap.end())
	{
#ifdef _DEBUG
		WString wSecond(StripURL((*it).first));
		PRINT("CheckResumes: user = %S\n", wSecond.getBuffer());
#endif

		if (StripURL((*it).first) == StripURL(user))
		{
			// User name matches

			if (fSettings->GetDownloads())
			{
				out += "\n" + tr("Trying to resume file %1 from user %2").arg((*it).second.fRemoteName).arg(user);
			}
			fFiles.AddTail((*it).second.fRemoteName);
			fLFiles.AddTail((*it).second.fLocalName);
			fResumeMap.erase(it);
			it = fResumeMap.begin(); // start from beginning
		}
		else
			it++;
	}
	rLock.unlock();
	if (fFiles.GetNumItems() > 0)
	{
		FixString(out);
		SystemEvent(this, out);
		// Make sure File Transfers window is open

		OpenDownload(); 

		fDLWindow->AddDownloadList(fFiles, fLFiles, u);
	}
}

void
WinShareWindow::SignalDownload(int type)
{
	QCustomEvent *qce = new QCustomEvent(type);
	if (qce) 
		QApplication::postEvent(fDLWindow, qce);
}
