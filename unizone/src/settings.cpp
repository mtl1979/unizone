#include <qapplication.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <Q3ValueList>
#include <fcntl.h>

#include "settings.h"
#include "colors.h"
#include "global.h"
#include "wfile.h"
#include "wstring.h"
#include "util.h"
#include "iogateway/MessageIOGateway.h"
#include "debugimpl.h"

QString SettingsFile; // name of settings file, default depends on host os

#ifdef _WIN32
extern QString gDataDir; // main.cpp
#endif

const char *
GetSettingsFile()
{
	if (SettingsFile.isEmpty())
	{
#ifdef _WIN32
		SettingsFile = MakePath(gDataDir, "settings.ini");
#elif defined(__APPLE__)
		SettingsFile = "../../../.unizone_settings";
#else
		SettingsFile = ".unizone_settings";
#endif
	}
	return SettingsFile;
}

QString
GetStringItem(const MessageRef &msg, const char * key, uint32 index)
{
	QString str;
	if (GetStringFromMessage(msg, key, index, str) == B_OK)
		return str;
	return QString::null;
}

QString
GetStringItem(const MessageRef &msg, const char * key)
{
	QString str;
	if (GetStringFromMessage(msg, key, str) == B_OK)
		return str;
	return QString::null;
}

void
SetSettingsFile(const char * sf)
{
	SettingsFile = QString::fromLocal8Bit(sf);
}

WSettings::WSettings()
{
	fColor = fColumn = fStatus = fUser = fServer = 0;
	fSet = GetMessageFromPool();
}

WSettings::~WSettings()
{
}

void
WSettings::AddServerItem(const QString & str)
{
	AddStringToMessage(fSet, SERVER_LIST, str);
}

QString
WSettings::GetServerItem(int index) const
{
	return GetStringItem(fSet, SERVER_LIST, index);
}

QString
WSettings::StartServerIter()
{
	fServer = 0;
	return GetServerItem(0);
}

QString
WSettings::GetNextServerItem()
{
	return GetServerItem(++fServer);
}

void
WSettings::EmptyServerList()
{
	fSet()->RemoveName(SERVER_LIST);
}

void
WSettings::SetCurrentServerItem(int32 item)
{
	fSet()->ReplaceInt32(true, SERVER_ITEM, item);
}

int32
WSettings::GetCurrentServerItem() const
{
	int32 i = 0;
	fSet()->FindInt32(SERVER_ITEM, &i);	// if it fails, who cares, return 0
	return i;
}

// users
void
WSettings::AddUserItem(const QString & str)
{
	AddStringToMessage(fSet, USER_LIST, str);
}

QString
WSettings::GetUserItem(int index) const
{
	return GetStringItem(fSet, USER_LIST, index);
}

QString
WSettings::StartUserIter()
{
	fUser = 0;
	return GetUserItem(0);
}

QString
WSettings::GetNextUserItem()
{
	return GetUserItem(++fUser);
}

void
WSettings::EmptyUserList()
{
	fSet()->RemoveName(USER_LIST);
}

void
WSettings::SetCurrentUserItem(int32 item)
{
	fSet()->ReplaceInt32(true, USER_ITEM, item);
}

int32
WSettings::GetCurrentUserItem() const
{
	int32 i = 0;
	fSet()->FindInt32(USER_ITEM, &i);
	return i;
}

// status
void
WSettings::AddStatusItem(const QString & str)
{
	AddStringToMessage(fSet, STATUS_LIST, str);
}

QString
WSettings::GetStatusItem(int index) const
{
	return GetStringItem(fSet, STATUS_LIST, index);
}

QString
WSettings::StartStatusIter()
{
	fStatus = 0;
	return GetStatusItem(0);
}

QString
WSettings::GetNextStatusItem()
{
	return GetStatusItem(++fStatus);
}

void
WSettings::EmptyStatusList()
{
	fSet()->RemoveName(STATUS_LIST);
}

void
WSettings::SetCurrentStatusItem(int32 item)
{
	fSet()->ReplaceInt32(true, STATUS_ITEM, item);
}

int32
WSettings::GetCurrentStatusItem() const
{
	int32 i = 0;
	fSet()->FindInt32(STATUS_ITEM, &i);
	return i;
}

// style
void
WSettings::SetStyle(WinShareWindow::Style style)
{
	fSet()->ReplaceInt32(true, STYLE, (int32)style);
}

WinShareWindow::Style
WSettings::GetStyle() const
{
	WinShareWindow::Style s = 
#ifdef WIN32
		WinShareWindow::Windows;
#else
		WinShareWindow::Motif;
#endif
	fSet()->FindInt32(STYLE, (int32 *)&s);
	return s;
}

