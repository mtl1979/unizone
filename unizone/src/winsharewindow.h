#ifndef WINSHAREWINDOW_H
#define WINSHAREWINDOW_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qmainwindow.h>
#include <qvbox.h>
#include <qmenubar.h>
#include <qmultilineedit.h>
#include <qtextview.h>
#include <qsplitter.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlistview.h>
#include <qtextbrowser.h>
#include <qthread.h>
#include <qtimer.h>
#include <qevent.h>
#include <qlayout.h>
#include <qtabwidget.h>

// #include "netclient.h"
// #include "serverclient.h"
// #include "updateclient.h"
#include "privatewindowimpl.h"
#include "qtsupport/QAcceptSocketsThread.h"
#include "regex/StringMatcher.h"
#include "support/MuscleSupport.h"
// #include "search.h"
#include "user.h"
#include "titanic.h"

using namespace muscle;

#define UPDATE_SERVER "www.raasu.org"
#define UPDATE_FILE "/tools/windows/version.txt"

// #define START_OUTPUT() fPrintOutput = false; PrintText("", true)
// #define END_OUTPUT() fPrintOutput = true; PrintText("", false)

#include <map>
using std::pair;
using std::multimap;
using std::iterator;

class WDownload;
class ChannelInfo;
class WSearchListItem;
class WFileThread;
class MenuBar;
class WUniListView;
class WHTMLView;
class WComboBox;
class NetClient;
class ServerClient;
class UpdateClient;


struct WFileInfo
{
	WUserRef fiUser;
	QString fiFilename;
	MessageRef fiRef;
	bool fiFirewalled;
	WSearchListItem * fiListItem;	// the list view item this file is tied to
};

struct WResumeInfo
{
	QString fRemoteName;
	QString fLocalName;
};

typedef pair<QString, WResumeInfo> WResumePair;
typedef multimap<QString, WResumeInfo> WResumeMap;
typedef WResumeMap::iterator WResumeIter;

typedef multimap<QString, WFileInfo *> WFIMap;
typedef pair<QString, WFileInfo *> WFIPair;
typedef WFIMap::iterator WFIIter;

typedef pair<QString, ChannelInfo *> WChannelPair;
typedef multimap<QString, ChannelInfo *> WChannelMap;
typedef WChannelMap::iterator WChannelIter;

inline 
WResumePair
MakePair(const QString & f, WResumeInfo u)
{
	WResumePair p;
	p.first = f;
	p.second = u;
	return p;
}

// quick inline method to generate a pair
inline
WFIPair 
MakePair(const QString & s, WFileInfo * fi)
{
	WFIPair p;
	p.first = s;
	p.second = fi;
	return p;
}

class WTextEvent;
class WChatText;
class WSettings;
class WStatusBar;

class WinShareWindow : public QMainWindow
{
	Q_OBJECT
public:
	WinShareWindow( QWidget* parent = 0, const char* name = 0, WFlags f = WType_TopLevel );
	virtual ~WinShareWindow();

	enum Style
	{
		Motif,
		Windows,
		Platinum,
		CDE,
		MotifPlus,
		SGI
	};

	// Events
	enum
	{
		ConnectRetry = QEvent::User + 8000,
		UpdatePrivateUsers
	};

	// Time Events
	enum
	{
		TimeRequest = 'UsTi',
		TimeReply
	};

	QString GetUserName() const;
	QString GetServer() const;
	QString GetStatus() const;
	QString GetUserID() const; 
#ifdef WIN32
	HWND GetHandle() { return fWinHandle; }
#endif

	void BeginMessageBatch();
	void EndMessageBatch();

	void SendChatText(const QString & sid, const QString & txt, WUserRef priv = WUserRef(NULL, NULL), bool * reply = NULL);

	static QString GetRemoteVersionString(const MessageRef);
	// launches a search
	static void LaunchSearch(const QString & pattern);	
	// launches a private window with multiple users in it
	void LaunchPrivate(const QString & pattern);		
	// Queue TTP transfer
	static void QueueFile(const QString & ref);			
	void QueueFileAux(const QString & ref);
	void StartQueue(const QString & session);
	void UpdateTransmitStats(uint64 t);
	void UpdateReceiveStats(uint64 r);

	bool IsIgnored(const QString & user, bool bTransfer = false);
	bool IsIgnored(const QString & user, bool bTransfer, bool bDisconnected);
	bool IsIgnoredIP(const QString & ip);
	bool AddIPIgnore(const QString & ip);
	bool RemoveIPIgnore(const QString & ip);

