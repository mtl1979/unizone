#include "winsharewindow.h"

#include <qapplication.h>
#include <q3mainwindow.h>
#include <q3stylesheet.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qimage.h>
#if !defined(QT_NO_STYLE_MOTIF)
#include <qmotifstyle.h>
#endif
#if !defined(QT_NO_STYLE_WINDOWS)
#include <qwindowsstyle.h>
#endif
#if defined(_WIN32)
# if !defined(QT_NO_STYLE_WINDOWSXP)
#  include <qwindowsxpstyle.h>
# endif
# if !defined(QT_NO_STYLE_WINDOWSVISTA)
#  include <qwindowsvistastyle.h>
# endif
#endif
#if !defined(QT_NO_STYLE_CDE)
#include <qcdestyle.h>
#endif
#if defined(__APPLE__)
# if !defined(QT_NO_STYLE_MAC)
#	include <qmacstyle_mac.h>
# endif
#endif
#if !defined(QT_NO_STYLE_PLASTIQUE)
#include <qplastiquestyle.h>
#endif
#include <qtextcodec.h>
#include <qdir.h>
#include <qinputdialog.h>
#include <qtoolbar.h>
#include <qregexp.h>
#include <QTimerEvent>
#include <QResizeEvent>
#include <QLabel>
#include <Q3GridLayout>
#include <Q3ValueList>
#include <QKeyEvent>
#include <QCustomEvent>
#include <Q3PopupMenu>
#include <Q3DockArea>

#include "util/StringTokenizer.h"
#include "iogateway/PlainTextMessageIOGateway.h"
#include "system/SystemInfo.h"
#include "zlib/ZLibUtilityFunctions.h"

#include "downloadimpl.h"
#include "downloadqueue.h"
#include "uploadimpl.h"
#include "events.h"
#include "version.h"
#include "debugimpl.h"
#include "chattext.h"
#include "textevent.h"
#include "htmlview.h"
#include "privatewindowimpl.h"
#include "picviewerimpl.h"
#include "wmessageevent.h"
#include "wpwevent.h"
#include "wsystemevent.h"
#include "wwarningevent.h"
#include "werrorevent.h"
#include "combo.h"
#include "menubar.h"
#include "settings.h"
#include "filethread.h"
#include "listthread.h"
#include "resolverthread.h"
#include "tokenizer.h"								// <postmaster@raasu.org> 20021114
#include "listutil.h"
#include "util.h"
#include "wstring.h"
#include "nicklist.h"
#include "searchitem.h"
#include "user.h"
#include "wstatusbar.h"
#include "netclient.h"
#include "serverclient.h"
#include "updateclient.h"
#include "gotourl.h"
#include "resolver.h"

#ifdef WIN32
#include <objbase.h>
#endif

// uptime

#if defined(__LINUX__) || defined(linux)
#include <sys/sysinfo.h>
#elif defined(__FreeBSD__) || defined(__QNX__) || defined(__APPLE__)
#include <sys/time.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#elif defined(__sun) && defined(__SVR4)
#include <utmpx.h>
#endif

#define NUM_TOOLBARS 3

enum
{
	UID_INVALID = -1,
	UID_SERVERBAR,
	UID_NICKBAR,
	UID_STATUSBAR
};

WinShareWindow * gWin = NULL;

WinShareWindow::WinShareWindow(QWidget * parent, const char* name, Qt::WFlags f)
	: ChatWindow(MainType, parent, name, f | Qt::WPaintDesktop | Qt::WPaintClever)
{
	fMenus = NULL;

	if ( !name )
		setName( "WinShareWindow" );

	// IMPORTANT!! :)
	CreateDirectories();
	gWin = this;
	fDLWindow = NULL;
	fULWindow = NULL;
	fSearch = NULL;
	fPicViewer = NULL;
	fAccept = NULL;
	fFileScanThread = NULL;
	fFilesScanned = false;
	fMaxUsers = 0;
	timerID = 0;

	InitLaunchThread();

	fUpdateThread = new UpdateClient(this);
	Q_CHECK_PTR(fUpdateThread);

	fDisconnectCount = 0;	// Initialize disconnection count
	fDisconnect = false; // No premature disconnection yet
	fDisconnectFlag = false; // User hasn't disconnected manually yet.
	fResumeEnabled = true;

	fNetClient = new NetClient(this);
	Q_CHECK_PTR(fNetClient);

	fChannels = new Channels(NULL, fNetClient);
	Q_CHECK_PTR(fChannels);

	fServerThread = new ServerClient(this);
	Q_CHECK_PTR(fServerThread);

	fSettings = new WSettings;
	Q_CHECK_PTR(fSettings);

	setWindowTitle("Unizone");

	resize(800, 600);
	InitGUI();

	connect(fNetClient, SIGNAL(UserDisconnected(const WUserRef &)),
			this, SLOT(UserDisconnected(const WUserRef &)));
	connect(fNetClient, SIGNAL(UserConnected(const WUserRef &)),
			this, SLOT(UserConnected(const WUserRef &)));
	connect(fNetClient, SIGNAL(UserNameChanged(const WUserRef &, const QString &, const QString &)),
			this, SLOT(UserNameChanged(const WUserRef &, const QString &, const QString &)));
	connect(fNetClient, SIGNAL(DisconnectedFromServer()),
			this, SLOT(DisconnectedFromServer()));
	connect(fNetClient, SIGNAL(UserStatusChanged(const WUserRef &, const QString &, const QString &)),
			this, SLOT(UserStatusChanged(const WUserRef &, const QString &, const QString &)));
	connect(fNetClient, SIGNAL(UserHostName(const WUserRef &, const QString &)),
			this, SLOT(UserHostName(const WUserRef &, const QString &)));
	connect(fInputText, SIGNAL(TabPressed(const QString &)),
			this, SLOT(TabPressed(const QString &)));
	connect(fChatText, SIGNAL(URLClicked(const QString &)),
			this, SLOT(URLClicked(const QString &)));
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(AboutToQuit()));

	// create popup menu
	fPrivate = new Q3PopupMenu(this); // have it deleted on destruction of window
	Q_CHECK_PTR(fPrivate);
	connect(fPrivate, SIGNAL(activated(int)), this, SLOT(PopupActivated(int)));
	connect(fUsers, SIGNAL(rightButtonClicked(Q3ListViewItem *, const QPoint &, int)),
			this, SLOT(RightButtonClicked(Q3ListViewItem *, const QPoint &, int)));
	connect(fUsers, SIGNAL(doubleClicked(Q3ListViewItem *)),
			this, SLOT(DoubleClicked(Q3ListViewItem *)));

	LoadSettings();

	// initialize local user
	fUserName = fUserList->currentText();
	fServer = fServerList->currentText();
	fUserStatus = fStatusList->currentText();

	// server check thread
	// run the thread here
	if (fSettings->GetAutoUpdateServers())
	{
		PrintSystem(tr("Updating server list..."));
		ThreadWorkerSessionRef scref(new ThreadWorkerSession());
		scref()->SetGateway(AbstractMessageIOGatewayRef(new PlainTextMessageIOGateway));
		if (fServerThread->StartInternalThread() == B_OK)
		{
			PRINT("Server check thread started\n");
			if (fServerThread->AddNewConnectSession("www.raasu.org", 80, scref) != B_OK)
			{
				PRINT("Failed to add new session to server check thread\n");
				fServerThread->Reset();
			}
			else
				PRINT("OK!\n");
		}
	}

	// version check thread
	if (fSettings->GetCheckNewVersions())
	{
		PrintSystem(tr("Checking for updates..."));
		ThreadWorkerSessionRef psref(new ThreadWorkerSession());
		psref()->SetGateway(AbstractMessageIOGatewayRef(new PlainTextMessageIOGateway));
		if (fUpdateThread->StartInternalThread() == B_OK)
		{
			PRINT("Update thread started\n");
			if (fUpdateThread->AddNewConnectSession(UPDATE_SERVER, 80, psref) != B_OK)
			{
				PRINT("Update thread new connection failed\n");
				fUpdateThread->Reset();
			}
			else
				PRINT("Added new connection!\n");
		}
	}

	if (fSettings->GetLogging())
		StartLogging();

	// check away status... as it is possible to close the program in away state
	if (fStatusList->currentText().stripWhiteSpace().lower() == fAwayMsg.stripWhiteSpace().lower())
		fAway = true;
	else
	{
		fAway = false;
		SetAutoAwayTimer();
	}

	if (fSettings->GetInfo())
	{
		PrintSystem(tr("Welcome to Unizone (English)!"));
		// <postmaster@raasu.org> 20030225
		PrintSystem(tr("Copyright (C) %1 Mika T. Lindqvist.").arg(GetUnizoneYears()));
		PrintSystem(tr("Original idea by Vitaliy Mikitchenko."));
		PrintSystem(tr("Released to public use under LGPL."));
		PrintSystem(tr("Type /help for a command reference."));
	}

	// setup accept thread
	if (fSettings->GetSharingEnabled())
		StartAcceptThread();

	if (fSettings->GetMaximized())
		showMaximized();

	fFileScanThread = new WFileThread(fNetClient, this, &fFileShutdownFlag);
	Q_CHECK_PTR(fFileScanThread);

	fListThread = new WListThread(fNetClient, fFileScanThread, this, &fFileShutdownFlag);
	Q_CHECK_PTR(fListThread);

	fResolverThread = new ResolverThread(&fFileShutdownFlag);
	Q_CHECK_PTR(fResolverThread);

	fResolverThread->start();

	fFileShutdownFlag = false;
	if (fSettings->GetTimeStamps())
		timerID = startTimer(1000);

	if (fSettings->GetSharingEnabled())
		ScanShares();

	if (fSettings->GetLoginOnStartup())
		ExecCommand("/connect");	// this is the LAST thing to do

}
bool
WinShareWindow::StartAcceptThread()
{
	if (fSettings->GetFirewalled())
	{
		if (fAccept)			// We don't need accept thread when we are firewalled.
		{
			StopAcceptThread();
		}
		return false;
	}
	if (fAccept)				// Already running, no need to start this time.
	{
		return false;
	}

	// setup accept thread
	fAccept = new QAcceptSocketsThread(this);
	Q_CHECK_PTR(fAccept);

	connect(fAccept, SIGNAL(ConnectionAccepted(const ConstSocketRef &)),
			this, SLOT(ConnectionAccepted(const ConstSocketRef &)));

	uint32 pStart = gWin->fSettings->GetBasePort();
	uint32 pEnd = pStart + gWin->fSettings->GetPortRange() - 1;

	for (uint32 i = pStart; i <= pEnd; i++)
	{
		// dynamically allocate a port if we can't get one of ours
		if (fAccept->SetPort((uint16) i) == B_OK)
		{
			if (fAccept->StartInternalThread() == B_OK)
			{
				if (fSettings->GetInfo())
				{
					SendSystemEvent(tr("Accept thread ready and listening on port %1.").arg(fAccept->GetPort()));
				}

				// let the net client know our port
				fNetClient->SetPort(fAccept->GetPort());
				// BUG FIX: Port not being set in certain occasions
				fNetClient->SetUserName(fUserName);

				return true;
			}
			else
			{
				SendErrorEvent(tr("Failed to start accept thread!"));
				PRINT("Failed to start accept thread\n");
			}
		}
		else
			PRINT("Failed on port %d\n", i);
	}
	// Only disable file sharing if all port allocations fail.
	fSettings->SetSharingEnabled(false);

	StopAcceptThread();
	return false;
}