// column
void
WSettings::AddColumnItem(int32 i)
{
	fSet()->AddInt32(COLUMN_LIST, i);
}

int32
WSettings::GetColumnItem(int index) const
{
	int32 i = -1;
	fSet()->FindInt32(COLUMN_LIST, index, &i);
	return i;
}

int
WSettings::StartColumnIter()
{
	fColumn = 0;
	return GetColumnItem(0);
}

int32
WSettings::GetNextColumnItem()
{
	return GetColumnItem(++fColumn);
}

void
WSettings::EmptyColumnList()
{
	fSet()->RemoveName(COLUMN_LIST);
}

// window
int32
WSettings::GetWindowHeight() const
{
	int32 i = -1;
	fSet()->FindInt32(WINDOW_HEIGHT, &i);
	return i;
}

int32
WSettings::GetWindowWidth() const
{
	int32 i = -1;
	fSet()->FindInt32(WINDOW_WIDTH, &i);
	return i;
}

int32
WSettings::GetWindowX() const
{
	int32 i = -1;
	fSet()->FindInt32(WINDOW_X, &i);
	return i;
}

int32
WSettings::GetWindowY() const
{
	int32 i = -1;
	fSet()->FindInt32(WINDOW_Y, &i);
	return i;
}

void
WSettings::SetWindowHeight(int32 h)
{
	fSet()->ReplaceInt32(true, WINDOW_HEIGHT, h);
}

void
WSettings::SetWindowWidth(int32 w)
{
	fSet()->ReplaceInt32(true, WINDOW_WIDTH, w);
}

void
WSettings::SetWindowX(int32 x)
{
	fSet()->ReplaceInt32(true, WINDOW_X, x);
}

void
WSettings::SetWindowY(int32 y)
{
	fSet()->ReplaceInt32(true, WINDOW_Y, y);
}

// sizes
Q3ValueList<int>
WSettings::GetChatSizes() const
{
	Q3ValueList<int> l;
	int32 j;
	for (int i = 0; fSet()->FindInt32(CHAT_SIZES, i, &j) == B_OK; i++)
		l.append(j);
	return l;
}

void
WSettings::SetChatSizes(Q3ValueList<int> & sizes)
{
	for (int i = 0; i < sizes.count(); i++)
		fSet()->ReplaceInt32(true, CHAT_SIZES, i, sizes[i]);
}

Q3ValueList<int>
WSettings::GetMainSizes() const
{
	Q3ValueList<int> l;
	int32 j;
	for (int i = 0; fSet()->FindInt32(USER_SIZES, i, &j) == B_OK; i++)
		l.append(j);
	return l;
}

void
WSettings::SetMainSizes(Q3ValueList<int> & sizes)
{
	for (int i = 0; i < sizes.count(); i++)
		fSet()->ReplaceInt32(true, USER_SIZES, i, sizes[i]);
}

// status messages
void
WSettings::SetAwayMsg(const QString & away)
{
	ReplaceStringInMessage(fSet, true, AWAY_MSG, away);
}

void
WSettings::SetHereMsg(const QString & here)
{
	ReplaceStringInMessage(fSet, true, HERE_MSG, here);
}

QString
WSettings::GetAwayMsg() const
{
	QString str("away");	// default
	GetStringFromMessage(fSet, AWAY_MSG, str);
	return str;
}

QString
WSettings::GetHereMsg() const
{
	QString str("here");
	GetStringFromMessage(fSet, HERE_MSG, str);
	return str;
}

// colors
void
WSettings::AddColorItem(const QString & c)
{
	AddStringToMessage(fSet, COLORS, c);
}

QString
WSettings::GetColorItem(int index) const
{
	QString str;
	if (GetStringFromMessage(fSet, COLORS, index, str) == B_OK)
		return str;
	// Default values
	switch (index)
	{
	case WColors::LocalName: 
		return "#008800";
	case WColors::RemoteName: 
		return "#000000";
	case WColors::Text: 
		return "#000000";
	case WColors::System:
		return "#0000FF";
	case WColors::Ping: 
		return "#DD2488";
	case WColors::Error:
		return "#FF0000";
	case WColors::ErrorMsg:
		return "#550000";
	case WColors::PrivText:
		return "#009999";
	case WColors::Action:
		return "#CC00CC";
	case WColors::URL:
		return "#0000FF";
	case WColors::NameSaid:
		return "#FF0033";
	case WColors::Warning:
		return "#FFAA00";
	case WColors::WarningMsg:
		return "#FFAA7F";
	default:	// This should never happen
		return "#000000";
	}
}

QString
WSettings::StartColorIter()
{
	fColor = 0;
	return GetColorItem(0);
}

