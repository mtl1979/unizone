#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "formatting.h"
#include "platform.h"       // <postmaster@raasu.org> 20021114
#include "settings.h"		//
#include "global.h"			// <postmaster@raasu.org> 20021217

#include <qapplication.h>

int 
GetFontSize()
{
	return gWin->fSettings->GetFontSize();
}

QString
GetColor(int c)
{
	return gWin->fSettings->GetColorItem(c);
}

QString
WFormat::tr2(const char *s)
{
	QString temp = tr(s).stripWhiteSpace();

	if (!temp.isEmpty())
	{
		temp += " ";
	}
	return temp;
}

QString 
WFormat::tr3(const QString &s)
{
	QString temp = s.stripWhiteSpace();
	if (temp != QString("."))
		return temp.prepend(" ");
	else
		return QString(".");
}

QString
FormatUserName(const QString &name, const QString &color)
{
	QString out;
	if (name.startsWith("<font"))
	{
		int e = name.find("</font>");
		int pos = name.find(">") + 1;
		if (e == name.length() - 7)
			return name;
		out = name.left(e + 7);
		out += FormatUserName(name.mid(e + 7), color);
	}
	else
	{
		int tag = name.find("<font");
		if (tag > 0)
		{
			out = WFormat::tr("<font color=\"%1\">").arg(color);
			out += name.left(tag);
			out += "</font>";
			out += FormatUserName(name.mid(tag), color);
		}
		else
		{
			out = WFormat::tr("<font color=\"%1\">").arg(color);
			out += name;
			out += "</font>";
		}
	}
	return out;
}

bool
CheckName(const QString &name)
{
	QString temp = name.stripWhiteSpace();
	if (temp.isEmpty())
		return false;
	if (temp == qApp->translate("WUser", "Unknown"))
		return false;
	if (temp == "Unknown")
		return false;
	return true;
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
	temp += "<b>";
	temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::LocalName));
	temp += "</b>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
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
	if (CheckName(name))
	{
		temp += " ";
		temp += "<b>";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += "</b>";
	}
	temp += ": </font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::RemoteText(const QString &session, const QString &name, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "<b>(";
	temp += session;
	temp += ")</b>";
	if (CheckName(name))
	{
		temp += " ";
		temp += "<b>";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += "</b>";
	}
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
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
	temp += ")</b>";
	if (CheckName(name))
	{
		temp += " ";
		temp += "<b>";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += "</b>";
	}
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Watch));
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

// text color...
QString 
WFormat::Text(const QString &text)
{
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
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
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::Watch));
	temp += tr("<font size=\"%1\">").arg(GetFontSize());
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::NameSaid(const QString &text)
{
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::NameSaid));
	temp += text;
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
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::System));
	temp += tr("<b>System:</b>");
	temp += "</font>";
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
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
	QString temp;
	if (CheckName(user))
	{
		temp  = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(user.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp  = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("has disconnected.");
	return temp.stripWhiteSpace();
}

QString WFormat::UserNameChangedNoOld(const QString &session, const QString &name)
{
	QString temp = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("is now known as");
	temp += " ";
	temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
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
	QString temp = tr("User #%1 (a.k.a").arg(session);
	temp += " ";
	temp += FormatUserName(oldname.stripWhiteSpace(), GetColor(WColors::RemoteName));
	temp += tr(")", "aka suffix");
	temp += " ";
	temp += tr("is now known as");
	temp += " ";
	temp += FormatUserName(newname.stripWhiteSpace(), GetColor(WColors::RemoteName));
	temp += tr3(tr(".", "'is now known as' suffix"));
	return temp.stripWhiteSpace();
}

QString WFormat::UserStatusChanged(const QString &session, const QString &user, const QString &status)
{
	QString temp;
	if (CheckName(user))
	{
		temp  = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(user.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp  = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("is now");
	temp += " ";
	temp += status;		// No need to strip spaces, it was done before translating status...
	temp += tr3(tr(".", "'is now' suffix"));
	return temp.stripWhiteSpace();
}

QString WFormat::UserIPAddress(const QString &user, const QString &ip)
{
	QString temp = tr2(QT_TRANSLATE_NOOP("WFormat", "ip_prefix"));
	temp += FormatUserName(user.stripWhiteSpace(), GetColor(WColors::RemoteName));
	if (tr("ip_space","Need space after username in IP address string?") == QString("yes")) 
		temp += " ";
	temp += tr("'s IP address is %1.").arg(ip);
	return temp.stripWhiteSpace();
}

QString WFormat::UserIPAddress2(const QString &session, const QString &ip)
{
	QString temp = tr("User #%1's IP address is %2.").arg(session).arg(ip);
	return temp.stripWhiteSpace();
}

// ping formatting
QString WFormat::PingText(uint32 time, const QString &version)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Ping));
	temp += tr("Ping returned in %1 milliseconds").arg(time);
	temp += " (";
	temp += version;
	temp += ")</font></font>";
	return temp.stripWhiteSpace();
}