void
WinShareWindow::StopAcceptThread()
{
	if (fAccept)
	{
		// Reset port number to 0, which means not accepting and
		// let the net client know the change
		fNetClient->SetPort(0);
		// BUG FIX: Port not being set in certain occasions
		fNetClient->SetUserName(fUserName);

		fAccept->ShutdownInternalThread();
		fAccept->WaitForInternalThreadToExit();
		delete fAccept;
		fAccept = NULL;
	}
}

WinShareWindow::~WinShareWindow()
{
	Cleanup();

	QApplication::exit(0);
}

void
WinShareWindow::Cleanup()
{
	if (timerID != 0)
	{
		killTimer(timerID);
		timerID = 0;
	}

	if (_ttpFiles.GetNumItems() > 0)
	{
		TTPInfo * ttpInfo;
		while (_ttpFiles.GetNumItems() > 0)
		{
			_ttpFiles.RemoveHead(ttpInfo);
			delete ttpInfo;
		}
	}

	// all the NetClients get deleted by Qt
	// since they are QObject's

	if (fUpdateThread)
	{
		fUpdateThread->Reset();
		fUpdateThread = NULL;
	}

	if (fServerThread)
	{
		fServerThread->Reset();
		fServerThread = NULL;
	}

	if (fAccept)
	{
		StopAcceptThread();
	}

	if (fNetClient)
	{
		fNetClient->Reset();
		fNetClient = NULL;
	}

	if (fMenus)
	{
//		Don't delete fMenus!
		fMenus = NULL;
	}

	if (fSearch)
	{
		fSearch->Cleanup();
	}

	if (fDLWindow)
	{
		fDLWindow->EmptyLists();
	}

	if (fFileScanThread)
	{
		WaitOnFileThread(true);
		delete fFileScanThread;
		fFileScanThread = NULL; // <postmaster@raasu.org> 20021027
	}

	if (fListThread)
	{
		WaitOnListThread(true);
		delete fListThread;
		fListThread = NULL;
	}

	DeinitLaunchThread();

	if (fResolverThread)
	{
		fResolverThread->wakeup();
		fResolverThread = NULL;
	}

	// Do these two after everything else

	if (fSettings)
	{
		PRINT("Saving settings\n");
		(void) SaveSettings();
		delete fSettings;
		fSettings = NULL; // <postmaster@raasu.org> 20021027
	}

	StopLogging();
}