QString
WSettings::GetNextColorItem()
{
	return GetColorItem(++fColor);
}

void
WSettings::EmptyColorList()
{
	fSet()->RemoveName(COLORS);
}

bool
WSettings::Load()
{
	bool ret = false;
	WFile file;
	WString wf(GetSettingsFile());
#ifdef _DEBUG
	PRINT("Settings file: %S\n", wf.getBuffer());
#endif
	if (file.Open(wf, 
#if defined(WIN32) || defined(_WIN32)
		O_RDONLY | O_BINARY
#else
		O_RDONLY
#endif
		))
	{
		ByteBufferRef buffer = GetByteBufferFromPool();
		if (buffer()->SetNumBytes((uint32) file.Size(), false) == B_NO_ERROR)
		{
			CheckSize(file.Size());
			if (file.ReadBlock32(buffer()->GetBuffer(), (uint32) file.Size()) == (int32) file.Size())
			{
				// reload settings
				ret = (fSet()->UnflattenFromByteBuffer(*buffer()) == B_OK);
			}
			if (ret == false)
			{
				QMessageBox::warning(NULL, qApp->translate( "WSettings", "Read Error" ), qApp->translate( "WSettings", "Unable to read data from file!" ), qApp->translate( "WSettings", "Bummer" ));
			}
		}
		file.Close();
	}
	else
	{
		PRINT("Failed opening settings file.\n");
	}
	return ret;
}

bool
WSettings::Save()
{
	bool ret = false;;
	WString wf(GetSettingsFile());
#ifdef _DEBUG
	PRINT("Settings file: %S\n", wf.getBuffer());
#endif
	WFile file;
	if (file.Open(wf, 
#ifdef WIN32
		O_WRONLY | O_CREAT | O_BINARY
#else
		O_WRONLY | O_CREAT
#endif
		))
	{
		uint32 s = fSet()->FlattenedSize();
		ByteBufferRef buffer = fSet()->FlattenToByteBuffer();
		if (buffer())
		{
			if (file.WriteBlock(buffer()->GetBuffer(), s) == s)
				ret = true;
		}
		file.Close();
	}
	if (ret == false)
	{
		QMessageBox::warning(NULL, qApp->translate( "WSettings", "Write Error" ), qApp->translate( "WSettings", "Couldn't save settings!" ), qApp->translate( "WSettings", "Bummer" ));
	}
	return ret;
}

#define SET_BOOL(X, Y) fSet()->ReplaceBool(true, X, Y)
#define GET_BOOL(X, D) \
	bool r = D; \
	fSet()->FindBool(X, &r); \
	return r;

void
WSettings::SetAutoUpdateServers(bool f)
{
	SET_BOOL(AUTO_UPDATE, f);
}

bool
WSettings::GetAutoUpdateServers() const
{
	GET_BOOL(AUTO_UPDATE, true);
}

void
WSettings::SetCheckNewVersions(bool c)
{
	SET_BOOL(CHECK_VERSION, c);
}

bool
WSettings::GetCheckNewVersions() const
{
	GET_BOOL(CHECK_VERSION, true);
}

void
WSettings::SetLoginOnStartup(bool s)
{
	SET_BOOL(LOGIN_STARTUP, s);
}

bool
WSettings::GetLoginOnStartup() const
{
	GET_BOOL(LOGIN_STARTUP, false);
}

void
WSettings::SetFirewalled(bool f)
{
	SET_BOOL(FIREWALLED, f);
}

bool
WSettings::GetFirewalled() const
{
	GET_BOOL(FIREWALLED, false);
}

void
WSettings::SetBinkyNuke(bool b)
{
	SET_BOOL(BINKYNUKE, b);
}

bool
WSettings::GetBinkyNuke() const
{
	GET_BOOL(BINKYNUKE, false);
}

void
WSettings::SetBlockDisconnected(bool b)
{
	SET_BOOL(BLOCKDISCONNECTED, b);
}

bool
WSettings::GetBlockDisconnected() const
{
	GET_BOOL(BLOCKDISCONNECTED, false);
}

void
WSettings::SetPreservePaths(bool b)
{
	SET_BOOL(PRESERVEPATHS, b);
}

bool
WSettings::GetPreservePaths() const
{
	GET_BOOL(PRESERVEPATHS, false);
}

void
WSettings::SetAutoClear(bool b)
{
	SET_BOOL(AUTOCLEAR, b);
}

bool
WSettings::GetAutoClear() const
{
	GET_BOOL(AUTOCLEAR, true);
}

void
WSettings::SetAutoClose(bool b)
{
	SET_BOOL(AUTOCLOSE, b);
}

bool
WSettings::GetAutoClose() const
{
	GET_BOOL(AUTOCLOSE, false);
}

