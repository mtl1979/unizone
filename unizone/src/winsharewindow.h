#ifndef WINSHAREWINDOW_H
#define WINSHAREWINDOW_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
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
#include <qtimer.h>
#include <qevent.h>
#include <qlayout.h>
#include <qtabwidget.h>

#include "support/MuscleSupport.h"
#include "qtsupport/QAcceptSocketsThread.h"

#include "privatewindowimpl.h"
#include "user.h"
#include "chatwindow.h"
#include "search.h"
#include "channels.h"
#include "titanic.h"

using namespace muscle;

#define UPDATE_SERVER "www.raasu.org"
#define UPDATE_FILE "/tools/windows/version.txt"

class WDownload;
class WUpload;
class ChannelInfo;
class WSearchListItem;
class WFileThread;
class WListThread;
class MenuBar;
class WUniListView;
class WHTMLView;
class WComboBox;
class NetClient;
class ServerClient;
class UpdateClient;
class WPicViewer;
class QRegExp;
class DownloadQueue;
class ResolverThread;

struct WResumeInfo
{
	QString fRemoteName;
	QString fLocalName;
	QString fPath;
};

typedef struct WResumePair
{
	QString user;
	Queue<WResumeInfo> files;
} WResumePair;

typedef Queue<WResumePair> WResumeMap;

class WTextEvent;
class WChatText;
class WSettings;
class WStatusBar;

