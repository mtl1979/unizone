#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "aboutdlgimpl.h"
#include "downloadimpl.h"
#include "winsharewindow.h"
#include "version.h"
#include "debugimpl.h"
#include "chattext.h"
#include "formatting.h"
#include "textevent.h"
#include "htmlview.h"
#include "privatewindowimpl.h"
#include "picviewerimpl.h"
#include "wpwevent.h"
#include "wsystemevent.h"
#include "wwarningevent.h"
#include "werrorevent.h"
#include "combo.h"
#include "menubar.h"
#include "util/StringTokenizer.h"
#include "regex/StringMatcher.h"
#include "iogateway/PlainTextMessageIOGateway.h"
#include "zlib/ZLibUtilityFunctions.h"
#include "settings.h"
#include "filethread.h"
#include "tokenizer.h"								// <postmaster@raasu.org> 20021114
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

#include <qapplication.h>
#include <qstylesheet.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qimage.h>
#if !defined(QT_NO_STYLE_MOTIF)
#include <qmotifstyle.h>
#endif
#if !defined(QT_NO_STYLE_WINDOWS)
#include <qwindowsstyle.h>
#endif
#if !defined(QT_NO_STYLE_PLATINUM)
#include <qplatinumstyle.h>
#endif
#if !defined(QT_NO_STYLE_CDE)
#include <qcdestyle.h>
#endif
#if !defined(QT_NO_STYLE_INTERLACE)
#include <qinterlacestyle.h>
#endif
#if !defined(QT_NO_STYLE_MOTIF)
#include <qmotifplusstyle.h>
#endif
#if !defined(QT_NO_STYLE_SGI)
#include <qsgistyle.h>
#endif
#include <qcstring.h>
#include <qtextcodec.h>
#include <qdir.h>
#include <qinputdialog.h>
#include <qtoolbar.h>

#ifdef WIN32
#include <objbase.h>
#endif

// uptime

#if defined(__LINUX__) || defined(linux) 
#include <sys/sysinfo.h>
#elif defined(__FreeBSD__) || defined(__QNX__)
#include <sys/time.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#define CLUMP_CHAR '\1'
#define NUM_TOOLBARS 4

const int kListSizes[6] = { 200, 75, 100, 150, 150, 75 };


WinShareWindow * gWin = NULL;