	bool IsBlackListedIP(const QString & ip);
	bool IsBlackListed(const QString & user);

	bool IsAutoPrivate(const QString & user);
	bool IsConnected(const QString & user);

	void SendRejectedNotification(MessageRef rej);

	// To use delayed search:
	// ----------------------
	//
	// 1. Set the pattern using SetDelayedSearchPattern(const QString &)
	// 2. Call Connect(QString)
	//
	void Connect(const QString & server);
	void SetDelayedSearchPattern(const QString & pattern);
	
	void TranslateStatus(QString & s);
	
	WUserRef FindUser(const QString & user);
	WUserRef FindUserByIPandPort(const QString & ip, uint32 port);
	int FillUserMap(const QString & filter, WUserMap & wmap);

	bool IsScanning() { return fScanning; }

	WSettings * fSettings;	// for use by prefs
	WDownload * fDLWindow;

	// UniShare
	int64 GetRegisterTime(const QString & nick) const; 

	void GotParams(bool g) { fGotParams = g; }
	bool GotParams() { return fGotParams; }

	void PrintSystem(const QString & msg, bool batch = false);
	void GotUpdateCmd(const char * param, const char * val);

public slots:
	/** File Menu **/
	void Connect();
	void Disconnect();
		/* Sep */
	void OpenSharedFolder();
	void OpenDownloadsFolder();
	void OpenLogsFolder();
		/* Sep */
	void ClearChatLog();
		/* Sep */
	void Exit();

	/** Edit Menu **/
	void Preferences();

	/*** Windows Menu ***/
	void OpenDownloads();

	/*** Help Menu ***/
	void AboutWinShare();

	// user connection/disconnection messages
	void UserNameChanged(const QString &, const QString &, const QString &);
	void UserConnected(const QString &);
	void UserDisconnected(const QString &, const QString &);
	void DisconnectedFromServer();
	void UserStatusChanged(const QString &, const QString &, const QString &);
	void UserHostName(const QString &, const QString &);

	// tab completion signal
	void TabPressed(const QString &);

	// URL's
	void URLClicked(const QString &);

	// popup menu
	void RightButtonClicked(QListViewItem *, const QPoint &, int);
	void DoubleClicked(QListViewItem *);
	void PopupActivated(int);

	// auto away timer
	void AutoAwayTimer();

	// connect timer
	void ConnectTimer();
	// reconnect timer
	void ReconnectTimer();

	void BeforeShown();
	void GotShown(const QString &);

	void AboutToQuit();
	void DownloadWindowClosed();

	void FileFailed(const QString &, const QString &, const QString &); // from WDownload
	void FileInterrupted(const QString &, const QString &, const QString &);

