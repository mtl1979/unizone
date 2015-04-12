// An extension of the main WinShare class
// Contains all the slots for menu messages (except Connect() and Disconnect())
#include <qapplication.h>
#include <qmessagebox.h>
#include <QCustomEvent>

#include "winsharewindow.h"
#include "aboutdlgimpl.h"
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
#include "uploadimpl.h"
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
	Clear();
}

void
WinShareWindow::AboutWinShare()
{
	AboutDlg * win = new AboutDlg;
	Q_CHECK_PTR(win);
	win->show();
}

// user slots

void
WinShareWindow::UserConnected(const WUserRef & uref)
{
	QString sid = uref()->GetUserID();
	if (fSettings->GetUserEvents())
	{
		QString text = FormatUserConnected(sid);
		SendSystemEvent(text);
	}
	uref()->AddToListView(fUsers);
	UpdateUserCount();
}
	
void
WinShareWindow::UserDisconnected(const WUserRef & uref)
{
	if (fSettings->GetUserEvents())
	{
		QString msg = FormatUserDisconnected(uref()->GetUserID(), FixString(uref()->GetUserName()));
		SendSystemEvent(msg);
	}
	uref()->RemoveFromListView(fUsers);
	UpdateUserCount();
}

void
WinShareWindow::UserNameChanged(const WUserRef & uref, const QString &old, const QString &newname)
{
	if (fSettings->GetUserEvents())
	{
		// <postmaster@raasu.org> 20030622
		QString sid = uref()->GetUserID();
		QString nameformat;
		if (WUser::CheckName(newname))
		{
			if (WUser::CheckName(old))
			{
				// <postmaster@raasu.org> 20021112, 20030622
				nameformat = FormatUserNameChanged(sid, FixString(old), FixString(newname));
			}
			else
			{
				// <postmaster@raasu.org> 20021112, 20030622
				nameformat = FormatUserNameChangedNoOld(sid, FixString(newname));
			}
			SendSystemEvent(nameformat);
			SendTextEvent(newname, WTextEvent::ResumeType);
		}
		else
		{
			// <postmaster@raasu.org> 20030819
			nameformat = FormatUserNameChangedNoNew(sid);
			SendSystemEvent(nameformat);
		}
	}
}

void
WinShareWindow::DisconnectedFromServer()
{
	if (fSearch)
		fSearch->StopSearch();
	fDisconnectCount++;

	setStatus(tr( "Not connected." ), 0);
	setStatus( QString::null, 1);
	setStatus( QString::null, 2);
	
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
		// Reconnect ;)
		QCustomEvent * recon = new QCustomEvent(WinShareWindow::ConnectRetry);
		if (recon)
			QApplication::postEvent(this, recon);
	}
}

void
WinShareWindow::UserStatusChanged(const WUserRef & uref, const QString &n, const QString &s)
{
	if (fSettings->GetUserEvents())
	{
		// <postmaster@raasu.org> 20020929,20030211,20030214
		// <postmaster@raasu.org> 20041229 -- Strip extra spaces before trying to translate
		QString status = TranslateStatus(s.stripWhiteSpace());

		if (status.isEmpty())
			return;

		// <postmaster@raasu.org> 20021112
		QString nameformat = FormatUserStatusChanged(uref()->GetUserID(), FixString(n), FixString(status));
		SendSystemEvent(nameformat);
	}
}

void
WinShareWindow::UserHostName(const WUserRef & uref, const QString &host)
{
	QString sid = uref()->GetUserID();
	if (fSettings->GetIPAddresses())
	{
		QString ip = FormatUserIPAddress2(sid, host);
		SendSystemEvent(ip);
	}
}

void
WinShareWindow::TabPressed(const QString &str)
{
	PRINT("Wow, tab received\n");
	QString result;
	if (DoTabCompletion(str, result))
	{
		fInputText->setText(result);
		fInputText->gotoEnd();
		PRINT("Returned true\n");
	}
#ifdef _DEBUG
	WString wres(result);
	PRINT("Tab completion result: %S\n", wres.getBuffer());
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
		if (url.startsWith("beshare:", false) || url.startsWith("share:", false))
		{
			surl = url.mid(url.find(":") + 1);
			LaunchSearch(surl);
		}
		else if (url.startsWith("priv:", false))	// <postmaster@raasu.org> 20021013
		{
			surl = url.mid(url.find(":") + 1);
			LaunchPrivate(surl);
		}
		else if (url.startsWith("ttp://", false))	// <postmaster@raasu.org> 20030911
		{
			surl = url.mid(url.find(":") + 3);		// skip ://
			QueueFile(surl);
		}
		else
			GotoURL(url);
	}
}

void
WinShareWindow::DoubleClicked(Q3ListViewItem * i)
{
	QString uid = i->text(1).stripWhiteSpace();
	LaunchPrivate(uid);
}

