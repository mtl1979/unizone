#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "winsharewindow.h"
#include "muscle/message/Message.h"
using namespace muscle;

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
#define HTTPPROXY           "httpproxy"
#define HTTPPORT            "httpport"
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
#define EMPTYWIN			"emptywin"
#define SHARING_ENABLED		"sharingenabled"
#define BASEPORT			"baseport"
#define PORTRANGE			"portrange"
#define MAX_UPLOADS			"maxuploads"
#define MAX_DOWNLOADS		"maxdownloads"
#define PACKET_SIZE			"packetsize"
#define MIN_QUEUED			"minqueued"
#define FONT_SIZE			"fontsize"
#define WATCH_PATTERN		"watchpattern"
#define IGNORE_PATTERN		"ignorepattern"
#define IPIGNORE_PATTERN	"ipignorepattern"
#define BLACKLIST			"blacklist"
#define WHITELIST			"whitelist"
#define FILTERLIST			"filterlist"
#define AUTOPRIV			"autopriv"
#define PM_REDIRECT			"pmredir"

#define ONCONN		"onconnect1"
#define ONCONN2		"onconnect2"

#define REGISTERTIME "registertime"

#define INSTALLID "installid"

#define MAXUSERS "maxusr_"		// Prefix

/* Configurable URL launcher */
#ifndef WIN32	
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
	void AddServerItem(const QString & str);
	QString GetServerItem(int index) const;
	QString StartServerIter();		// get first item in the server list, initializing the internal iterator
	QString GetNextServerItem();	// if returns QString::null, out of items
	void EmptyServerList();
	void SetCurrentServerItem(int32 item);	// the currently selected item
	int32 GetCurrentServerItem() const;

	// user list
	void AddUserItem(const QString & str);
	QString GetUserItem(int index) const;
	QString StartUserIter();
	QString GetNextUserItem();
	void EmptyUserList();
	void SetCurrentUserItem(int32 item);
	int32 GetCurrentUserItem() const;

	// status list
	void AddStatusItem(const QString & str);
	QString GetStatusItem(int index) const;
	QString StartStatusIter();
	QString GetNextStatusItem();
	void EmptyStatusList();
	void SetCurrentStatusItem(int32 item);
	int32 GetCurrentStatusItem() const;

	// query list
	void AddQueryItem(const QString & str);
	QString GetQueryItem(int index) const;
	void EmptyQueryList();
	void SetCurrentQueryItem(int32 item);
	int32 GetCurrentQueryItem() const;

	// resume list
	void AddResumeItem(WResumePair wrp);
	bool GetResumeItem(int index, WResumePair & wrp) const;
	int32 GetResumeCount() const;
	void SetResumeCount(int32 c);
	void EmptyResumeList();

	// style
	void SetStyle(WinShareWindow::Style style);
	WinShareWindow::Style GetStyle() const;

	// column list
	void AddColumnItem(int32 i);
	int32 GetColumnItem(int index) const;
	int StartColumnIter();
	int32 GetNextColumnItem();	// returns < 0 when out of items
	void EmptyColumnList();

	// window stuff
	int32 GetWindowHeight() const;
	void SetWindowHeight(int32 h);
	int32 GetWindowWidth() const;
	void SetWindowWidth(int32 w);
	void SetWindowX(int32 x);
	int32 GetWindowX() const;
	void SetWindowY(int32 y);
	int32 GetWindowY() const;

	// chat splitter sizes
	QValueList<int> GetChatSizes() const;
	void SetChatSizes(QValueList<int> & sizes);

	// main splitter sizes
	QValueList<int> GetMainSizes() const;
	void SetMainSizes(QValueList<int> & sizes);

	// status messages
	void SetAwayMsg(const QString & away);
	QString GetAwayMsg() const;
	void SetHereMsg(const QString & here);
	QString GetHereMsg() const;

	// On Connect
	void SetOnConnect(const QString & s);
	QString GetOnConnect() const;
	void SetOnConnect2(const QString & s);
	QString GetOnConnect2() const;

	// colors
	void AddColorItem(const QString & c);
	QString GetColorItem(int index) const;
	QString StartColorIter();
	QString GetNextColorItem();
	void EmptyColorList();

	// other settings
	void SetAutoUpdateServers(bool f);
	bool GetAutoUpdateServers() const;
	void SetCheckNewVersions(bool c);
	bool GetCheckNewVersions() const;
	void SetLoginOnStartup(bool s);
	bool GetLoginOnStartup() const;
	void SetFirewalled(bool f);
	bool GetFirewalled() const;
	void SetBinkyNuke(bool b);
	bool GetBinkyNuke() const;
	void SetBlockDisconnected(bool b);
	bool GetBlockDisconnected() const;
	void SetAutoClear(bool b);
	bool GetAutoClear() const;
	void SetMultiColor(bool m);
	bool GetMultiColor() const;

	void SetConnection(const QString & str);
	QString GetConnection() const;
	void SetHTTPProxy(const QString & str);
	QString GetHTTPProxy() const;
	void SetHTTPPort(uint32 port);
	uint32 GetHTTPPort() const;
	void SetEncoding(const QString & server, uint16 port, uint32 encoding);
	uint32 GetEncoding(const QString & server, uint16 port) const;
	void SetMaxUsers(const QString & server, uint16 port, uint32 users);
	uint32 GetMaxUsers(const QString & server, uint16 port) const;

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

	bool GetTimeStamps() const;
	bool GetUserEvents() const;
	bool GetUploads() const;
	bool GetDownloads() const;
	bool GetChat() const;
	bool GetPrivate() const;
	bool GetInfo() const;
	bool GetWarning() const;
	bool GetError() const;
	bool GetSounds() const;
	bool GetIPAddresses() const;

	// auto away
	void SetAutoAway(int32 i);	// index of time
	int32 GetAutoAway() const;

	// UniShare
	void SetRegisterTime(int64 i); // Nick Registration Time
	void SetRegisterTime(const QString & nick, int64 i);
	int64 GetRegisterTime() const;
	int64 GetRegisterTime(const QString & nick) const;

	// window flashing
	enum
	{
		FlashNone,
		FlashMain = 0x00000001,
		FlashPriv = 0x00000002,
		FlashAll = FlashMain | FlashPriv
	};

	void SetFlash(int32);	// set flash flags
	int32 GetFlash() const;

	void SetEmptyWindows(int32); // What do we do if all users leave some window?
	int32 GetEmptyWindows() const;
	
	// file sharing
	bool GetSharingEnabled() const;
	void SetSharingEnabled(bool b);

	int32 GetBasePort() const;
	void SetBasePort(int32 bp);

	int32 GetPortRange() const;
	void SetPortRange(int32 pr);

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
	int32 GetMaxUploads() const;
	void SetMaxUploads(int32 u);

	int32 GetMaxDownloads() const;
	void SetMaxDownloads(int32 d);

	// font size
	int32 GetFontSize() const;
	void SetFontSize(int32 f);

	// watch pattern
	QString GetWatchPattern() const;
	void SetWatchPattern(const QString & p);

	// ignore pattern
	QString GetIgnorePattern() const;
	void SetIgnorePattern(const QString & p);

	// ip ignore pattern
	QString GetIPIgnorePattern() const;
	void SetIPIgnorePattern(const QString & p);

	// blacklist pattern
	QString GetBlackListPattern() const;
	void SetBlackListPattern(const QString & p);

	// whitelist pattern
	QString GetWhiteListPattern() const;
	void SetWhiteListPattern(const QString & p);

	// filter list pattern
	QString GetFilterListPattern() const;
	void SetFilterListPattern(const QString & p);

	// auto-private pattern
	QString GetAutoPrivatePattern() const;
	void SetAutoPrivatePattern(const QString & p);

	// private message redirection
	QString GetPMRedirect() const;
	void SetPMRedirect(const QString & p);

