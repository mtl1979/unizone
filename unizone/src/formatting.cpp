#include "formatting.h"
#include "platform.h"       // <postmaster@raasu.org> 20021114
#include "settings.h"		//
#include "global.h"			// <postmaster@raasu.org> 20021217

#if (QT_VERSION < 0x030000)
#ifndef QT_NO_TRANSLATION
#include <qapplication.h>
#endif
#endif

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

int 
GetFontSize()
{
	return gWin->fSettings->GetFontSize();
}

QString
WFormat::tr2(const char *s)
{
	QString temp = tr(s).stripWhiteSpace();
	if (temp.length() > 0)
		temp += " ";
	return temp;
}

QString 
WFormat::tr3(const QString &s)
{
	QString temp(" ");
	temp += s;
	return temp.stripWhiteSpace();
}

// formatting for:
//	(id) UserName

// <postmaster@raasu.org> 20020930
//
// Change that ID is always in black and colon isn't part of nick ;)

// <postmaster@raasu.org> 20040511
//
// We can't use tr() with URLs, it just doesn't work!!!
// This includes user names and status texts

QString
WFormat::LocalText(const QString &session, const QString &name, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "<b>(";
	temp += session;
	temp += ")</b> ";
	temp += tr("<font color=\"%1\">").arg(WColors::LocalName);
	temp += "<b>";
	temp += name;
	temp += "</b>";
	temp += "</font>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(WColors::Text);
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString 
WFormat::RemoteName(const QString &session, const QString &name)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<b>(%1)</b>").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += "<b>" + name + "</b>";
	temp += "</font>: </font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::RemoteText(const QString &session, const QString &name, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "<b>(";
	temp += session;
	temp += ")</b> ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += "<b>";
	temp += name;
	temp += "</b>";
	temp += "</font>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(WColors::Text);
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString
WFormat::RemoteWatch(const QString &session, const QString &name, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "<b>(";
	temp += session;
	temp += ")</b> ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += "<b>";
	temp += name;
	temp += "</b>";
	temp += "</font>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(WColors::Watch);
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

// text color...
QString 
WFormat::Text(const QString &text)
{
	QString temp = tr("<font color=\"%1\">").arg(WColors::Text);
	temp += tr("<font size=\"%1\">").arg(GetFontSize());
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp.stripWhiteSpace();
}

// watch color...
QString
WFormat::Watch(const QString &text)
{
	QString temp = tr("<font color=\"%1\">").arg(WColors::Watch);
	temp += tr("<font size=\"%1\">").arg(GetFontSize());
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp.stripWhiteSpace();
}

//    |
//   \ /
//	System: User #?? is now connected.
QString 
WFormat::SystemText(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::System);
	temp += tr("<b>System:</b>");
	temp += "</font>";
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::Text);
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp.stripWhiteSpace();
}

QString WFormat::UserConnected(const QString &session)
{
	return tr("User #%1 is now connected.").arg(session);
}

QString WFormat::UserDisconnected(const QString &session, const QString &user)
{
	QString temp = tr("User #%1 (a.k.a.").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += user;
	temp += "</font>";
	temp += tr(") has disconnected.");
	return temp.stripWhiteSpace();
}

QString WFormat::UserDisconnected2(const QString &session)
{
	return tr("User #%1 has disconnected.").arg(session);
}

QString WFormat::UserNameChangedNoOld(const QString &session, const QString &name)
{
	QString temp = tr("User #%1 is now known as").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += name;
	temp += "</font>";
	temp += tr3(tr(".", "'is now known as' suffix"));
	return temp.stripWhiteSpace();
}

QString WFormat::UserNameChangedNoNew(const QString &session)
{
	QString temp = tr("User #%1 is now").arg(session);
	temp += " ";
	temp += tr("nameless");
	temp += tr3(tr(".", "'is now nameless' suffix"));
	return temp.stripWhiteSpace();
}

QString WFormat::UserNameChanged(const QString &session, const QString &oldname, const QString &newname)
{
	QString temp = tr("User #%1 (a.k.a.").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += oldname;
	temp += "</font>";
	temp += tr(") is now known as");
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += newname;
	temp += "</font>";
	temp += tr3(tr(".", "'is now known as' suffix"));
	return temp.stripWhiteSpace();
}

QString WFormat::UserStatusChanged(const QString &session, const QString &user, const QString &status)
{
	QString temp = tr("User #%1 (a.k.a.").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += user;
	temp += "</font>";
	temp += tr(") is now");
	temp += " ";
	temp += status;
	temp += tr3(tr(".", "'is now' suffix"));
	return temp.stripWhiteSpace();
}

QString WFormat::UserStatusChanged2(const QString &session, const QString &status)
{
	QString temp = tr("User #%1 is now").arg(session);
	temp += " ";
	temp += status;
	temp += tr3(tr(".", "'is now' suffix"));
	return temp.stripWhiteSpace();
}

QString WFormat::UserIPAddress(const QString &user, const QString &ip)
{
	QString temp = tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += user;
	temp += "</font>";
	temp += tr("'s IP address is %1.").arg(ip);
	return temp.stripWhiteSpace();
}

QString WFormat::UserIPAddress2(const QString &session, const QString &ip)
{
	return tr("User #%1's IP address is %2.").arg(session).arg(ip);
}

// ping formatting
QString WFormat::PingText(int32 time, const QString &version)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::Ping);
	temp += tr("Ping returned in %1 milliseconds").arg(time);
	temp += " (";
	temp += version;
	temp += ")</font></font>";
	return temp.stripWhiteSpace();
}

