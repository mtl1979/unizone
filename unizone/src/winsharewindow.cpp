#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "aboutdlgimpl.h"
#include "winsharewindow.h"
#include "version.h"
#include "debugimpl.h"
#include "chattext.h"
#include "formatting.h"
#include "textevent.h"
#include "htmlview.h"
#include "privatewindowimpl.h"
#include "wpwevent.h"
#include "combo.h"
#include "util/StringTokenizer.h"
#include "regex/StringMatcher.h"
#include "iogateway/PlainTextMessageIOGateway.h"
#include "settings.h"
#include "filethread.h"
#include "tokenizer.h"								// <postmaster@raasu.org> 20021114
#include "platform.h"
#include "nicklist.h"

#include <qapplication.h>
#include <qstylesheet.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qimage.h>
#include <qmotifstyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qcdestyle.h>
#include <qinterlacestyle.h>
#include <qmotifplusstyle.h>
#include <qsgistyle.h>
#include <qcstring.h>
#include <qtextcodec.h>
#include <qdir.h>

#ifdef WIN32
#include <objbase.h>
#elif defined(__linux__)
// uptime
#include <sys/sysinfo.h>
#endif

#define CLUMP_CHAR '\1'

WinShareWindow * gWin = NULL;

WinShareWindow::WinShareWindow(QWidget * parent, const char* name, WFlags f)
	: QMainWindow(parent, name, f | WPaintDesktop | WPaintClever), fMenus(NULL)
{
	if ( !name ) 
		setName( "WinShareWindow" );
	// IMPORTANT!! :)
	CreateDirectories();
	gWin = this;
	fDLWindow = NULL;
	fSearchWindow = NULL;
	fChannels = NULL;
	fAccept = NULL;
	
	RedirectDebugOutput();

	fUpdateThread = new NetClient(this);
	CHECK_PTR(fUpdateThread);

	fPrintOutput = false;

	fDisconnectCount = 0;	// Initialize disconnection count
	fDisconnect = false;	// No premature disconnection yet
	
	fNetClient = new NetClient(this);
	CHECK_PTR(fNetClient);

	fServerThread = new NetClient(this);
	CHECK_PTR(fServerThread);

	fSettings = new WSettings;
	CHECK_PTR(fSettings);

//	IncrementBuild();
	setCaption("Unizone");

	fMenus = new MenuBar(this);
	CHECK_PTR(fMenus);

	InitGUI();
	resize(800, 600);

	connect(fNetClient, SIGNAL(UserDisconnected(QString, QString)), this,
			SLOT(UserDisconnected(QString, QString)));
	connect(fNetClient, SIGNAL(UserConnected(QString)), this,
			SLOT(UserConnected(QString)));
	connect(fNetClient, SIGNAL(UserNameChanged(QString, QString, QString)), this,
			SLOT(UserNameChanged(QString, QString, QString)));
	connect(fNetClient, SIGNAL(DisconnectedFromServer()), this,
			SLOT(DisconnectedFromServer()));
	connect(fNetClient, SIGNAL(UserStatusChanged(QString, QString, QString)), this,
			SLOT(UserStatusChanged(QString, QString, QString)));
	connect(fInputText, SIGNAL(TabPressed(QString)), this, SLOT(TabPressed(QString)));
	connect(fChatText, SIGNAL(highlighted(const QString&)), this,
			SLOT(URLSelected(const QString&)));
	connect(fChatText, SIGNAL(URLClicked()), this, SLOT(URLClicked()));
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(AboutToQuit()));
#ifndef WIN32
	connect(fChatText, SIGNAL(GotShown(const QString &)), this, SLOT(GotShown(const QString &)));
#endif
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

	fAway = false;
	// check away status... as it is possible to close the program in away state
	if (fStatusList->currentText().stripWhiteSpace().lower() == fAwayMsg.stripWhiteSpace().lower())
		fAway = true;

	if (fSettings->GetInfo())
	{
		START_OUTPUT();
#ifdef WIN32
		PrintSystem(tr("Welcome to Unizone (English)! <b>THE</b> MUSCLE client for Windows!"), true);
#else
		PrintSystem(tr("Welcome to Unizone (English)! <b>THE</b> MUSCLE client for Linux!"), true);
#endif
		// <postmaster@raasu.org> 20030225
		PrintSystem(tr("Copyright (C) 2002-2003 Mika T. Lindqvist."), true);
		PrintSystem(tr("Original idea by Vitaliy Mikitchenko."), true);
		PrintSystem(tr("Released to public use under LGPL."), true);
		PrintSystem(tr("Type /help for a command reference."), true);
		END_OUTPUT();
	}

	if (!fAway)
		SetAutoAwayTimer();