QString WFormat::PingUptime(const QString &uptime, const QString &logged)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Ping));
	temp += tr("Uptime: %1, Logged on for %2").arg(uptime).arg(logged);
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

// error format
QString WFormat::Error(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Error));
	temp += tr("<b>Error:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::ErrorMsg));
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}
// warning format
QString WFormat::Warning(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Warning));
	temp += tr("<b>Warning:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::WarningMsg));
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
	temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::LocalName));
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
	temp += "<b>";
	temp += FormatUserName(myname.stripWhiteSpace(), GetColor(WColors::LocalName));
	temp += "</b>";
	temp += " -> ";
	temp += "<b>";
	if (CheckName(othername))
		temp += FormatUserName(othername.stripWhiteSpace(), GetColor(WColors::RemoteName));
	else
		temp += FormatUserName(qApp->translate("WUser", "Unknown"), GetColor(WColors::RemoteName));
	temp += "</b>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
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
	temp += "<b>";
	if (CheckName(othername))
		temp += FormatUserName(othername.stripWhiteSpace(), GetColor(WColors::RemoteName));
	else
		temp += FormatUserName(qApp->translate("WUser", "Unknown"), GetColor(WColors::RemoteName));
	temp += "</b>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::PrivText));
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::Action(const QString &msg)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Action));
	temp += tr("<b>Action:</b>");
	temp += "</font>";
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += msg;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString
WFormat::Action(const QString &name, const QString &msg)
{
	QString temp(name.stripWhiteSpace());
	temp += " ";
	temp += msg;
	return Action(temp);
}

QString 
WFormat::URL(const QString &url)
{ 
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::URL));
	temp += "<u>";
	temp += url;
	temp += "</u></font>";
	return temp.stripWhiteSpace();
}

QString WFormat::GotPinged(const QString &session, const QString &name)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Ping));
	if (CheckName(name))
	{
		temp += tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp += tr("User #%1").arg(session);
	temp += " ";
	temp += tr("pinged you.");
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::PingSent(const QString &session, const QString &name)
{
	QString temp = tr("Ping sent to");
	temp += " ";
	if (CheckName(name))
	{
		temp += tr("user #%1 (a.k.a","Ping sent to user...").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		if (tr(")", "aka suffix") != QString(","))
			temp += tr(")", "aka suffix");
	}
	else
		temp += tr("user #%1", "Ping sent to user...").arg(session);
	temp += ".";
	return temp.stripWhiteSpace();
}

QString 
WFormat::TimeStamp(const QString &stamp)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += "<b>";
	temp += stamp;
	temp += "</b></font> </font>";
	return temp.stripWhiteSpace();
}

QString
WFormat::TimeRequest(const QString &session, const QString &username)
{
	QString temp = tr("Time request sent to");
	temp += " ";
	if (CheckName(username))
	{
		temp += tr("user #%1 (a.k.a","Ping sent to user...").arg(session);
		temp += " ";
		temp += FormatUserName(username.stripWhiteSpace(), GetColor(WColors::RemoteName));
		if (tr(")", "aka suffix") != QString(","))
			temp += tr(")", "aka suffix");
	}
	else
		temp += tr("user #%1", "Ping sent to user...").arg(session);
	temp += "."; 
	return temp.stripWhiteSpace();						
}

QString
WFormat::PrivateIsBot(const QString &session, const QString &name)
{
	QString temp;
	if (CheckName(name))
	{
		temp = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("is a bot!");
	return temp.stripWhiteSpace();
}

QString
WFormat::PrivateRemoved(const QString &session, const QString &name)
{
	QString temp;
	if (CheckName(name))
	{
		temp = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("was removed from the private chat window.");
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
