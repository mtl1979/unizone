#ifndef FORMATTING_H
#define FORMATTING_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qstring.h>
#include <qobject.h>

#include "colors.h"
#include "debugimpl.h"
#include "support/MuscleSupport.h"

class WFormat : private QObject
{
	Q_OBJECT
public:
	// formatting for:
	//	(id) UserName
	static QString LocalName(const QString &session, const QString &name);
	static QString RemoteName(const QString &session, const QString &name);

	// text color...
	static QString Text(const QString &text);
	static QString Watch(const QString &text);

	//    |
	//   \ /
	//	System: User #?? is now connected.
	static QString SystemText(const QString &text);
	static QString UserConnected(const QString &session);
	static QString UserDisconnected(const QString &session, const QString &user);
	static QString UserDisconnected2(const QString &session);
	static QString UserNameChangedNoOld(const QString &session, const QString &name);
	static QString UserNameChangedNoNew(const QString &session);
	static QString UserNameChanged(const QString &session, const QString &oldname, const QString &newname);
	static QString UserStatusChanged(const QString &session, const QString &user, const QString &status);
	static QString UserStatusChanged2(const QString &session, const QString &status);
	static QString UserIPAddress(const QString &user, const QString &ip);
	static QString UserIPAddress2(const QString &session, const QString &ip);

	// ping formatting
	static QString PingText(int32 time, const QString &version);
	static QString PingUptime(const QString &uptime, const QString &logged);

	// error format
	static QString Error(const QString &text);

	// warning format
	static QString Warning(const QString &text);

	// local stuff
	static QString StatusChanged(const QString &status);
	static QString NameChanged(const QString &name);

	// private messages
	static QString SendPrivMsg(const QString &session, const QString &myname, const QString &othername);
	static QString ReceivePrivMsg(const QString &session, const QString &othername, const QString &text);

	// action
	static QString Action();

	// URL -- <postmaster@raasu.org> 20020930,20040511
	static QString URL(const QString &url);

	// You got pinged
	static QString GotPinged(const QString &session, const QString &name);

	static QString TimeStamp(const QString &stamp);
};

QString ParseChatText(const QString & str);		// this is a whole different type of
												// parse... it looks for URL's etc.
void ParseString(QString & str);
QString ParseStringStr(const QString & str);

void EscapeHTML(QString & str);					// RUN THIS BEFORE ParseString()
QString EscapeHTMLStr(const QString & str);

void FixString(QString & str);
QString FixStringStr(const QString & str);

#endif	// FORMATTING_H