void
WinShareWindow::customEvent(QEvent * event)
{
	PRINT("\tWinShareWindow::customEvent\n");
	if (fNetClient)		// do this to avoid bad crash
	{
		switch ((int) event->type())
		{
		case WMessageEvent::MessageEventType:
			{
				WMessageEvent *wme = dynamic_cast<WMessageEvent *>(event);
				if (wme)
				{
					switch (wme->MessageType())
					{
					case WMessageEvent::HandleMessage:
						{
							HandleMessage(wme->Message());
							break;
						}
					case WMessageEvent::ServerParametersMessage:
						{
							ServerParametersReceived(wme->Message());
							break;
						}
					}
				}
				return;
			}
		case WinShareWindow::ConnectRetry:
			{
				PRINT("\tWinShareWindow::ConnectRetry\n");
				if (!fReconnectTimer->isActive())
				{
					// Connect();
					SendSystemEvent(tr( "Reconnecting in 1 minute!" ));
					fReconnectTimer->start(60000, true); // 1 minute
				}
				return;
			}
		case WFileThread::ScanDone:
			{
				PRINT("\tWinShareWindow::ScanDone\n");
				fFilesScanned = true;
				if (fSettings->GetInfo())
					SendSystemEvent(tr("Finished scanning shares."));
				if (fGotParams)
					UpdateShares();
				fFileScanThread->ShutdownInternalThread();
				return;
			}
		case WListThread::ListDone:
			{
				fScanning = false;
				if (fULWindow)
				{
					SignalUpload(WUpload::DequeueUploads);
				}
				PRINT("Done sending file list\n");
				return;
			}
		case NetClient::SESSION_ATTACHED:
			{
				PRINT("Received SessionAttached message\n");
				return;
			}
		case NetClient::SESSION_CONNECTED:
			{
				PRINT("Received SessionConnected message\n");

				// Stop timers first, so if we get reconnect event while we are negotiating it will be discarded
				if (fReconnectTimer->isActive())
				{
					SendSystemEvent(tr("Reconnect timer stopped"));
					fReconnectTimer->stop();
				}

				if (fConnectTimer->isActive())
				{
					fConnectTimer->stop();
				}

				setStatus( tr( "Negotiating..." ) );
				// Start timer again to detect stale negotiation...
				fConnectTimer->start(60000);
				fDisconnect = false;
				fDisconnectFlag = false;
				fDisconnectCount = 0;

				// Set Outgoing Message Encoding
				uint32 enc = fSettings->GetEncoding(GetServerName(fServer), GetServerPort(fServer));
				fNetClient->SetOutgoingMessageEncoding( enc );

				MessageRef setref(GetMessageFromPool(PR_COMMAND_SETPARAMETERS));
				if (setref())
				{
					// Set maximum number of update message items
					setref()->AddInt32(PR_NAME_MAX_UPDATE_MESSAGE_ITEMS, 15);

					// Set Incoming Message Encoding
					if (enc != 0)
					{
						setref()->AddInt32(PR_NAME_REPLY_ENCODING, enc);
					}

					fNetClient->SendMessageToSessions(setref);
				}

				fGotParams = false; // set to false here :)
				if (fSearch)
					fSearch->SetGotResults(true); // fake that we got results, as there is no search yet.
				// send a message out to the server asking for our parameters
				MessageRef askref(GetMessageFromPool(PR_COMMAND_GETPARAMETERS));
				fNetClient->SendMessageToSessions(askref);

				if (fSettings->GetInfo())
					SendSystemEvent(tr("Negotiating..."));

				return;
			}

		case NetClient::DISCONNECTED:
			{
				PRINT("\tNetClient::Disconnected\n");
				DisconnectedFromServer();
				return;
			}

		case WTextEvent::TextType:
			{
				PRINT("\tWTextEvent::TextType\n");
				// a message is being sent... no longer away
				SendChatText(dynamic_cast<WTextEvent *>(event));
				return;
			}

		case WTextEvent::ComboType:
			{
				PRINT("\tWTextEvent::ComboType\n");
				HandleComboEvent(dynamic_cast<WTextEvent *>(event));
				return;
			}

		case WTextEvent::ResumeType:
			{
				PRINT("\tWTextEvent::ResumeType\n");
				WTextEvent * wte = dynamic_cast<WTextEvent *>(event);
				if (wte)
				{
					CheckResumes( wte->Text() );
				}
				return;
			}
		case WTextEvent::ChatTextEvent:
			{
				WTextEvent * wte = dynamic_cast<WTextEvent *>(event);
				if (wte)
				{
					PrintText( wte->Text() );
				}
				return;
			}
		case WTextEvent::UserUpdateEvent:
			{
				WTextEvent * wte = dynamic_cast<WTextEvent *>(event);
				if (wte)
				{
					WUserRef uref = FindUser(wte->Text());
					if (uref())
					{
						uref()->UpdateListViews();
					}
				}
				return;
			}
		case WPWEvent::TextEvent:
			{
				PRINT("\tWPWEvent::TextEvent\n");
				WPWEvent * wpe = dynamic_cast<WPWEvent *>(event);
				if (wpe)
				{
					WTextEvent te;

#ifdef _DEBUG
					WString wtext(wpe->GetText());
					PRINT("wpe->GetText() = %S\n", wtext.getBuffer());
#endif

					te.SetText(wpe->GetText());
					te.SetEncrypted(wpe->Encrypted());
					if (wpe->GetWantReply())	// reply wanted... do the following...
					{
						bool rep = false;
#ifdef _DEBUG
						wtext = te.Text();
						PRINT("Sending the following text to SendChatText %S\n", wtext.getBuffer());
#endif

						SendChatText(&te, &rep);
						if (rep) // does this event WANT a reply
						{
							// send a reply to our friend
							WPWEvent *wpw = new WPWEvent(WPWEvent::TextPosted, te.Text());
							if (wpw)
								QApplication::postEvent(wpe->SendTo(), wpw);
						}
					}
					else
					{
						SendChatText(&te);	// just regular handler...
					}
				}
				return;
			}
		case WPWEvent::TabComplete:
			{
				PRINT("\tWPWEvent::TabComplete\n");
				WPWEvent * wpe = dynamic_cast<WPWEvent *>(event);
				if (wpe)
				{
					QString res;
					if (DoTabCompletion(wpe->GetText(), res))
					{
						WPWEvent *wpw = new WPWEvent(WPWEvent::TabCompleted, res);
						if (wpw)
							QApplication::postEvent(wpe->SendTo(), wpw);
					}
				}
				return;
			}

		case WPWEvent::Closed:
			{
				PRINT("Window closed...\n");
				WPWEvent * wpe = dynamic_cast<WPWEvent *>(event);
				if (wpe)
				{
					pLock.Lock();
					for (unsigned int i = 0; i < fPrivateWindows.GetNumItems(); i++)
					{
						if (fPrivateWindows[i] == wpe->SendTo())
						{
							fPrivateWindows.RemoveItemAt(i);
							PRINT("Removed\n");
							break;
						}
					}
					pLock.Unlock();
				}
				return;
			}

		case WSystemEvent::SystemEvent:
			{
				WSystemEvent *wse = dynamic_cast<WSystemEvent *>(event);
				if (wse)
					PrintSystem(wse->GetText());
				return;
			}

		case WWarningEvent::WarningEvent:
			{
				WWarningEvent *wwe = dynamic_cast<WWarningEvent *>(event);
				if (wwe)
					PrintWarning(wwe->GetText());
				return;
			}

		case WErrorEvent::ErrorEvent:
			{
				WErrorEvent *wee = dynamic_cast<WErrorEvent *>(event);
				if (wee)
					PrintError(wee->GetText());
				return;
			}
		}
	}
}

void
WinShareWindow::HandleComboEvent(WTextEvent * e)
{
	if (e)
	{
		WComboBox * sender = (WComboBox *)e->data();
		// see who sent this
		QString txt = e->Text().stripWhiteSpace();
		if (sender == fUserList)
		{
			PRINT("Received text change event from UserName combo\n");
			// change the user name
			NameChanged(txt);
		}
		else if (sender == fStatusList)
		{
			PRINT("Received text change event from Status combo\n");
			StatusChanged(txt);
			if (txt.lower() != fAwayMsg.stripWhiteSpace().lower())
			{
				SetAutoAwayTimer();
				fAway = false; // in case away was true;
			}
		}
		else if (sender == fServerList)
		{
			PRINT("Received text change event from Server combo\n");
			ServerChanged(txt);
		}
	}
}

void
WinShareWindow::StatusChanged(const QString & newStatus)
{
	fNetClient->SetUserStatus(newStatus); // <postmaster@raasu.org> 20021001
	fUserStatus = newStatus;

	QString status = FormatStatusChanged( FixString( TranslateStatus( fUserStatus ) ) );
	PrintSystem(status);
}

void
WinShareWindow::ServerChanged(const QString & newServer)
{
	fServer = newServer;
	if (fServer.endsWith(":2960"))
		fServer.truncate(fServer.length() - 5); // strip default port

	if (fServer.startsWith("[") && fServer.endsWith("]"))
		fServer = fServer.mid(1, fServer.length() - 2); // strip brackets from IPv6 address with default or no port

	if (fNetClient->IsConnected() || fReconnectTimer->isActive())
		Connect();
}

void
WinShareWindow::NameChanged(const QString & newName)
{
	int64 nr;
	// Make sure old user name has register time
	if (!fUserName.isEmpty())
	{
		nr = fSettings->GetRegisterTime(fUserName);
		fSettings->SetRegisterTime(fUserName, nr);
	}
	if (!newName.isEmpty())
	{
		if (newName != fUserName)
		{
			QString status = FormatNameChanged(FixString(newName));
			SendSystemEvent(status);
			fUserName = newName;
		}
		fNetClient->SetUserName(newName); // <postmaster@raasu.org> 20021011
		// Make sure new user name has register time
		nr = fSettings->GetRegisterTime(fUserName);
		fSettings->SetRegisterTime(fUserName, nr);
		// see if it exists in the list yet...
		for (int i = 0; i < fUserList->count(); i++)
		{
			if ( newName == fUserList->text(i) )	// found match?
			{
				fUserList->setCurrentItem(i);
				return;
			}
		}
		// otherwise, insert
		fUserList->insertItem(newName, 0);
		fUserList->setCurrentItem(0);
	}
}