void
WinShareWindow::RightButtonClicked(Q3ListViewItem * i, const QPoint & p, int /* c */)
{
	// empty menu
	while (fPrivate->count() > 0)
		fPrivate->removeItemAt(0);
	if (i)
	{
		QString uid = i->text(1).stripWhiteSpace();		// session ID
		bool ok;
		uint32 sid = uid.toULong(&ok);
		WUserMap & umap = fNetClient->Users();
		if (umap.ContainsKey(sid))
		{
			WUserRef uref;
			umap.Get(sid, uref);
			// <postmaster@raasu.org> 20020927 -- Added internationalization
			QString txt = tr("Private Chat With %1").arg(StripURL(uref()->GetUserName()));
			// <postmaster@raasu.org> 20020924 -- Added ',1'
			fPrivate->insertItem(txt, 1);
			// <postmaster@raasu.org> 20020924 -- Added id 2
			fPrivate->insertItem(tr("List All Files"), 2);
			// <postmaster@raasu.org> 20020926 -- Added id 3
			fPrivate->insertItem(tr("Get IP Address"), 3);
			// <postmaster@raasu.org> 20030307 -- Inserted new item as id 4, moved old as id 5
			fPrivate->insertItem(tr("Get Address Info"), 4);
			txt = tr("Ping %1").arg(StripURL(uref()->GetUserName()));
			fPrivate->insertItem(txt, 5);
			
			fPopupUser = uid;
			fPrivate->popup(p);
		}
	}
}

void
WinShareWindow::PopupActivated(int id)
{
	// <postmaster@raasu.org> 20020924 -- Add id detection
	WUserRef uref = fNetClient->FindUser(fPopupUser);
	if (uref())
	{
		switch (id)
		{
			
		case 1:
			{
				WPrivateWindow * window = new WPrivateWindow(this, fNetClient, NULL);
				Q_CHECK_PTR(window);
				window->AddUser(uref);
				window->show();
				pLock.Lock();
				fPrivateWindows.AddTail(window);
				pLock.Unlock();
			}
			break;
		case 2:
			{
				QString qPattern = "*@";
				qPattern += uref()->GetUserID();
				WinShareWindow::LaunchSearch(qPattern);
			}
			break;
		case 3:
			{
				QString qTemp = FormatUserIPAddress(FixString(uref()->GetUserName()), uref()->GetUserHostName()); // <postmaster@raasu.org> 20021112
				SendSystemEvent(qTemp);
			}
			break;
		case 4:
			{
				GetAddressInfo(uref()->GetUserID());
			}
			break;
		case 5:
			{
				QString pingMsg("/ping ");
				pingMsg += uref()->GetUserID();
				SendPingOrMsg(pingMsg, true);
			}
			break;

		}

	}
}

