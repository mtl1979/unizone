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

#include "netclient.h"
#include "privatewindowimpl.h"
#include "system/SetupSystem.h"
#include "accept.h"
#include "search.h"
#include "user.h"

#define UPDATE_SERVER "www.raasu.org"
#define UPDATE_FILE "http://www.raasu.org/tools/windows/version.txt"

#define START_OUTPUT() fPrintOutput = false; PrintText("", true)
#define END_OUTPUT() fPrintOutput = true; PrintText("", false)

#include <map>
using std::pair;
using std::multimap;
using std::iterator;

class ChannelInfo;
class WSearchListItem;
class WFileThread;
class MenuBar;
class WUniListView;
class WHTMLView;
class WComboBox;

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
MakePair(QString f, WResumeInfo u)
{
	WResumePair p;
	p.first = f;
	p.second = u;
	return p;
}

// quick inline method to generate a pair
inline
WFIPair 
MakePair(const QString s, WFileInfo * fi)
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
	void UserNameChanged(QString, QString, QString);
	void UserConnected(QString);
	void UserDisconnected(QString, QString);
	void DisconnectedFromServer();
	void UserStatusChanged(QString, QString, QString);

	// tab completion signal
	void TabPressed(QString str);

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

	void GotShown(const QString &);

	void AboutToQuit();
	void DownloadWindowClosed();

	void FileFailed(QString, QString, QString); // from WDownload
	void FileInterrupted(QString, QString, QString);

	// Channels
	void ChannelAdded(const QString, const QString, int64);
	void ChannelAdmins(const QString, const QString, const QString);
	void ChannelTopic(const QString, const QString, const QString);
	void ChannelPublic(const QString, const QString, bool);
	void ChannelOwner(const QString, const QString, const QString);
	void UserIDChanged(QString, QString);

protected:
	virtual void customEvent(QCustomEvent * event);
	virtual void resizeEvent(QResizeEvent * event);

private slots:
	void GoSearch();
	void StopSearch();
	void ClearList();
	void Download();
	void ClearHistory();

	// Search

	void AddFile(const QString, const QString, bool, MessageRef);
	void RemoveFile(const QString, const QString);

	// Channels

	void CreateChannel();
	void JoinChannel();

private:
	mutable NetClient * fNetClient;
	mutable NetClient * fServerThread;	// used to get latest servers from beshare.tycomsystems.com
	mutable NetClient * fUpdateThread;

	mutable WAcceptThread * fAccept;
	WFileThread * fFileScanThread;
	bool fFileShutdownFlag;

	MenuBar * fMenus;

	muscle::CompleteSetupSystem fMuscle;

	// gui
	QSplitter * fMainSplitter;	// splits user list and the rest of the window

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

	int64 fLoginTime;

	QString fUserName, fUserStatus, fServer;
	QString fPopupUser;
	QPopupMenu * fPrivate;		// private window popup

	WPrivMap fPrivateWindows;	// private windows;
	QMutex pLock;				// private window mutex
	QMutex rLock;				// resume list mutex

	bool fGotResults;			// see if we got initial Search Results
	bool fGotParams;			// see if the initial "Get Params" message was sent
	bool fAway;
	bool fPrintOutput;
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

	// UniShare

	void TransferCallbackRejected(QString qFrom, int64 timeLeft, uint32 port);
	
	bool IsIgnored(const WUser * user);
	bool Ignore(QString & user);
	bool UnIgnore(QString & user);

	bool IsBlackListed(const WUser * user);
	bool BlackList(QString & user);
	bool UnBlackList(QString & user);

	bool IsAutoPrivate(const WUser * user);
	bool AutoPrivate(QString & user);
	bool UnAutoPrivate(QString & user);

	void Disconnect2();					// Internal disconnection handling

	void CheckResumes(QString user);	// Check if resume list contains downloads from 'user'
	void ListResumes();					// List files waiting to be resumed

	bool Remote(String session, QString text);  // handle remote commands
	QString fRemote;							// remote password
	void ExecCommand(QString command);			// execute specified command

	bool NameSaid2(String sname, QString & msg, unsigned long index = 0); // Private version for recursing

	QTimer * fAutoAway;
	QTimer * fConnectTimer;
	QTimer * fReconnectTimer;

	WLog fMainLog;

#ifdef WIN32			// if the OS is Windows, 
	HWND fWinHandle;	// handle to our window for flashing