void
WinShareWindow::InitGUI()
{
	setWindowIcon(QIcon(":/unizone.png"));

	// divide our splitter(s)
	Q3ValueList<int> splitList;
	splitList.append(500);
	splitList.append(100);
	//

	fMenus = new MenuBar(this);
	Q_CHECK_PTR(fMenus);

	/*
	 * Setup combo/labels
	 *
	 * We define the combos as Q3ComboBox, but use WComboBox for
	 * messaging purposes :)
	 *
	 */

	// Server
	//

	fTBServer = new Q3ToolBar( this, tr( "Server bar" ) );
	Q_CHECK_PTR(fTBServer);

	fServerLabel = new QLabel(tr("Server:"), fTBServer);
	Q_CHECK_PTR(fServerLabel);
	fServerLabel->setMinimumWidth(75);

	fServerList = new WComboBox(this, fTBServer, "fServerList");
	Q_CHECK_PTR(fServerList);
	fServerList->setEditable(true);
	fServerLabel->setBuddy(fServerList);

	// Nick
	//

	fTBNick = new Q3ToolBar( this, tr( "Nickbar" ) );
	Q_CHECK_PTR(fTBNick);

	fUserLabel = new QLabel(tr("Nick:"), fTBNick);
	Q_CHECK_PTR(fUserLabel);
	fUserLabel->setMinimumWidth(75);

	fUserList = new WComboBox(this, fTBNick, "fUserList");
	Q_CHECK_PTR(fUserList);
	fUserList->setEditable(true);
	fUserLabel->setBuddy(fUserList);

	// Status
	//

	fTBStatus = new Q3ToolBar( this, tr( "Statusbar" ) );
	Q_CHECK_PTR(fTBStatus);

	fStatusLabel = new QLabel(tr("Status:"), fTBStatus);
	Q_CHECK_PTR(fStatusLabel);
	fStatusLabel->setMinimumWidth(75);

	fStatusList = new WComboBox(this, fTBStatus, "fStatusList");
	Q_CHECK_PTR(fStatusList);
	fStatusList->setEditable(true);
	fStatusLabel->setBuddy(fStatusList);

	// chat widget
	fMainWidget = new QWidget(this);
	Q_CHECK_PTR(fMainWidget);

	setCentralWidget(fMainWidget);

	fMainTab = new Q3GridLayout(fMainWidget, 14, 10, 0, -1, "Chat Tab");
	Q_CHECK_PTR(fMainTab);

	// main splitter
	fMainSplitter = new QSplitter(fMainWidget);
	Q_CHECK_PTR(fMainSplitter);

	fMainTab->addMultiCellWidget(fMainSplitter, 0, 12, 0, 9);

	// user list
	fUsersBox = new Q3HGroupBox(fMainSplitter);
	Q_CHECK_PTR(fUsersBox);

	fUsers = new WUniListView(fUsersBox);
	Q_CHECK_PTR(fUsers);

	// initialize the list view
	InitUserList(fUsers);

	// left pane
	fLeftPane = new Q3VGroupBox(fMainSplitter);
	Q_CHECK_PTR(fLeftPane);

	fMainSplitter->moveToLast(fUsersBox);		// put list box in right pane
	fMainSplitter->moveToFirst(fLeftPane);

	fMainSplitter->setSizes(splitList);

	/*
	 *
	 * chat widgets
	 *
	 */

	// chat splitter first though...
	fChatSplitter = new QSplitter(fLeftPane);
	Q_CHECK_PTR(fChatSplitter);
	fChatSplitter->setOrientation(Qt::Vertical);

	fChatText = new WHTMLView(fChatSplitter);
	Q_CHECK_PTR(fChatText);
	fChatText->setTextFormat(Qt::RichText);

	/*
	 *
	 * fInputText is a QMultiLineEdit, but we create
	 * a WChatText for more advanced handling
	 *
	 */

	fInputText = new WChatText(this, fChatSplitter);
	Q_CHECK_PTR(fInputText);
	fInputText->setWordWrap(Q3MultiLineEdit::WidgetWidth);
	fChatSplitter->setSizes(splitList);

	// Main Status Bar

	fStatusBar = new WStatusBar(fMainWidget, "fStatusBar", 5);
	Q_CHECK_PTR(fStatusBar);
	fStatusBar->setSizeGripEnabled(false);
	fMainTab->addMultiCellWidget(fStatusBar, 13, 13, 0, 9);

	setStatus(tr( "Not connected." ), 0);

	// setup some defaults
	// <postmaster@raasu.org> 20020924

	fUserList->insertItem("Unizone Binky");
	fServerList->insertItem("beshare.tycomsystems.com");
	fStatusList->insertItem("here");
	fStatusList->insertItem("away");

	// do some resizing policies
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	fUserList->setSizePolicy(sizePolicy);
	fUserList->setDuplicatesEnabled(false);
	fUserList->setAutoCompletion(true);
	fUserList->setMinimumWidth(75);
	fStatusList->setSizePolicy(sizePolicy);
	fStatusList->setDuplicatesEnabled(false);
	fStatusList->setAutoCompletion(true);
	fStatusList->setMinimumWidth(75);
	fServerList->setSizePolicy(sizePolicy);
	fServerList->setDuplicatesEnabled(false);
	fServerList->setAutoCompletion(true);
	fServerList->setMinimumWidth(75);

	fGotParams = false;

	if (fSearch)
		fSearch->SetGotResults(true);

	// setup autoaway timer
	fAutoAway = new QTimer;
	Q_CHECK_PTR(fAutoAway);
	connect(fAutoAway, SIGNAL(timeout()), this, SLOT(AutoAwayTimer()));

	// setup connect timer
	fConnectTimer = new QTimer;
	Q_CHECK_PTR(fConnectTimer);
	connect(fConnectTimer, SIGNAL(timeout()), this, SLOT(ConnectTimer()));

	// setup autoconnect timer
	fReconnectTimer = new QTimer;
	Q_CHECK_PTR(fReconnectTimer);
	connect(fReconnectTimer, SIGNAL(timeout()), this, SLOT(ReconnectTimer()));
}

QString
WinShareWindow::GetUserName() const
{
	return fUserName;
}

QString
WinShareWindow::GetServer() const
{
	return fServer;
}

QString
WinShareWindow::GetStatus() const
{
	return fUserStatus;
}

QString
WinShareWindow::MakeHumanDiffTime(uint64 time)
{
	uint64 seconds = time / 1000000;
	uint64 minutes = seconds / 60;  seconds = seconds % 60;
	uint64 hours   = minutes / 60;  minutes = minutes % 60;

	QString s;

	char buf[26];
#ifdef __amd64__
	sprintf(buf, "%lu:%02lu:%02lu", hours, minutes, seconds);
#elif __STDC_WANT_SECURE_LIB__
	sprintf_s(buf, 26, "%llu:%02llu:%02llu", hours, minutes, seconds);
#else
	sprintf(buf, "%llu:%02llu:%02llu", hours, minutes, seconds);
#endif
	s = buf;

	return s;
}

QString
WinShareWindow::MakeHumanTime(uint64 time)
{
	if (time < 1000000)
		return tr("%1 seconds").arg(0);

	uint64 seconds = time / 1000000;
	uint64 minutes = seconds / 60;  seconds = seconds % 60;
	uint64 hours	= minutes / 60;  minutes = minutes % 60;
	uint64 days 	= hours	 / 24;  hours	 = hours   % 24;
	uint64 weeks	= days	 /  7;  days	 = days	  % 7;

	QString s, qTime;

	if (weeks == 1)
	{
		s = tr("1 week");
		s += ", ";
	}
	else if (weeks > 1)
	{
		s = tr("%1 weeks").arg((long)weeks);
		s += ", ";
	}

	if (days == 1)
	{
		s += tr("1 day");
		s += ", ";
	}
	else if (days > 1)
	{
		s += tr("%1 days").arg((long)days);
		s += ", ";
	}

	if (hours == 1)
	{
		s += tr("1 hour");
		s += ", ";
	}
	else if (hours > 1)
	{
		s += tr("%1 hours").arg((long)hours);
		s += ", ";
	}

	if (minutes == 1)
	{
		s += tr("1 minute");
		s += ", ";
	}
	else if (minutes > 1)
	{
		s += tr("%1 minutes").arg((long)minutes);
		s += ", ";
	}

	if (seconds == 1)
	{
		s += tr("1 second");
		s += ", ";
	}
	else if (seconds > 1)
	{
		s += tr("%1 seconds").arg((long)seconds);
		s += ", ";
	}

	if ((s.length() > 2) && s.endsWith(", "))
	{
		s.truncate(s.length() - 2);
	}

	int cp = s.findRev(", ");

	if (cp >= 0)
	{
		return s.left(cp)+" "+tr("and")+" "+s.mid(cp+2);
	}
	else
	{
		return s;
	}
}

bool
WinShareWindow::ParseUserTargets(const QString & text, WUserSearchMap & sendTo, QString & setTargetStr, QString & setRestOfString, NetClient * net)
{
	QStringTokenizer wholeStringTok(text, " ");
	QString restOfString2(wholeStringTok.GetRemainderOfString());
	QString w2 = wholeStringTok.GetNextToken();
	if (w2 != QString::null)
	{
		setTargetStr = w2;

		setRestOfString = wholeStringTok.GetRemainderOfString();

		QStringTokenizer tok(w2, ",");
		Queue<QString> clauses;
		QString next;
		while((next = tok.GetNextToken()) != QString::null)
			clauses.AddTail(next.stripWhiteSpace());

		// find users by session id
		for (int i = clauses.GetNumItems() - 1; i >= 0; i--)
		{
			WUserRef user = net->FindUser( clauses[i] );
#ifdef DEBUG2
			WString wuser(clauses[i]);
			PRINT2("Checking for user %S\n", wuser.getBuffer());
#endif

			if (user() != NULL)
			{
				WUserSearchPair pair;
				pair.user = user;
				pair.string = setRestOfString; // <postmaster@raasu.org> 20021007
				sendTo.AddTail(pair);
				clauses.RemoveItemAt(i);
			}
		}

		PRINT("Checking using usernames\n");
		for (int j = clauses.GetNumItems() - 1; j >= 0; j--)
		{
			QString tstr(clauses[j]);
			tstr = tstr.stripWhiteSpace().lower();
			ConvertToRegex(tstr);
			QRegExp qr(tstr, false);

			bool foundMatches = false;
			WUserIter iter = net->UsersIterator(HTIT_FLAG_NOREGISTER);
			while (iter.HasData())
			{
				WUserRef user = iter.GetValue();
				iter++;
				QString userName = StripURL(user()->GetUserName().stripWhiteSpace());

				if (Match(userName, qr) == 0)
				{
					WUserSearchPair pair;
					pair.user = user;
					pair.string = setRestOfString; // <postmaster@raasu.org> 20021007
					sendTo.AddTail(pair);
					foundMatches = true;
				}
			}
			if (foundMatches)
			{
				PRINT("Found\n");
				clauses.RemoveItemAt(j);
			}
		}
		PRINT("Checking using tab stuff\n");
		// still no items?
		if (sendTo.IsEmpty())
		{
			// tab-completion thingy :)
			WUserIter iter = net->UsersIterator(HTIT_FLAG_NOREGISTER);
			while (iter.HasData())
			{
				WUserRef user = iter.GetValue();
				iter++;
				QString uName = user()->GetUserName();
				QString userName = StripURL(uName.stripWhiteSpace()) + " ";

				if (!userName.isEmpty() && restOfString2.startsWith(userName))
				{
					PRINT("Found\n");
					WUserSearchPair pair;
					pair.user = user;
					pair.string = restOfString2.mid( userName.length() );
					sendTo.AddTail(pair);
					setTargetStr = uName;
				}
			}
		}
		return true;
	}
	return false;
}