class WinShareWindow : public QMainWindow, public ChatWindow
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
		SGI,
		Mac
	};

	// Events
	enum
	{
		ConnectRetry = QEvent::User + 8000
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
	QString GetLocalIP() const;

	void SendChatText(const QString & sid, const QString & txt);
	void SendChatText(const QString & sid, const QString & txt, const WUserRef & priv, bool * reply, bool enc);

	QString GetRemoteVersionString(MessageRef);
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

	bool IsWhiteListed(const QString & user);

	bool IsAutoPrivate(const QString & user);
	bool IsConnected(const QString & user);

	void SendRejectedNotification(const MessageRef &rej);

	// To use delayed search:
	// ----------------------
	//
	// 1. Set the pattern using SetDelayedSearchPattern(const QString &)
	// 2. Call Connect(const QString &)
	//
	void Connect(const QString & server);
	void SetDelayedSearchPattern(const QString & pattern);

	QString CurrentServer() { return fServer; }
	
	QString TranslateStatus(const QString & s);
	
	WUserRef FindUser(const QString & user);
	WUserRef FindUserByIPandPort(const QString & ip, uint32 port);
	int FillUserMap(const QString & filter, WUserMap & wmap);

	bool IsScanning() { return fScanning; }

	WSettings * fSettings;	// for use by prefs
	WDownload * fDLWindow;
	WUpload * fULWindow;
	WSearch * fSearch;
	Channels * fChannels;

	// UniShare
	int64 GetRegisterTime(const QString & nick) const; 

	void GotParams(const MessageRef &);
	bool GotParams() { return fGotParams; }

	void GotUpdateCmd(const QString & param, const QString & val);

	void LogString(const char *);
	void LogString(const QString &);
	QWidget *Window();

	void SendTextEvent(const QString &text, WTextEvent::Type t);
	void SendSystemEvent(const QString &message);
	void SendErrorEvent(const QString &message);
	void SendWarningEvent(const QString &message);

	void SendPicture(const QString &target, const QString &file);
	bool FindSharedFile(QString &file);

public slots:
	/** File Menu **/
	void Connect();
	void Disconnect();
		/* Sep */
	void OpenSharedFolder();
	void OpenDownloadsFolder();
	void OpenLogsFolder();
		/* Sep */
	void OpenSearch();
		/* Sep */
	void Exit();

	/** Search... Menu **/
	void SearchMusic();
	void SearchVideos();
	void SearchPictures();
	void SearchImages();

	/** Edit Menu **/
	void ClearChatLog();
		/* Sep */
	void Preferences();

	/*** Windows Menu ***/
	void OpenChannels();
	void OpenDownloads();
	void OpenUploads();
	void OpenViewer();

	/*** Help Menu ***/
	void AboutWinShare();

	// user connection/disconnection messages
	void UserNameChanged(const WUserRef &, const QString &, const QString &);
	void UserConnected(const WUserRef &);
	void UserDisconnected(const WUserRef &);
	void DisconnectedFromServer();
	void UserStatusChanged(const WUserRef &, const QString &, const QString &);
	void UserHostName(const WUserRef &, const QString &);

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

	void AboutToQuit();
	void DownloadWindowClosed();
	void UploadWindowClosed();

	void FileFailed(const QString &, const QString &, const QString &, const QString &); // from WDownload
	void FileInterrupted(const QString &, const QString &, const QString &, const QString &);

protected:
	virtual void customEvent(QCustomEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void timerEvent(QTimerEvent *);

private slots:

	// Accept Thread
	void ConnectionAccepted(const SocketRef &);

private:
	friend class WDownload;
	friend class WUpload;
	friend class ResolverThread;

	mutable NetClient * fNetClient;
	mutable ServerClient * fServerThread;	// used to get latest servers from www.raasu.org
	mutable UpdateClient * fUpdateThread;	// used to get latest version information from www.raasu.org

	mutable QAcceptSocketsThread * fAccept;
	WFileThread * fFileScanThread;
	WListThread * fListThread;
	ResolverThread * fResolverThread;
	bool fFilesScanned;
	bool fFileShutdownFlag;
	bool fResumeEnabled;

	MenuBar * fMenus;

	// gui

	// Toolbars

#ifndef __APPLE__
	QToolBar * fTBMenu;
#endif
	QToolBar * fTBNick;
	QToolBar * fTBServer;
	QToolBar * fTBStatus;

	QHGroupBox * fUsersBox;		// frame around fUsers
	WUniListView * fUsers;

	QVGroupBox * fLeftPane;

	QComboBox * fServerList, * fUserList, * fStatusList;
	QLabel * fServerLabel, * fUserLabel, * fStatusLabel;
	QSplitter * fChatSplitter;
	WChatText * fInputText;

	WStatusBar * fStatusBar;

	QString fUserName, fUserStatus, fServer;
	QString fPopupUser;
	QPopupMenu * fPrivate;		// private window popup

	Queue<WPrivateWindow *> fPrivateWindows;	// private windows;
	mutable Mutex pLock;		// private window mutex
	mutable Mutex rLock;		// resume list mutex

	mutable Mutex fTTPLock;	// TTP list mutex

	bool fGotParams;			// see if the initial "Get Params" message was sent
	bool fAway;
	bool fScanning;				// Is File Scan Thread active?

	int  timerID;               // ID of timer to update timestamps

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
	QString fWhiteList;			// whilelist pattern
	QString fFilterList;		// wordfilter pattern
	QString fAutoPriv;			// Auto-private pattern
	QString fPMRedirect;		// Private Message redirection
	QString fOnConnect;			// On connect perform this command
	QString fOnConnect2;		// On connect perform this too ;)

	// transmit/receive statistics

	uint64 tx,rx;		// cumulative
	uint64 tx2,rx2;		// in the beginning of session

	// Install ID

	int64 fInstallID;

	// UniShare

	void TransferCallbackRejected(const QString &qFrom, int64 timeLeft, uint32 port);
	
	bool IsIgnored(const WUserRef & user);
	bool Ignore(const QString & user);
	bool UnIgnore(const QString & user);

	bool IsBlackListed(const WUserRef & user);
	bool BlackList(const QString & user);
	bool UnBlackList(const QString & user);

	bool IsWhiteListed(const WUserRef & user);
	bool WhiteList(const QString & user);
	bool UnWhiteList(const QString & user);

	bool IsFilterListed(const QString & pattern);
	bool FilterList(const QString & pattern);
	bool UnFilterList(const QString & pattern);

	bool IsAutoPrivate(const WUserRef & user);
	bool AutoPrivate(const QString & user);
	bool UnAutoPrivate(const QString & user);

	// Internal disconnection handling
	void Disconnect2();					

	// Check if resume list contains downloads from 'user'
	void CheckResumes(const QString &user);
	// List files waiting to be resumed
	void ListResumes();
	// Remove file from resume list
	void KillResume(uint32 index);
	// Clear the resume list
	void ClearResumes();

	// handle remote commands
	bool Remote(const QString &session, const QString &text);
	// remote password
	QString fRemote;
	// execute specified command
	void ExecCommand(const QString &command);			


	void SignalDownload(int);
	void SignalUpload(int);

	void UpdateUserCount();

	QTimer * fAutoAway;
	QTimer * fConnectTimer;
	QTimer * fReconnectTimer;

	WLog fMainLog;

	WPicViewer * fPicViewer;

	void InitGUI();
	void InitToolbars();

	void Cleanup();

	void SendChatText(WTextEvent *, bool * reply = NULL);

	void HandleMessage(MessageRef);
	void HandleChatText(const WUserRef &from, const QString &text, bool priv);

	void HandleComboEvent(WTextEvent *);

	QString MakeHumanTime(uint64 time);
	QString MakeHumanDiffTime(uint64 time);

	void NameChanged(const QString & newName);
	void StatusChanged(const QString & newStatus);
	void ServerChanged(const QString & newServer);
	void SetStatus(const QString & s);
	void setStatus(const QString & s, unsigned int i = 0);

	// stolen from BeShare :) thanx Jeremy
	static bool ParseUserTargets(const QString & text, WUserSearchMap & sendTo, QString & setTargetStr, QString & setRestOfString, NetClient * net);
	void SendPingOrMsg(QString & text, bool isping, bool * reply = NULL, bool enc = false);

	void GetAddressInfo(const QString & user, bool verbose = true);

	void ShowHelp(const QString & command = QString::null);

	// parsing stuff...
	bool MatchUserFilter(const WUserRef & user, const QString & filter);
	bool MatchFilter(const QString & user, const QString & filter);

	int MatchUserName(const QString & un, QString & result, const QString & filter);
	bool DoTabCompletion(const QString & origText, QString & result);

	QString MapUsersToIDs(const QString & pattern);
	QString MapIPsToNodes(const QString & pattern);

	void SetWatchPattern(const QString & pattern);

	void ServerParametersReceived(MessageRef msg);

	void LoadSettings();
	bool SaveSettings();
	void EmptyUsers();
	void SetAutoAwayTimer();
	void WaitOnFileThread(bool);
	void WaitOnListThread(bool);

	void StartLogging();
	void StopLogging();
	void UpdateShares();
	void CancelShares();
	void ScanShares(bool rescan = false);
	void CreateDirectories();

	bool StartAcceptThread();
	void StopAcceptThread();

	QString GetUptimeString();
	uint64 GetUptime();

	friend class DownloadQueue;
	void OpenDownload();

	void OpenUpload();

	friend class WPrivateWindow;
	friend class Channel;
	friend class WUser;
	friend class WSearch;
	friend class NetClient;

	WResumeMap fResumeMap;

	QGridLayout * fMainBox;
	
	QWidget * fMainWidget;
	QGridLayout * fMainTab;
	
	// splits user list and the rest of the window
	QSplitter * fMainSplitter;	

	uint32 fMaxUsers;

	// UniShare

	int64 GetRegisterTime() const;

	// Titanic Transfer Protocol

	Queue<TTPInfo *> _ttpFiles;

signals:
	void UpdatePrivateUserLists();
	void NewChannelText(const QString &, const QString &, const QString &);
};


#endif
