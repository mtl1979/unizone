#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "winsharewindow.h"
#include "muscle/message/Message.h"
using namespace muscle;

// #include "filethread.h"	// for WStrList

#include <qvaluelist.h>

#define SERVER_LIST		"serverlist"
#define SERVER_ITEM		"serveritem"
#define USER_LIST		"userlist"
#define USER_ITEM		"useritem"
#define STATUS_LIST		"statuslist"
#define STATUS_ITEM		"statusitem"
#define STYLE			"style"
#define COLUMN_LIST		"columnlist"
#define WINDOW_WIDTH	"windowwidth"
#define WINDOW_HEIGHT	"windowheight"
#define WINDOW_X		"windowx"
#define WINDOW_Y		"windowy"
#define CHAT_SIZES		"chatsizes"
#define USER_SIZES		"usersizes"		// the splitter between the user list and the chat stuff
#define AWAY_MSG		"awaymsg"
#define HERE_MSG		"heremsg"
#define COLORS			"colors"

#define AUTO_UPDATE			"autoupdateservers"
#define CHECK_VERSION		"checknewversion"
#define LOGIN_STARTUP		"loginonstartup"
#define FIREWALLED			"firewalled"
#define BINKYNUKE			"binkynuke"
#define BLOCKDISCONNECTED	"blockdisconnected"
#define AUTOCLEAR			"autoclear"
#define MULTICOLOR			"multicolor"
#define CONNECTION			"connection"
#define TIME_STAMPS			"timestamps"
#define USER_EVENTS			"userevents"
#define UPLOADS				"uploads"
#define DOWNLOADS			"downloads"
#define CHAT				"chat"
#define PRIVATE				"private"
#define INFO				"info"
#define WARNING				"warning"
#define ERRORS				"errorS"
#define IPADDRESSES			"ipaddr"

#define AUTO_AWAY			"autoaway"
#define FLASH				"flash"
#define SHARING_ENABLED		"sharingenabled"
#define BASEPORT			"baseport"
#define MAX_UPLOADS			"maxuploads"
#define MAX_DOWNLOADS		"maxdownloads"
#define PACKET_SIZE			"packetsize"
#define MIN_QUEUED			"minqueued"
#define FONT_SIZE			"fontsize"
#define WATCH_PATTERN		"watchpattern"
#define IGNORE_PATTERN		"ignorepattern"
#define IPIGNORE_PATTERN	"ipignorepattern"
#define BLACKLIST			"blacklist"
#define AUTOPRIV			"autopriv"

#define ONCONN		"onconnect1"
#define ONCONN2		"onconnect2"

#define REGISTERTIME "registertime"

/* Linux was our only configurable URL launcher */
/* Now let's try it with FreeBSD too            */
#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	
# define MAILTO_LAUNCHER	"mailtolauncher"
# define HTTP_LAUNCHER		"httplauncher"
# define FTP_LAUNCHER		"ftplauncher"
# define DEFAULT_LAUNCHER	"defaultlauncher"
#endif

#define UL_LIMIT	"ullimit"
#define DL_LIMIT	"dllimit"
#define BL_LIMIT	"bllimit"
#define CHAT_LIMIT	"chatlimit"

#define MAXIMIZED	"maximized"
#define LOGGING		"logging"

#define TX_STATS "txstats"
#define RX_STATS "rxstats"

#define NL_SORTCOL "nlsortcol" // Nick List Sort Column
#define NL_SORTASC "nlsortasc" //           Sort Ascending

#define SL_SORTCOL "slsortcol" // Search Query List Sort Column
#define SL_SORTASC "slsortasc" //                   Sort Ascending

#define REMOTEPASSWORD "remotepassword"

#define QUERY_LIST "querylist"
#define QUERY_ITEM "queryitem"

#define RESUMELIST "resumelist"
#define RESUMEFILE "resumefile"
#define RESUMEFIL2 "resumefile2"
#define RESUMEUSER "resumeuser"

#define SOUNDS     "sounds"

class WSettings
{
public:
	WSettings();
	~WSettings();

	bool Load();
	bool Save();