QString
WinShareWindow::GetRemoteVersionString(MessageRef msg)
{
	QString versionString("?");
	const char * version;

        if (msg()->FindString("version", version) == B_OK)
	{
		if (muscleInRange(version[0], '0', '9'))
		{
			versionString = "BeShare ";
			versionString += QString::fromUtf8(version);
		}
		else
			versionString = QString::fromUtf8(version);
	}
	return versionString;
}

void
WinShareWindow::LoadSettings()
{
	if (fSettings->Load())
	{
		fServerList->clear();
		fStatusList->clear();
		fUserList->clear();
		if (fSearch)
			fSearch->LoadSettings();

		if (fPicViewer)
			fPicViewer->LoadSettings();
		int i;
		int size;
		QString str;
		// load servers
		for (i = 0; (str = fSettings->GetServerItem(i).stripWhiteSpace()) != QString::null; i++)
			fServerList->insertItem(str, i);
		i = fSettings->GetCurrentServerItem();
		if (i < fServerList->count())
			fServerList->setCurrentItem(i);
		fServer = fServerList->currentText();

		// load usernames
		for (i = 0; (str = fSettings->GetUserItem(i).stripWhiteSpace()) != QString::null; i++)
			fUserList->insertItem(str, i);
		i = fSettings->GetCurrentUserItem();
		if (i < fUserList->count())
			fUserList->setCurrentItem(i);
		fUserName = fUserList->currentText();

		// load status
		for (i = 0; (str = fSettings->GetStatusItem(i).stripWhiteSpace()) != QString::null; i++)
		{
			// Skip old 'testing' statuses
			if (!str.startsWith(tr("Testing Unizone (English)")) && // new internationalized
				!str.startsWith("Testing Unizone (")				// old format
				)
				fStatusList->insertItem(str);
		}

		QString status;
#ifdef BETA
		// Add new 'testing' status
		status = tr("Testing Unizone (English)");
		status += " ";
		status += WinShareVersionString();
		fStatusList->insertItem(status);
#endif
		// reselect previous status line if possible
		int ci = fSettings->GetCurrentStatusItem();
		status = fSettings->GetStatusItem(ci);
		for (i = 0; i < fStatusList->count(); i++)
		{
			if (fStatusList->text(i) == status)
			{
				fStatusList->setCurrentItem(i);
				break;
			}
		}
		ci = fStatusList->currentItem();
		fUserStatus = fStatusList->text(ci);

		// load the style
#ifndef DISABLE_STYLES
		switch (fSettings->GetStyle())
		{
			case Motif:
#if !defined(QT_NO_STYLE_MOTIF)
				qApp->setStyle(new QMotifStyle);
#endif
				break;
			case Windows:
#if !defined(QT_NO_STYLE_WINDOWS)
				qApp->setStyle(new QWindowsStyle);
#endif
				break;
			case WindowsXP:
#if defined(_WIN32)
# if !defined(QT_NO_STYLE_WINDOWSXP)
				qApp->setStyle(new QWindowsXPStyle);
# endif
#endif
				break;
			case CDE:
#if !defined(QT_NO_STYLE_CDE)
				qApp->setStyle(new QCDEStyle);
#endif
				break;
			case Mac:
#if defined(__APPLE__)
# if !defined(QT_NO_STYLE_MAC)
				qApp->setStyle(new QMacStyle);
# endif
#endif
				break;
			case Plastique:
#if !defined(QT_NO_STYLE_PLASTIQUE)
				qApp->setStyle(new QPlastiqueStyle);
#endif
				break;
			case WindowsVista:
#if defined(_WIN32)
# if !defined(QT_NO_STYLE_WINDOWSVISTA)
				qApp->setStyle(new QWindowsVistaStyle);
# endif
#endif
				break;
		}
#endif

		// load column sizes
		for (i = 0; (size = fSettings->GetColumnItem(i)) > 0; i++)
			fUsers->setColumnWidth(i, size);

		// load size/position
		int a, b;
		if ((a = fSettings->GetWindowWidth()) > 0 && (b = fSettings->GetWindowHeight()) > 0)
			resize(a, b);
		if ((a = fSettings->GetWindowX()) >= 0 && (b = fSettings->GetWindowY()) >= 0)
			move(a, b);

		Q3ValueList<int> sizes;
		// load chat sizes
		sizes = fSettings->GetChatSizes();
		if (sizes.count() > 0)
		{
			fChatSplitter->setSizes(sizes);
			// empty
			sizes.clear();
		}

		// load main sizes
		sizes = fSettings->GetMainSizes();
		if (sizes.count() > 0)
			fMainSplitter->setSizes(sizes);

		// status messages
		fAwayMsg = fSettings->GetAwayMsg();

#ifdef _DEBUG
		WString waway(fAwayMsg);
		PRINT("Away Msg: %S\n", waway.getBuffer());
#endif

		fHereMsg = fSettings->GetHereMsg();

#ifdef _DEBUG
		WString where(fHereMsg);
		PRINT("Here Msg: %S\n", where.getBuffer());
#endif

		fWatch = fSettings->GetWatchPattern();
		fIgnore = fSettings->GetIgnorePattern();
		fIgnoreIP = fSettings->GetIPIgnorePattern();
		fBlackList = fSettings->GetBlackListPattern();
		fWhiteList = fSettings->GetWhiteListPattern();
		fFilterList = fSettings->GetFilterListPattern();
		fAutoPriv = fSettings->GetAutoPrivatePattern();
		fPMRedirect = fSettings->GetPMRedirect();

		fOnConnect = fSettings->GetOnConnect();
		fOnConnect2 = fSettings->GetOnConnect2();

		tx = fSettings->GetTransmitStats(); tx2 = tx;
		rx = fSettings->GetReceiveStats(); rx2 = rx;

		fUsers->setSorting(fSettings->GetNickListSortColumn(), fSettings->GetNickListSortAscending());

		fRemote = fSettings->GetRemotePassword();

		// Load resume list

		rLock.Lock();
		fResumeMap.Clear();

		for (i = 0; i <= fSettings->GetResumeCount(); i++)
		{
			QString ruser, luser = QString::null;
			int rind = -1;
			WResumeInfo wri;
			if (fSettings->GetResumeItem(i, ruser, wri))
			{
				if (luser != ruser)
				{
					for (unsigned int r = 0; r < fResumeMap.GetNumItems(); r++)
					{
						if (fResumeMap[r].user == ruser)
						{
							rind = r;
							luser = ruser;
							fResumeMap[r].files.AddTail(wri);
							break;
						}
					}
					// No entry yet
					{
						WResumePair wrp;
						wrp.user = ruser;
						wrp.files.AddTail(wri);
						fResumeMap.AddTail(wrp);
						rind = fResumeMap.GetNumItems() - 1;
					}
				}
				else
				{
					// Same user as previous resume item
					fResumeMap[rind].files.AddTail(wri);
				}
			}
		}
		rLock.Unlock();

		fInstallID = fSettings->GetInstallID();
	}
	else	// file doesn't exist, or error
	{
#ifndef DISABLE_STYLES
# if defined(WIN32) || defined(_WIN32)
#	if !defined(QT_NO_STYLE_WINDOWS)
		qApp->setStyle(new QWindowsStyle);
#	endif // !QT_NO_STYLE_WINDOWS
# else
#	if !defined(QT_NO_STYLE_MOTIF)
		qApp->setStyle(new QMotifStyle);
#	endif // !QT_NO_STYLE_MOTIF
# endif // WIN32
#endif // DISABLE_STYLES

		fAwayMsg = "away";
		fHereMsg = "here";
		fWatch = QString::null;
		fIgnore = QString::null;
		fIgnoreIP = QString::null;
		fBlackList = QString::null;
		fWhiteList = "Atrus, Bubbles";
		fFilterList = QString::null;
		fAutoPriv = QString::null;
		fPMRedirect = QString::null;
		fOnConnect = QString::null;
		fOnConnect2 = QString::null;

		if (fUserName != QString::null)
		{
			int64 rn = fSettings->GetRegisterTime(fUserName);
			fSettings->SetRegisterTime(fUserName, rn);
		}

		tx = 0;
		rx = 0;
		tx2 = 0;
		rx2 = 0;
		srand(time(NULL));
		uint64 i1 = rand()*(ULONG_MAX/RAND_MAX);
		uint64 i2 = rand()*(ULONG_MAX/RAND_MAX);
		fInstallID = (i1 << 32) + i2;
	}

	// Toolbar Layout


	InitToolbars();
}