	// Channels
	void ChannelAdded(const QString &, const QString &, int64);
	void ChannelAdmins(const QString &, const QString &, const QString &);
	void ChannelTopic(const QString &, const QString &, const QString &);
	void ChannelPublic(const QString &, const QString &, bool);
	void ChannelOwner(const QString &, const QString &, const QString &);
	void UserIDChanged(const QString &, const QString &);

protected:
	virtual void customEvent(QCustomEvent * event);
	virtual void resizeEvent(QResizeEvent * event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void GoSearch();
	void StopSearch();
	void ClearList();
	void Download();
	void ClearHistory();

	// Search

	void AddFile(const QString &, const QString &, bool, MessageRef);
	void RemoveFile(const QString &, const QString &);

	// Channels

	void CreateChannel();
	void JoinChannel();

	// Accept Thread
	void ConnectionAccepted(SocketHolderRef socketRef);

private:
	mutable NetClient * fNetClient;
	mutable ServerClient * fServerThread;	// used to get latest servers from beshare.tycomsystems.com
	mutable UpdateClient * fUpdateThread;	// used to get latest version information from www.raasu.org

	mutable QAcceptSocketsThread * fAccept;
	WFileThread * fFileScanThread;
	bool fFileShutdownFlag;

	MenuBar * fMenus;

	// gui

	// Toolbars

	QToolBar * fTBMenu;
	QToolBar * fTBNick;
	QToolBar * fTBServer;
	QToolBar * fTBStatus;

	QHGroupBox * fUsersBox;		// frame around fUsers
	WUniListView * fUsers;

	QVGroupBox * fLeftPane;

	QComboBox * fServerList, * fUserList, * fStatusList;
	QLabel * fServerLabel, * fUserLabel, * fStatusLabel;
	QSplitter * fChatSplitter;
	WHTMLView * fChatText;
	WChatText * fInputText;

	WStatusBar * fStatusBar;

	int64 fLoginTime;

	QString fUserName, fUserStatus, fServer;
	QString fPopupUser;
	QPopupMenu * fPrivate;		// private window popup

	WPrivMap fPrivateWindows;	// private windows;
	mutable QMutex pLock;		// private window mutex
	mutable QMutex rLock;		// resume list mutex

	bool fGotResults;			// see if we got initial Search Results
	bool fGotParams;			// see if the initial "Get Params" message was sent
	bool fAway;
	// bool fPrintOutput;
	QString fOutput;
	bool fScrollDown;			// do we need to scroll the view down after an insertion?
	bool fScanning;				// Is File Scan Thread active?

	bool fDisconnect;			// true : disconnected prematurely
	bool fDisconnectFlag;		// false: no disconnects
								// true : user disconnection (for when file sharing disabled)
	int  fDisconnectCount;		// Number of connection drops (1-2 for normal, 3- for premature)

	QString fAwayMsg;
	QString fHereMsg;
	QString fWatch;				// watch pattern
	QString fIgnore;			// ignore pattern
	QString fIgnoreIP;			// ip ignore pattern
	QString fBlackList;			// blacklist pattern
	QString fAutoPriv;			// Auto-private pattern
	QString fOnConnect;			// On connect perform this command
	QString fOnConnect2;		// On connect perform this too ;)

	// transmit/receive statistics

	uint64 tx,rx;		// cumulative
	uint64 tx2,rx2;		// in the beginning of session

	// Install ID

	int64 fInstallID;

	// UniShare

	void TransferCallbackRejected(const QString &qFrom, int64 timeLeft, uint32 port);
	
	bool IsIgnored(const WUser * user);
	bool Ignore(const QString & user);
	bool UnIgnore(const QString & user);

	bool IsBlackListed(const WUser * user);
	bool BlackList(const QString & user);
	bool UnBlackList(const QString & user);

	bool IsAutoPrivate(const WUser * user);
	bool AutoPrivate(const QString & user);
	bool UnAutoPrivate(const QString & user);

	// Internal disconnection handling
	void Disconnect2();					

	// Check if resume list contains downloads from 'user'
	void CheckResumes(const QString &user);
	// List files waiting to be resumed
	void ListResumes();
	// Clear the resume list
	void ClearResumes();

	// handle remote commands
	bool Remote(const String &session, const QString &text);
	// remote password
	QString fRemote;
	// execute specified command
	void ExecCommand(const QString &command);			

	bool NameSaid2(const String &sname, QString & msg, unsigned long index = 0); // Private version for recursing

	void SignalDownload(int);

	QTimer * fAutoAway;
	QTimer * fConnectTimer;
	QTimer * fReconnectTimer;

	WLog fMainLog;

#ifdef WIN32			// if the OS is Windows, 
	HWND fWinHandle;	// handle to our window for flashing
#endif

	void InitGUI();
	void Cleanup();
	void HandleSignal();
	void SendChatText(WTextEvent *, bool * reply = NULL);

	void HandleMessage(MessageRef);
	void HandleComboEvent(WTextEvent *);

	void UpdateTextView();		// moves the stuff in the chat screen so that the latest stuff is displayed
	void UpdateUserList();

	QString MakeHumanTime(int64 time);

	void PrintText(const QString & str, bool begin);
	void PrintText(const QString & str);
	void PrintError(const QString & error, bool batch = false);
	void PrintWarning(const QString & warning, bool batch = false);

	void NameChanged(const QString & newName);
	void StatusChanged(const QString & newStatus);
	void ServerChanged(const QString & newServer);
	void SetStatus(const QString & s);

	// stolen from BeShare :) thanx Jeremy
	static bool ParseUserTargets(const QString & text, WUserSearchMap & sendTo, String & setTargetStr, String & setRestOfString, NetClient * net);
	void SendPingOrMsg(QString & text, bool isping, bool * reply = NULL);
	void Action(const QString & name, const QString & msg, bool batch = false);
	void GetAddressInfo(const QString & user);
	void PrintAddressInfo(WUserRef user);
	bool PrintAddressInfo(uint32 address);
	
	void ShowHelp(const QString & command = QString::null);

	// parsing stuff...
	bool MatchUserFilter(const WUser * user, const char * filter);
	bool MatchFilter(const QString & user, const char * filter);

	int MatchUserName(const QString & un, QString & result, const char * filter);
	bool DoTabCompletion(const QString & origText, QString & result, const char * filter);

	QString MapUsersToIDs(const QString & pattern);
	QString MapIPsToNodes(const QString & pattern);


	// see if we were named...
	bool NameSaid(QString & msg);	// msg will be syntaxed if needed
	void SetWatchPattern(const QString & pattern);

	void ServerParametersReceived(const MessageRef msg);

	void LoadSettings();
	void SaveSettings();
	void EmptyUsers();
	void SetAutoAwayTimer();
	void WaitOnFileThread();
	void CheckScrollState();

	void StartLogging();
	void StopLogging();
	void CancelShares();
	void ScanShares(bool rescan = false);
	void CreateDirectories();

	bool StartAcceptThread();
	void StopAcceptThread();

	QString GetUptimeString();
	int64 GetUptime();

	void OpenDownload();

	friend class WPrivateWindow;
	friend class Channel;
	friend class WUser;
	friend class WSearch;
	friend class WDownload;
	friend class NetClient;

	WResumeMap fResumeMap;

	QGridLayout * fMainBox;
	QTabWidget * fTabs;
	
	QWidget * fMainWidget;
	QGridLayout * fMainTab;
// splits user list and the rest of the window
	QSplitter * fMainSplitter;	

	QWidget * fSearchWidget;
	QGridLayout * fSearchTab;
	
	QWidget * fChannelsWidget;
	QGridLayout * fChannelsTab;
	QListView * ChannelList;
	QPushButton * Create;
	QPushButton * Join;
	// Search Pane

	QLabel * fSearchLabel;
	WComboBox * fSearchEdit;
	WUniListView * fSearchList;
	QPushButton * fDownload;
	QPushButton * fClear;
	QPushButton * fStop;
	QPushButton * fClearHistory;
	WStatusBar * fStatus;
	MessageRef fQueue;
	// mutable QMutex fLock;		// to lock the list so only one method can be using it at a time

	QString fCurrentSearchPattern;
	StringMatcher fFileRegExp, fUserRegExp;
	QString fFileRegExpStr, fUserRegExpStr;

	bool fIsRunning;

	WFIMap fFileList;

	void StartQuery(const QString & sidRegExp, const QString & fileRegExp);
	void SetResultsMessage();
	void SetSearchStatus(const QString & status, int index = 0);
	void SetSearch(const QString & pattern);

	void QueueDownload(const QString & file, WUser * user);
	void EmptyQueues();

	// void Lock() { fSearchLock.lock(); }
	// void Unlock() { fSearchLock.unlock(); }

	mutable QMutex fSearchLock;		// to lock the list so only one method can be using it at a time

	// UniShare

	int64 GetRegisterTime() const { return GetRegisterTime( GetUserName() ); }

	// Channels

	void ChannelCreated(const QString &, const QString &, int64);
	void ChannelJoin(const QString &, const QString &);
	void ChannelPart(const QString &, const QString &);
	void ChannelInvite(const QString &, const QString &, const QString &);
	void ChannelKick(const QString &, const QString &, const QString &);

	WChannelMap fChannels;

	void UpdateAdmins(WChannelIter iter);
	void UpdateUsers(WChannelIter iter);
	void UpdateTopic(WChannelIter iter);
	void UpdatePublic(WChannelIter iter);
	void JoinChannel(const QString &channel);

	bool IsOwner(const QString & channel, const QString & user);
	bool IsPublic(const QString & channel);
	void SetPublic(const QString & channel, bool pub);

	void PartChannel(const QString & channel, const QString & user);

	bool IsAdmin(const QString & channel, const QString & user);
	void AddAdmin(const QString & channel, const QString & user);
	void RemoveAdmin(const QString & channel, const QString & user);
	QString GetAdmins(const QString & channel);

	void SetTopic(const QString & channel, const QString & topic);

	Queue<TTPInfo *> _ttpFiles;

signals:
	void UpdatePrivateUserLists();
	void ChannelAdminsChanged(const QString &, const QString &);
	void NewChannelText(const QString &, const QString &, const QString &);
};


#endif