void
WinShareWindow::Preferences()
{
	WPrefs * prefs = new WPrefs(NULL, NULL, true);
	Q_CHECK_PTR(prefs);
	bool oldSharing = fSettings->GetSharingEnabled();
	bool oldLogging = fSettings->GetLogging();
	bool oldFirewalled = fSettings->GetFirewalled();
	bool oldStamps = fSettings->GetTimeStamps();

	if (prefs->exec() == QDialog::Accepted)	// only do the below code if the dialog was ACCEPTED!
	{
		(void) SaveSettings();	// in case we crash :)

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
					if (fFilesScanned)
					{
						if (fNetClient->IsConnected())
							UpdateShares();
					}
					else if (QMessageBox::information(this, tr( "File Scan" ), tr( "Scan your shared files now?" ), tr( "Yes" ), tr( "No" )) == 0)
					{
						ScanShares();
					}
			}
			if (fNetClient->IsConnected())
				fNetClient->SetLoad(0, fSettings->GetMaxUploads());
		}

		if (fDLWindow)
		{
			SignalDownload(WDownload::DequeueDownloads);
		}

		if (fULWindow)
		{
			SignalUpload(WUpload::DequeueUploads);
		}			

		if (oldLogging && !fSettings->GetLogging())
		{
			if (fSettings->GetInfo())
				SendSystemEvent(tr("Logging disabled."));

			StopLogging();

			pLock.Lock();
			for (unsigned int i = 0; i < fPrivateWindows.GetNumItems(); i++)
				fPrivateWindows[i]->StopLogging();
			pLock.Unlock();

			fChannels->StartLogging();
		}
		else if (!oldLogging && fSettings->GetLogging())
		{
			if (fSettings->GetInfo())
				SendSystemEvent(tr("Logging enabled."));

			StartLogging();

			pLock.Lock();
			for (unsigned int i = 0; i < fPrivateWindows.GetNumItems(); i++)
				fPrivateWindows[i]->StartLogging();
			pLock.Unlock();

			fChannels->StopLogging();
		}

		if (oldStamps && !fSettings->GetTimeStamps())
		{
			setStatus(QString::null, 3);
			killTimer(timerID);
			timerID = 0;
		}
		else if (!oldStamps && fSettings->GetTimeStamps())
		{
			timerID = startTimer(1000);
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

	setStatus(tr( "Not connected." ), 0);
	setStatus( QString::null, 1);
	setStatus( QString::null, 2);

	if (fSettings->GetError())
		SendErrorEvent(tr("Connection to server failed!"));

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

void
WinShareWindow::UploadWindowClosed()
{
	PRINT("Upload window closed!\n");
	fULWindow = NULL;
}

/*
 * Insert failed download to resume list
 * -------------------------------------
 *
 * Failed downloads gets auto-resumed immediately
 *
 */

void
WinShareWindow::FileFailed(const QString &file, const QString &lfile, const QString &path, const QString &tfile, const QString &user)
{
	FileInterrupted(file, lfile, path, tfile, user);

	SendTextEvent(user, WTextEvent::ResumeType);
}

// Insert interrupted download to resume list
//

void
WinShareWindow::FileInterrupted(const QString &file, const QString &lfile, const QString &path, const QString &tfile, const QString &user)
{
	WResumeInfo wri;
	wri.fRemoteName = file;
	wri.fLocalName = lfile;
	wri.fPath = path;
	wri.fTempName = tfile;

	rLock.Lock();
	if (!fResumeMap.IsEmpty())
	{
		// Check if file is already in the resume list
		for (unsigned int x = 0; x < fResumeMap.GetNumItems(); x++)
		{
         if (fResumeMap[x].user == user)
         {
            for (unsigned int y = 0; y < fResumeMap[x].files.GetNumItems(); y++)
            {
               if (
                  (fResumeMap[x].files[y].fRemoteName == file) &&
                  (fResumeMap[x].files[y].fLocalName == lfile) &&
                  (fResumeMap[x].files[y].fPath == path) &&
                  (fResumeMap[x].files[y].fTempName == tfile)
                  )
               {
                  rLock.Unlock();
                  return;
               }
            }
            fResumeMap[x].files.AddTail(wri);
            rLock.Unlock();
            return;
         }
		}
	}

   // Create entry for user and then add file as initial entry
   {
      WResumePair wrp;
      wrp.user = user;
      wrp.files.AddTail(wri);

      fResumeMap.AddTail(wrp);
   }

   rLock.Unlock();
}

// Check username against resume list
//

void
WinShareWindow::CheckResumes(const QString &user)
{
#ifdef _DEBUG
	WString wuser(StripURL(user));
	PRINT("CheckResumes: user = %S\n", wuser.getBuffer());
#endif

	// No need to check if empty!
	if (fResumeMap.IsEmpty())
		return;

	if (!fResumeEnabled)
		return;

	WUserRef u = FindUser(user);
	
	if (u() == NULL)
		return;

	rLock.Lock();

	Queue<QString> fFiles;
	Queue<QString> fLFiles;
	Queue<QString> fPaths;
	Queue<QString> fTFiles;
	QString out;
	
	for (unsigned int x = 0; x < fResumeMap.GetNumItems(); x++)
	{
#ifdef _DEBUG
		wuser = StripURL(fResumeMap[x].user);
		PRINT("CheckResumes: user = %S\n", wuser.getBuffer());
#endif

		if (StripURL(fResumeMap[x].user) == StripURL(user))
		{
			// User name matches

         for (unsigned int y = 0; y < fResumeMap[x].files.GetNumItems(); y++)
         {
            if (fSettings->GetDownloads())
            {
               if (!out.isEmpty())
                  out += "\n";
               out += tr("Trying to resume file %1 from user %2").arg(fResumeMap[x].files[y].fRemoteName).arg(user);
            }
            fFiles.AddTail(fResumeMap[x].files[y].fRemoteName);
            fLFiles.AddTail(fResumeMap[x].files[y].fLocalName);
            fPaths.AddTail(fResumeMap[x].files[y].fPath);
			fTFiles.AddTail(fResumeMap[x].files[y].fTempName);
         }
			fResumeMap.RemoveItemAt(x);
         break;
		}
	}
	rLock.Unlock();
	if (fFiles.GetNumItems() > 0)
	{
		SendSystemEvent(FixString(out));
		// Make sure File Transfers window is open

		OpenDownload();

		fDLWindow->AddDownloadList(fFiles, fLFiles, fPaths, fTFiles, u);
	}
}

void
WinShareWindow::SignalDownload(int type)
{
	QCustomEvent *qce = new QCustomEvent(type);
	if (qce)
		QApplication::postEvent(fDLWindow, qce);
}

void
WinShareWindow::SignalUpload(int type)
{
	QCustomEvent *qce = new QCustomEvent(type);
	if (qce)
		QApplication::postEvent(fULWindow, qce);
}