void
WinShareWindow::InitToolbars()
{
	int	i;

	int32 _dock[NUM_TOOLBARS];
	int32 _index[NUM_TOOLBARS];
	bool _nl[NUM_TOOLBARS];
	int32 _extra[NUM_TOOLBARS];

	topDock()->moveDockWindow(fTBServer, 0);
	topDock()->moveDockWindow(fTBNick, 1);
	topDock()->moveDockWindow(fTBStatus, 2);

	for (i = 0; i < NUM_TOOLBARS; i++)
		if (!fSettings->GetToolBarLayout(i, _dock[i], _index[i], _nl[i], _extra[i]))
			return;

	// Start from dock position 0
	// Start moving toolbars from index value of 0

	for (int d1 = 0; d1 < 7; d1++)			// dock position
	{
		for (int ip = 0; ip < NUM_TOOLBARS; ip++) 	// index position
		{
			for (int i1 = 0; i1 < NUM_TOOLBARS; i1++) // layout index
			{
				if ((_dock[i1] == d1) && (_index[i1] == ip))
				{
					Q3ToolBar * tb = NULL;
					switch (i1)
					{
						case UID_SERVERBAR:
							tb = fTBServer;
							break;
						case UID_NICKBAR:
							tb = fTBNick;
							break;
						case UID_STATUSBAR:
							tb = fTBStatus;
							break;
					}
					if (tb)
						moveToolBar(tb, (Qt::ToolBarDock) _dock[i1], _nl[i1], 3, _extra[i1]);
				}
			}
		}
	}
}

bool
WinShareWindow::SaveSettings()
{
	// update our settings message (it's not updated until SaveSettings() is called
	fSettings->EmptyColumnList();
	fSettings->EmptyServerList();
	fSettings->EmptyStatusList();
	fSettings->EmptyUserList();
	// don't worry about the color list, prefs does it

	// save server list
	int i;

	for (i = 0; i < fServerList->count(); i++)
	{
		QString qServer = fServerList->text(i).stripWhiteSpace();
		fSettings->AddServerItem(qServer);

#ifdef _DEBUG
		WString wserver(qServer);
		PRINT("Saved server %S\n", wserver.getBuffer());
#endif
	}
	fSettings->SetCurrentServerItem(fServerList->currentItem());

	// save user list

	for (i = 0; i < fUserList->count(); i++)
	{
		QString qUser = fUserList->text(i).stripWhiteSpace();
		fSettings->AddUserItem(qUser);

#ifdef _DEBUG
		WString wuser(qUser);
		PRINT("Saved user %S\n", wuser.getBuffer());
#endif
	}
	fSettings->SetCurrentUserItem(fUserList->currentItem());

	// save status list

	for (i = 0; i < fStatusList->count(); i++)
	{
		QString qStatus = fStatusList->text(i).stripWhiteSpace();
		fSettings->AddStatusItem(qStatus);

#ifdef _DEBUG
		WString wstatus(qStatus);
		PRINT("Saved status %S\n", wstatus.getBuffer());
#endif
	}
	fSettings->SetCurrentStatusItem(fStatusList->currentItem());

	// save query history

	if (fSearch)
		fSearch->SaveSettings();

	if (fPicViewer)
		fPicViewer->SaveSettings();

	// don't worry about style, Prefs does it for us

	// save list view column widths
	for (i = 0; i < fUsers->columns(); i++)
		fSettings->AddColumnItem(fUsers->columnWidth(i));

	// save window height/position
	fSettings->SetWindowWidth(width());
	fSettings->SetWindowHeight(height());
	fSettings->SetWindowX(x());
	fSettings->SetWindowY(y());

	Q3ValueList<int> sizes;
	// save chat sizes
	sizes = fChatSplitter->sizes();
	fSettings->SetChatSizes(sizes);
	// main splitter...
	sizes = fMainSplitter->sizes();
	fSettings->SetMainSizes(sizes);
	// status messages
	fSettings->SetAwayMsg(fAwayMsg);
	fSettings->SetHereMsg(fHereMsg);

	// watch, ignore, blacklist & auto-private patterns
	fSettings->SetWatchPattern(fWatch);
	fSettings->SetIgnorePattern(fIgnore);
	fSettings->SetIPIgnorePattern(fIgnoreIP);
	fSettings->SetBlackListPattern(fBlackList);
	fSettings->SetWhiteListPattern(fWhiteList);
	fSettings->SetFilterListPattern(fFilterList);
	fSettings->SetAutoPrivatePattern(fAutoPriv);
	fSettings->SetPMRedirect(fPMRedirect);

	// on connect

	fSettings->SetOnConnect(fOnConnect);
	fSettings->SetOnConnect2(fOnConnect2);

	// transfer stats

	fSettings->SetTransmitStats(tx);
	fSettings->SetReceiveStats(rx);

	// nick list sort settings

	fSettings->SetNickListSortColumn(fUsers->sortColumn());
	fSettings->SetNickListSortAscending(fUsers->sortAscending());

	// remote control

	fSettings->SetRemotePassword(fRemote);

	// maximized
	fSettings->SetMaximized(isMaximized());

	// save color settings
	// don't worry about it... prefs does it

	// Save resume list

	rLock.Lock();
	fSettings->EmptyResumeList();

	for ( unsigned int x = 0; x < fResumeMap.GetNumItems(); x++ )
	{
		for ( unsigned int y = 0; y < fResumeMap[x].files.GetNumItems(); y++ )
		{
			fSettings->AddResumeItem(fResumeMap[x].user, fResumeMap[x].files[y]);
		}
	}
	rLock.Unlock();
	fSettings->SetResumeCount(fResumeMap.GetNumItems());

	// UniShare

	if (fUserName != QString::null)
	{
		int64 rn = fSettings->GetRegisterTime(fUserName);
		fSettings->SetRegisterTime(fUserName, rn);
	}

	// Toolbar Layout

	Qt::Dock _dock;
	int _index, _extra;
	bool _nl;

	getLocation(fTBServer, _dock, _index, _nl, _extra);
	fSettings->SetToolBarLayout(UID_SERVERBAR, _dock, _index, _nl, _extra);

	getLocation(fTBNick, _dock, _index, _nl, _extra);
	fSettings->SetToolBarLayout(UID_NICKBAR, _dock, _index, _nl, _extra);

	getLocation(fTBStatus, _dock, _index, _nl, _extra);
	fSettings->SetToolBarLayout(UID_STATUSBAR, _dock, _index, _nl, _extra);

	fSettings->SetInstallID(fInstallID);

	return fSettings->Save();
}

void
WinShareWindow::SetStatus(const QString & s)
{
	for (int i = 0; i < fStatusList->count(); i++)
	{
		QString status = fStatusList->text(i).stripWhiteSpace().lower();
		if (status == s.stripWhiteSpace().lower())
		{
			fStatusList->setCurrentItem(i);
			StatusChanged(fStatusList->currentText());
			return;
		}
	}
	// not found?
	// insert the status
	fStatusList->insertItem(s, 0);
	fStatusList->setCurrentItem(0);
	StatusChanged(s);
}

void
WinShareWindow::setStatus(const QString &s, unsigned int i)
{
	fStatusBar->setText(s, i);
}

void
WinShareWindow::SetAutoAwayTimer()
{
	switch (fSettings->GetAutoAway())
	{
		case 0:		// disabled, don't do anything
			fAutoAway->stop();
			break;

		case 1:		// 2 minutes
			fAutoAway->start(2 * 60000, true);
			break;

		case 2:		// 5 minutes
			fAutoAway->start(5 * 60000, true);
			break;

		case 3:		// 10 minutes
			fAutoAway->start(10 * 60000, true);
			break;

		case 4:		// 15 minutes
			fAutoAway->start(15 * 60000, true);
			break;

		case 5:		// 20 minutes
			fAutoAway->start(20 * 60000, true);
			break;

		case 6:		// 30 minutes
			fAutoAway->start(30 * 60000, true);
			break;

		case 7:		// 1 hour
			fAutoAway->start(60 * 60000, true);
			break;

		case 8:		// 2 hours
			fAutoAway->start(120 * 60000, true);
			break;
	};
}

