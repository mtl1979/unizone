#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <q3listview.h>
#include <qwidget.h>

#include "util/String.h"

#include "htmlview.h"

using namespace muscle;

class WSettings;
class QListView;
class QString;

typedef enum
{
	MainType,
	PrivateType,
	ChannelType
} ChatType;

class ChatWindow : public Q3MainWindow
{
	Q_OBJECT
public:
	ChatWindow(ChatType type, QWidget* parent, const char* name, Qt::WFlags fl);
	virtual ~ChatWindow();

	WSettings *Settings();

	virtual void LogString(const QString & txt) = 0;
	virtual void LogString(const char * txt) = 0;
	virtual QWidget *Window() = 0;

	void Action(const QString & name, const QString & msg);
	void PrintText(const QString & str);
	void PrintSystem(const QString & msg);
	void PrintError(const QString & error);
	void PrintWarning(const QString & warning);

	void beep();
	void Clear();

	QString FormatNameSaid(const QString & msg); // Check and format message for Name Said...
	// formatting for:
	//	(id) UserName
	QString FormatLocalText(const QString &session, const QString &name, const QString &text);

	QString FormatRemoteName(const QString &session, const QString &name);
	QString FormatRemoteText(const QString &session, const QString &name, const QString &text);
	QString FormatRemoteWatch(const QString &session, const QString &name, const QString &text);

	// text color...
	QString FormatText(const QString &text);
	QString FormatWatch(const QString &text);

	//    |
	//   \ /
	//	System: User #?? is now connected.
	QString FormatSystemText(const QString &text);
	QString FormatUserConnected(const QString &session);
	QString FormatUserDisconnected(const QString &session, const QString &user);
	QString FormatUserNameChangedNoOld(const QString &session, const QString &name);
	QString FormatUserNameChangedNoNew(const QString &session);
	QString FormatUserNameChanged(const QString &session, const QString &oldname, const QString &newname);
	QString FormatUserStatusChanged(const QString &session, const QString &user, const QString &status);
	QString FormatUserIPAddress(const QString &user, const QString &ip);
	QString FormatUserIPAddress2(const QString &session, const QString &ip);

	// ping formatting
	QString FormatPingText(uint32 time, const QString &version);
	QString FormatPingUptime(const QString &uptime, const QString &logged);

	// local stuff
	QString FormatStatusChanged(const QString &status);
	QString FormatNameChanged(const QString &name);

	// private messages
	QString FormatSendPrivMsg(const QString &session, const QString &myname, const QString &othername, const QString &text);

	QString FormatReceivePrivMsg(const QString &session, const QString &othername, const QString &text);

	// URL -- <postmaster@raasu.org> 20020930,20040511
	QString FormatURL(const QString &url);

	// You got pinged
	QString FormatGotPinged(const QString &session, const QString &name);
	QString FormatPingSent(const QString &session, const QString &name);

	QString FormatTimeStamp(const QString &stamp);

	// Private Windows
	QString FormatPrivateRemoved(const QString &session, const QString &name);
	QString FormatPrivateIsBot(const QString &session, const QString &name);

	// UniShare
	QString FormatTimeRequest(const QString &session, const QString &username);

protected:

	WHTMLView * fChatText;

	void InitUserList(Q3ListView * lv);

	// action
	QString FormatAction(const QString &msg);
	QString FormatAction(const QString &name, const QString &msg);

	// error format
	QString FormatError(const QString &text);

	QString ParseString(const QString & str);
	QString FixString(const QString & str);

	QString GetTimeStamp2();
private:

	int GetFontSize();
	QString GetColor(int c);

	QString FormatUserName(const QString &name, const QString &color);

	// warning format
	QString FormatWarning(const QString &text);

	// this is a whole different type of parse... it looks for URL's etc.
	QString ParseChatText(const QString & str);

	// Get nice time stamp ;)
	QString GetTimeStamp();
	QString GetTimeStampAux(const QString & stamp);
	QString GetDateStampAux(const QString & stamp);

	// see if we were named...
	QString _FormatNameSaid(const QString &text);
	bool NameSaid(QString & msg);	// msg will be syntaxed if needed
	bool NameSaid2(const QString &sname, QString & msg, unsigned long index = 0); // Private version for recursing

	QString tr2(const char *s);		// Appends space if not empty
	QString tr3(const QString &s);	// Adds leading space if needed.

	ChatType _type;
	bool _inTable;
};

#endif