#ifndef WIN32
	QString GetFTPLauncher() const;
	void SetFTPLauncher(const QString & l);

	QString GetHTTPLauncher() const;
	void SetHTTPLauncher(const QString & l);

	QString GetMailLauncher() const;
	void SetMailLauncher(const QString & l);

	QString GetDefaultLauncher() const;
	void SetDefaultLauncher(const QString & l);
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
	
	int32 GetChatLimit() const;
	void SetChatLimit(int32);
	int32 GetULLimit() const;
	void SetULLimit(int32);
	int32 GetDLLimit() const;
	void SetDLLimit(int32);
	int32 GetBLLimit() const;
	void SetBLLimit(int32);
	int32 GetPacketSize() const;
	void SetPacketSize(int32);
	int32 GetMinQueued() const;
	int64 GetMinQueuedSize();
	void SetMinQueued(int32);
	
	static int ConvertToBytes(int);	// converts a limit constant into bytes
	
	// is the main window maximized?
	bool GetMaximized() const;
	void SetMaximized(bool);

	// Logging
	bool GetLogging() const;
	void SetLogging(bool);

	// Transfer Statistics

	uint64 GetTransmitStats() const;
	void SetTransmitStats(uint64 t);

	uint64 GetReceiveStats() const;
	void SetReceiveStats(uint64 r);

	// Nick List Sorting

	int32 GetNickListSortColumn() const;
	void SetNickListSortColumn(int32 c);

	bool GetNickListSortAscending() const;
	void SetNickListSortAscending(bool a);

	// Search Query List Sorting

	int32 GetSearchListSortColumn() const;
	void SetSearchListSortColumn(int32 c);

	bool GetSearchListSortAscending() const;
	void SetSearchListSortAscending(bool a);

	// remote control
	QString GetRemotePassword() const;
	void SetRemotePassword(const QString & pw);

	// toolbar layout
	void GetToolBarLayout(int toolbar, int32 & dock, int32 & index, bool & nl, int32 & extra) const;
	void SetToolBarLayout(int toolbar, int32 dock, int32 index, bool nl, int32 extra);

	// Install ID
	int64 GetInstallID();
	void SetInstallID(int64 iid);
private:
	MessageRef fSet;
	int fColor, fColumn, fStatus, fUser, fServer;	// iterators
};

#endif