void
WSettings::SetMultiColor(bool m)
{
	SET_BOOL(MULTICOLOR, m);
}

bool
WSettings::GetMultiColor() const
{
	GET_BOOL(MULTICOLOR, true);
}

void
WSettings::SetConnection(const QString & str)
{
	ReplaceStringInMessage(fSet, true, CONNECTION, str);
}

QString
WSettings::GetConnection() const
{
	QString str;
	GetStringFromMessage(fSet, CONNECTION, str);
	if (str.isEmpty())
		return qApp->translate("Connection", "Unknown");
	return str;
}

void
WSettings::SetHTTPProxy(const QString &str)
{
	ReplaceStringInMessage(fSet, true, HTTPPROXY, str);
}

QString
WSettings::GetHTTPProxy() const
{
	QString str;
	GetStringFromMessage(fSet, HTTPPROXY, str);
	return str;
}

void
WSettings::SetHTTPPort(uint32 port)
{
	fSet()->ReplaceInt32(true, HTTPPORT, port);
}

uint32
WSettings::GetHTTPPort() const
{
	uint32 p = 0;
	fSet()->FindInt32(HTTPPORT, (int32 *) &p);
	return p;
}

void 
WSettings::SetEncoding(const QString & server, uint16 port, uint32 encoding)
{
	QString key = server+":"+QString::number(port);
	fSet()->ReplaceInt32(true, (const char *) key.utf8(), encoding);
}

uint32
WSettings::GetEncoding(const QString & server, uint16 port) const
{
	QString key = server+":"+QString::number(port);
	uint32 encoding = MUSCLE_MESSAGE_ENCODING_DEFAULT;
	fSet()->FindInt32((const char *) key.utf8(), (int32 *) &encoding);
	return encoding;
}

void
WSettings::SetMaxUsers(const QString & server, uint16 port, uint32 users)
{
	QString key = MAXUSERS + server+":"+QString::number(port);
	fSet()->ReplaceInt32(true, (const char *) key.utf8(), users);
}

uint32
WSettings::GetMaxUsers(const QString & server, uint16 port) const
{
	QString key = MAXUSERS + server+":"+QString::number(port);
	uint32 users = 0;
	fSet()->FindInt32((const char *) key.utf8(), (int32 *) &users);
	return users;
}

void
WSettings::SetTimeStamps(bool b)
{
	SET_BOOL(TIME_STAMPS, b);
}

bool
WSettings::GetTimeStamps() const
{
	GET_BOOL(TIME_STAMPS, true);
}

void
WSettings::SetUserEvents(bool b)
{
	SET_BOOL(USER_EVENTS, b);
}

bool
WSettings::GetUserEvents() const
{
	GET_BOOL(USER_EVENTS, true);
}

void
WSettings::SetIPAddresses(bool b)
{
	SET_BOOL(IPADDRESSES, b);
}

bool
WSettings::GetIPAddresses() const
{
	GET_BOOL(IPADDRESSES, false);
}

void
WSettings::SetUploads(bool b)
{
	SET_BOOL(UPLOADS, b);
}

bool
WSettings::GetUploads() const
{
	GET_BOOL(UPLOADS, true);
}

void
WSettings::SetDownloads(bool b)
{
	SET_BOOL(DOWNLOADS, b);
}

bool
WSettings::GetDownloads() const
{
	GET_BOOL(DOWNLOADS, true);
}

void
WSettings::SetChat(bool b)
{
	SET_BOOL(CHAT, b);
}

bool
WSettings::GetChat() const
{
	GET_BOOL(CHAT, true);
}

void
WSettings::SetPrivate(bool b)
{
	SET_BOOL(PRIVATE, b);
}

bool
WSettings::GetPrivate() const
{
	GET_BOOL(PRIVATE, true);
}

void
WSettings::SetInfo(bool b)
{
	SET_BOOL(INFO, b);
}

bool
WSettings::GetInfo() const
{
	GET_BOOL(INFO, true);
}

void
WSettings::SetWarning(bool b)
{
	SET_BOOL(WARNING, b);
}

bool
WSettings::GetWarning() const
{
	GET_BOOL(WARNING, true);
}

void
WSettings::SetError(bool b)
{
	SET_BOOL(ERRORS, b);
}

bool
WSettings::GetError() const
{
	GET_BOOL(ERRORS, true);
}

void
WSettings::SetSounds(bool b)
{
	SET_BOOL(SOUNDS, b);
}

bool
WSettings::GetSounds() const
{
	GET_BOOL(SOUNDS, true);
}

void
WSettings::SetSoundFile(const QString &fn)
{
	ReplaceStringInMessage(fSet, true, SOUND_FILE, fn);
}