QString WFormat::PingUptime(const QString &uptime, const QString &logged)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::Ping);
	temp += tr("Uptime: %1, Logged on for %2").arg(uptime).arg(logged);
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

// error format
QString WFormat::Error(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::Error);
	temp += tr("<b>Error:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\">").arg(WColors::ErrorMsg);
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}
// warning format
QString WFormat::Warning(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::Warning);
	temp += tr("<b>Warning:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\">").arg(WColors::WarningMsg);
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString 
WFormat::StatusChanged(const QString &status)
{
	QString temp = tr2(QT_TRANSLATE_NOOP("WFormat", "You are now"));
	temp += status;
	temp += tr3(tr(".", "'You are now' suffix"));
	return temp.stripWhiteSpace();
}

QString
WFormat::NameChanged(const QString &name)
{
	QString temp = tr2(QT_TRANSLATE_NOOP("WFormat", "Name changed to"));
	temp += tr("<font color=\"%1\">").arg(WColors::LocalName);
	temp += name;
	temp += "</font>";
	temp += tr3(tr(".", "'Name changed to' suffix"));
	return temp.stripWhiteSpace();
}

// priv messages

// <postmaster@raasu.org> 20020930,20030127 
// Changed that ID is always black, uid in --

QString
WFormat::SendPrivMsg(const QString &session, const QString &myname, const QString &othername, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "-";
	temp += "<b>";
	temp += session;
	temp += "</b>";
	temp += "- ";
	temp += tr("<font color=\"%1\">").arg(WColors::LocalName);
	temp += "<b>";
	temp += myname;
	temp += "</b>";
	temp += "</font>";
	temp += " -> ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += "<b>";
	temp += othername;
	temp += "</b>";
	temp += "</font>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(WColors::Text);
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString WFormat::ReceivePrivMsg(const QString &session, const QString &othername, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "-";
	temp += "<b>";
	temp += session;
	temp += "</b>";
	temp += "- ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += "<b>";
	temp += othername;
	temp += "</b>";
	temp += "</font>: ";
	temp += tr("<font color=\"%1\">").arg(WColors::PrivText);
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::Action(const QString &msg)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::Action);
	temp += tr("<b>Action:</b>");
	temp += "</font>";
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::Text);
	temp += msg;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString
WFormat::Action(const QString &name, const QString &msg)
{
	QString temp(name);
	temp += " ";
	temp += msg;
	return Action(temp);
}

QString 
WFormat::URL(const QString &url)
{ 
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::URL);
	temp += "<u>";
	temp += url;
	temp += "</u></font></font>";
	return temp.stripWhiteSpace();
}

QString WFormat::GotPinged(const QString &session, const QString &name)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::Ping);
	temp += tr("User #%1 (a.k.a.").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += name;
	temp += "</font>";
	temp += tr(") pinged you.");
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString 
WFormat::TimeStamp(const QString &stamp)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(WColors::Text);
	temp += "<b>";
	temp += stamp;
	temp += "</b></font> </font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::TimeRequest(const QString &username)
{
	QString temp = tr("Time request sent to");
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += username;
	temp += "</font>."; 
	return temp.stripWhiteSpace();						
}

#if (QT_VERSION < 0x030000)
//
// Qt 2.x seems to miss this definition
//
#ifndef QT_NO_TRANSLATION
QString WFormat::tr(const char* s, const char* c)
{
    return ((QNonBaseApplication*)qApp)->translate("WFormat",s,c);
}
#endif
#endif
