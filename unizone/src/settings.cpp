#include "settings.h"
#include "global.h"
#include "iogateway/MessageIOGateway.h"

#include <qfile.h>
#include <qmessagebox.h>

const char * SettingsFile; // name of settings file, default depends on host os

const char *
GetSettingsFile()
{
	if (SettingsFile == NULL)
	{
#ifdef WIN32
		SettingsFile = "settings.ini";
#else
		SettingsFile = ".unizone_settings";
#endif
	}
	return SettingsFile;
}

void
SetSettingsFile(const char * sf)
{
	SettingsFile = sf;
}

WSettings::WSettings()
{
	fColor = fColumn = fStatus = fUser = fServer = 0;
	fSet = GetMessageFromPool();
	//CHECK_PTR(fSet);
}

WSettings::~WSettings()
{
	//delete fSet;
	//fSet = NULL; // <postmaster@raasu.org> 20021027
}

void
WSettings::AddServerItem(const QString & str)
{
	fSet()->AddString(SERVER_LIST, (const char *) str.utf8());
}

QString
WSettings::GetServerItem(int index)
{
	String str;
	if (fSet()->FindString(SERVER_LIST, index, str) == B_OK)
		return QString::fromUtf8(str.Cstr());
	return QString::null;
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
WSettings::SetCurrentServerItem(int item)
{
	fSet()->RemoveName(SERVER_ITEM);
	fSet()->AddInt32(SERVER_ITEM, (int32)item);
}

int
WSettings::GetCurrentServerItem()
{
	int i = 0;
	fSet()->FindInt32(SERVER_ITEM, (int32 *)&i);	// if it fails, who cares, return 0
	return i;
}

// users
void
WSettings::AddUserItem(const QString & str)
{
	fSet()->AddString(USER_LIST, (const char *) str.utf8());
}

QString
WSettings::GetUserItem(int index)
{
	String str;
	if (fSet()->FindString(USER_LIST, index, str) == B_OK)
		return QString::fromUtf8(str.Cstr());
	return QString::null;
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
WSettings::SetCurrentUserItem(int item)
{
	fSet()->RemoveName(USER_ITEM);
	fSet()->AddInt32(USER_ITEM, (int32)item);
}

int
WSettings::GetCurrentUserItem()
{
	int i = 0;
	fSet()->FindInt32(USER_ITEM, (int32 *)&i);
	return i;
}

// status
void
WSettings::AddStatusItem(const QString & str)
{
	fSet()->AddString(STATUS_LIST, (const char *) str.utf8());
}

QString
WSettings::GetStatusItem(int index)
{
	String str;
	if (fSet()->FindString(STATUS_LIST, index, str) == B_OK)
		return QString::fromUtf8(str.Cstr());
	return QString::null;
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
WSettings::SetCurrentStatusItem(int item)
{
	fSet()->RemoveName(STATUS_ITEM);
	fSet()->AddInt32(STATUS_ITEM, (int32)item);
}

int
WSettings::GetCurrentStatusItem()
{
	int i = 0;
	fSet()->FindInt32(STATUS_ITEM, (int32 *)&i);
	return i;
}

// style
void
WSettings::SetStyle(WinShareWindow::Style style)
{
	fSet()->RemoveName(STYLE);
	fSet()->AddInt32(STYLE, (int32)style);
}

WinShareWindow::Style
WSettings::GetStyle()
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
WSettings::AddColumnItem(int i)
{
	fSet()->AddInt32(COLUMN_LIST, i);
}

int
WSettings::GetColumnItem(int index)
{
	int i = -1;
	fSet()->FindInt32(COLUMN_LIST, index, (int32 *)&i);
	return i;
}

int
WSettings::StartColumnIter()
{
	fColumn = 0;
	return GetColumnItem(0);
}

int
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
int
WSettings::GetWindowHeight()
{
	int i = -1;
	fSet()->FindInt32(WINDOW_HEIGHT, (int32 *)&i);
	return i;
}

int
WSettings::GetWindowWidth()
{
	int i = -1;
	fSet()->FindInt32(WINDOW_WIDTH, (int32 *)&i);
	return i;
}

int
WSettings::GetWindowX()
{
	int i = -1;
	fSet()->FindInt32(WINDOW_X, (int32 *)&i);
	return i;
}

int
WSettings::GetWindowY()
{
	int i = -1;
	fSet()->FindInt32(WINDOW_Y, (int32 *)&i);
	return i;
}

void
WSettings::SetWindowHeight(int h)
{
	fSet()->RemoveName(WINDOW_HEIGHT);
	fSet()->AddInt32(WINDOW_HEIGHT, (int32)h);
}

void
WSettings::SetWindowWidth(int w)
{
	fSet()->RemoveName(WINDOW_WIDTH);
	fSet()->AddInt32(WINDOW_WIDTH, (int32)w);
}

void
WSettings::SetWindowX(int x)
{
	fSet()->RemoveName(WINDOW_X);
	fSet()->AddInt32(WINDOW_X, (int32)x);
}

void
WSettings::SetWindowY(int y)
{
	fSet()->RemoveName(WINDOW_Y);
	fSet()->AddInt32(WINDOW_Y, (int32)y);
}

// sizes
QValueList<int>
WSettings::GetChatSizes()
{
	QValueList<int> l;
	int j;
	for (int i = 0; fSet()->FindInt32(CHAT_SIZES, i, (int32 *)&j) == B_OK; i++)
		l.append(j);
	return l;
}

void
WSettings::SetChatSizes(QValueList<int> & sizes)
{
	fSet()->RemoveName(CHAT_SIZES);
	for (unsigned int i = 0; i < sizes.count(); i++)
		fSet()->AddInt32(CHAT_SIZES, (int32)sizes[i]);
}

QValueList<int>
WSettings::GetMainSizes()
{
	QValueList<int> l;
	int j;
	for (int i = 0; fSet()->FindInt32(USER_SIZES, i, (int32 *)&j) == B_OK; i++)
		l.append(j);
	return l;
}

void
WSettings::SetMainSizes(QValueList<int> & sizes)
{
	fSet()->RemoveName(USER_SIZES);
	for (unsigned int i = 0; i < sizes.count(); i++)
		fSet()->AddInt32(USER_SIZES, (int32)sizes[i]);
}

// status messages
void
WSettings::SetAwayMsg(const QString & away)
{
	fSet()->RemoveName(AWAY_MSG);
	fSet()->AddString(AWAY_MSG, (const char *) away.utf8());
}

void
WSettings::SetHereMsg(const QString & here)
{
	fSet()->RemoveName(HERE_MSG);
	fSet()->AddString(HERE_MSG, (const char *) here.utf8());
}

QString
WSettings::GetAwayMsg()
{
	String str("away");	// default
	fSet()->FindString(AWAY_MSG, str);
	return QString::fromUtf8(str.Cstr());
}

QString
WSettings::GetHereMsg()
{
	String str("here");
	fSet()->FindString(HERE_MSG, str);
	return QString::fromUtf8(str.Cstr());
}

// colors
void
WSettings::AddColorItem(const QString & c)
{
	fSet()->AddString(COLORS, (const char *) c.utf8());
}

QString
WSettings::GetColorItem(int index)
{
	String str;
	if (fSet()->FindString(COLORS, index, str) == B_OK)
		return QString::fromUtf8(str.Cstr());
	return QString::null;
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
	QFile file(GetSettingsFile());
	if (file.open(IO_Raw | IO_ReadOnly))
	{
		uint8 * buffer = new uint8[file.size()];
		if (buffer)
		{
			if (file.readBlock((char *)buffer, file.size()) < (long) file.size())
			{
				QMessageBox::warning(NULL, QObject::tr( "Read Error" ), QObject::tr( "Unable to read data from file!" ), QObject::tr( "Bummer" ));
			}
			else
			{
				// reload settings
				fSet()->Unflatten(buffer, file.size());
				ret = true;
			}
			delete [] buffer;
		}
		file.close();
	}
	return ret;
}

bool
WSettings::Save()
{
	bool ret = false;;
	QFile file(GetSettingsFile());
	if (file.open(IO_Raw | IO_WriteOnly))
	{
		uint8 * buffer = new uint8[fSet()->FlattenedSize()];
		if (buffer)
		{
			fSet()->Flatten(buffer);
			file.writeBlock((const char *)buffer, fSet()->FlattenedSize());
			delete [] buffer;
			ret = true;
		}
		file.close();
	}
	else
	{
		QMessageBox::warning(NULL, QObject::tr( "Write Error" ), QObject::tr( "Couldn't save settings!" ), QObject::tr( "Bummer" ));
	}
	return ret;
}

#define SET_BOOL(X, Y) \
	fSet()->RemoveName(X); \
	fSet()->AddBool(X, Y)
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
WSettings::GetAutoUpdateServers()
{
	GET_BOOL(AUTO_UPDATE, true);
}

void
WSettings::SetCheckNewVersions(bool c)
{
	SET_BOOL(CHECK_VERSION, c);
}

bool
WSettings::GetCheckNewVersions()
{
	GET_BOOL(CHECK_VERSION, true);
}

void
WSettings::SetLoginOnStartup(bool s)
{
	SET_BOOL(LOGIN_STARTUP, s);
}

bool
WSettings::GetLoginOnStartup()
{
	GET_BOOL(LOGIN_STARTUP, false);
}

void
WSettings::SetFirewalled(bool f)
{
	SET_BOOL(FIREWALLED, f);
}

bool
WSettings::GetFirewalled()
{
	GET_BOOL(FIREWALLED, false);
}

void
WSettings::SetBinkyNuke(bool b)
{
	SET_BOOL(BINKYNUKE, b);
}

bool
WSettings::GetBinkyNuke()
{
	GET_BOOL(BINKYNUKE, false);
}

void
WSettings::SetBlockDisconnected(bool b)
{
	SET_BOOL(BLOCKDISCONNECTED, b);
}

bool
WSettings::GetBlockDisconnected()
{
	GET_BOOL(BLOCKDISCONNECTED, false);
}

void
WSettings::SetAutoClear(bool b)
{
	SET_BOOL(AUTOCLEAR, b);
}

bool
WSettings::GetAutoClear()
{
	GET_BOOL(AUTOCLEAR, true);
}

void
WSettings::SetMultiColor(bool m)
{
	SET_BOOL(MULTICOLOR, m);
}

bool
WSettings::GetMultiColor()
{
	GET_BOOL(MULTICOLOR, true);
}

void
WSettings::SetConnection(const QString & str)
{
	fSet()->RemoveName(CONNECTION);
	fSet()->AddString(CONNECTION, (const char *)str.utf8());
}

QString
WSettings::GetConnection()
{
	String str = "?";
	fSet()->FindString(CONNECTION, str);
	return QString::fromUtf8(str.Cstr());
}

void 
WSettings::SetEncoding(const QString & server, uint16 port, uint32 encoding)
{
	QString key = server+":"+QString::number(port);
	fSet()->RemoveName((const char *) key.utf8());
	fSet()->AddInt32((const char *) key.utf8(), encoding);
}

uint32
WSettings::GetEncoding(const QString & server, uint16 port)
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
	fSet()->RemoveName((const char *) key.utf8());
	fSet()->AddInt32((const char *) key.utf8(), users);
}

uint32
WSettings::GetMaxUsers(const QString & server, uint16 port)
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
WSettings::GetTimeStamps()
{
	GET_BOOL(TIME_STAMPS, true);
}

void
WSettings::SetUserEvents(bool b)
{
	SET_BOOL(USER_EVENTS, b);
}

bool
WSettings::GetUserEvents()
{
	GET_BOOL(USER_EVENTS, true);
}

void
WSettings::SetIPAddresses(bool b)
{
	SET_BOOL(IPADDRESSES, b);
}

bool
WSettings::GetIPAddresses()
{
	GET_BOOL(IPADDRESSES, false);
}

void
WSettings::SetUploads(bool b)
{
	SET_BOOL(UPLOADS, b);
}

bool
WSettings::GetUploads()
{
	GET_BOOL(UPLOADS, true);
}

void
WSettings::SetDownloads(bool b)
{
	SET_BOOL(DOWNLOADS, b);
}

bool
WSettings::GetDownloads()
{
	GET_BOOL(DOWNLOADS, true);
}

void
WSettings::SetChat(bool b)
{
	SET_BOOL(CHAT, b);
}

bool
WSettings::GetChat()
{
	GET_BOOL(CHAT, true);
}

void
WSettings::SetPrivate(bool b)
{
	SET_BOOL(PRIVATE, b);
}

bool
WSettings::GetPrivate()
{
	GET_BOOL(PRIVATE, true);
}

void
WSettings::SetInfo(bool b)
{
	SET_BOOL(INFO, b);
}

bool
WSettings::GetInfo()
{
	GET_BOOL(INFO, true);
}

void
WSettings::SetWarning(bool b)
{
	SET_BOOL(WARNING, b);
}

bool
WSettings::GetWarning()
{
	GET_BOOL(WARNING, true);
}

void
WSettings::SetError(bool b)
{
	SET_BOOL(ERRORS, b);
}

bool
WSettings::GetError()
{
	GET_BOOL(ERRORS, true);
}

void
WSettings::SetSounds(bool b)
{
	SET_BOOL(SOUNDS, b);
}

bool
WSettings::GetSounds()
{
	GET_BOOL(SOUNDS, true);
}

void
WSettings::SetAutoAway(int i)
{
	fSet()->RemoveName(AUTO_AWAY);
	fSet()->AddInt32(AUTO_AWAY, (int32)i);
}

int
WSettings::GetAutoAway()
{
	int i = 0;
	fSet()->FindInt32(AUTO_AWAY, (int32 *)&i);
	return i;
}

void
WSettings::SetRegisterTime(int64 i)
{
	fSet()->RemoveName(REGISTERTIME);
	fSet()->AddInt64(REGISTERTIME, i);
}

void
WSettings::SetRegisterTime(const QString & nick, int64 i)
{
	String ni = String(REGISTERTIME);
	ni += "_";
	ni += (const char *) nick.utf8();
	fSet()->RemoveName(ni);
	fSet()->AddInt64(ni, i);
};

int64
WSettings::GetRegisterTime()
{
	int64 i = GetCurrentTime64();
	fSet()->FindInt64(REGISTERTIME, &i);
	return i;
}

int64
WSettings::GetRegisterTime(const QString & nick)
{
	String ni = String(REGISTERTIME);
	ni += "_";
	ni += (const char *) nick.utf8();
	int64 i = GetCurrentTime64();
	fSet()->FindInt64(ni, &i);
	return i;
}

void
WSettings::SetFlash(int flags)
{
	fSet()->RemoveName(FLASH);
	fSet()->AddInt32(FLASH, (int32)flags);
}

int
WSettings::GetFlash()
{
	int i = FlashAll;
	fSet()->FindInt32(FLASH, (int32 *)&i);
	return i;
}

void
WSettings::SetEmptyWindows(int e)
{
	fSet()->RemoveName(EMPTYWIN);
	fSet()->AddInt32(EMPTYWIN, (int32)e);
}

int
WSettings::GetEmptyWindows()
{
	int i = 1;
	fSet()->FindInt32(EMPTYWIN, (int32 *)&i);
	return i;
}

bool
WSettings::GetSharingEnabled()
{
	GET_BOOL(SHARING_ENABLED, true);
}

void
WSettings::SetSharingEnabled(bool b)
{
	SET_BOOL(SHARING_ENABLED, b);
}

int
WSettings::GetBasePort()
{
	int i = 7000;
	fSet()->FindInt32(BASEPORT, (int32 *)&i);
	return i;
}

void
WSettings::SetBasePort(int bp)
{
	fSet()->RemoveName(BASEPORT);
	fSet()->AddInt32(BASEPORT, (int32)bp);
}

int
WSettings::GetPortRange()
{
	int i = 100;
	fSet()->FindInt32(PORTRANGE, (int32 *)&i);
	return i;
}

void
WSettings::SetPortRange(int pr)
{
	fSet()->RemoveName(PORTRANGE);
	fSet()->AddInt32(PORTRANGE, (int32)pr);
}

int
WSettings::GetMaxUploads()
{
	int i = Five;
	fSet()->FindInt32(MAX_UPLOADS, (int32 *)&i);
	return i;
}

void
WSettings::SetMaxUploads(int u)
{
	fSet()->RemoveName(MAX_UPLOADS);
	fSet()->AddInt32(MAX_UPLOADS, (int32)u);
}

int
WSettings::GetMaxDownloads()
{
	int i = Five;
	fSet()->FindInt32(MAX_DOWNLOADS, (int32 *)&i);
	return i;
}

void
WSettings::SetMaxDownloads(int d)
{
	fSet()->RemoveName(MAX_DOWNLOADS);
	fSet()->AddInt32(MAX_DOWNLOADS, (int32)d);
}

int
WSettings::GetFontSize()
{
	int i = 3;
	fSet()->FindInt32(FONT_SIZE, (int32 *)&i);
	return i;
}

void
WSettings::SetFontSize(int f)
{
	fSet()->RemoveName(FONT_SIZE);
	fSet()->AddInt32(FONT_SIZE, (int32)f);
}

QString
WSettings::GetWatchPattern()
{
	String w = "";
	fSet()->FindString(WATCH_PATTERN, w);
	return QString::fromUtf8(w.Cstr());
}

void
WSettings::SetWatchPattern(const QString & p)
{
	fSet()->RemoveName(WATCH_PATTERN);
	fSet()->AddString(WATCH_PATTERN, (const char *) p.utf8());
}

QString
WSettings::GetIgnorePattern()
{
	String i = "";
	fSet()->FindString(IGNORE_PATTERN, i);
	return QString::fromUtf8(i.Cstr());
}

void
WSettings::SetIgnorePattern(const QString & p)
{
	fSet()->RemoveName(IGNORE_PATTERN);
	fSet()->AddString(IGNORE_PATTERN, (const char *) p.utf8());
}

QString
WSettings::GetIPIgnorePattern()
{
	String i = "";
	fSet()->FindString(IPIGNORE_PATTERN, i);
	return QString::fromUtf8(i.Cstr());
}

void
WSettings::SetIPIgnorePattern(const QString & p)
{
	fSet()->RemoveName(IPIGNORE_PATTERN);
	fSet()->AddString(IPIGNORE_PATTERN, (const char *) p.utf8());
}

QString
WSettings::GetBlackListPattern()
{
	String i = "";
	fSet()->FindString(BLACKLIST, i);
	return QString::fromUtf8(i.Cstr());
}

QString
WSettings::GetWhiteListPattern()
{
	String i = "";
	fSet()->FindString(WHITELIST, i);
	return QString::fromUtf8(i.Cstr());
}

QString
WSettings::GetFilterListPattern()
{
	String i = "";
	fSet()->FindString(FILTERLIST, i);
	return QString::fromUtf8(i.Cstr());
}

void
WSettings::SetAutoPrivatePattern(const QString & p)
{
	fSet()->RemoveName(AUTOPRIV);
	fSet()->AddString(AUTOPRIV, (const char *) p.utf8());
}
	
QString
WSettings::GetAutoPrivatePattern()
{
	String i = "";
	fSet()->FindString(AUTOPRIV, i);
	return QString::fromUtf8(i.Cstr());
}

void
WSettings::SetBlackListPattern(const QString & p)
{
	fSet()->RemoveName(BLACKLIST);
	fSet()->AddString(BLACKLIST, (const char *) p.utf8());
}

void
WSettings::SetWhiteListPattern(const QString & p)
{
	fSet()->RemoveName(WHITELIST);
	fSet()->AddString(WHITELIST, (const char *) p.utf8());
}

void
WSettings::SetFilterListPattern(const QString & p)
{
	fSet()->RemoveName(FILTERLIST);
	fSet()->AddString(FILTERLIST, (const char *) p.utf8());
}

QString
WSettings::GetOnConnect()
{
	String i = "";
	fSet()->FindString(ONCONN, i);
	return QString::fromUtf8(i.Cstr());
}

void
WSettings::SetOnConnect(const QString & s)
{
	fSet()->RemoveName(ONCONN);
	fSet()->AddString(ONCONN, (const char *) s.utf8());
}

QString
WSettings::GetOnConnect2()
{
	String i = "";
	fSet()->FindString(ONCONN2, i);
	return QString::fromUtf8(i.Cstr());
}

void
WSettings::SetOnConnect2(const QString & s)
{
	fSet()->RemoveName(ONCONN2);
	fSet()->AddString(ONCONN2, (const char *) s.utf8());
}

#if defined(__LINUX__) || defined(linux) || defined(__FreeBSD__)	|| defined(__QNX__)
void
WSettings::SetFTPLauncher(const QString & l)
{
	fSet()->RemoveName(FTP_LAUNCHER);
	fSet()->AddString(FTP_LAUNCHER, (const char *) l.utf8());
}

void
WSettings::SetMailLauncher(const QString & l)
{
	fSet()->RemoveName(MAILTO_LAUNCHER);
	fSet()->AddString(MAILTO_LAUNCHER, (const char *) l.utf8());
}

void
WSettings::SetHTTPLauncher(const QString & l)
{
	fSet()->RemoveName(HTTP_LAUNCHER);
	fSet()->AddString(HTTP_LAUNCHER, (const char *) l.utf8());
}

void
WSettings::SetDefaultLauncher(const QString & l)
{
	fSet()->RemoveName(DEFAULT_LAUNCHER);
	fSet()->AddString(DEFAULT_LAUNCHER, (const char *) l.utf8());
}

QString
WSettings::GetFTPLauncher()
{
	String s = "konqueror";	// the default for everything is konqueror
	fSet()->FindString(FTP_LAUNCHER, s);
	return QString::fromUtf8(s.Cstr());
}

QString
WSettings::GetMailLauncher()
{
	String s = "konqueror";
	fSet()->FindString(MAILTO_LAUNCHER, s);
	return QString::fromUtf8(s.Cstr());
}

QString
WSettings::GetHTTPLauncher()
{
	String s = "konqueror";
	fSet()->FindString(HTTP_LAUNCHER, s);
	return QString::fromUtf8(s.Cstr());
}

QString
WSettings::GetDefaultLauncher()
{
	String s = "konqueror";
	fSet()->FindString(DEFAULT_LAUNCHER, s);
	return QString::fromUtf8(s.Cstr());
}
#endif

int
WSettings::GetChatLimit()
{
	int i = LimitNone;
	fSet()->FindInt32(CHAT_LIMIT, (int32 *)&i);
	return i;
}

void
WSettings::SetChatLimit(int l)
{
	fSet()->RemoveName(CHAT_LIMIT);
	fSet()->AddInt32(CHAT_LIMIT, l);
}

int
WSettings::GetULLimit()
{
	int i = LimitNone;
	fSet()->FindInt32(UL_LIMIT, (int32 *)&i);
	return i;
}

void
WSettings::SetULLimit(int l)
{
	fSet()->RemoveName(UL_LIMIT);
	fSet()->AddInt32(UL_LIMIT, l);
}

int
WSettings::GetDLLimit()
{
	int i = LimitNone;
	fSet()->FindInt32(DL_LIMIT, (int32 *)&i);
	return i;
}

void
WSettings::SetBLLimit(int l)
{
	fSet()->RemoveName(BL_LIMIT);
	fSet()->AddInt32(BL_LIMIT, l);
}

int
WSettings::GetBLLimit()
{
	int i = LimitNone;
	fSet()->FindInt32(BL_LIMIT, (int32 *)&i);
	return i;
}

void
WSettings::SetDLLimit(int l)
{
	fSet()->RemoveName(DL_LIMIT);
	fSet()->AddInt32(DL_LIMIT, l);
}

int
WSettings::GetPacketSize()
{
	int i = 8;
	fSet()->FindInt32(PACKET_SIZE, (int32 *)&i);
	return i;
}

void
WSettings::SetPacketSize(int l)
{
	fSet()->RemoveName(PACKET_SIZE);
	fSet()->AddInt32(PACKET_SIZE, l);
}

int
WSettings::GetMinQueued()
{
	int i = 0;
	fSet()->FindInt32(MIN_QUEUED, (int32 *)&i);
	return i;
}

uint64
WSettings::GetMinQueuedSize()
{
	uint64 mqs = 0;
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
WSettings::SetMinQueued(int l)
{
	fSet()->RemoveName(MIN_QUEUED);
	fSet()->AddInt32(MIN_QUEUED, l);
}

uint64
WSettings::GetTransmitStats()
{
	uint64 t = 0;
	fSet()->FindInt64(TX_STATS, (int64 *)&t);
	return t;
}

void
WSettings::SetTransmitStats(uint64 t)
{
	fSet()->RemoveName(TX_STATS);
	fSet()->AddInt64(TX_STATS, t);
}

uint64
WSettings::GetReceiveStats()
{
	uint64 r = 0;
	fSet()->FindInt64(RX_STATS, (int64 *)&r);
	return r;
}

void
WSettings::SetReceiveStats(uint64 r)
{
	fSet()->RemoveName(RX_STATS);
	fSet()->AddInt64(RX_STATS, r);
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
WSettings::SetNickListSortColumn(int c)
{
	fSet()->RemoveName(NL_SORTCOL);
	fSet()->AddInt32(NL_SORTCOL, (int32)c);
}

int
WSettings::GetNickListSortColumn()
{
	int i = 0;
	fSet()->FindInt32(NL_SORTCOL, (int32 *)&i);	// if it fails, who cares, return 0
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
WSettings::SetSearchListSortColumn(int c)
{
	fSet()->RemoveName(SL_SORTCOL);
	fSet()->AddInt32(SL_SORTCOL, (int32)c);
}

int
WSettings::GetSearchListSortColumn()
{
	int i = 0;
	fSet()->FindInt32(SL_SORTCOL, (int32 *)&i);
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
	fSet()->RemoveName(REMOTEPASSWORD);
	fSet()->AddString(REMOTEPASSWORD, (const char *) pw.utf8());
}

QString
WSettings::GetRemotePassword()
{
	String pw = "";	// the default for everything is konqueror
	fSet()->FindString(REMOTEPASSWORD, pw);
	return QString::fromUtf8(pw.Cstr());
}

void
WSettings::AddQueryItem(const QString & str)
{
	fSet()->AddString(QUERY_LIST, (const char *) str.utf8());
}

QString
WSettings::GetQueryItem(int index)
{
	String str;
	if (fSet()->FindString(QUERY_LIST, index, str) == B_OK)
		return QString::fromUtf8(str.Cstr());
	return QString::null;
}

void
WSettings::EmptyQueryList()
{
	fSet()->RemoveName(QUERY_LIST);
}

void
WSettings::SetCurrentQueryItem(int item)
{
	fSet()->RemoveName(QUERY_ITEM);
	fSet()->AddInt32(QUERY_ITEM, (int32)item);
}

int
WSettings::GetCurrentQueryItem()
{
	int i = 0;
	fSet()->FindInt32(QUERY_ITEM, (int32 *)&i);
	return i;
}

void 
WSettings::AddResumeItem(WResumePair wrp)
{
	fSet()->AddString(RESUMEUSER, (const char *) wrp.first.utf8());
	fSet()->AddString(RESUMEFILE, (const char *) wrp.second.fRemoteName.utf8());
	fSet()->AddString(RESUMEFIL2, (const char *) wrp.second.fLocalName.utf8());
}

bool 
WSettings::GetResumeItem(int index, WResumePair & wrp)
{
	String file;
	String user;
	if (
		(fSet()->FindString(RESUMEFILE, index, file) == B_OK) &&
		(fSet()->FindString(RESUMEUSER, index, user) == B_OK)
		)
	{
		wrp.first = QString::fromUtf8(user.Cstr());
		wrp.second.fRemoteName = QString::fromUtf8(file.Cstr());

		// For backwards compatibility don't require entry for local file name
		//

		if (fSet()->FindString(RESUMEFIL2, index, file) == B_OK)
			wrp.second.fLocalName = QString::fromUtf8(file.Cstr());
		else
			wrp.second.fLocalName = QString::null;

		return true;
	}
	return false;
}

int
WSettings::GetResumeCount()
{
	int i = 0;
	fSet()->FindInt32(RESUMELIST, (int32 *)&i);	// if it fails, return 0
	return i;
}

void 
WSettings::SetResumeCount(int c)
{
	fSet()->RemoveName(RESUMELIST);
	fSet()->AddInt32(RESUMELIST, (int32)c);
}

void
WSettings::EmptyResumeList()
{
	fSet()->RemoveName(RESUMEFIL2);
	fSet()->RemoveName(RESUMEFILE);
	fSet()->RemoveName(RESUMEUSER);
}

void 
WSettings::GetToolBarLayout(int toolbar, int & dock, int & index, bool & nl, int & extra)
{
#ifdef WIN32
	dock = (int) QMainWindow::ToolBarDock::Top;
#else
	dock = (int) QMainWindow::Top;
#endif
	index = toolbar;

	if (toolbar == 1)
		nl = true;
	else
		nl = false;

	extra = -1;

	String sToolBar = "toolbar";
	sToolBar += toolbar;

	MessageRef mref;
	if (fSet()->FindMessage(sToolBar, mref) == B_OK)
	{
		mref()->FindInt32("dock", (int32 *) &dock);
		mref()->FindInt32("index", (int32 *) &index);
		mref()->FindBool("nl", &nl);
		mref()->FindInt32("extra", (int32 *) &extra);
	}
}

void 
WSettings::SetToolBarLayout(int toolbar, int dock, int index, bool nl, int extra)
{
	String sToolBar = "toolbar";
	sToolBar += toolbar;

	fSet()->RemoveName(sToolBar);
	
	MessageRef mref(GetMessageFromPool());
	if (mref())
	{
		mref()->AddInt32("dock", dock);
		mref()->AddInt32("index", index);
		mref()->AddBool("nl", nl);
		mref()->AddInt32("extra", extra);

		fSet()->AddMessage(sToolBar, mref);
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
	fSet()->RemoveName(INSTALLID);
	fSet()->AddInt64(INSTALLID, iid);
}