QString
WSettings::GetSoundFile() const
{
	return GetStringItem(fSet(), SOUND_FILE);
}

void
WSettings::SetAutoAway(int32 i)
{
	fSet()->ReplaceInt32(true, AUTO_AWAY, i);
}

int32
WSettings::GetAutoAway() const
{
	int32 i = 0;
	fSet()->FindInt32(AUTO_AWAY, &i);
	return i;
}

void
WSettings::SetRegisterTime(int64 i)
{
	fSet()->ReplaceInt64(true, REGISTERTIME, i);
}

void
WSettings::SetRegisterTime(const QString & nick, int64 i)
{
	String ni = String(REGISTERTIME);
	ni += "_";
	ni += (const char *) nick.utf8();
	fSet()->ReplaceInt64(true, ni, i);
};

int64
WSettings::GetRegisterTime() const
{
	int64 i = GetCurrentTime64();
	fSet()->FindInt64(REGISTERTIME, &i);
	return i;
}

int64
WSettings::GetRegisterTime(const QString & nick) const
{
	String ni = String(REGISTERTIME);
	ni += "_";
	ni += (const char *) nick.utf8();
	int64 i = GetCurrentTime64();
	fSet()->FindInt64(ni, &i);
	return i;
}

void
WSettings::SetFlash(int32 flags)
{
	fSet()->ReplaceInt32(true, FLASH, flags);
}

int32
WSettings::GetFlash() const
{
	int32 i = FlashAll;
	fSet()->FindInt32(FLASH, &i);
	return i;
}

void
WSettings::SetEmptyWindows(int32 e)
{
	fSet()->ReplaceInt32(true, EMPTYWIN, e);
}

int32
WSettings::GetEmptyWindows() const
{
	int32 i = 1;
	fSet()->FindInt32(EMPTYWIN, &i);
	return i;
}

bool
WSettings::GetSharingEnabled() const
{
	GET_BOOL(SHARING_ENABLED, true);
}

void
WSettings::SetSharingEnabled(bool b)
{
	SET_BOOL(SHARING_ENABLED, b);
}

int32
WSettings::GetBasePort() const
{
	int32 i = 7000;
	fSet()->FindInt32(BASEPORT, &i);
	return i;
}

void
WSettings::SetBasePort(int32 bp)
{
	fSet()->ReplaceInt32(true, BASEPORT, bp);
}

int32
WSettings::GetPortRange() const
{
	int32 i = 100;
	fSet()->FindInt32(PORTRANGE, &i);
	return i;
}

void
WSettings::SetPortRange(int32 pr)
{
	fSet()->ReplaceInt32(true, PORTRANGE, pr);
}

int32
WSettings::GetMaxUploads() const
{
	int32 i = Five;
	fSet()->FindInt32(MAX_UPLOADS, &i);
	return i;
}

void
WSettings::SetMaxUploads(int32 u)
{
	fSet()->ReplaceInt32(true, MAX_UPLOADS, u);
}

int32
WSettings::GetMaxDownloads() const
{
	int32 i = Five;
	fSet()->FindInt32(MAX_DOWNLOADS, &i);
	return i;
}

void
WSettings::SetMaxDownloads(int32 d)
{
	fSet()->ReplaceInt32(true, MAX_DOWNLOADS, d);
}

int32
WSettings::GetFontSize() const
{
	int32 i = 3;
	fSet()->FindInt32(FONT_SIZE, &i);
	return i;
}

void
WSettings::SetFontSize(int32 f)
{
	fSet()->ReplaceInt32(true, FONT_SIZE, f);
}

QString
WSettings::GetWatchPattern() const
{
	return GetStringItem(fSet, WATCH_PATTERN);
}

void
WSettings::SetWatchPattern(const QString & p)
{
	ReplaceStringInMessage(fSet, true, WATCH_PATTERN, p);
}

QString
WSettings::GetIgnorePattern() const
{
	return GetStringItem(fSet, IGNORE_PATTERN);
}

void
WSettings::SetIgnorePattern(const QString & p)
{
	ReplaceStringInMessage(fSet(), true, IGNORE_PATTERN, p);
}

QString
WSettings::GetIPIgnorePattern() const
{
	return GetStringItem(fSet, IPIGNORE_PATTERN);
}

void
WSettings::SetIPIgnorePattern(const QString & p)
{
	ReplaceStringInMessage(fSet, true, IPIGNORE_PATTERN, p);
}

QString
WSettings::GetBlackListPattern() const
{
	return GetStringItem(fSet, BLACKLIST);
}

QString
WSettings::GetWhiteListPattern() const
{
	return GetStringItem(fSet, WHITELIST);
}