void
WinShareWindow::WaitOnFileThread(bool abort)
{
	fFileShutdownFlag = abort;
	if (fFileScanThread->IsInternalThreadRunning())
	{
		if (abort)
			fFilesScanned = false;

		PrintSystem(tr("Waiting for file scan thread to finish..."));
		fFileScanThread->WaitForInternalThreadToExit();
	}
}

void
WinShareWindow::WaitOnListThread(bool abort)
{
	fFileShutdownFlag = abort;
	if (fListThread->IsInternalThreadRunning())
	{
		PrintSystem(tr("Waiting for file list thread to finish..."));
		fListThread->WaitForInternalThreadToExit();
	}
}

void
WinShareWindow::LaunchSearch(const QString & pattern)
{
	gWin->OpenSearch();
	// (be)share://server/pattern
	if (pattern.find("//") == 0)
	{
		// Strip server name in front of search pattern
		QString qServer = pattern.mid(2);						// remove //
		int sp = qServer.find("/");
		if (sp > -1)
		{
			QString qPattern = qServer.mid(sp + 1);	// get everything after /
			qServer.truncate(sp);					// get everything before /
			gWin->SetDelayedSearchPattern(qPattern);
			gWin->Connect(qServer);
			return;
		}
		gWin->fSearch->SetSearch(pattern.mid(2));
		gWin->fSearch->show();
		return;
	}
	// (be)share:pattern
	gWin->fSearch->SetSearch(pattern);
	gWin->fSearch->show();
}

QString
WinShareWindow::MapIPsToNodes(const QString & pattern)
{
	QString qResult;
	QString qItem;
	QStringTokenizer qTok(pattern, ",");
	while ((qItem = qTok.GetNextToken()) != QString::null)
	{
		AddToList(qResult, "/" + qItem + "/*");
	}

#ifdef _DEBUG
	WString wres(qResult);
	PRINT("MapIPsToNodes: %S\n", wres.getBuffer());
#endif

	return qResult;
}

QString
WinShareWindow::MapUsersToIDs(const QString & pattern)
{
	QString qResult("");
	QString qItem;
	QStringTokenizer tok(pattern, ",");
	while ((qItem = tok.GetNextToken()) != QString::null)
	{
		WUserIter it = gWin->fNetClient->UsersIterator(HTIT_FLAG_NOREGISTER);
		// Space in username? (replaced with '*' for BeShare compatibility)
		qItem.replace(QRegExp("*"), " ");

		while (it.HasData())
		{
			WUserRef uref = it.GetValue();
			if ((uref()->GetUserID() == qItem) ||
				(StripURL(uref()->GetUserName().lower()) == StripURL(qItem.lower())))
			{
				AddToList(qResult, "/"+uref()->GetUserHostName() + "/" + uref()->GetUserID());
				break;
			}
			it++;
		}
	}

#ifdef _DEBUG
	WString wres(qResult);
	PRINT("MapUsersToIDs: %S\n", wres.getBuffer());
#endif

	return qResult;
}

// <postmaster@raasu.org> 20021013 -- Opens new Private Window and adds users in 'pattern' to that window
// <postmaster@raasu.org> 20021114 -- Uses QStringTokenizer ;)

void
WinShareWindow::LaunchPrivate(const QString & pattern)
{
	QString users(pattern);
	if (users.startsWith("//"))
	{
		users = users.mid(2);
	}
	int iUsers = 0;

	WUserMap umap;

	QString qItem;
	QStringTokenizer qTok(users,",");

	while ((qItem = qTok.GetNextToken()) != QString::null)
	{
		// Space in username? (replaced with '*' for BeShare compatibility)
		qItem.replace(QRegExp("*"), " ");

		WUserIter it = gWin->fNetClient->UsersIterator(HTIT_FLAG_NOREGISTER);

		while (it.HasData())
		{
			WUserRef uref = it.GetValue();
			if ((uref()->GetUserID() == qItem) ||
				(StripURL(uref()->GetUserName().lower()) == StripURL(qItem.lower())))
			{
				umap.Put(uref()->GetUserID().toULong(), uref);
				iUsers++;
				break;
			}
			it++;
		}
	}
	if (iUsers == 0) // No valid users?
	{
		return;
	}
	WPrivateWindow * window = new WPrivateWindow(this, fNetClient, NULL);
	if (!window)
		return;
	window->AddUsers(umap);
	window->show();
	pLock.Lock();
	gWin->fPrivateWindows.AddTail(window);
	pLock.Unlock();
}

void
WinShareWindow::QueueFile(const QString & ref)
{
	gWin->QueueFileAux(ref);
}

void
WinShareWindow::QueueFileAux(const QString & ref)
{
	QString from = ref.section('/', 0, 0);
	QString file = ref.section('/', 1, 1);
	QString sfile = TTPDecode(file);
	TTPInfo * ttpInfo = new TTPInfo;
	if (ttpInfo)
	{
		ttpInfo->bot = from;
		ttpInfo->file = sfile;
		_ttpFiles.AddTail(ttpInfo);
		SendSystemEvent(tr("Queued file %1 from user #%2.").arg( sfile ).arg(from));
	}
}

void
WinShareWindow::StartQueue(const QString &session)
{
	if (_ttpFiles.GetNumItems() > 0)
	{
		WUserRef user = FindUser(session);
		DownloadQueue fQueue;

		if (user() == NULL)
			return;

		unsigned int i = 0;
		fTTPLock.Lock();
		while (_ttpFiles.GetNumItems() > 0)
		{
			TTPInfo * ttpInfo = NULL;
			_ttpFiles.GetItemAt(i, ttpInfo);
			if (ttpInfo)
			{
				if (ttpInfo->bot == session)
				{
					fQueue.addItem(ttpInfo->file, QString::null, user);
					_ttpFiles.RemoveItemAt(i);
					SendSystemEvent(tr("Downloading file %1 from user #%2.").arg( ttpInfo->file ).arg(session));
				}
				else i++;
				if (i >= _ttpFiles.GetNumItems())
					break;
			}
		}
		fQueue.run();
		fTTPLock.Unlock();
	}
}

void
WinShareWindow::StartLogging()
{
	fMainLog.Create();
	if (!fMainLog.InitCheck())
	{
		if (fSettings->GetError())
				SendErrorEvent( tr("Failed to create log file.") );
	}
}

void
WinShareWindow::StopLogging()
{
	fMainLog.Close();
}

// Remove old shares
void
WinShareWindow::CancelShares()
{
	MessageRef delData(GetMessageFromPool(PR_COMMAND_REMOVEDATA));
	if (delData())
	{
		delData()->AddString(PR_NAME_KEYS, "beshare/fi*");
		fNetClient->SendMessageToSessions(delData, 0);
	}
}

// Create our most important directories
void
WinShareWindow::CreateDirectories()
{
	QDir dir = QDir::current();
	if (!dir.exists("shared"))
		dir.mkdir("shared");
	if (!dir.exists("downloads"))
		dir.mkdir("downloads");
	if (!dir.exists("temp"))
		dir.mkdir("temp");
	if (!dir.exists("logs"))
		dir.mkdir("logs");
}

QString
WinShareWindow::GetUptimeString()
{
	return MakeHumanTime(GetUptime());
}

uint64
WinShareWindow::GetUptime()
{
#ifdef WIN32
	return ((uint64)GetTickCount64()) * 1000;
#elif defined(__LINUX__) || defined(linux)
	struct sysinfo sinfo;
	sysinfo(&sinfo);
	uint64 uptime = sinfo.uptime;
	uptime *= 1000000L;
	return uptime;
#elif defined(__FreeBSD__) || defined(__QNX__) || defined(__APPLE__)
	uint64 uptime = 0;
	struct timeval boottime;
	time_t now;
	size_t size;
	int mib[2];

	time(&now);
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);
	if ((sysctl(mib, 2, &boottime, &size, NULL, 0) != -1) && (boottime.tv_sec != 0))
	{
		uptime = now - boottime.tv_sec;
		uptime *= 1000000L;
	}
	return uptime;
#elif defined(__sun) && defined(__SVR4) // SunOS <edayan@optonline.net>
	uint64 boottime = 0;
	uint64 upTime = 0;
	struct utmpx *utxent;

	setutxent();
	while ((utxent = getutxent())) {
		if (strcmp("system boot", utxent->ut_line) == 0) {
			bootTime = utxent->ut_tv.tv_sec;
			endutxent();
			break;
		}
	}

	if (bootTime != 0) {
		int currentTime = time(NULL);
		upTime = (currentTime - bootTime) * 1000000L;
	}

	return upTime;
#else
# error "Uptime not implemented for your OS"
#endif
}

