#ifndef FORMATTING_H
#define FORMATTING_H

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
	static QString LocalText(const QString &session, const QString &name, const QString &text);

	static QString RemoteName(const QString &session, const QString &name);
	static QString RemoteText(const QString &session, const QString &name, const QString &text);
	static QString RemoteWatch(const QString &session, const QString &name, const QString &text);

	// text color...
	static QString Text(const QString &text);
	static QString Watch(const QString &text);
	static QString NameSaid(const QString &text);

	//    |
	//   \ /
	//	System: User #?? is now connected.
	static QString SystemText(const QString &text);
	static QString UserConnected(const QString &session);
	static QString UserDisconnected(const QString &session, const QString &user);
	static QString UserNameChangedNoOld(const QString &session, const QString &name);
	static QString UserNameChangedNoNew(const QString &session);
	static QString UserNameChanged(const QString &session, const QString &oldname, const QString &newname);
	static QString UserStatusChanged(const QString &session, const QString &user, const QString &status);
	static QString UserIPAddress(const QString &user, const QString &ip);
	static QString UserIPAddress2(const QString &session, const QString &ip);

	// ping formatting
	static QString PingText(uint32 time, const QString &version);
	static QString PingUptime(const QString &uptime, const QString &logged);

	// error format
	static QString Error(const QString &text);

	// warning format
	static QString Warning(const QString &text);

	// local stuff
	static QString StatusChanged(const QString &status);
	static QString NameChanged(const QString &name);

	// private messages
	static QString SendPrivMsg(const QString &session, const QString &myname, const QString &othername, const QString &text);

	static QString ReceivePrivMsg(const QString &session, const QString &othername, const QString &text);

	// action
	static QString Action(const QString &msg);
	static QString Action(const QString &name, const QString &msg);

	// URL -- <postmaster@raasu.org> 20020930,20040511
	static QString URL(const QString &url);

	// You got pinged
	static QString GotPinged(const QString &session, const QString &name);
	static QString PingSent(const QString &session, const QString &name);

	static QString TimeStamp(const QString &stamp);
	
	// Private Windows
	static QString PrivateRemoved(const QString &session, const QString &name);
	static QString PrivateIsBot(const QString &session, const QString &name);

	// UniShare
	static QString TimeRequest(const QString &session, const QString &username);

	static QString tr2(const char *s);	// Appends space if not empty
	static QString tr3(const QString &s);	// Adds leading space if needed.
};

bool CheckName(const QString &name);

#endif	// FORMATTING_H