#ifdef WIN32
	// try to find our handle
	QString title = tr("[Freeware] - Unizone");
	wchar_t * wtitle = qStringToWideChar(title);
	fWinHandle = FindWindow(NULL, wtitle); // <postmaster@raasu.org> 20021021 -- Use Unicode macro L"..."
	delete [] wtitle;

	if (fWinHandle)
	{
		PRINT("Got Handle!\n");
	}

#endif

	// setup accept thread
	if (fSettings->GetSharingEnabled())
		StartAcceptThread();


	if (fSettings->GetMaximized())
		showMaximized();
	fFileScanThread = new WFileThread(fNetClient, this, &fFileShutdownFlag);
	CHECK_PTR(fFileScanThread);

	fFileShutdownFlag = false;
	fScrollDown = true;

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
	fAccept = new WAcceptThread(this);
	CHECK_PTR(fAccept);
	for (uint16 i = DEFAULT_LISTEN_PORT; i <= DEFAULT_LISTEN_PORT + LISTEN_PORT_RANGE; i++)
	{
		// dynamically allocate a port if we can't get one of ours
		if (fAccept->SetPort(i) == B_OK)
		{
			if (fAccept->StartInternalThread() == B_OK)
			{
				if (fSettings->GetInfo())
				{
					START_OUTPUT();
					PrintSystem(tr("Accept thread ready and listening on port %1.").arg(fAccept->GetPort()), true);
					END_OUTPUT();
				}

				fNetClient->SetPort(fAccept->GetPort());	// let the net client know our port
				fNetClient->SetUserName(fUserName);			// BUG FIX: Port not being set in
															// certain occasions

				return true;
			}
			else
			{
				START_OUTPUT();
				PrintError(tr("Failed to start accept thread!"), true);
				END_OUTPUT();
				PRINT("Failed to start accept thread\n");
				fSettings->SetSharingEnabled(false);
			}
		}
		else
			PRINT("Failed on port %d\n", i);
	}
	StopAcceptThread();
	return false;
}

void
WinShareWindow::StopAcceptThread()
{
	if (!fAccept)
		return;
	fAccept->ShutdownInternalThread();
	fAccept->WaitForInternalThreadToExit();
	delete fAccept;
	fAccept = NULL;
}

WinShareWindow::~WinShareWindow()
{
	// all the NetClients get deleted by Qt
	// since they are QObject's
	fUpdateThread->Reset();
	fServerThread->Reset();
	fNetClient->Reset();
	fNetClient = NULL;

	fServerThread = NULL;
	fUpdateThread = NULL;

	delete fMenus;
	fMenus = NULL;

	PRINT("Saving settings\n");
	SaveSettings();
	delete fSettings;
	fSettings = NULL; // <postmaster@raasu.org> 20021027

	WaitOnFileThread();
	delete fFileScanThread;
	fFileScanThread = NULL; // <postmaster@raasu.org> 20021027

	CleanupDebug();

	StopLogging();

	QApplication::exit(0);
}