void
WinShareWindow::UpdateTransmitStats(uint64 t)
{
	tx += t;
}

void
WinShareWindow::UpdateReceiveStats(uint64 r)
{
	rx += r;
}

const char * statusList[] = {
	QT_TRANSLATE_NOOP("WinShareWindow", "here"),
	QT_TRANSLATE_NOOP("WinShareWindow", "away"),
	QT_TRANSLATE_NOOP("WinShareWindow", "idle"),
	QT_TRANSLATE_NOOP("WinShareWindow", "busy"),
	QT_TRANSLATE_NOOP("WinShareWindow", "at work"),
	QT_TRANSLATE_NOOP("WinShareWindow", "around"),
	QT_TRANSLATE_NOOP("WinShareWindow", "sleeping"),
	QT_TRANSLATE_NOOP("WinShareWindow", "wandering"),
	NULL
};

QString
WinShareWindow::TranslateStatus(const QString & s)
{
	QString st = s.lower();
	const char *t;
	for (unsigned int x = 0; (t = statusList[x]) != NULL; x++)
	{
		if (st == t)
			return tr(t);
	}
	return s;
}

void
WinShareWindow::OpenDownload()
{
	if (!fDLWindow)
	{
		PRINT("New DL Window\n");
		fDLWindow = new WDownload(NULL, GetUserID());
		Q_CHECK_PTR(fDLWindow);

		connect(fDLWindow, SIGNAL(FileFailed(const QString &, const QString &, const QString &, const QString &, const QString &)),
			this, SLOT(FileFailed(const QString &, const QString &, const QString &, const QString &, const QString &)));
		connect(fDLWindow, SIGNAL(FileInterrupted(const QString &, const QString &, const QString &, const QString &, const QString &)),
			this, SLOT(FileInterrupted(const QString &, const QString &, const QString &, const QString &, const QString &)));
		connect(fDLWindow, SIGNAL(Closed()), this, SLOT(DownloadWindowClosed()));
	}
	fDLWindow->show();
}

void
WinShareWindow::OpenUpload()
{
	if (!fULWindow)
	{
		PRINT("New UL Window\n");
		fULWindow = new WUpload(NULL, fFileScanThread);
		Q_CHECK_PTR(fULWindow);

		connect(fULWindow, SIGNAL(Closed()), this, SLOT(UploadWindowClosed()));
	}
	fULWindow->show();
}

void
WinShareWindow::OpenDownloads()
{
	if (fDLWindow && fDLWindow->isVisible())
		fDLWindow->hide();
	else
		OpenDownload();
}

void
WinShareWindow::OpenUploads()
{
	if (fULWindow && fULWindow->isVisible())
		fULWindow->hide();
	else
		OpenUpload();
}

void
WinShareWindow::OpenSearch()
{
	if (!fSearch)
	{
		fSearch = new WSearch(NULL, fNetClient);
		Q_CHECK_PTR(fSearch);
		fSearch->SetGotResults(true);
		fSearch->LoadSettings();
	}

	if (fSearch->isVisible())
		fSearch->hide();
	else
		fSearch->show();
}

void
WinShareWindow::SearchMusic()
{
	OpenSearch();
	fSearch->SetSearch("*.mp3,*.ogg,*.wma,*.wav");
	fSearch->GoSearch();
}

void
WinShareWindow::SearchVideos()
{
	OpenSearch();
	fSearch->SetSearch("*.mpg,*.mpeg,*.m2v,*.vob,*.avi,*.wmv,*.asf");
	fSearch->GoSearch();
}

void
WinShareWindow::SearchPictures()
{
	OpenSearch();
	fSearch->SetSearch(imageFormats().replace(";", ","));
	fSearch->GoSearch();
}

void
WinShareWindow::SearchImages()
{
	OpenSearch();
	fSearch->SetSearch("*.iso,*.be,*.ccd");
	fSearch->GoSearch();
}

void
WinShareWindow::OpenViewer()
{
	if (!fPicViewer)
	{
		fPicViewer = new WPicViewer(NULL);
		Q_CHECK_PTR(fPicViewer);
		fPicViewer->LoadSettings();
	}

	if (fPicViewer->isVisible())
		fPicViewer->hide();
	else
		fPicViewer->show();
}

void
WinShareWindow::OpenChannels()
{
	if (fChannels->isVisible())
		fChannels->hide();
	else
		fChannels->show();
}

void
WinShareWindow::SetDelayedSearchPattern(const QString & pattern)
{
	if ((fOnConnect != QString::null) && (fOnConnect.length() > 2))
	{
		fOnConnect2 = fOnConnect;
	}

	fOnConnect =  "/search ";
	fOnConnect += pattern;
}

void
WinShareWindow::ScanShares(bool rescan)
{
	// is sharing enabled?
	if (!fSettings->GetSharingEnabled())
	{
		if (fSettings->GetError())
		{
			SendErrorEvent(tr("File sharing not enabled."));
		}
		return;
	}

	WaitOnFileThread(rescan);

	// already running?
	if (fFileScanThread->IsInternalThreadRunning())
	{
		if (fSettings->GetError())
		{
			SendErrorEvent(tr("Already scanning!"));
		}
		return;
	}

	fScanning = true;

	if (fSettings->GetInfo())
	{
		if (rescan)
		{
			SendSystemEvent(tr("Rescanning shared files..."));
			fFilesScanned = false;
		}
		else
		{
			SendSystemEvent(tr("Scanning shares..."));
		}
	}

	PRINT("Starting...\n");
	fFileScanThread->StartInternalThread();
}

int64
WinShareWindow::GetRegisterTime(const QString & nick) const
{
	return fSettings->GetRegisterTime(nick);
}

int64
WinShareWindow::GetRegisterTime() const
{
	return fSettings->GetRegisterTime( GetUserName() );
}

void
WinShareWindow::keyPressEvent(QKeyEvent *event)
{
	ChatWindow::keyPressEvent(event);
}

QWidget *
WinShareWindow::Window()
{
	return this;
}

void
WinShareWindow::LogString(const QString &text)
{
	fMainLog.LogString(text, true);
}

void
WinShareWindow::LogString(const char *text)
{
	fMainLog.LogString(text, true);
}

void
WinShareWindow::UpdateShares()
{
	if (fSettings->GetSharingEnabled()) // Make sure sharing is enabled and fully functional
	{
		CancelShares();

		if (fListThread->IsInternalThreadRunning())
			fListThread->ShutdownInternalThread();

		fListThread->StartInternalThread();
	}
}

void
WinShareWindow::timerEvent(QTimerEvent *)
{
	if (fSettings->GetTimeStamps())
	{
		setStatus(GetTimeStamp2(), 4);
		if (fNetClient->IsLoggedIn())
		{
			uint64 fLoginTime = GetCurrentTime64() - fNetClient->LoginTime();
			if (fLoginTime >= 1000000)
			{
				QString tmp = tr("Logged In: %1").arg(MakeHumanDiffTime(fLoginTime));
				setStatus(tmp, 3);
			}
		}
	}
}

void
WinShareWindow::SendTextEvent(const QString &text, WTextEvent::Type t)
{
	TextEvent(this, text, t);
}

void
WinShareWindow::SendSystemEvent(const QString &message)
{
	SystemEvent(this, message);
}

void
WinShareWindow::SendErrorEvent(const QString &message)
{
	ErrorEvent(this, message);
}

void
WinShareWindow::SendWarningEvent(const QString &message)
{
	WarningEvent(this, message);
}

bool
WinShareWindow::FindSharedFile(QString & file)
{
	bool found = false;
	QString res;

	fFileScanThread->Lock();
	file = fFileScanThread->ResolveLink(file);

	MessageRef ref;
	for (int n = 0; n < fFileScanThread->GetNumFiles(); n++)
	{
		fFileScanThread->GetSharedFile(n, ref);
		QFileInfo info(file);
		String spath, sfile;
		QString qpath, qfile;
		if ((ref()->FindString("beshare:Path", spath) == B_OK) &&
			(ref()->FindString("beshare:File Name", sfile) == B_OK)
			)
		{
			qpath = QString::fromUtf8(spath.Cstr());
			qfile = QString::fromUtf8(sfile.Cstr());

			if (
#ifdef WIN32
				(qpath.lower() == info.dirPath(true).lower()) &&
				(qfile.lower() == info.fileName().lower())
#else
				(qpath == info.dirPath(true)) &&
				(qfile == info.fileName())
#endif
				)
			{
				res = info.fileName();
				ConvertToRegex(res, true);
				res.replace(QRegExp(" "), "?");
				res.replace(QRegExp("@"), "?");		// Try to avoid @ in filenames
				found = true;
				break;
			}
		}
	}

	fFileScanThread->Unlock();

	if (found)
		file = res;

	return found;
}