QString
WSettings::GetFilterListPattern() const
{
	return GetStringItem(fSet, FILTERLIST);
}

void
WSettings::SetAutoPrivatePattern(const QString & p)
{
	ReplaceStringInMessage(fSet, true, AUTOPRIV, p);
}
	
QString
WSettings::GetAutoPrivatePattern() const
{
	return GetStringItem(fSet, AUTOPRIV);
}

void
WSettings::SetPMRedirect(const QString & p)
{
	ReplaceStringInMessage(fSet, true, PM_REDIRECT, p);
}

QString
WSettings::GetPMRedirect() const
{
	return GetStringItem(fSet, PM_REDIRECT);
}

void
WSettings::SetBlackListPattern(const QString & p)
{
	ReplaceStringInMessage(fSet, true, BLACKLIST, p);
}

void
WSettings::SetWhiteListPattern(const QString & p)
{
	ReplaceStringInMessage(fSet, true, WHITELIST, p);
}

void
WSettings::SetFilterListPattern(const QString & p)
{
	ReplaceStringInMessage(fSet(), true, FILTERLIST, p);
}

QString
WSettings::GetOnConnect() const
{
	return GetStringItem(fSet, ONCONN);
}

void
WSettings::SetOnConnect(const QString & s)
{
	ReplaceStringInMessage(fSet, true, ONCONN, s);
}

QString
WSettings::GetOnConnect2() const
{
	return GetStringItem(fSet, ONCONN2);
}

void
WSettings::SetOnConnect2(const QString & s)
{
	ReplaceStringInMessage(fSet, true, ONCONN2, s);
}

#ifndef WIN32
void
WSettings::SetFTPLauncher(const QString & l)
{
	ReplaceStringInMessage(fSet, true, FTP_LAUNCHER, l);
}

void
WSettings::SetMailLauncher(const QString & l)
{
	ReplaceStringInMessage(fSet, true, MAILTO_LAUNCHER, l);
}

void
WSettings::SetHTTPLauncher(const QString & l)
{
	ReplaceStringInMessage(fSet, true, HTTP_LAUNCHER, l);
}

void
WSettings::SetDefaultLauncher(const QString & l)
{
	ReplaceStringInMessage(fSet, true, DEFAULT_LAUNCHER, l);
}

QString
WSettings::GetFTPLauncher() const
{
	QString s("konqueror");	// the default for everything is konqueror
	GetStringFromMessage(fSet, FTP_LAUNCHER, s);
	return s;
}

QString
WSettings::GetMailLauncher() const
{
	QString s("konqueror");
	GetStringFromMessage(fSet, MAILTO_LAUNCHER, s);
	return s;
}

QString
WSettings::GetHTTPLauncher() const
{
	QString s("konqueror");
	GetStringFromMessage(fSet, HTTP_LAUNCHER, s);
	return s;
}

QString
WSettings::GetDefaultLauncher() const
{
	QString s("konqueror");
	GetStringFromMessage(fSet, DEFAULT_LAUNCHER, s);
	return s;
}
#endif

int32
WSettings::GetChatLimit() const
{
	int32 i = LimitNone;
	fSet()->FindInt32(CHAT_LIMIT, &i);
	return i;
}

void
WSettings::SetChatLimit(int32 l)
{
	fSet()->ReplaceInt32(true, CHAT_LIMIT, l);
}

int32
WSettings::GetULLimit() const
{
	int32 i = LimitNone;
	fSet()->FindInt32(UL_LIMIT, &i);
	return i;
}

void
WSettings::SetULLimit(int32 l)
{
	fSet()->ReplaceInt32(true, UL_LIMIT, l);
}

int32
WSettings::GetDLLimit() const
{
	int32 i = LimitNone;
	fSet()->FindInt32(DL_LIMIT, &i);
	return i;
}

void
WSettings::SetBLLimit(int32 l)
{
	fSet()->ReplaceInt32(true, BL_LIMIT, l);
}

int32
WSettings::GetBLLimit() const
{
	int32 i = LimitNone;
	fSet()->FindInt32(BL_LIMIT, &i);
	return i;
}

void
WSettings::SetDLLimit(int32 l)
{
	fSet()->ReplaceInt32(true, DL_LIMIT, l);
}

double
WSettings::GetPacketSize() const
{
	double d = 8;
	int32 i = 8; // old format
	if (fSet()->FindInt32(PACKET_SIZE, &i) == B_OK)
	{
		d = i;
	}
	else
	{
		fSet()->FindDouble(PACKET_SIZE, &d);
	}
	return d;
}

void
WSettings::SetPacketSize(double l)
{
	fSet()->RemoveName(PACKET_SIZE);	// For backwards compatibility
	fSet()->AddDouble(PACKET_SIZE, l);
}

