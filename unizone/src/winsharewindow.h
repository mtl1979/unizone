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

#include "menubar.h"
#include "netclient.h"
#include "privatewindowimpl.h"
#include "channelimpl.h"
#include "channelsimpl.h"
#include "system/SetupSystem.h"
#include "regex/StringMatcher.h"
#include "chattext.h"
#include "downloadimpl.h"
#include "accept.h"
#include "filethread.h"
#include "search.h"
#include "Log.h"

#define UPDATE_SERVER "www.raasu.org"
#define UPDATE_FILE "http://www.raasu.org/tools/windows/version.txt"

#define START_OUTPUT() fPrintOutput = false; PrintText("", true)
#define END_OUTPUT() fPrintOutput = true; PrintText("", false)

#include <map>
using std::pair;
using std::multimap;
using std::iterator;

typedef pair<QString, QString> WResumePair;
typedef multimap<QString, QString> WResumeMap;
typedef WResumeMap::iterator WResumeIter;

inline 
WResumePair
MakePair(QString f, QString u)
{
	WResumePair p;
	p.first = f;
	p.second = u;
	return p;
}

class WTextEvent;
class WChatText;
class WSettings;

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
	void OpenChannels();
		/* Sep */
	void AboutWinShare();
		/* Sep */
	void Exit();

	/** Prefs menu **/
	void Preferences();

	// user connection/disconnection messages
	void UserNameChanged(QString, QString, QString);
	void UserConnected(QString);
	void UserDisconnected(QString, QString);
	void DisconnectedFromServer();
	void UserStatusChanged(QString, QString, QString);

	// tab completion signal
	void TabPressed(QString str);
	// URL's
	void URLSelected(const QString &);
	void URLClicked();

	// popup menu
	void RightButtonClicked(QListViewItem *, const QPoint &, int);
	void PopupActivated(int);

	// auto away timer
	void AutoAwayTimer();

	// reconnect timer
	void ReconnectTimer();

	// open a new search dialog
	void SearchDialog();

	// this won't be emitted under Windows
	void GotShown(const QString &);

	void AboutToQuit();
	void SearchWindowClosed();
	void DownloadWindowClosed();
	void ChannelsWindowClosed();

	void FileFailed(QString, QString); // from WDownload
	void FileInterrupted(QString, QString);

protected:
	virtual void customEvent(QCustomEvent * event);
	virtual void resizeEvent(QResizeEvent * event);

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

	QHGroupBox * fUsersBox;	// frame around fUsers
	WUniListView * fUsers;

	QVGroupBox * fLeftPane;
	QHGroupBox * fInfoPane;	// holds server list, user name, user status
	QComboBox * fServerList, * fUserList, * fStatusList;
	QLabel * fServerLabel, * fUserLabel, * fStatusLabel;
	QSplitter * fChatSplitter;
	WHTMLView * fChatText;
	WChatText * fInputText;

	int64 fLoginTime;

	QString fUserName, fUserStatus, fServer;
	QString fCurURL;			// currently selected URL
	QString fPopupUser;
	QPopupMenu * fPrivate;		// private window popup

	WPrivMap fPrivateWindows;	// private windows;
	QMutex pLock;				// private window mutex
	QMutex rLock;				// resume list mutex


	bool fGotParams;	// see if the initial "Get Params" message was sent
	bool fAway;
	bool fPrintOutput;
	bool fScrollDown;	// do we need to scroll the view down after an insertion?

	bool fDisconnect;		// True if disconnected prematurely
	bool fDisconnectFlag;	// false for no disconnects, true for user disconnection (for when file sharing disabled)
	int  fDisconnectCount;	// Number of connection drops (1-2 for normal, 3- for premature)

	QString fAwayMsg;
	QString fHereMsg;
	QString fWatch;		// watch pattern
	QString fIgnore;	// ignore pattern
	QString fIgnoreIP;	// ip ignore pattern
	QString fBlackList; // blacklist pattern
	QString fAutoPriv;	// Auto-private pattern
	QString fOnConnect;	// On connect perform this command
	QString fOnConnect2; // On connect perform this too ;)

	// transmit/receive statistics

	uint64 tx,rx;		// cumulative
	uint64 tx2,rx2;		// in the beginning of session

	// UniShare
	int64 fRegistered;

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
	QTimer * fReconnectTimer;

	WSearch * fSearchWindow;
	Channels * fChannels;
	WLog fMainLog;

#ifdef WIN32			// if the OS is Windows, 
	HWND fWinHandle;	// handle to our window for flashing
#endif

	void InitGUI();
	void HandleSignal();
	void SendChatText(WTextEvent *, bool * reply = NULL);

	void HandleMessage(Message *);
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


	// see if we were named...
	bool NameSaid(QString & msg);	// msg will be syntaxed if needed
	void SetWatchPattern(QString pattern);

	void ServerParametersReceived(const Message * msg);

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

	static QString GetTimeStamp();

	void OpenDownload();

	friend class WPrivateWindow;
	friend class Channel;
	friend class WUser;
	friend class WSearch;
	friend class WDownload;

	WResumeMap fResumeMap;

public:
	QString GetUserName() const;
	QString GetServer() const;
	QString GetStatus() const;
	QString GetUserID() const { return fNetClient->LocalSessionID(); }
#ifdef WIN32
	HWND GetHandle() { return fWinHandle; }
#endif

	void SendChatText(QString sid, QString txt, WUserRef priv = WUserRef(NULL, NULL), bool * reply = NULL);

	static QString GetRemoteVersionString(const Message *);
	static QString ParseForShown(const QString & str);	// parses the string during shown notifications from
														// the HTML view (under Linux)
	static void LaunchSearch(const QString & pattern);	// launches a search
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

	// To use delayed search, first set the pattern using SetDelayedSearchPattern(QString) and
	// then call Connect(QString)
	void Connect(QString server);
	void SetDelayedSearchPattern(QString pattern);
	
	void TranslateStatus(QString & s);
	
	WUserRef FindUser(QString user);

	WSettings * fSettings;	// for use by prefs
	WDownload * fDLWindow;

	// UniShare
	int64 GetRegisterTime() { return fRegistered; }


signals:
	void UpdatePrivateUserLists();
	void ChannelCreated(const QString, const QString, int64);
	void ChannelJoin(const QString, const QString);
	void ChannelPart(const QString, const QString);
	void ChannelInvite(const QString, const QString, const QString);
	void ChannelKick(const QString, const QString, const QString);
	void ChannelTopic(const QString, const QString, const QString);
	void ChannelPublic(const QString, const QString, bool);
	void NewChannelText(const QString, const QString, const QString);
};


#endif