	// server list
	void AddServerItem(QString str);
	QString GetServerItem(int index);
	QString StartServerIter();		// get first item in the server list, initializing the internal iterator
	QString GetNextServerItem();	// if returns QString::null, out of items
	void EmptyServerList();
	void SetCurrentServerItem(int item);	// the currently selected item
	int GetCurrentServerItem();

	// user list
	void AddUserItem(QString str);
	QString GetUserItem(int index);
	QString StartUserIter();
	QString GetNextUserItem();
	void EmptyUserList();
	void SetCurrentUserItem(int item);
	int GetCurrentUserItem();

	// status list
	void AddStatusItem(QString str);
	QString GetStatusItem(int index);
	QString StartStatusIter();
	QString GetNextStatusItem();
	void EmptyStatusList();
	void SetCurrentStatusItem(int item);
	int GetCurrentStatusItem();

	// query list
	void AddQueryItem(QString str);
	QString GetQueryItem(int index);
	void EmptyQueryList();
	void SetCurrentQueryItem(int item);
	int GetCurrentQueryItem();

	// resume list
	void AddResumeItem(WResumePair wrp);
	bool GetResumeItem(int index, WResumePair & wrp);
	int GetResumeCount();
	void SetResumeCount(int c);
	void EmptyResumeList();

	// style
	void SetStyle(WinShareWindow::Style style);
	WinShareWindow::Style GetStyle();

	// column list
	void AddColumnItem(int i);
	int GetColumnItem(int index);
	int StartColumnIter();
	int GetNextColumnItem();	// returns < 0 when out of items
	void EmptyColumnList();

	// window stuff
	int GetWindowHeight();
	void SetWindowHeight(int h);
	int GetWindowWidth();
	void SetWindowWidth(int w);
	void SetWindowX(int x);
	int GetWindowX();
	void SetWindowY(int y);
	int GetWindowY();

	// chat splitter sizes
	QValueList<int> GetChatSizes();
	void SetChatSizes(QValueList<int> & sizes);

	// main splitter sizes
	QValueList<int> GetMainSizes();
	void SetMainSizes(QValueList<int> & sizes);

	// status messages
	void SetAwayMsg(QString away);
	QString GetAwayMsg();
	void SetHereMsg(QString here);
	QString GetHereMsg();

	// On Connect
	void SetOnConnect(QString s);
	QString GetOnConnect();
	void SetOnConnect2(QString s);
	QString GetOnConnect2();

	// colors
	void AddColorItem(QString c);
	QString GetColorItem(int index);
	QString StartColorIter();
	QString GetNextColorItem();
	void EmptyColorList();

	// other settings
	void SetAutoUpdateServers(bool f);
	bool GetAutoUpdateServers();
	void SetCheckNewVersions(bool c);
	bool GetCheckNewVersions();
	void SetLoginOnStartup(bool s);
	bool GetLoginOnStartup();
	void SetFirewalled(bool f);
	bool GetFirewalled();
	void SetBinkyNuke(bool b);
	bool GetBinkyNuke();
	void SetBlockDisconnected(bool b);
	bool GetBlockDisconnected();
	void SetAutoClear(bool b);
	bool GetAutoClear();
	void SetMultiColor(bool m);
	bool GetMultiColor();

	void SetConnection(QString str);
	QString GetConnection();
	void SetEncoding(QString server, uint16 port, uint32 encoding);
	uint32 GetEncoding(QString server, uint16 port);

	void SetTimeStamps(bool b);
	void SetUserEvents(bool b);
	void SetUploads(bool b);
	void SetDownloads(bool b);
	void SetChat(bool b);
	void SetPrivate(bool b);
	void SetInfo(bool b);
	void SetWarning(bool b);
	void SetError(bool b);
	void SetSounds(bool b);
	void SetIPAddresses(bool b);

	bool GetTimeStamps();
	bool GetUserEvents();
	bool GetUploads();
	bool GetDownloads();
	bool GetChat();
	bool GetPrivate();
	bool GetInfo();
	bool GetWarning();
	bool GetError();
	bool GetSounds();
	bool GetIPAddresses();

	// auto away
	void SetAutoAway(int i);	// index of time
	int GetAutoAway();

