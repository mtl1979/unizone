#include "formatting.h"
#include "platform.h"       // <postmaster@raasu.org> 20021114
#include "settings.h"		//
#include "global.h"			// <postmaster@raasu.org> 20021217

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

int 
GetFontSize()
{
	return gWin->fSettings->GetFontSize();
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
WFormat::LocalName(const QString &session, const QString &name)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<b>(%1)</b>").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::LocalName);
	temp += "<b>";
	temp += name; 
	temp += "</b>";
	temp += "</font>: </font>";
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
	return temp;
}

// text color...
QString 
WFormat::Text(const QString &text)
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Text).arg(GetFontSize());
	temp += text;
	temp += "</font>";
	return temp;
}

// watch color...
QString
WFormat::Watch(const QString &text)
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Watch).arg(GetFontSize());
	temp += text;
	temp += "</font>";
	return temp;
}

//    |
//   \ /
//	System: User #?? is now connected.
QString 
WFormat::SystemText(const QString &text)
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::System).arg(GetFontSize());
	temp += tr("<b>System:</b>");
	temp += " </font>";
	temp += tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Text).arg(GetFontSize());
	temp += text;
	temp += "</font>";
	return temp;
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
	return temp;
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
	temp += "</font>.";
	return temp;
}

QString WFormat::UserNameChangedNoNew(const QString &session)
{
	QString temp = tr("User #%1 is now").arg(session);
	temp += " ";
	temp += tr("nameless");
	temp += ".";
	return temp;
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
	temp += "</font>.";
	return temp;
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
	temp += ".";
	return temp;
}

QString WFormat::UserStatusChanged2(const QString &session, const QString &status)
{
	QString temp = tr("User #%1 is now").arg(session);
	temp += " ";
	temp += status;
	temp += ".";
	return temp;
}

QString WFormat::UserIPAddress(const QString &user, const QString &ip)
{
	QString temp = tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += user;
	temp += "</font>";
	temp += tr("'s IP address is %1.").arg(ip);
	return temp;
}

QString WFormat::UserIPAddress2(const QString &session, const QString &ip)
{
	return tr("User #%1's IP address is %2.").arg(session).arg(ip);
}

// ping formatting
QString WFormat::PingText(int32 time, const QString &version)
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Ping).arg(GetFontSize());
	temp += tr("Ping returned in %1 milliseconds").arg(time);
	temp += " (";
	temp += version;
	temp += ")</font>";
	return temp;
}

QString WFormat::PingUptime(const QString &uptime, const QString &logged)
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Ping).arg(GetFontSize());
	temp += tr("Uptime: %1, Logged on for %2").arg(uptime).arg(logged);
	temp += "</font>";
	return temp;
}

// error format
QString WFormat::Error(const QString &text)
{
	int font = GetFontSize();
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Error).arg(font);
	temp += tr("<b>Error:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\" size=\"%2\">").arg(WColors::ErrorMsg).arg(font);
	temp += text;
	temp += "</font>";
	return temp;
}
// warning format
QString WFormat::Warning(const QString &text)
{
	int font = GetFontSize();
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Warning).arg(font);
	temp += tr("<b>Warning:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\" size=\"%2\">").arg(WColors::WarningMsg).arg(font);
	temp += text;
	temp += "</font>";
	return temp;
}

QString 
WFormat::StatusChanged(const QString &status)
{
	QString temp = tr("You are now");
	temp += " ";
	temp += status;
	temp += ".";
	return temp;
}

QString
WFormat::NameChanged(const QString &name)
{
	QString temp = tr("Name changed to");
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::LocalName);
	temp += name;
	temp += "</font>.";
	return temp;
}

// priv messages

// <postmaster@raasu.org> 20020930,20030127 
// Changed that ID is always black, uid in --

QString 
WFormat::SendPrivMsg(const QString &session, const QString &myname, const QString &othername)
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
	temp += ": </font>";	
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
	temp += "</font>: </font>";
	temp += tr("<font color=\"%1\" size=\"%2\">").arg(WColors::PrivText).arg(GetFontSize());
	temp += text;
	temp += "</font>";
	return temp;
}

QString WFormat::Action()
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Action).arg(GetFontSize());
	temp += tr("<b>Action:</b>");
	temp += "</font> ";
	return temp;
}

// <postmaster@raasu.org> 20020930
// WFormat::URL Doesn't work because % is a valid character in URLs

QString 
WFormat::URL(const QString &url)
{ 
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::URL).arg(GetFontSize());
	temp += "<u>";
	temp += url;
	temp += "</u></font>";
	return temp;
}

QString WFormat::GotPinged(const QString &session, const QString &name)
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Ping).arg(GetFontSize());
	temp += tr("User #%1 (a.k.a.").arg(session);
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += name;
	temp += "</font>";
	temp += tr(") pinged you.");
	temp += "</font>";
	return temp;
}

QString 
WFormat::TimeStamp(const QString &stamp)
{
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Text).arg(GetFontSize());
	temp += "<b>";
	temp += stamp;
	temp += "</b></font> ";
	return temp;
}

QString
WFormat::TimeRequest(const QString &username)
{
	QString temp = tr("Time request sent to");
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += username;
	temp += "</font>."; 
	return temp;						
}