int32
WSettings::GetMinQueued() const
{
	int32 i = 0;
	fSet()->FindInt32(MIN_QUEUED, &i);
	return i;
}

int64
WSettings::GetMinQueuedSize()
{
	int64 mqs = 0;
	switch (GetMinQueued())
	{
	case 1:
		{
			mqs = 1024; 
			break;
		}
	case 2: 
		{
			mqs = 2048; 
			break;
		}
	case 3: 
		{
			mqs = 4096; 
			break;
		}
	case 4: 
		{
			mqs = 8192; 
			break;
		}
	case 5: 
		{
			mqs = 16384; 
			break;
		}
	case 6:
		{
			mqs = 32768; 
			break;
		}
	case 7:		
		{
			mqs = 65536; 
			break;
		}
	case 8:
		{
			mqs = 131072;
			break;
		}
	case 9:
		{
			mqs = 262144;
			break;
		}
	case 10:
		{
			mqs = 524288;
			break;
		}
	case 11:
		{
			mqs = 1048576;
			break;
		}
	default:	
		{
			mqs = 0; 
			break;
		}
	}
	return mqs;
}

void
WSettings::SetMinQueued(int32 l)
{
	fSet()->ReplaceInt32(true, MIN_QUEUED, l);
}

uint64
WSettings::GetTransmitStats() const
{
	uint64 t = 0;
	fSet()->FindInt64(TX_STATS, (int64 *)&t);
	return t;
}

void
WSettings::SetTransmitStats(uint64 t)
{
	fSet()->ReplaceInt64(true, TX_STATS, t);
}

uint64
WSettings::GetReceiveStats() const
{
	uint64 r = 0;
	fSet()->FindInt64(RX_STATS, (int64 *)&r);
	return r;
}

void
WSettings::SetReceiveStats(uint64 r)
{
	fSet()->ReplaceInt64(true, RX_STATS, r);
}

int
WSettings::ConvertToBytes(int l)
{
	switch (l)
	{
		case Limit128:		return 128;
		case Limit256:		return 256;
		case Limit512:		return 512;
		case Limit1KB:		return 1024;
		case Limit2KB:		return 1024 * 2;
		case Limit4KB:		return 1024 * 4;
		case Limit8KB:		return 1024 * 8;
		case Limit16KB:		return 1024 * 16;
		case Limit32KB:		return 1024 * 32;
		case Limit64KB:		return 1024 * 64;
		case Limit128KB:	return 1024 * 128;
		case Limit256KB:	return 1024 * 256;
		case Limit512KB:	return 1024 * 512;
		case Limit1MB:		return 1024 * 1024;
		case Limit2MB:		return 1024 * 1024 * 2;
		case Limit4MB:		return 1024 * 1024 * 4;
		case Limit8MB:		return 1024 * 1024 * 8;
		case Limit16MB:		return 1024 * 1024 * 16;
		case Limit32MB:		return 1024 * 1024 * 32;
		default:			return 0;	// if 0 is returned, then don't limit
	}
}

bool
WSettings::GetMaximized() const
{
	GET_BOOL(MAXIMIZED, false);
}

void
WSettings::SetMaximized(bool max)
{
	SET_BOOL(MAXIMIZED, max);
}

bool
WSettings::GetLogging() const
{
	GET_BOOL(LOGGING, true);
}

void
WSettings::SetLogging(bool log)
{
	SET_BOOL(LOGGING, log);
}

void
WSettings::SetNickListSortColumn(int32 c)
{
	fSet()->ReplaceInt32(true, NL_SORTCOL, c);
}

int32
WSettings::GetNickListSortColumn() const
{
	int32 i = 0;
	fSet()->FindInt32(NL_SORTCOL, &i);	// if it fails, who cares, return 0
	return i;
}

bool
WSettings::GetNickListSortAscending() const
{
	GET_BOOL(NL_SORTASC, true);
}

void
WSettings::SetNickListSortAscending(bool a)
{
	SET_BOOL(NL_SORTASC, a);
}

void
WSettings::SetSearchListSortColumn(int32 c)
{
	fSet()->ReplaceInt32(true, SL_SORTCOL, c);
}

int32
WSettings::GetSearchListSortColumn() const
{
	int32 i = 0;
	fSet()->FindInt32(SL_SORTCOL, &i);
	return i;
}

bool
WSettings::GetSearchListSortAscending() const
{
	GET_BOOL(SL_SORTASC, true);
}

void
WSettings::SetSearchListSortAscending(bool a)
{
	SET_BOOL(SL_SORTASC, a);
}

void
WSettings::SetRemotePassword(const QString & pw)
{
	ReplaceStringInMessage(fSet, true, REMOTEPASSWORD, pw);
}