#endif

	void InitGUI();
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
	void PrintSystem(const QString & msg, bool batch = false);

	void NameChanged(const QString & newName);
	void StatusChanged(const QString & newStatus);
	void ServerChanged(const QString & newServer);
	void SetStatus(const QString & s);

	// stolen from BeShare :) thanx Jeremy
	static bool ParseUserTargets(QString text, WUserSearchMap & sendTo, String & setTargetStr, String & setRestOfString, NetClient * net);
	void SendPingOrMsg(QString & text, bool isping, bool * reply = NULL);
	void Action(const QString & name, const QString & msg, bool batch = false);
	void GetAddressInfo(QString user);
	
	void ShowHelp(QString command = QString::null);

	// parsing stuff...
	bool MatchUserFilter(const WUser * user, const char * filter);
	bool MatchFilter(const QString user, const char * filter);

	int MatchUserName(QString un, QString & result, const char * filter);
	bool DoTabCompletion(QString origText, QString & result, const char * filter);
	void GotUpdateCmd(const char * param, QString val);

	QString MapUsersToIDs(const QString & pattern);
	QString MapIPsToNodes(const QString & pattern);


	// see if we were named...
	bool NameSaid(QString & msg);	// msg will be syntaxed if needed
	void SetWatchPattern(QString pattern);

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
	bool CheckVersion(const char *, QString * = NULL);

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

	WResumeMap fResumeMap;

	QGridLayout * fMainBox;
    QTabWidget * fTabs;
	
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
	QMutex fLock;		// to lock the list so only one method can be using it at a time

	QString fCurrentSearchPattern;
	StringMatcher fFileRegExp, fUserRegExp;
	QString fFileRegExpStr, fUserRegExpStr;

	bool fIsRunning;

	WFIMap fFileList;

	void StartQuery(QString sidRegExp, QString fileRegExp);
	void SetResultsMessage();
	void SetSearchStatus(const QString & status, int index = 0);
	void SetSearch(QString pattern);

	void QueueDownload(QString file, WUser * user);
	void EmptyQueues();

	void Lock() { fSearchLock.lock(); }
	void Unlock() { fSearchLock.unlock(); }

	QMutex fSearchLock;		// to lock the list so only one method can be using it at a time

	// UniShare

	int64 GetRegisterTime() { return GetRegisterTime( GetUserName() ); }

	// Channels

	void ChannelCreated(const QString, const QString, int64);
	void ChannelJoin(const QString, const QString);
	void ChannelPart(const QString, const QString);
	void ChannelInvite(const QString, const QString, const QString);
	void ChannelKick(const QString, const QString, const QString);

	WChannelMap fChannels;

	void UpdateAdmins(WChannelIter iter);
	void UpdateUsers(WChannelIter iter);
	void UpdateTopic(WChannelIter iter);
	void UpdatePublic(WChannelIter iter);
	void JoinChannel(QString channel);

	bool IsOwner(QString channel, QString user);
	bool IsPublic(QString channel);
	void SetPublic(QString channel, bool pub);

	void PartChannel(QString channel, QString user = QString::null);

	bool IsAdmin(QString channel, QString user);
	void AddAdmin(QString channel, QString user);
	void RemoveAdmin(QString channel, QString user);
	QString GetAdmins(QString channel);

	void SetTopic(QString channel, QString topic);

public:
	QString GetUserName() const;
	QString GetServer() const;
	QString GetStatus() const;
	QString GetUserID() const { return fNetClient->LocalSessionID(); }
#ifdef WIN32
	HWND GetHandle() { return fWinHandle; }
#endif

	void SendChatText(QString sid, QString txt, WUserRef priv = WUserRef(NULL, NULL), bool * reply = NULL);

	static QString GetRemoteVersionString(const MessageRef);
	static void LaunchSearch(QString & pattern);		// launches a search
	void LaunchPrivate(const QString & pattern);		// launches a private window with multiple users in it
	void UpdateTransmitStats(uint64 t);
	void UpdateReceiveStats(uint64 r);

	bool IsIgnored(QString & user, bool bTransfer = false);
	bool IsIgnored(QString & user, bool bTransfer, bool bDisconnected);
	bool IsIgnoredIP(QString ip);
	bool AddIPIgnore(QString ip);
	bool RemoveIPIgnore(QString ip);

	bool IsBlackListedIP(QString & ip);
	bool IsBlackListed(QString & user);

	bool IsAutoPrivate(QString user);
	bool IsConnected(QString user);

	void SendRejectedNotification(MessageRef rej);

	// To use delayed search:
	// ----------------------
	//
	// 1. Set the pattern using SetDelayedSearchPattern(QString)
	// 2. Call Connect(QString)
	//
	void Connect(QString server);
	void SetDelayedSearchPattern(QString pattern);
	
	void TranslateStatus(QString & s);
	
	WUserRef FindUser(QString user);

	bool IsScanning() { return fScanning; }

	WSettings * fSettings;	// for use by prefs
	WDownload * fDLWindow;

	// UniShare
	int64 GetRegisterTime(QString nick); 

signals:
	void UpdatePrivateUserLists();
	void ChannelAdminsChanged(const QString, const QString);
	void NewChannelText(const QString, const QString, const QString);
};


#endif