	// UniShare
	void SetRegisterTime(int64 i); // Nick Registration Time
	void SetRegisterTime(QString nick, int64 i);
	int64 GetRegisterTime();
	int64 GetRegisterTime(QString nick);

	// window flashing
	enum
	{
		FlashNone,
		FlashMain = 0x00000001,
		FlashPriv = 0x00000002,
		FlashAll = FlashMain | FlashPriv
	};

	void SetFlash(int);	// set flash flags
	int GetFlash();
	
	// file sharing
	bool GetSharingEnabled();
	void SetSharingEnabled(bool b);

	int GetBasePort();
	void SetBasePort(int bp);

	enum
	{
		// for upload/download limits
		One = 1,
		Two,
		Three,
		Four,
		Five,
		Ten = 10,
		Fifteen = 15,
		Twenty = 20,
		Thirty = 30,
		Unlimited = 999999
	};

	// 
	int GetMaxUploads();
	void SetMaxUploads(int u);

	int GetMaxDownloads();
	void SetMaxDownloads(int d);

	// font size
	int GetFontSize();
	void SetFontSize(int f);

	// watch pattern
	QString GetWatchPattern();
	void SetWatchPattern(QString p);

	// ignore pattern
	QString GetIgnorePattern();
	void SetIgnorePattern(QString p);

	// ip ignore pattern
	QString GetIPIgnorePattern();
	void SetIPIgnorePattern(QString p);

	// blacklist pattern
	QString GetBlackListPattern();
	void SetBlackListPattern(QString p);

	// auto-private pattern
	QString GetAutoPrivatePattern();
	void SetAutoPrivatePattern(QString p);

#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)
	QString GetFTPLauncher();
	void SetFTPLauncher(QString l);

	QString GetHTTPLauncher();
	void SetHTTPLauncher(QString l);

	QString GetMailLauncher();
	void SetMailLauncher(QString l);

	QString GetDefaultLauncher();
	void SetDefaultLauncher(QString l);
#endif

	enum	// throttling constants
	{
		LimitNone,
		Limit128,
		Limit256,
		Limit512,
		Limit1KB,
		Limit2KB,
		Limit4KB,
		Limit8KB,
		Limit16KB,
		Limit32KB,
		Limit64KB,
		Limit128KB,
		Limit256KB,
		Limit512KB,
		Limit1MB,
		Limit2MB,
		Limit4MB,
		Limit8MB,
		Limit16MB,
		Limit32MB,
	};
	
	int GetChatLimit();
	void SetChatLimit(int);
	int GetULLimit();
	void SetULLimit(int);
	int GetDLLimit();
	void SetDLLimit(int);
	int GetBLLimit();
	void SetBLLimit(int);
	int GetPacketSize();
	void SetPacketSize(int);
	int GetMinQueued();
	uint64 GetMinQueuedSize();
	void SetMinQueued(int);
	
	static int ConvertToBytes(int);	// converts a limit constant into bytes
	
	// is the main window maximized?
	bool GetMaximized() const;
	void SetMaximized(bool);

	// Logging
	bool GetLogging() const;
	void SetLogging(bool);

	// Transfer Statistics

	uint64 GetTransmitStats();
	void SetTransmitStats(uint64 t);

	uint64 GetReceiveStats();
	void SetReceiveStats(uint64 r);

	// Nick List Sorting

	int GetNickListSortColumn();
	void SetNickListSortColumn(int c);

	bool GetNickListSortAscending() const;
	void SetNickListSortAscending(bool a);

	// Search Query List Sorting

	int GetSearchListSortColumn();
	void SetSearchListSortColumn(int c);

	bool GetSearchListSortAscending() const;
	void SetSearchListSortAscending(bool a);

	// remote control
	QString GetRemotePassword();
	void SetRemotePassword(QString pw);

	// toolbar layout
	void GetToolBarLayout(int toolbar, int & dock, int & index, bool & nl, int & extra);
	void SetToolBarLayout(int toolbar, int dock, int index, bool nl, int extra);

private:
	MessageRef fSet;
	int fColor, fColumn, fStatus, fUser, fServer;	// iterators
};

#endif