QString
WSettings::GetRemotePassword() const
{
	return GetStringItem(fSet, REMOTEPASSWORD);
}

void
WSettings::AddQueryItem(const QString & str)
{
	AddStringToMessage(fSet, QUERY_LIST, str);
}

QString
WSettings::GetQueryItem(int index) const
{
	return GetStringItem(fSet, QUERY_LIST, index);
}

void
WSettings::EmptyQueryList()
{
	fSet()->RemoveName(QUERY_LIST);
}

void
WSettings::SetCurrentQueryItem(int32 item)
{
	fSet()->ReplaceInt32(true, QUERY_ITEM, item);
}

int32
WSettings::GetCurrentQueryItem() const
{
	int32 i = 0;
	fSet()->FindInt32(QUERY_ITEM, &i);
	return i;
}

void 
WSettings::AddResumeItem(const QString & user, const WResumeInfo & wri)
{
	AddStringToMessage(fSet, RESUMEUSER, user);
	AddStringToMessage(fSet, RESUMEFILE, wri.fRemoteName);
	AddStringToMessage(fSet, RESUMEFIL2, wri.fLocalName);
	AddStringToMessage(fSet, RESUMEPATH, wri.fPath);
}

bool 
WSettings::GetResumeItem(int index, QString & user, WResumeInfo & wri) const
{
	QString file, path;
	if (
		(GetStringFromMessage(fSet, RESUMEFILE, index, file) == B_OK) &&
		(GetStringFromMessage(fSet, RESUMEUSER, index, user) == B_OK) 
		)
	{
		wri.fRemoteName = file;

		// For backwards compatibility don't require entry for local file name or remote path
		//

		if (GetStringFromMessage(fSet, RESUMEFIL2, index, file) == B_OK)
			wri.fLocalName = file;
		else
			wri.fLocalName = QString::null;

		if (GetStringFromMessage(fSet, RESUMEPATH, index, path) == B_OK)
			wri.fPath = path;
		else
			wri.fPath = QString::null;

		return true;
	}
	return false;
}

int32
WSettings::GetResumeCount() const
{
	int32 i = 0;
	fSet()->FindInt32(RESUMELIST, &i);	// if it fails, return 0
	return i;
}

void 
WSettings::SetResumeCount(int32 c)
{
	fSet()->ReplaceInt32(true, RESUMELIST, c);
}

void
WSettings::EmptyResumeList()
{
   fSet()->RemoveName(RESUMEPATH);
	fSet()->RemoveName(RESUMEFIL2);
	fSet()->RemoveName(RESUMEFILE);
	fSet()->RemoveName(RESUMEUSER);
}

void 
WSettings::GetToolBarLayout(int toolbar, int32 & dock, int32 & index, bool & nl, int32 & extra) const
{
	dock = (int) Qt::DockTop;
	index = toolbar;

	if (toolbar == 1)
		nl = true;
	else
		nl = false;

	extra = -1;

	String sToolBar = "toolbar";
	sToolBar << toolbar;

	MessageRef mref;
	if (fSet()->FindMessage(sToolBar, mref) == B_OK)
	{
		mref()->FindInt32("dock", &dock);
		mref()->FindInt32("index", &index);
		mref()->FindBool("nl", &nl);
		mref()->FindInt32("extra", &extra);
	}
}

void 
WSettings::SetToolBarLayout(int toolbar, int32 dock, int32 index, bool nl, int32 extra)
{
	String sToolBar = "toolbar";
	sToolBar << toolbar;

	MessageRef mref(GetMessageFromPool());
	if (mref())
	{
		mref()->AddInt32("dock", dock);
		mref()->AddInt32("index", index);
		mref()->AddBool("nl", nl);
		mref()->AddInt32("extra", extra);

		fSet()->ReplaceMessage(true, sToolBar, mref);
	}
}

int64
WSettings::GetInstallID()
{
	srand(time(NULL));
	uint64 i1 = rand()*(ULONG_MAX/RAND_MAX);
	uint64 i2 = rand()*(ULONG_MAX/RAND_MAX);
	int64 iid = (i1 << 32) + i2;
	fSet()->FindInt64(INSTALLID, &iid);
	return iid;
}

void
WSettings::SetInstallID(int64 iid)
{
	fSet()->ReplaceInt64(true, INSTALLID, iid);
}

void
WSettings::SetLastImageDir(const QString &dir)
{
   ReplaceStringInMessage(fSet, true, IMAGEDIR, dir);
}

QString
WSettings::GetLastImageDir() const
{
   QString dir = GetStringItem(fSet, IMAGEDIR);
   if (dir.isEmpty())
      return downloadDir();
   else
      return dir;
}