void
WinShareWindow::customEvent(QCustomEvent * event)
{
	PRINT("\tWinShareWindow::customEvent\n");
	if (!fNetClient)		// do this to avoid bad crash
	{
		return;
	}
	else
	{
		switch (event->type())
		{
		case WinShareWindow::ConnectRetry:
			{
				PRINT("\tWinShareWindow::ConnectRetry\n");
				if (!fNetClient->IsInternalThreadRunning() && !fReconnectTimer->isActive())
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
				CancelShares();
				if (fSettings->GetSharingEnabled()) // Make sure sharing is enabled and fully functional
				{
					fFileScanThread->Lock();
					if (fSettings->GetInfo())
						PrintSystem(tr("Sharing %1 file(s).").arg(fFileScanThread->GetSharedFiles().size()));
					fNetClient->SetFileCount(fFileScanThread->GetSharedFiles().size());
					PRINT("Doing a scan of the returned files for uploading.\n");
					for (WMsgListIter it = fFileScanThread->GetSharedFiles().begin(); it != fFileScanThread->GetSharedFiles().end(); it++)
					{
						// stop iterating if we are waiting for file scan thread to finish
						if (fFileShutdownFlag)
							break;
						qApp->processEvents(300);
						MessageRef mref = (*it); 
						String s;
						if (mref()->FindString("secret:NodePath", s) == B_OK)
							fNetClient->SetNodeValue(s.Cstr(), mref);
					}
					fFileScanThread->Unlock();
					PRINT("Done\n");
				}
				return;
			}
			
		case WAcceptThreadEvent::Type:
			{
				PRINT("\tWAcceptThreadEvent::Type\n");
				WAcceptThreadEvent * te = dynamic_cast<WAcceptThreadEvent *>(event);
				if (te)
				{
					SocketHolderRef ref = te->Get();
					int socket = ref() ? (ref()->ReleaseSocket()) : -1;
					uint32 ip;
					if (socket >= 0 && (ip = GetPeerIPAddress(socket)) > 0)
					{
						OpenDownload();
						fDLWindow->AddUpload(socket, ip, false);
					}
				}
				return;
			}
			
		case NetClient::SignalEvent:
			{
				PRINT("SignalEvent\n");
				HandleSignal();
				return;
			}
		case NetClient::SessionAttached:
			{
				PRINT("Received SessionAttached message\n");
				return;
			}
		case NetClient::SessionConnected:
			{
				PRINT("Received SessionConnected message\n");

				// Set Message Encoding
				fNetClient->SetOutgoingMessageEncoding( fSettings->GetEncoding(GetServerName(fServer), GetServerPort(fServer)) );

				PRINT("Uploading public data\n");
				fGotParams = false; // set to false here :)
				// send a message out to the server asking for our parameters
				MessageRef askref(GetMessageFromPool(PR_COMMAND_GETPARAMETERS));
				fNetClient->SendMessageToSessions(askref);
				// get a list of users as well
				fNetClient->AddSubscription("SUBSCRIBE:beshare/*"); // all user info :)
				fNetClient->AddSubscription("SUBSCRIBE:unishare/*");
				fNetClient->AddSubscription("SUBSCRIBE:unishare/channeldata/*");
				fNetClient->SetUserName(GetUserName());
				fNetClient->SetUserStatus(GetStatus());
				fNetClient->SetConnection(fSettings->GetConnection());
				fNetClient->SetFileCount(0);
				if (fSettings->GetSharingEnabled())
					fNetClient->SetLoad(0, fSettings->GetMaxUploads());
				
				
				fNetClient->SendMessageToSessions(GetMessageFromPool(PR_COMMAND_PING));
				
				if (fSettings->GetInfo())
					PrintSystem(tr("Connected."));

				if (fReconnectTimer->isActive())
				{
					PrintSystem(tr("Reconnect timer stopped"));
					fReconnectTimer->stop();
				}

				fDisconnect = false;
				fDisconnectCount = 0;
				return;
			}
			
		case NetClient::Disconnected:
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
					if ( IsConnected( wte->Text() ) )
						CheckResumes( wte->Text() );
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

					wchar_t * wText = qStringToWideChar(wpe->GetText());
					PRINT("wpe->GetText() %S\n", wText);
					delete [] wText;

					te.SetText(wpe->GetText());
					if (wpe->GetWantReply())	// reply wanted... do the following...
					{
						bool rep = false;
						wText = qStringToWideChar(te.Text());
						PRINT("Sending the following text to SendChatText %S\n", wText);
						delete [] wText;

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
	}
}

void
WinShareWindow::StatusChanged(const QString & newStatus)
{
	fNetClient->SetUserStatus(newStatus); // <postmaster@raasu.org> 20021001
	fUserStatus = newStatus;
	QString pstatus = fUserStatus;
	EscapeHTML(pstatus);

	// <postmaster@raasu.org> 20020929,20030211
	TranslateStatus(pstatus);
	//

	PrintSystem(WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(
				WFormat::StatusChanged().arg(FixStringStr(pstatus))));
}

void
WinShareWindow::ServerChanged(const QString & newServer)
{
	fServer = newServer;
	if (fServer.right(5) == ":2960")
		fServer = fServer.left(fServer.length()-5); // strip default port
	if (fNetClient->IsInternalThreadRunning())
		Connect();
}

void
WinShareWindow::NameChanged(const QString & newName)
{
	fNetClient->SetUserName(newName); // <postmaster@raasu.org> 20021011
	QString nameChanged = WFormat::SystemText().arg(WColors::System).arg(fSettings->GetFontSize());
	nameChanged += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(tr("Name changed to <font color=\"%2\">%1</font>.").
		arg(FixStringStr(newName)).arg(WColors::LocalName)); // <postmaster@raasu.org> 20021001
	PrintText(nameChanged);
	fUserName = newName;
}

void
WinShareWindow::resizeEvent(QResizeEvent * event)
{
	if (fMenus)
	{
		if (fInfoPane)
			fMainSplitter->resize(event->size().width(), event->size().height() - fMenus->height() - fInfoPane->height());
		else
			fMainSplitter->resize(event->size().width(), event->size().height() - fMenus->height());
	}
	if (fInfoPane)
		fInfoPane->resize(event->size().width(), 48);
}

void
WinShareWindow::InitGUI()
{
	// divide our splitter(s)
	QValueList<int> splitList;
	splitList.append(4);
	splitList.append(1);

	// setup the info box
	fInfoPane = new QHGroupBox(this);
	CHECK_PTR(fInfoPane);

	fInfoPane->move(0, fMenus->height() + 4);
	fInfoPane->resize(this->frameSize().width(),48);

	// setup combo/labels
	// we define the combos as QComboBox, but use WComboBox for 
	// messaging purposes :)
	fServerLabel = new QLabel(tr("Server:"), fInfoPane);
	CHECK_PTR(fServerLabel);
	fServerList = new WComboBox(this, fInfoPane, "fServerList");
	CHECK_PTR(fServerList);
	fServerList->setEditable(true);
	fServerLabel->setBuddy(fServerList);

	fUserLabel = new QLabel(tr("Nick:"), fInfoPane);
	CHECK_PTR(fUserLabel);
	fUserList = new WComboBox(this, fInfoPane, "fUserList");
	CHECK_PTR(fUserList);
	fUserList->setEditable(true);
	fUserLabel->setBuddy(fUserList);

	fStatusLabel = new QLabel(tr("Status:"), fInfoPane);
	CHECK_PTR(fStatusLabel);
	fStatusList = new WComboBox(this, fInfoPane, "fStatusList");
	CHECK_PTR(fStatusList);
	fStatusList->setEditable(true);
	fStatusLabel->setBuddy(fStatusList);

	// main splitter
	fMainSplitter = new QSplitter(this);
	CHECK_PTR(fMainSplitter);
	fMainSplitter->move(0, fMenus->height() + fInfoPane->height() + 4);

	// user list
	fUsersBox = new QHGroupBox(fMainSplitter);
	CHECK_PTR(fUsersBox);
	fUsersBox->move(0, fMenus->height() + fInfoPane->height() + 4);
	fUsers = new WUniListView(fUsersBox);
	CHECK_PTR(fUsers);
	// initialize the list view
	fUsers->addColumn(tr("Name"));
	fUsers->addColumn(tr("ID"));
	fUsers->addColumn(tr("Status"));
	fUsers->addColumn(tr("Files"));
	fUsers->addColumn(tr("Connection"));
	fUsers->addColumn(tr("Load"));
	fUsers->addColumn(tr("Client"));		// as of now... WinShare specific, WinShare pings all the users and parses the string for client info
	

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

	// chat widgets
	// chat splitter first though...
	fChatSplitter = new QSplitter(fLeftPane);
	CHECK_PTR(fChatSplitter);
	fChatSplitter->setOrientation(QSplitter::Vertical);

	fChatText = new WHTMLView(fChatSplitter);
	CHECK_PTR(fChatText);
	fChatText->setTextFormat(QTextView::RichText);

	// fInputText is a QMultiLineEdit, but we create
	// a WChatText for more advanced handling
	fInputText = new WChatText(this, fChatSplitter);
	CHECK_PTR(fInputText);
	fInputText->setWordWrap(QMultiLineEdit::WidgetWidth);
	fChatSplitter->setSizes(splitList);

	// setup some defaults
	// <postmaster@raasu.org> 20020924
	fUserList->insertItem("Unizone Binky");
	fServerList->insertItem("beshare.tycomsystems.com");
#ifdef BETA
	QString status = tr("Testing Unizone (English)");
	status += " ";
	status += WinShareVersionString();
	fStatusList->insertItem(status);
#endif
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

	// setup autoaway timer
	fAutoAway = new QTimer;
	CHECK_PTR(fAutoAway);
	connect(fAutoAway, SIGNAL(timeout()), this, SLOT(AutoAwayTimer()));

	// setup autoconnect timer
	fReconnectTimer = new QTimer;
	CHECK_PTR(fReconnectTimer);
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

void
WinShareWindow::UpdateTextView()
{
	// <postmaster@raasu.org> 20021021 -- Fixed too long line in debug output
	PRINT("UPDATETEXTVIEW: ContentsX = %d, ContentsY = %d\n", fChatText->contentsX(),		fChatText->contentsY());
	PRINT("              : ContentsW = %d, ContentsH = %d\n", fChatText->contentsWidth(),	fChatText->contentsHeight());
	if (fScrollDown)
		fChatText->setContentsPos(0, fChatText->contentsHeight());
#ifndef WIN32	// linux only...
	fChatText->repaintContents(fChatText->contentsX(), fChatText->contentsY(),
					fChatText->contentsWidth(), fChatText->contentsHeight(),
					false);
#endif
}

void
WinShareWindow::UpdateUserList()
{
	WUserMap & users = fNetClient->Users();

	for (WUserIter it = users.begin(); it != users.end(); it++)
		(*it).second()->AddToListView(fUsers);
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
		s = s.left(s.length()-2);
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

void
WinShareWindow::PrintText(const QString & str, bool begin)
{
	static QString output = "";
	static bool first = false;
	if (begin)	// starting message batch
	{
#ifndef WIN32
		output = "\t";	// reset (we always start with a tab..., this is a linux bug workaround)
#else
		output = "";
#endif
		first = true;
	}
	else
	{
		if (!fPrintOutput)	// just append
		{
			if (!first)
				output += "<br>";	// add a new line
			else
				first = false;

			if (fSettings->GetTimeStamps())
				output += GetTimeStamp();

			output += str;
		}
		else
		{
#ifndef WIN32
			if (output != "\t")	// do we have something?
#else
			if (output != "")
#endif
			{
				CheckScrollState();
				fChatText->append(output);
				fMainLog.LogString(output);
				UpdateTextView();
			}
		}
	}
}

// non-batch version of PrintText
void
WinShareWindow::PrintText(const QString & str)
{
	CheckScrollState();
	QString out("");
	if (fSettings->GetTimeStamps())
		out = GetTimeStamp();
	out += str;
	fChatText->append(
#ifndef WIN32
						'\t' +
#endif
						out
	);
	fMainLog.LogString(out);
	UpdateTextView();
}

void
WinShareWindow::CheckScrollState()
{
	QScrollBar * scroll = fChatText->verticalScrollBar();
	PRINT("CHECKSCROLLSTATE: value = %d, maxValue = %d, minValue = %d\n", scroll->value(), scroll->maxValue(), scroll->minValue());
	if (scroll->value() >= scroll->maxValue())
		fScrollDown = true;
	else
		fScrollDown = false;
}

bool
WinShareWindow::ParseUserTargets(QString text, WUserSearchMap & sendTo, String & setTargetStr, String & setRestOfString, NetClient * net)
{
	StringTokenizer wholeStringTok((const char *) text.utf8(), " ");
	String restOfString2(wholeStringTok.GetRemainderOfString());
	restOfString2.Replace(CLUMP_CHAR, ' ');
	const char * w2 = wholeStringTok.GetNextToken();
	if (w2)
	{
		setTargetStr = w2;
		setTargetStr.Replace(CLUMP_CHAR, ' ');
		w2 = setTargetStr.Cstr();

		setRestOfString = wholeStringTok.GetRemainderOfString();

		StringTokenizer tok(w2, ",");
		Queue<String> clauses;
		const char * next;
		while((next = tok.GetNextToken()) != NULL)
			clauses.AddTail(String(next).Trim());
	
		// find users by session id
		for (int i = clauses.GetNumItems() - 1; i >= 0; i--)
		{
			WUserRef user;
			PRINT("Checking for user %s\n", clauses[i].Cstr());
			if ((user = net->FindUser(QString::fromUtf8(clauses[i].Cstr())))() != NULL)
			{
				WUserSearchPair pair = MakePair(QString::fromUtf8(clauses[i].Cstr()), user, QString::fromUtf8(setRestOfString.Cstr())); // <postmaster@raasu.org> 20021007
				sendTo.insert(pair);
				clauses.RemoveItemAt(i);
			}
		}

		PRINT("Checking using usernames\n");
		for (int j = clauses.GetNumItems() - 1; j >= 0; j--)
		{
			String tstr(clauses[j].Cstr());
			tstr.Trim();
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
				String userName = String((const char *) user()->GetUserName().utf8()).Trim();
				userName = StripURL(userName);

				if (userName.Length() > 0 && restOfString2.StartsWith(userName))
				{
					PRINT("Found\n");
					WUserSearchPair pair = MakePair(user()->GetUserID(), user,
													QString::fromUtf8(restOfString2.Substring(strlen((const char *) user()->GetUserName().utf8())).Trim().Cstr())); // <postmaster@raasu.org> 20021007
					sendTo.insert(pair);
					setTargetStr = (const char *) user()->GetUserName().utf8();
				}
				iter++;
			}
		}
		return true;
	}
	return false;
}

void
WinShareWindow::Action(const QString & name, const QString & msg, bool batch)
{
	QString chat = WFormat::Action().arg(WColors::Action).arg(fSettings->GetFontSize());
	QString nameText = FixStringStr(msg);
	if (NameSaid(nameText))
		QApplication::beep();
	chat += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(tr("%1 %2").arg(FixStringStr(name)).arg(nameText));
	if (batch)
	{
		PrintText(chat, false);
	}
	else
	{
		CheckScrollState();
		PrintText(chat);
	}
}

void
WinShareWindow::PrintError(const QString & error, bool batch)
{
	if (fSettings->GetError())
	{
		QString e = WFormat::Error().arg(WColors::Error).arg(fSettings->GetFontSize());
		e += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(fSettings->GetFontSize()).arg(error);
		if (batch)
		{
			PrintText(e, false);
		}
		else
		{
			CheckScrollState();
			PrintText(e);
		}
	}
}

void
WinShareWindow::PrintWarning(const QString & warning, bool batch)
{
	if (fSettings->GetError())
	{
		QString e = WFormat::Warning().arg(WColors::Error).arg(fSettings->GetFontSize());
		e += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(fSettings->GetFontSize()).arg(warning);
		if (batch)
		{
			PrintText(e, false);
		}
		else
		{
			CheckScrollState();
			PrintText(e);
		}
	}
}

void
WinShareWindow::PrintSystem(const QString & msg, bool batch)
{
	QString s = WFormat::SystemText().arg(WColors::System).arg(fSettings->GetFontSize());
	s += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(ParseChatText((QString &)msg));
	if (batch)
		PrintText(s, false);
	else
		PrintText(s);
}

QString
WinShareWindow::GetRemoteVersionString(const Message * msg)
{

	QString versionString = "?";
	const char * version;

	if (msg->FindString("version", &version) == B_OK)
	{
		if (version[0] > '0' && version[0] <= '9')
		{
			versionString = "BeShare v";
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

		int i;
		int size;
		QString str;
		// load servers
		for (i = 0; (str = fSettings->GetServerItem(i)) != QString::null; i++)
			fServerList->insertItem(str, i);
		fServerList->setCurrentItem(fSettings->GetCurrentServerItem());
		fServer = fServerList->currentText();
		
		// load usernames
		for (i = 0; (str = fSettings->GetUserItem(i)) != QString::null; i++)
			fUserList->insertItem(str, i);
		fUserList->setCurrentItem(fSettings->GetCurrentUserItem());
		fUserName = fUserList->currentText();


		// load status
		for (i = 0; (str = fSettings->GetStatusItem(i)) != QString::null; i++)
			fStatusList->insertItem(str, i);
		fStatusList->setCurrentItem(fSettings->GetCurrentStatusItem());
		fUserStatus = fStatusList->currentText();

		// load the style
		switch (fSettings->GetStyle())
		{
			case Motif:
				qApp->setStyle(new QMotifStyle);
				break;
			case Windows:
				qApp->setStyle(new QWindowsStyle);
				break;
			case Platinum:
				qApp->setStyle(new QPlatinumStyle);
				break;
			case CDE:
				qApp->setStyle(new QCDEStyle);
				break;
			case MotifPlus:
				qApp->setStyle(new QMotifPlusStyle);
				break;
			case SGI:
				qApp->setStyle(new QSGIStyle);
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

		wchar_t * wAwayMsg = qStringToWideChar(fAwayMsg);
		PRINT("Away Msg: %S\n", wAwayMsg);
		delete [] wAwayMsg;
		
		fHereMsg = fSettings->GetHereMsg();

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

		fWatch = fSettings->GetWatchPattern();
		fIgnore = fSettings->GetIgnorePattern();
		fIgnoreIP = fSettings->GetIPIgnorePattern();
		fBlackList = fSettings->GetBlackListPattern();
		fAutoPriv = fSettings->GetAutoPrivatePattern();

		fOnConnect = fSettings->GetOnConnect();
		fOnConnect2 = fSettings->GetOnConnect2();

		tx = fSettings->GetTransmitStats(); tx2 = tx;
		rx = fSettings->GetReceiveStats(); rx2 = rx;
		
		fUsers->setSorting(fSettings->GetNickListSortColumn(), fSettings->GetNickListSortAscending());

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

		// UniShare

		fRegistered = fSettings->GetRegisterTime();
	}
	else	// file doesn't exist, or error
	{
		qApp->setStyle(
#ifdef WIN32
						new QWindowsStyle
#else
						new QMotifStyle
#endif	
						);

		fAwayMsg = "away";
		fHereMsg = "here";
		fWatch = "";
		fIgnore = "";
		fIgnoreIP = "";
		fBlackList = "";
		fAutoPriv = "";
		fOnConnect = "";
		fOnConnect2 = "";
		fRegistered = GetCurrentTime64();
		tx = 0;
		rx = 0;
		tx2 = 0;
		rx2 = 0;
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
	// don't worry about the color list, prefs does it
	
	// save server list
	int i;
	wchar_t * wServer;
	for (i = 0; i < fServerList->count(); i++)
	{
		fSettings->AddServerItem(fServerList->text(i));
		wServer = qStringToWideChar(fServerList->text(i));
		PRINT("Saved server %S\n", wServer);
		delete [] wServer;
	}
	fSettings->SetCurrentServerItem(fServerList->currentItem());
	
	// save user list
	wchar_t * wUser;
	for (i = 0; i < fUserList->count(); i++)
	{
		fSettings->AddUserItem(fUserList->text(i));
		wUser = qStringToWideChar(fUserList->text(i));
		PRINT("Saved user %S\n", wUser);
		delete [] wUser;
	}
	fSettings->SetCurrentUserItem(fUserList->currentItem());
	
	// save status list
	wchar_t * wStatus;
	for (i = 0; i < fStatusList->count(); i++)
	{
		fSettings->AddStatusItem(fStatusList->text(i));
		wStatus = qStringToWideChar(fStatusList->text(i));
		PRINT("Saved status %S\n", wStatus);
		delete wStatus;
	}
	fSettings->SetCurrentStatusItem(fStatusList->currentItem());
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

	fSettings->SetRegisterTime(fRegistered);

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
WinShareWindow::WaitOnFileThread()
{
	fFileShutdownFlag = true;
	if (fFileScanThread->IsRunning())
	{
		PrintSystem(tr("Waiting for file scan thread to finish..."),false);
		while (fFileScanThread->IsRunning()) 
		{
			//fFileScanThread->Lock();
			//fFileScanThread->wait();
			//fFileScanThread->Unlock();
			qApp->processEvents(300);
		}
	}
}

void
WinShareWindow::LaunchSearch(QString & pattern)
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
			qServer = qServer.left(sp);				// get everything before /
			gWin->SetDelayedSearchPattern(qPattern);
			gWin->Connect(qServer);
			return;
		}
		// fall through
		pattern = pattern.mid(2);
	}
	// (be)share:pattern
	if (!gWin->fSearchWindow)
	{
		gWin->fSearchWindow = new WSearch(gWin->fNetClient, NULL);
		CHECK_PTR(gWin->fSearchWindow);
		gWin->fSearchWindow->show();
		connect(gWin->fSearchWindow, SIGNAL(Closed()), gWin, SLOT(SearchWindowClosed()));
	}
	gWin->fSearchWindow->SetSearch(pattern);
}

QString
WinShareWindow::MapIPsToNodes(const QString & pattern)
{
	QString qResult = "";
	QString qItem;
	QStringTokenizer qTok(qResult, ",");
	while ((qItem = qTok.GetNextToken()) != QString::null)
	{
		qResult += "/" + qItem + "/*,";
	}
	if (qResult.right(1) == ",") 
	{
		qResult = qResult.left(qResult.length()-1);
	}

	wchar_t * wResult = qStringToWideChar(qResult);
	PRINT("MapIPsToNodes: %S\n", wResult);
	delete [] wResult;

	return qResult;
}

QString 
WinShareWindow::MapUsersToIDs(const QString & pattern)
{
	QString qResult = "";
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
				for (int i = 0; i < qItem.length(); i++)
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
		qResult = qResult.left(qResult.length()-1);
	}

	wchar_t * wResult = qStringToWideChar(qResult);
	PRINT("MapUsersToIDs: %S\n", wResult);
	delete [] wResult;

	return qResult;
}

// <postmaster@raasu.org> 20021013 -- Opens new Private Window and adds users in 'pattern' to that window
// <postmaster@raasu.org> 20021114 -- Uses QStringTokenizer ;)

void
WinShareWindow::LaunchPrivate(const QString & pattern)
{
	QString users = pattern;
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
			for (int i = 0; i < qItem.length(); i++)
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
	// as i just iterate through the list
	WPrivPair p = MakePair(window);
	pLock.lock();
	gWin->fPrivateWindows.insert(p);
	pLock.unlock();
}

void
WinShareWindow::StartLogging()
{
	fMainLog.Create();
	if (!fMainLog.InitCheck())
	{
		if (fSettings->GetError())
            PrintError("Failed to create log file.");
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
	delData()->AddString(PR_NAME_KEYS, "beshare/fi*");
	fNetClient->SendMessageToSessions(delData);
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
#elif defined(__linux__)
	struct sysinfo sinfo;
	sysinfo(&sinfo);
	return sinfo.uptime * 1000 * 1000;
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

void
WinShareWindow::TranslateStatus(QString & s)
{
		if (s == "here")
		{
			s = tr("here");
		}
		else if (s == "away")
		{
			s = tr("away");
		}
		else if (s == "idle")
		{
			s = tr("idle");
		}
		else if (s == "busy")
		{
			s = tr("busy");
		}
		else if (s == "at work")
		{
			s = tr("at work");
		}
		else if (s == "around")
		{
			s = tr("around");
		}
		else if (s == "sleeping")
		{
			s = tr("sleeping");
		}
}

void 
WinShareWindow::OpenDownload()
{
	if (!fDLWindow)
	{
		PRINT("New DL Window\n");
		fDLWindow = new WDownload(NULL, fNetClient->LocalSessionID(), fFileScanThread);
		CHECK_PTR(fDLWindow);
		
		connect(fDLWindow, SIGNAL(FileFailed(QString, QString)), this, SLOT(FileFailed(QString, QString)));
		connect(fDLWindow, SIGNAL(FileInterrupted(QString, QString)), this, SLOT(FileInterrupted(QString, QString)));
		connect(fDLWindow, SIGNAL(Closed()), this, SLOT(DownloadWindowClosed()));
	}
}

void
WinShareWindow::OpenDownloads()
{
	OpenDownload();
	fDLWindow->show();
}

void
WinShareWindow::OpenChannels()
{
	if (!fChannels)
	{
		fChannels = new Channels(this, fNetClient);
		CHECK_PTR(fChannels);

		connect(fChannels, SIGNAL(Closed()), gWin, SLOT(ChannelsWindowClosed()));
	}
	fChannels->show();
}

void
WinShareWindow::SetDelayedSearchPattern(QString pattern)
{
	if ((fOnConnect != QString::null) && (fOnConnect.length() > 2))
	{
		fOnConnect2 = fOnConnect;
	}
		
	fOnConnect = tr("/search %1").arg(pattern);
}

void
WinShareWindow::ScanShares(bool rescan)
{
	// are we connected?
	if (!fNetClient->IsInternalThreadRunning())	
	{
		if (fSettings->GetError())
		{
			PrintError(tr("Not connected."), false);
		}
		return;
	}

	// is sharing enabled?
	if (!fSettings->GetSharingEnabled()) 
	{
		if (fSettings->GetError())
		{
			PrintError(tr("File sharing not enabled."), false);
		}
		return;
	}

	// already running?
	if (fFileScanThread->IsRunning())
	{
		if (fSettings->GetError())
		{
			PrintError(tr("Already scanning!"), false);
		}
		return;
	}

	if (fSettings->GetInfo())
	{
		if (rescan)
		{
			PrintSystem(tr("Rescanning shared files..."));
		}
		else
		{
			PrintSystem(tr("Scanning shares..."));
		}
	}

	fFileScanThread->SetFirewalled(fSettings->GetFirewalled());
	PRINT("Starting...\n");
	fFileScanThread->start();
}
