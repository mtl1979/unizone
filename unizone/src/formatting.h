#ifndef FORMATTING_H
#define FORMATTING_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qstring.h>
#include <qobject.h>

#include "colors.h"
#include "debugimpl.h"

class WFormat
{
public:
	// formatting for:
	//	(id) UserName
	static QString LocalName;
	static QString RemoteName;

	// text color...
	static QString Text;

	//    |
	//   \ /
	//	System: User #?? is now connected.
	static QString SystemText;
	static QString UserConnected;
	static QString UserDisconnected;
	static QString UserNameChangedNoOld;
	static QString UserNameChanged;
	static QString UserStatusChanged;

	// ping formatting
	static QString PingText;
	static QString PingUptime;

	// error format
	static QString Error;
	// error text color (just regular text)
	static QString ErrorMsg;

	// local stuff
	static QString StatusChanged;

	// private messages
	static QString SendPrivMsg;
	static QString ReceivePrivMsg;

	// action
	static QString Action;

	// URL -- <postmaster@raasu.org> 20020930
	static QString URL1;
	static QString URL2;

	// You got pinged
	static QString GotPinged;

	static QString TimeStamp;
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