WinShareWindow::WinShareWindow(QWidget * parent, const char* name, WFlags f)
	: QMainWindow(parent, name, f | WPaintDesktop | WPaintClever),  
	ChatWindow(MainType), pLock(true), rLock(true), fSearchLock(true)
{
	fMenus = NULL;

	if ( !name ) 
		setName( "WinShareWindow" );

	// IMPORTANT!! :)
	CreateDirectories();
	gWin = this;
	fDLWindow = NULL;
	fAccept = NULL;
	fFileScanThread = NULL;
	fFilesScanned = false;
	fPicViewer = new WPicViewer(this);
	CHECK_PTR(fPicViewer);
	
	InitLaunchThread();

	fUpdateThread = new UpdateClient(this);
	CHECK_PTR(fUpdateThread);

	fDisconnectCount = 0;	// Initialize disconnection count
	fDisconnect = false;	// No premature disconnection yet
	fDisconnectFlag = false; // User hasn't disconnected manually yet.
	
	fNetClient = new NetClient(this);
	CHECK_PTR(fNetClient);

	fServerThread = new ServerClient(this);
	CHECK_PTR(fServerThread);

	fSettings = new WSettings;
	CHECK_PTR(fSettings);

	setCaption("Unizone");

	resize(800, 600);
	InitGUI();

	connect(fNetClient, SIGNAL(UserDisconnected(const QString &, const QString &)), 
			this, SLOT(UserDisconnected(const QString &, const QString &)));
	connect(fNetClient, SIGNAL(UserConnected(const QString &)), 
			this, SLOT(UserConnected(const QString &)));
	connect(fNetClient, SIGNAL(UserNameChanged(const QString &, const QString &, const QString &)), 
			this, SLOT(UserNameChanged(const QString &, const QString &, const QString &)));
	connect(fNetClient, SIGNAL(DisconnectedFromServer()), 
			this, SLOT(DisconnectedFromServer()));
	connect(fNetClient, SIGNAL(UserStatusChanged(const QString &, const QString &, const QString &)), 
			this, SLOT(UserStatusChanged(const QString &, const QString &, const QString &)));
	connect(fNetClient, SIGNAL(UserHostName(const QString &, const QString &)), 
			this, SLOT(UserHostName(const QString &, const QString &)));
	connect(fInputText, SIGNAL(TabPressed(const QString &)), 
			this, SLOT(TabPressed(const QString &)));
	connect(fChatText, SIGNAL(URLClicked(const QString &)), 
			this, SLOT(URLClicked(const QString &)));
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(AboutToQuit()));

	// create popup menu
	fPrivate = new QPopupMenu(this);	// have it deleted on destruction of window
	CHECK_PTR(fPrivate);
	connect(fPrivate, SIGNAL(activated(int)), this, SLOT(PopupActivated(int)));
	connect(fUsers, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(RightButtonClicked(QListViewItem *, const QPoint &, int)));
	connect(fUsers, SIGNAL(doubleClicked(QListViewItem *)),
			this, SLOT(DoubleClicked(QListViewItem *)));

	LoadSettings();

	// initialize local user
	fUserName = fUserList->currentText();
	fServer = fServerList->currentText();
	fUserStatus = fStatusList->currentText();

	// server check thread
	// run the thread here
	if (fSettings->GetAutoUpdateServers())
	{
		AbstractReflectSessionRef scref(new ThreadWorkerSession(), NULL);
		scref()->SetGateway(AbstractMessageIOGatewayRef(new PlainTextMessageIOGateway, NULL));
		if (fServerThread->StartInternalThread() == B_OK)
		{
			PRINT("Server check thread started\n");
			if (fServerThread->AddNewConnectSession("beshare.tycomsystems.com", 80, scref) != B_OK)
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
		AbstractReflectSessionRef psref(new ThreadWorkerSession(), NULL);
		psref()->SetGateway(AbstractMessageIOGatewayRef(new PlainTextMessageIOGateway, NULL));
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
#if defined(WIN32)
		PrintSystem(tr("Welcome to Unizone (English)! <b>THE</b> MUSCLE client for Windows!"));
#elif defined(__LINUX__) || defined(linux)
		PrintSystem(tr("Welcome to Unizone (English)! <b>THE</b> MUSCLE client for Linux!"));
#elif defined(__FreeBSD__)
		PrintSystem(tr("Welcome to Unizone (English)! <b>THE</b> MUSCLE client for FreeBSD!"));
#elif defined(__QNX__)
		PrintSystem(tr("Welcome to Unizone (English)! <b>THE</b> MUSCLE client for QNX Neutrino!"));
#endif
		// <postmaster@raasu.org> 20030225
		PrintSystem(tr("Copyright (C) %1 Mika T. Lindqvist.").arg(GetUnizoneYears()));
		PrintSystem(tr("Original idea by Vitaliy Mikitchenko."));
		PrintSystem(tr("Released to public use under LGPL."));
		PrintSystem(tr("Type /help for a command reference."));
	}

#ifdef WIN32
	// try to find our handle
	FindWindowHandle("Unizone");
#endif

	// setup accept thread
	if (fSettings->GetSharingEnabled())
		StartAcceptThread();

	if (fSettings->GetMaximized())
		showMaximized();

	fFileScanThread = new WFileThread(fNetClient, this, &fFileShutdownFlag);
	CHECK_PTR(fFileScanThread);

	fFileShutdownFlag = false;
//	fScrollDown = true;

	if (fSettings->GetSharingEnabled())
		ScanShares();

	if (fSettings->GetLoginOnStartup())
		Connect();	// this is the LAST thing to do

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
	CHECK_PTR(fAccept);

	connect(fAccept, SIGNAL(ConnectionAccepted(SocketHolderRef)), 
			this, SLOT(ConnectionAccepted(SocketHolderRef)));

	uint32 pStart = (uint32) gWin->fSettings->GetBasePort();
	uint32 pEnd = pStart + (uint32) gWin->fSettings->GetPortRange() - 1;

	for (uint16 i = pStart; i <= pEnd; i++)
	{
		// dynamically allocate a port if we can't get one of ours
		if (fAccept->SetPort(i) == B_OK)
		{
			if (fAccept->StartInternalThread() == B_OK)
			{
				if (fSettings->GetInfo())
				{
					PrintSystem(tr("Accept thread ready and listening on port %1.").arg(fAccept->GetPort()));
				}

				// let the net client know our port
				fNetClient->SetPort(fAccept->GetPort());	
				// BUG FIX: Port not being set in certain occasions
				fNetClient->SetUserName(fUserName);			

				return true;
			}
			else
			{
				PrintError(tr("Failed to start accept thread!"));
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
	// Search Pane

	StopSearch();
	ClearList();

	fIsRunning = false;

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
		delete fMenus;
		fMenus = NULL;
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

	DeinitLaunchThread();

	// Do these two after everything else

	if (fSettings)
	{
		PRINT("Saving settings\n");
		SaveSettings();
		delete fSettings;
		fSettings = NULL; // <postmaster@raasu.org> 20021027
	}

	StopLogging();
}

void
WinShareWindow::customEvent(QCustomEvent * event)
{
	PRINT("\tWinShareWindow::customEvent\n");
	if (fNetClient)		// do this to avoid bad crash
	{
		switch ((int) event->type())
		{
		case WinShareWindow::ConnectRetry:
			{
				PRINT("\tWinShareWindow::ConnectRetry\n");
				if (!fReconnectTimer->isActive())
				{
					// Connect();
					PrintSystem(tr( "Reconnecting in 1 minute!" ));
					fReconnectTimer->start(60000, true); // 1 minute
				}
				return;
			}
		case WinShareWindow::UpdatePrivateUsers:
			{
				PRINT("\tWinShareWindow::UpdatePrivateUsers\n");
				emit UpdatePrivateUserLists();
				return;
			}
		case WFileThread::ScanDone:
			{
				PRINT("\tWinShareWindow::ScanDone\n");
				fFilesScanned = true;
				if (fSettings->GetInfo())
					PrintSystem(tr("Finished scanning shares."));
				if (fGotParams)
					UpdateShares();
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
					PrintSystem(tr("Reconnect timer stopped"));
					fReconnectTimer->stop();
				}

				if (fConnectTimer->isActive())
				{
					fConnectTimer->stop();
				}

				fDisconnect = false;
				fDisconnectFlag = false;
				fDisconnectCount = 0;

				// Set Outgoing Message Encoding
				uint32 enc = fSettings->GetEncoding(GetServerName(fServer), GetServerPort(fServer));
				fNetClient->SetOutgoingMessageEncoding( enc );
				fStatusBar->setText(tr( "Current compression: %1" ).arg(enc - MUSCLE_MESSAGE_ENCODING_DEFAULT), 1);
				

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

				

				PRINT("Uploading public data\n");
				fGotParams = false; // set to false here :)
				fGotResults = true; // fake that we got results, as there is no search yet.
				// send a message out to the server asking for our parameters
				MessageRef askref(GetMessageFromPool(PR_COMMAND_GETPARAMETERS));
				fNetClient->SendMessageToSessions(askref);
				// get a list of users as well
				static String subscriptionList[] = {
					"SUBSCRIBE:beshare",		// base BeShare node
					"SUBSCRIBE:beshare/*",		// all user info :)
					"SUBSCRIBE:unishare/*",		// all unishare-specific user data
					"SUBSCRIBE:unishare/channeldata/*", // all unishare-specific channel data
					NULL
				};
				fNetClient->AddSubscriptionList(subscriptionList); 
				fNetClient->SetUserName(GetUserName());
				fNetClient->SetUserStatus(GetStatus());
				fNetClient->SetConnection(fSettings->GetConnection());
				fNetClient->SetFileCount(0);
				if (fSettings->GetSharingEnabled())
				{
					fNetClient->SetLoad(0, fSettings->GetMaxUploads());
					// Fake that we are scanning, so uploads get queued until we scan the very first time.
					fScanning = true; 
				}
				
				fNetClient->SendMessageToSessions(GetMessageFromPool(PR_COMMAND_PING));
				
				if (fSettings->GetInfo())
					PrintSystem(tr("Connected."));

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
		case WPWEvent::TextEvent:
			{
				PRINT("\tWPWEvent::TextEvent\n");
				WPWEvent * wpe = dynamic_cast<WPWEvent *>(event);
				if (wpe)
				{
					WTextEvent te("");

#ifdef _DEBUG
					WString wText(wpe->GetText());
					PRINT("wpe->GetText() = %S\n", wText.getBuffer());
#endif

					te.SetText(wpe->GetText());
					if (wpe->GetWantReply())	// reply wanted... do the following...
					{
						bool rep = false;
#ifdef _DEBUG
						wText = te.Text();
						PRINT("Sending the following text to SendChatText %S\n", wText.getBuffer());
#endif

						SendChatText(&te, &rep);
						if (rep)	// does this event WANT a reply
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
					if (DoTabCompletion(wpe->GetText(), res, NULL))
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
					pLock.lock();
					WPrivIter it = fPrivateWindows.find((WPrivateWindow * const)wpe->SendTo());
					if (it != fPrivateWindows.end())
					{
						fPrivateWindows.erase(it);
						PRINT("Removed\n");
					}
					pLock.unlock();
				}
				return;
			}
		
		case WSystemEvent::SystemEvent:
			{
				WSystemEvent *wse = dynamic_cast<WSystemEvent *>(event);
				if (wse)
					PrintSystem(wse->GetText());
			}

		case WWarningEvent::WarningEvent:
			{
				WWarningEvent *wwe = dynamic_cast<WWarningEvent *>(event);
				if (wwe)
					PrintWarning(wwe->GetText());
			}

		case WErrorEvent::ErrorEvent:
			{
				WErrorEvent *wee = dynamic_cast<WErrorEvent *>(event);
				if (wee)
					PrintError(wee->GetText());
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
		if (sender == fUserList)
		{
			PRINT("Received text change event from UserName combo\n");
			// change the user name
			NameChanged(e->Text());
		}
		else if (sender == fStatusList)
		{
			PRINT("Received text change event from Status combo\n");
			StatusChanged(e->Text());
			if (e->Text().stripWhiteSpace().lower() != fAwayMsg.stripWhiteSpace().lower())
			{
				SetAutoAwayTimer();
				fAway = false;	// in case away was true;
			}
		}
		else if (sender == fServerList)
		{
			PRINT("Received text change event from Server combo\n");
			ServerChanged(e->Text());
		}
		else if (sender == fSearchEdit)
		{
			PRINT("Received text change event from Search combo\n");
			GoSearch();
		}

	}
}

void
WinShareWindow::StatusChanged(const QString & newStatus)
{
	fNetClient->SetUserStatus(newStatus); // <postmaster@raasu.org> 20021001
	fUserStatus = newStatus;
	QString pstatus(fUserStatus);
	EscapeHTML(pstatus);

	// <postmaster@raasu.org> 20020929,20030211
	TranslateStatus(pstatus);
	//

	QString status = WFormat::StatusChanged( FixStringStr(pstatus) );
	PrintSystem(WFormat::Text(status));
}

void
WinShareWindow::ServerChanged(const QString & newServer)
{
	fServer = newServer;
	if (fServer.right(5) == ":2960")
		fServer.truncate(fServer.length() - 5); // strip default port

	if (fNetClient->IsConnected())
		Connect();
}

void
WinShareWindow::NameChanged(const QString & newName)
{
	int64 nr;
	// Make sure old user name has register time
	if (fUserName != QString::null)
	{
		nr = fSettings->GetRegisterTime(fUserName);
		fSettings->SetRegisterTime(fUserName, nr);
	}
	fNetClient->SetUserName(newName); // <postmaster@raasu.org> 20021011
	QString status = WFormat::NameChanged(newName);
	QString nameChanged = WFormat::SystemText(status); // <postmaster@raasu.org> 20021001
	PrintText(nameChanged);
	fUserName = newName;
	// Make sure new user name has register time
	if (fUserName != QString::null)
	{
		nr = fSettings->GetRegisterTime(fUserName);
		fSettings->SetRegisterTime(fUserName, nr);
	}
}

void
WinShareWindow::resizeEvent(QResizeEvent * event)
{
	QMainWindow::resizeEvent(event);
}

void
WinShareWindow::InitGUI()
{
	// divide our splitter(s)
	QValueList<int> splitList;
	splitList.append(4);
	splitList.append(1);
	//

	fTBMenu = new QToolBar( this );
	addToolBar( fTBMenu, tr( "Menubar" ), Top, FALSE );

	fMenus = new MenuBar(this, fTBMenu);
	CHECK_PTR(fMenus);

	fTBMenu->setStretchableWidget( fMenus );
	setDockEnabled( fTBMenu, Left, FALSE );
	setDockEnabled( fTBMenu, Right, FALSE );


	// setup combo/labels
	// we define the combos as QComboBox, but use WComboBox for 
	// messaging purposes :)

	// Server
	//
	
	fTBServer = new QToolBar( this );
	addToolBar( fTBServer, tr( "Server bar" ), Top, TRUE );

	fServerLabel = new QLabel(tr("Server:"), fTBServer);
	CHECK_PTR(fServerLabel);
	fServerLabel->setMinimumWidth(75);

	fServerList = new WComboBox(this, fTBServer, "fServerList");
	CHECK_PTR(fServerList);
	fServerList->setEditable(true);
	fServerLabel->setBuddy(fServerList);

	// Nick
	//

	fTBNick = new QToolBar( this );
	addToolBar( fTBNick, tr( "Nickbar" ), Top, FALSE );

	fUserLabel = new QLabel(tr("Nick:"), fTBNick);
	CHECK_PTR(fUserLabel);
	fUserLabel->setMinimumWidth(75);

	fUserList = new WComboBox(this, fTBNick, "fUserList");
	CHECK_PTR(fUserList);
	fUserList->setEditable(true);
	fUserLabel->setBuddy(fUserList);

	// Status
	//

	fTBStatus = new QToolBar( this );
	addToolBar( fTBStatus, tr( "Statusbar" ), Top, FALSE );

	fStatusLabel = new QLabel(tr("Status:"), fTBStatus);
	CHECK_PTR(fStatusLabel);
	fStatusLabel->setMinimumWidth(75);

	fStatusList = new WComboBox(this, fTBStatus, "fStatusList");
	CHECK_PTR(fStatusList);
	fStatusList->setEditable(true);
	fStatusLabel->setBuddy(fStatusList);

	//
	
	fTabs = new QTabWidget(this);
	CHECK_PTR(fTabs);

	setCentralWidget(fTabs);

	// Initialize variables for Search Pane

	fCurrentSearchPattern = "";
	fIsRunning = false;
	
	// Create the Search Pane
	
	fSearchWidget = new QWidget(fTabs, "Search Widget");
	CHECK_PTR(fSearchWidget);

	fSearchTab = new QGridLayout(fSearchWidget, 14, 10, 0, -1, "Search Tab");
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
	
	fSearchList = new WUniListView(fSearchWidget);
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

	fStatus = new WStatusBar(fSearchWidget);
	CHECK_PTR(fStatus);
	fStatus->setSizeGripEnabled(false);

	fSearchTab->addMultiCellWidget(fStatus, 13, 13, 0, 9);

	// Search Query Label

	fSearchLabel = new QLabel(fSearchWidget);
	CHECK_PTR(fSearchLabel);

	fSearchTab->addMultiCellWidget(fSearchLabel, 1, 1, 0, 3); 

	// Search Query Combo Box

	fSearchEdit = new WComboBox(this, fSearchWidget, "fSearchEdit");
	CHECK_PTR(fSearchEdit);

	fSearchEdit->setEditable(true);
	fSearchEdit->setMinimumWidth((int) (this->width()*0.75));
	fSearchEdit->setDuplicatesEnabled(false);
	fSearchEdit->setAutoCompletion(true);

	fSearchLabel->setBuddy(fSearchEdit);
	fSearchLabel->setText(tr("Search:"));

	fSearchTab->addMultiCellWidget(fSearchEdit, 1, 1, 4, 9);
	
	// Download Button

	fDownload = new QPushButton(tr("Download"), fSearchWidget);
	CHECK_PTR(fDownload);

	fSearchTab->addMultiCellWidget(fDownload, 10, 10, 0, 2);

	// Stop Button

	fStop = new QPushButton(tr("Stop"), fSearchWidget);
	CHECK_PTR(fStop);

	fSearchTab->addMultiCellWidget(fStop, 10, 10, 4, 6);

	// Clear Button

	fClear = new QPushButton(tr("Clear"), fSearchWidget);
	CHECK_PTR(fClear);

	fSearchTab->addMultiCellWidget(fClear, 10, 10, 8, 9);

	// Clear History Button

	fClearHistory = new QPushButton(tr("Clear History"), fSearchWidget);
	CHECK_PTR(fClearHistory);

	fSearchTab->addMultiCellWidget(fClearHistory, 11, 11, 8, 9);

	// connect up slots

	connect(fNetClient, SIGNAL(AddFile(const QString &, const QString &, bool, MessageRef)), 
			this, SLOT(AddFile(const QString &, const QString &, bool, MessageRef)));
	connect(fNetClient, SIGNAL(RemoveFile(const QString &, const QString &)), 
			this, SLOT(RemoveFile(const QString &, const QString &)));

	connect(fClear, SIGNAL(clicked()), this, SLOT(ClearList()));
	connect(fStop, SIGNAL(clicked()), this, SLOT(StopSearch()));
	connect(fDownload, SIGNAL(clicked()), this, SLOT(Download()));
	connect(fClearHistory, SIGNAL(clicked()), this, SLOT(ClearHistory()));

	fQueue = GetMessageFromPool();

	SetSearchStatus(tr("Idle."));

	//
	// End of Search Pane
	//

	// Create the Channels Pane

	fChannelsWidget = new QWidget(fTabs, "Channels Widget");
	CHECK_PTR(fChannelsWidget);

	QGridLayout * fChannelsTab = new QGridLayout(fChannelsWidget, 7, 5, 0, -1, "Channels Tab");
	CHECK_PTR(fChannelsTab);

	fChannelsTab->addRowSpacing(5, 20);

	fChannelsTab->setRowStretch(0, 2);
	fChannelsTab->setRowStretch(1, 2);
	fChannelsTab->setRowStretch(2, 2);
	fChannelsTab->setRowStretch(3, 2);
	fChannelsTab->setRowStretch(4, 2);
	fChannelsTab->setRowStretch(5, 0);
	fChannelsTab->setRowStretch(6, 1);

	fChannelsTab->addColSpacing(0, 20);
	fChannelsTab->addColSpacing(2, 20);
	fChannelsTab->addColSpacing(4, 20);

	ChannelList = new QListView( fChannelsWidget, "ChannelList" );
	CHECK_PTR(ChannelList);

	ChannelList->addColumn( tr( "Name" ) );
	ChannelList->addColumn( tr( "Topic" ) );
	ChannelList->addColumn( tr( "Users" ) );
	ChannelList->addColumn( tr( "Admins" ) );
	ChannelList->addColumn( tr( "Public" ) );

	fChannelsTab->addMultiCellWidget(ChannelList, 0, 4, 0, 4);
	
	Create = new QPushButton( tr( "&Create" ), fChannelsWidget, "Create" );
	CHECK_PTR(Create);

	fChannelsTab->addWidget(Create, 6, 1);

	Join = new QPushButton( tr( "&Join" ), fChannelsWidget, "Join" );
	CHECK_PTR(Join);

	fChannelsTab->addWidget(Join, 6, 3);

	connect( 
		fNetClient, SIGNAL(ChannelAdded(const QString &, const QString &, int64)), 
		this, SLOT(ChannelAdded(const QString &, const QString &, int64)) 
		);
	connect(
		fNetClient, SIGNAL(ChannelTopic(const QString &, const QString &, const QString &)),
		this, SLOT(ChannelTopic(const QString &, const QString &, const QString &))
		);
	connect(
		fNetClient, SIGNAL(ChannelOwner(const QString &, const QString &, const QString &)),
		this, SLOT(ChannelOwner(const QString &, const QString &, const QString &))
		);
	connect(
		fNetClient, SIGNAL(ChannelPublic(const QString &, const QString &, bool)),
		this, SLOT(ChannelPublic(const QString &, const QString &, bool))
		);
	connect(
		fNetClient, SIGNAL(ChannelAdmins(const QString &, const QString &, const QString &)),
		this, SLOT(ChannelAdmins(const QString &, const QString &, const QString &))
		);
	connect(
		fNetClient, SIGNAL(UserIDChanged(const QString &, const QString &)),
		this, SLOT(UserIDChanged(const QString &, const QString &))
		);

	// Window widget events
	connect(Create, SIGNAL(clicked()), this, SLOT(CreateChannel()));
	connect(Join, SIGNAL(clicked()), this, SLOT(JoinChannel()));

	//
	// End of Channels Pane
	//

	// chat widget
	fMainWidget = new QWidget(fTabs);
	CHECK_PTR(fMainWidget);

	fMainTab = new QGridLayout(fMainWidget, 14, 10, 0, -1, "Chat Tab");
	CHECK_PTR(fSearchTab);

	// main splitter
	fMainSplitter = new QSplitter(fMainWidget);
	CHECK_PTR(fMainSplitter);

	fMainTab->addMultiCellWidget(fMainSplitter, 0, 12, 0, 9);

	// user list
	fUsersBox = new QHGroupBox(fMainSplitter);
	CHECK_PTR(fUsersBox);

	fUsers = new WUniListView(fUsersBox);
	CHECK_PTR(fUsers);
	// initialize the list view
	fUsers->addColumn(tr("Name"));
	fUsers->addColumn(tr("ID"));
	fUsers->addColumn(tr("Status"));
	fUsers->addColumn(tr("Files"));
	fUsers->addColumn(tr("Connection"));
	fUsers->addColumn(tr("Load"));
	// as of now... WinShare specific, WinShare pings all the users and parses the string for client info
	fUsers->addColumn(tr("Client"));		
	

	fUsers->setColumnAlignment(WNickListItem::ID, AlignRight); // <postmaster@raasu.org> 20021005
	fUsers->setColumnAlignment(WNickListItem::Files, AlignRight); // <postmaster@raasu.org> 20021005
	fUsers->setColumnAlignment(WNickListItem::Load, AlignRight); // <postmaster@raasu.org> 20021005

	for (int column = 0; column < 6; column++)
		fUsers->setColumnWidthMode(column, QListView::Manual);

	// set the sort indicator to show
	fUsers->setShowSortIndicator(true);
	
	fUsers->setAllColumnsShowFocus(true);

	// left pane
	fLeftPane = new QVGroupBox(fMainSplitter);
	CHECK_PTR(fLeftPane);

	fMainSplitter->moveToLast(fUsersBox);		// put list box in right pane
	fMainSplitter->moveToFirst(fLeftPane);

	fMainSplitter->setSizes(splitList);

	//
	// chat widgets
	//

	// chat splitter first though...
	fChatSplitter = new QSplitter(fLeftPane);
	CHECK_PTR(fChatSplitter);
	fChatSplitter->setOrientation(QSplitter::Vertical);

	fChatText = new WHTMLView(fChatSplitter);
	CHECK_PTR(fChatText);
	fChatText->setTextFormat(QTextView::RichText);

	//
	// fInputText is a QMultiLineEdit, but we create
	// a WChatText for more advanced handling
	//

	fInputText = new WChatText(this, fChatSplitter);
	CHECK_PTR(fInputText);
	fInputText->setWordWrap(QMultiLineEdit::WidgetWidth);
	fChatSplitter->setSizes(splitList);

	// Main Status Bar

	fStatusBar = new WStatusBar(fMainWidget);
	CHECK_PTR(fStatusBar);
	fStatusBar->setSizeGripEnabled(false);
	fMainTab->addMultiCellWidget(fStatusBar, 13, 13, 0, 9);

	fStatusBar->setText(tr( "Not connected." ), 0);

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
	fGotResults = true;

	// setup autoaway timer
	fAutoAway = new QTimer;
	CHECK_PTR(fAutoAway);
	connect(fAutoAway, SIGNAL(timeout()), this, SLOT(AutoAwayTimer()));

	// setup connect timer
	fConnectTimer = new QTimer;
	CHECK_PTR(fConnectTimer);
	connect(fConnectTimer, SIGNAL(timeout()), this, SLOT(ConnectTimer()));

	// setup autoconnect timer
	fReconnectTimer = new QTimer;
	CHECK_PTR(fReconnectTimer);
	connect(fReconnectTimer, SIGNAL(timeout()), this, SLOT(ReconnectTimer()));

	// Insert tabs

	fTabs->insertTab(fMainWidget, tr("Chat"));
	fTabs->insertTab(fSearchWidget, tr("Search"));
	fTabs->insertTab(fChannelsWidget, tr("Channels"));
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

/*
void
WinShareWindow::UpdateTextView()
{
	// <postmaster@raasu.org> 20021021 -- Fixed too long line in debug output
#ifdef DEBUG2
	PRINT("UPDATETEXTVIEW: ContentsX = %d, ContentsY = %d\n", fChatText->contentsX(),		fChatText->contentsY());
	PRINT("              : ContentsW = %d, ContentsH = %d\n", fChatText->contentsWidth(),	fChatText->contentsHeight());
#endif
	if (fScrollDown)
	{
		fScrollY = fChatText->contentsHeight();
	}
	if (fScrollX != fChatText->contentsX() || fScrollY != fChatText->contentsY())
	{
		fChatText->setContentsPos(fScrollX, fScrollY);
#ifndef WIN32	// linux only... (FreeBSD???)
		fChatText->repaintContents(
									fChatText->contentsX(), fChatText->contentsY(),
									fChatText->contentsWidth(), fChatText->contentsHeight(),
									false);
#endif
	}
}
*/
void
WinShareWindow::UpdateUserList()
{
	WUserMap users = fNetClient->Users();

	WUserIter it = users.begin();
	while (it != users.end())
	{
		if ((*it).second())
		{
			(*it).second()->AddToListView(fUsers);
		}
		it++;
	}
}

QString
WinShareWindow::MakeHumanTime(int64 time)
{
	int64 seconds = time / 1000000;
	int64 minutes = seconds / 60;  seconds = seconds % 60;
	int64 hours   = minutes / 60;  minutes = minutes % 60;
	int64 days    = hours   / 24;  hours   = hours   % 24;
	int64 weeks   = days    /  7;  days    = days    % 7;

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
	
	if ((s.length() > 2) && (s.right(2) == ", "))
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
/*
void
WinShareWindow::CheckScrollState()
{
	QScrollBar * scroll = fChatText->verticalScrollBar();
#ifdef DEBUG2
	PRINT("CHECKSCROLLSTATE: value = %d, maxValue = %d, minValue = %d\n", scroll->value(), scroll->maxValue(), scroll->minValue());
#endif
	fScrollX = fChatText->contentsX();
	fScrollY = fChatText->contentsY();
	if (scroll->value() >= scroll->maxValue())
		fScrollDown = true;
	else
		fScrollDown = false;
}
*/
bool
WinShareWindow::ParseUserTargets(const QString & text, WUserSearchMap & sendTo, String & setTargetStr, String & setRestOfString, NetClient * net)
{
	StringTokenizer wholeStringTok((const char *) text.utf8(), " ");
	String restOfString2(wholeStringTok.GetRemainderOfString());
//	restOfString2.Replace(CLUMP_CHAR, ' ');
	const char * w2 = wholeStringTok.GetNextToken();
	if (w2)
	{
		setTargetStr = w2;
//		setTargetStr.Replace(CLUMP_CHAR, ' ');
//		w2 = setTargetStr.Cstr();

		setRestOfString = wholeStringTok.GetRemainderOfString();

		StringTokenizer tok(w2, ",");
		Queue<String> clauses;
		const char * next;
		while((next = tok.GetNextToken()) != NULL)
			clauses.AddTail(String(next).Trim());
	
		// find users by session id
		for (int i = clauses.GetNumItems() - 1; i >= 0; i--)
		{
			WUserRef user = net->FindUser( QString::fromUtf8( clauses[i].Cstr() ) );
			PRINT("Checking for user %s\n", clauses[i].Cstr());
			if (user() != NULL)
			{
				WUserSearchPair pair = MakePair(QString::fromUtf8(clauses[i].Cstr()), user, QString::fromUtf8(setRestOfString.Cstr())); // <postmaster@raasu.org> 20021007
				sendTo.insert(pair);
				clauses.RemoveItemAt(i);
			}
		}

		PRINT("Checking using usernames\n");
		for (int j = clauses.GetNumItems() - 1; j >= 0; j--)
		{
			String tstr(clauses[j]);
			tstr.Trim();
			ConvertToRegex(tstr);
			MakeRegexCaseInsensitive(tstr);
			StringMatcher sm(tstr.Cstr());

			bool foundMatches = false;
			WUserRef user;
			WUserIter iter = net->Users().begin();
			while (iter != net->Users().end())
			{
				user = (*iter).second;
				String userName = String((const char *) user()->GetUserName().utf8()).Trim();
				userName = StripURL(userName);

				if (userName.Length() > 0 && sm.Match(userName.Cstr()))
				{
					WUserSearchPair pair = MakePair(user()->GetUserID(), user, QString::fromUtf8(setRestOfString.Cstr())); // <postmaster@raasu.org> 20021007
					sendTo.insert(pair);
					foundMatches = true;
				}
				iter++;
			}
			if (foundMatches)
			{
				PRINT("Found\n");
				clauses.RemoveItemAt(j);
			}
		}

		PRINT("Checking using tab stuff\n");
		// still no items?
		if (sendTo.empty())
		{
			// tab-completion thingy :)
			WUserRef user;
			WUserIter iter = net->Users().begin();
			while (iter != net->Users().end())
			{
				user = (*iter).second;
				QCString uName = user()->GetUserName().utf8();
				String userName = String((const char *) uName).Trim();
				userName = StripURL(userName);

				if (userName.Length() > 0 && restOfString2.StartsWith(userName))
				{
					PRINT("Found\n");
					WUserSearchPair pair = MakePair(user()->GetUserID(), user,
													QString::fromUtf8(restOfString2.Substring(userName.Length()).Cstr()));
					sendTo.insert(pair);
					setTargetStr = (const char *) uName;
				}
				iter++;
			}
		}
		return true;
	}
	return false;
}

QString
WinShareWindow::GetRemoteVersionString(const MessageRef msg)
{
	QString versionString("?");
	const char * version;

	if (msg()->FindString("version", &version) == B_OK)
	{
		if (version[0] > '0' && version[0] <= '9')
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
		fSearchEdit->clear();

		int i;
		int size;
		QString str;
		// load servers
		for (i = 0; (str = fSettings->GetServerItem(i)) != QString::null; i++)
			fServerList->insertItem(str, i);
		i = fSettings->GetCurrentServerItem();
		if (i < fServerList->count())
			fServerList->setCurrentItem(i);
		fServer = fServerList->currentText();
		
		// load usernames
		for (i = 0; (str = fSettings->GetUserItem(i)) != QString::null; i++)
			fUserList->insertItem(str, i);
		i = fSettings->GetCurrentUserItem();
		if (i < fUserList->count())
			fUserList->setCurrentItem(i);
		fUserName = fUserList->currentText();

		// load status
		for (i = 0; (str = fSettings->GetStatusItem(i)) != QString::null; i++)
		{
			// Skip old 'testing' statuses
			if ((str.startsWith(tr("Testing Unizone (English)")) == false) &&	// new internationalized
				(str.startsWith("Testing Unizone (") == false)					// old format
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

		// load query history
		for (i = 0; (str = gWin->fSettings->GetQueryItem(i)) != QString::null; i++)
			fSearchEdit->insertItem(str, i);
		i = fSettings->GetCurrentQueryItem();
		if (i < fSearchEdit->count())
			fSearchEdit->setCurrentItem(i);

		// load the style
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
			case Platinum:
#if !defined(QT_NO_STYLE_PLATINUM)
				qApp->setStyle(new QPlatinumStyle);
#endif
				break;
			case CDE:
#if !defined(QT_NO_STYLE_CDE)
				qApp->setStyle(new QCDEStyle);
#endif
				break;
			case MotifPlus:
#if !defined(QT_NO_STYLE_MOTIF)
				qApp->setStyle(new QMotifPlusStyle);
#endif
				break;
			case SGI:
#if !defined(QT_NO_STYLE_SGI)
				qApp->setStyle(new QSGIStyle);
#endif
				break;
		}

		// load column sizes
		for (i = 0; (size = fSettings->GetColumnItem(i)) > 0; i++)
			fUsers->setColumnWidth(i, size);

		// load size/position
		int a, b;
		if ((a = fSettings->GetWindowWidth()) > 0 && (b = fSettings->GetWindowHeight()) > 0)
			resize(a, b);
		if ((a = fSettings->GetWindowX()) >= 0 && (b = fSettings->GetWindowY()) >= 0)
			move(a, b);

		QValueList<int> sizes;
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
		WString wAwayMsg(fAwayMsg);
		PRINT("Away Msg: %S\n", wAwayMsg.getBuffer());
#endif
		
		fHereMsg = fSettings->GetHereMsg();

#ifdef _DEBUG
		WString wHereMsg(fHereMsg);
		PRINT("Here Msg: %S\n", wHereMsg.getBuffer());
#endif

		// load colors
		if ((str = fSettings->GetColorItem(0)) != QString::null)
			WColors::LocalName = str;
		if ((str = fSettings->GetColorItem(1)) != QString::null)
			WColors::RemoteName = str;
		if ((str = fSettings->GetColorItem(2)) != QString::null)
			WColors::Text = str;
		if ((str = fSettings->GetColorItem(3)) != QString::null)
			WColors::System = str;
		if ((str = fSettings->GetColorItem(4)) != QString::null)
			WColors::Ping = str;
		if ((str = fSettings->GetColorItem(5)) != QString::null)
			WColors::Error = str;
		if ((str = fSettings->GetColorItem(6)) != QString::null)
			WColors::ErrorMsg = str;
		if ((str = fSettings->GetColorItem(7)) != QString::null)
			WColors::PrivText = str;
		if ((str = fSettings->GetColorItem(8)) != QString::null)
			WColors::Action = str;
		if ((str = fSettings->GetColorItem(9)) != QString::null)
			WColors::URL = str;
		if ((str = fSettings->GetColorItem(10)) != QString::null)
			WColors::NameSaid = str;
		if ((str = fSettings->GetColorItem(11)) != QString::null)
			WColors::Warning = str;
		if ((str = fSettings->GetColorItem(12)) != QString::null)
			WColors::WarningMsg = str;

		fWatch = fSettings->GetWatchPattern();
		fIgnore = fSettings->GetIgnorePattern();
		fIgnoreIP = fSettings->GetIPIgnorePattern();
		fBlackList = fSettings->GetBlackListPattern();
		fWhiteList = fSettings->GetWhiteListPattern();
		fFilterList = fSettings->GetFilterListPattern();
		fAutoPriv = fSettings->GetAutoPrivatePattern();

		fOnConnect = fSettings->GetOnConnect();
		fOnConnect2 = fSettings->GetOnConnect2();

		tx = fSettings->GetTransmitStats(); tx2 = tx;
		rx = fSettings->GetReceiveStats(); rx2 = rx;
		
		fUsers->setSorting(fSettings->GetNickListSortColumn(), fSettings->GetNickListSortAscending());
		fSearchList->setSorting(fSettings->GetSearchListSortColumn(), fSettings->GetSearchListSortAscending());

		fRemote = fSettings->GetRemotePassword();

		// Load resume list

		rLock.lock();
		fResumeMap.clear();

		for (i = 0; i <= fSettings->GetResumeCount(); i++)
		{
			WResumePair wrp;
			if (fSettings->GetResumeItem(i, wrp))
				fResumeMap.insert(wrp);
		}
		rLock.unlock();

		fInstallID = fSettings->GetInstallID();
	}
	else	// file doesn't exist, or error
	{
#if defined(WIN32)
#  if !defined(QT_NO_STYLE_WINDOWS)
		qApp->setStyle(new QWindowsStyle);
#  endif // !QT_NO_STYLE_WINDOWS
#else
#  if !defined(QT_NO_STYLE_MOTIF)
		qApp->setStyle(new QMotifStyle);				
#  endif // !QT_NO_STYLE_MOTIF
#endif // WIN32	

		fAwayMsg = "away";
		fHereMsg = "here";
		fWatch = "";
		fIgnore = "";
		fIgnoreIP = "";
		fBlackList = "";
		fWhiteList = "Atrus, Bubbles";
		fFilterList = "";
		fAutoPriv = "";
		fOnConnect = "";
		fOnConnect2 = "";
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

	int _dock[NUM_TOOLBARS];
	int _index[NUM_TOOLBARS];
	bool _nl[NUM_TOOLBARS];
	int _extra[NUM_TOOLBARS];
	
	for (i = 0; i < NUM_TOOLBARS; i++)
		fSettings->GetToolBarLayout(i, _dock[i], _index[i], _nl[i], _extra[i]); 
	
	// Start from dock position 0
	// Start moving toolbars from index value of 0
	
	for (int d1 = 0; d1 < 7; d1++)			// dock position
	{
		for (int ip = 0; ip < NUM_TOOLBARS; ip++)		// index position
		{
			for (int i1 = 0; i1 < NUM_TOOLBARS; i1++)	// layout index
			{
				if ((_dock[i1] == d1) && (_index[i1] == ip))
				{
					QToolBar * tb = NULL;
					switch (i1)
					{
					case 0: tb = fTBMenu;
					case 1: tb = fTBServer;
					case 2: tb = fTBNick;
					case 3: tb = fTBStatus;
					}
					moveToolBar(tb, (QMainWindow::ToolBarDock) _dock[i1], _nl[i1], 3, _extra[i1]);
				}
			}
		}
	}
}

void
WinShareWindow::SaveSettings()
{
	// update our settings message (it's not updated until SaveSettings() is called
	fSettings->EmptyColumnList();
	fSettings->EmptyServerList();
	fSettings->EmptyStatusList();
	fSettings->EmptyUserList();
	fSettings->EmptyQueryList();
	// don't worry about the color list, prefs does it
	
	// save server list
	int i;
#ifdef _DEBUG
	WString wServer;
#endif
	for (i = 0; i < fServerList->count(); i++)
	{
		fSettings->AddServerItem(fServerList->text(i));

#ifdef _DEBUG
		wServer = fServerList->text(i);
		PRINT("Saved server %S\n", wServer.getBuffer());
#endif
	}
	fSettings->SetCurrentServerItem(fServerList->currentItem());
	
	// save user list
#ifdef _DEBUG
	WString wUser;
#endif
	for (i = 0; i < fUserList->count(); i++)
	{
		fSettings->AddUserItem(fUserList->text(i));

#ifdef _DEBUG
		wUser = fUserList->text(i);
		PRINT("Saved user %S\n", wUser.getBuffer());
#endif
	}
	fSettings->SetCurrentUserItem(fUserList->currentItem());
	
	// save status list
#ifdef _DEBUG
	WString wStatus;
#endif
	for (i = 0; i < fStatusList->count(); i++)
	{
		fSettings->AddStatusItem(fStatusList->text(i));

#ifdef _DEBUG
		wStatus = fStatusList->text(i);
		PRINT("Saved status %S\n", wStatus.getBuffer());
#endif
	}
	fSettings->SetCurrentStatusItem(fStatusList->currentItem());

	// save query history
#ifdef _DEBUG
	WString wQuery;
#endif
	for (i = 0; i < fSearchEdit->count(); i++)
	{
		fSettings->AddQueryItem(fSearchEdit->text(i));

#ifdef _DEBUG
		wQuery = fSearchEdit->text(i);
		PRINT("Saved query %S\n", wQuery.getBuffer());
#endif
	}
	fSettings->SetCurrentQueryItem(fSearchEdit->currentItem());

	// don't worry about style, Prefs does it for us
	
	// save list view column widths
	for (i = 0; i < fUsers->columns(); i++)
		fSettings->AddColumnItem(fUsers->columnWidth(i));
	
	// save window height/position
	fSettings->SetWindowWidth(width());
	fSettings->SetWindowHeight(height());
	fSettings->SetWindowX(x());
	fSettings->SetWindowY(y());

	QValueList<int> sizes;
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

	// on connect

	fSettings->SetOnConnect(fOnConnect);
	fSettings->SetOnConnect2(fOnConnect2);

	// transfer stats

	fSettings->SetTransmitStats(tx);
	fSettings->SetReceiveStats(rx);

	// nick list sort settings

	fSettings->SetNickListSortColumn(fUsers->sortColumn());
	fSettings->SetNickListSortAscending(fUsers->sortAscending());

	// search query list sort settings

	fSettings->SetSearchListSortColumn(fSearchList->sortColumn());
	fSettings->SetSearchListSortAscending(fSearchList->sortAscending());

	// remote control

	fSettings->SetRemotePassword(fRemote);

	// maximized
	fSettings->SetMaximized(isMaximized());

	// save color settings
	// don't worry about it... prefs does it

	// Save resume list

	rLock.lock();
	fSettings->EmptyResumeList();

	i = 0;

	WResumeIter wri = fResumeMap.begin();
	while ( wri != fResumeMap.end() )
	{
		fSettings->AddResumeItem((*wri));
		wri++;
		i++;
	}
	rLock.unlock();
	fSettings->SetResumeCount(i);

	// UniShare

	if (fUserName != QString::null)
	{
		int64 rn = fSettings->GetRegisterTime(fUserName);
		fSettings->SetRegisterTime(fUserName, rn);
	}

	// Toolbar Layout

	int _dock, _index, _extra;
	bool _nl;

	getLocation(fTBMenu, (QMainWindow::ToolBarDock &) _dock, _index, _nl, _extra);
	fSettings->SetToolBarLayout(0, _dock, _index, _nl, _extra);

	getLocation(fTBServer, (QMainWindow::ToolBarDock &) _dock, _index, _nl, _extra);
	fSettings->SetToolBarLayout(1, _dock, _index, _nl, _extra);

	getLocation(fTBNick, (QMainWindow::ToolBarDock &) _dock, _index, _nl, _extra);
	fSettings->SetToolBarLayout(2, _dock, _index, _nl, _extra);

	getLocation(fTBStatus, (QMainWindow::ToolBarDock &) _dock, _index, _nl, _extra);
	fSettings->SetToolBarLayout(3, _dock, _index, _nl, _extra);

	fSettings->SetInstallID(fInstallID);

	fSettings->Save();
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
WinShareWindow::LaunchSearch(const QString & pattern)
{
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
		gWin->SetSearch(pattern.mid(2));
		return;
	}
	// (be)share:pattern
	gWin->SetSearch(pattern);
}

QString
WinShareWindow::MapIPsToNodes(const QString & pattern)
{
	QString qResult("");
	QString qItem;
	QStringTokenizer qTok(pattern, ",");
	while ((qItem = qTok.GetNextToken()) != QString::null)
	{
		qResult += "/" + qItem + "/*,";
	}
	if (qResult.right(1) == ",") 
	{
		qResult.truncate(qResult.length() - 1);
	}

#ifdef _DEBUG
	WString wResult(qResult);
	PRINT("MapIPsToNodes: %S\n", wResult.getBuffer());
#endif

	return qResult;
}

QString 
WinShareWindow::MapUsersToIDs(const QString & pattern)
{
	QString qResult("");
	WUserIter it = gWin->fNetClient->Users().begin();
	QString qTemp = pattern;
	QString qItem;
	int cPos;
	while (qTemp.length() > 0)
		{
			cPos = qTemp.find(",");
			if (cPos < 1) 
			{ 
				cPos = qTemp.length()+1;
			}

			qItem = qTemp.left(cPos-1);

			if (qItem.find("*") > 0) // Space in username? (replaced with '*' for BeShare compatibility)
			{
				for (unsigned int i = 0; i < qItem.length(); i++)
				{
					if (qItem[i] == '*')
						qItem.replace(i,1," ");
				}
			}
		
			while (it != gWin->fNetClient->Users().end())
			{
				if (((*it).second()->GetUserID() == qItem) || ((*it).second()->GetUserName().lower() == qItem.lower()))
				{
					qResult += "/"+(*it).second()->GetUserHostName()+"/";
					qResult += (*it).second()->GetUserID()+",";
					break;
				}
				it++;
			}
			qTemp = qTemp.mid(cPos+1);
		}
	if (qResult.right(1) == ",")
	{
		qResult.truncate(qResult.length() - 1);
	}

#ifdef _DEBUG
	WString wResult(qResult);
	PRINT("MapUsersToIDs: %S\n", wResult.getBuffer());
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
	WPrivateWindow * window = new WPrivateWindow(this, fNetClient, NULL);
	if (!window) 
		return;
	QString qItem;
	QStringTokenizer qTok(users,",");
	
	while ((qItem = qTok.GetNextToken()) != QString::null)
	{
		if (qItem.find("*") > 0) // Space in username? (replaced with '*' for BeShare compatibility)
		{
			for (unsigned int i = 0; i < qItem.length(); i++)
			{
				if (qItem[i] == '*')
					qItem.replace(i,1," ");
			}
		}
		
		WUserIter it = gWin->fNetClient->Users().begin();
		
		while (it != gWin->fNetClient->Users().end())
		{
			if (((*it).second()->GetUserID() == qItem) || (StripURL((*it).second()->GetUserName().lower()) == StripURL(qItem.lower())))
			{
				window->AddUser((*it).second);
				iUsers++;
				break;
			}
			it++;
		}
	}
	if (iUsers == 0) // No valid users?
	{
		delete window;
		return;
	}
	window->show();
	// it's a map... but so far, there is no need for a key
	// as I just iterate through the list
	WPrivPair p = MakePair(window);
	pLock.lock();
	gWin->fPrivateWindows.insert(p);
	pLock.unlock();
}

void
WinShareWindow::QueueFile(const QString & ref)
{
	gWin->QueueFileAux(ref);
}

void
WinShareWindow::QueueFileAux(const QString & ref)
{
	QString from = ref.left(ref.find("/"));
	QString file = ref.mid(ref.find("/") + 1);
	QString sfile = TTPDecode(file);
	TTPInfo * ttpInfo = new TTPInfo;
	if (ttpInfo)
	{
		ttpInfo->bot = from;
		ttpInfo->file = sfile;
		_ttpFiles.AddTail(ttpInfo);
		PrintSystem(tr("Queued file %1 from user #%2.").arg( sfile ).arg(from));
	}
}

void
WinShareWindow::StartQueue(const QString &session)
{
	if (_ttpFiles.GetNumItems() > 0)
	{
		WUserRef user = FindUser(session);
		if (user() == NULL)
			return;

		unsigned int i = 0;
		fSearchLock.lock();
		while (_ttpFiles.GetNumItems() > 0)
		{
			TTPInfo * ttpInfo = NULL;
			_ttpFiles.GetItemAt(i, ttpInfo);
			if (ttpInfo)
			{
				if (ttpInfo->bot == session)
				{
					QueueDownload(ttpInfo->file, user);
					_ttpFiles.RemoveItemAt(i);
					PrintSystem(tr("Downloading file %1 from user #%2.").arg( ttpInfo->file ).arg(session));
				}
				else i++;
				if (i >= _ttpFiles.GetNumItems())
					break;
			}
		}
		EmptyQueues();
		fSearchLock.unlock();
	}
}

void
WinShareWindow::StartLogging()
{
	fMainLog.Create();
	if (!fMainLog.InitCheck())
	{
		if (fSettings->GetError())
            PrintError( tr("Failed to create log file.") );
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
		fNetClient->SendMessageToSessions(delData);
	}
}

// Create our most important directories
void
WinShareWindow::CreateDirectories()
{
	QDir dir("./");
	dir.mkdir("shared");
	dir.mkdir("downloads");
	dir.mkdir("logs");
}

QString
WinShareWindow::GetUptimeString()
{
	return MakeHumanTime(GetUptime());
}

int64
WinShareWindow::GetUptime()
{
#ifdef WIN32
	return ((int64)GetTickCount()) * 1000;
#elif defined(__LINUX__) || defined(linux)
	struct sysinfo sinfo;
	sysinfo(&sinfo);
	int64 uptime = sinfo.uptime;
	uptime *= 1000000L;
	return uptime;
#elif defined(__FreeBSD__) || defined(__QNX__)
	int64 uptime = 0;
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

void
WinShareWindow::TranslateStatus(QString & s)
{
	QString st = s.lower();
	const char *t;
	for (unsigned int x = 0; (t = statusList[x]) != NULL; x++)
	{
		if (st == t)
		{
			s = tr(t);
			break;
		}
	}
}

void 
WinShareWindow::OpenDownload()
{
	if (!fDLWindow)
	{
		PRINT("New DL Window\n");
		fDLWindow = new WDownload(NULL, GetUserID(), fFileScanThread);
		CHECK_PTR(fDLWindow);
		
		connect(fDLWindow, SIGNAL(FileFailed(const QString &, const QString &, const QString &)), 
				this, SLOT(FileFailed(const QString &, const QString &, const QString &)));
		connect(fDLWindow, SIGNAL(FileInterrupted(const QString &, const QString &, const QString &)), 
				this, SLOT(FileInterrupted(const QString &, const QString &, const QString &)));
		connect(fDLWindow, SIGNAL(Closed()), this, SLOT(DownloadWindowClosed()));
	}
	fDLWindow->show();
}

void
WinShareWindow::OpenDownloads()
{
	if (fDLWindow && !fDLWindow->isHidden())
		fDLWindow->hide();
	else
		OpenDownload();
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
			PrintError(tr("File sharing not enabled."));
		}
		return;
	}

	WaitOnFileThread(rescan);

	// already running?
	if (fFileScanThread->IsInternalThreadRunning())
	{
		if (fSettings->GetError())
		{
			PrintError(tr("Already scanning!"));
		}
		return;
	}

	fScanning = true;

	if (fSettings->GetInfo())
	{
		if (rescan)
		{
			PrintSystem(tr("Rescanning shared files..."));
			fFilesScanned = false;
		}
		else
		{
			PrintSystem(tr("Scanning shares..."));
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
	if (event->key() == Key_F12 && event->state() & ControlButton)
	{
		int c = fTabs->currentPageIndex();
		c++;
		if (c > 2) c = 0;
		fTabs->setCurrentPage(c);
	}
	else if (event->key() == Key_F11 && event->state() & ControlButton)
	{
		int c = fTabs->currentPageIndex();
		c--;
		if (c < 0) c = 2;
		fTabs->setCurrentPage(c);
	}
	else
		QMainWindow::keyPressEvent(event);
}

QWidget *
WinShareWindow::Window()
{
	return this;
}

void
WinShareWindow::LogString(const QString &text)
{
	fMainLog.LogString(text);
}

void
WinShareWindow::LogString(const char *text)
{
	fMainLog.LogString(text);
}

void
WinShareWindow::UpdateShares()
{
	if (fSettings->GetSharingEnabled()) // Make sure sharing is enabled and fully functional
	{
		CancelShares();

		fFileScanThread->Lock();
		int numShares = fFileScanThread->GetNumFiles();
		if (fSettings->GetInfo())
			PrintSystem(tr("Sharing %1 file(s).").arg(numShares));
		fNetClient->SetFileCount(numShares);
		PRINT("Doing a scan of the returned files for uploading.\n");
		int m = 0;
		MessageRef refScan(GetMessageFromPool(PR_COMMAND_SETDATA));
		
		if (refScan())
		{
			MessageRef mref;
			HashtableIterator<String, QString> filesIter = fFileScanThread->GetSharedFiles().GetIterator();
			while (filesIter.HasMoreKeys())
			{
				// stop iterating if we are waiting for file scan thread to finish
				if (fFileShutdownFlag)
					break;
				
				qApp->processEvents(300);
				
				String s;
				MessageRef mref;
				filesIter.GetNextKey(s);
				
				if (fFileScanThread->FindFile(s, mref))
				{
					MakeNodePath(s);
					uint32 enc = fSettings->GetEncoding(GetServerName(fServer), GetServerPort(fServer));
					// Use encoded file attributes?
					if (enc != 0)
					{
						MessageRef packed = DeflateMessage(mref, enc, true);
						if (packed())
						{
							refScan()->AddMessage(s, packed);
							m++;
						}
						else	
						{
							// Failed to pack the message?
							refScan()->AddMessage(s, mref);
							m++;
						}
					}
					else
					{
						refScan()->AddMessage(s, mref);
						m++;
					}
					if (m == 20)
					{
						m = 0;
						fNetClient->SendMessageToSessions(refScan, 0);
						refScan = GetMessageFromPool(PR_COMMAND_SETDATA);
					}
				}
			}
			if (refScan())
			{
				if (!refScan()->IsEmpty())
				{
					fNetClient->SendMessageToSessions(refScan, 0);
					refScan.Reset();
				}
			}
		}
		
		fFileScanThread->Unlock();
		fScanning = false;
		if (fDLWindow)
		{
			SignalDownload(WDownload::DequeueUploads);
		}
		PRINT("Done\n");
	}				
}
