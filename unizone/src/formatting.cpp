#include "formatting.h"
#include "platform.h"       // <postmaster@raasu.org> 20021114
#include "tokenizer.h"		//
#include "util/Queue.h"		//
#include "settings.h"		//
#include "global.h"			// <postmaster@raasu.org> 20021217

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

using muscle::Queue;

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
	temp += "(";
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
	QString temp = tr("<font color=\"%1\" size=\"%2\">").arg(WColors::Error).arg(font);
	temp += tr("<b>Warning:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\" size=\"%2\">").arg(WColors::ErrorMsg).arg(font);
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
	temp += tr("<b>-%1-</b>").arg(session);
	temp += "<b>";
	temp += tr("<font color=\"%1\">").arg(WColors::LocalName);
	temp += myname;
	temp += "</font>";
	temp += " -> (";
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += othername;
	temp += "</font>";
	temp += ")</b>: </font>";	
	return temp;
}

QString WFormat::ReceivePrivMsg(const QString &session, const QString &othername, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<b>-%1-</b>").arg(session);
	temp += tr("<font color=\"%1\">").arg(WColors::RemoteName);
	temp += tr("<b>%1</b>").arg(othername);
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
ParseChatText(const QString & str)
{
	// <postmaster@raasu.org> 20021106,20021114 -- Added support for URL labels, uses QStringTokenizer now ;)
	Queue<QString> qUrls;
	Queue<QString> qLabels;  // in this list, "" means no label
	
	QString qText = str;
	bool lastWasURL = false, inLabel = false;
	QStringTokenizer qTok(qText, " \t\n");
	QString qToken;
	
	while ((qToken = qTok.GetNextToken()) != QString::null)
	{
		bool bInTag = false;	// <postmaster@raasu.org> 20021012,20021114
		
		if (inLabel)			// label contains space(s) ???
		{
			if (qToken.right(1) == "]")
			{
				qLabels.Tail() += qToken.left(qToken.length() - 1);
				inLabel = false;
			}
			else if (qToken.find("]") >= 0)
			{
				qLabels.Tail() += qToken.left(qToken.find("]") - 1);
				inLabel = false;
			}
			else 
				qLabels.Tail() += qToken + " ";
		} 
		else if (IsURL(qToken))
		{
			if (
				(qToken.lower().startsWith("beshare:") == false) && 
				(qToken.lower().startsWith("share:") == false)
				)
			{
				while (qToken.length() > 1)
				{
					QString last = qToken.right(1);
					
					// <postmaster@raasu.org> 20021012,20021114,20030203
					//

					if (last == ">")					
					{
						bInTag = true;

						// Skip html tags that are just after the url
						//
						while (bInTag)
						{
							if (qToken.right(1) == "<")
								bInTag = false;

							qToken.truncate(qToken.length() - 1);
						}

						last = qToken.right(1);
					}

					// <postmaster@raasu.org> 20030203 
					// Fix for BeBook file: urls
					//
					if (qToken.right(2) == "()") 
						break;

					if 	(
						((last >= "0") && (last <= "9")) ||
						((last >= "a") && (last <= "z")) ||
						((last >= "A") && (last <= "Z")) ||
						(last == "/")
						)
					{
						break;
					}
					else
						qToken.truncate(qToken.length() - 1);
				}
			}
			else
			{
				// Remove html tag after the url...
				if (qToken.right(1) == ">")
				{
					bool bInTag = true;
					while (bInTag)
					{
						if (qToken.right(1) == "<")
							bInTag = false;
						qToken.truncate(qToken.length() - 1);
					}
				}
				// ...and ensure that URL doesn't end with a dot, comma or colon
				bool cont = true;
				while ((qToken.length() > 0) && cont)
				{
					unsigned int pos = qToken.length() - 1;
					switch ((QChar) qToken.at(pos))
					{
					case '.':
					case ',':
					case ':':
							qToken.truncate(pos);
							break;
					default:
							cont = false;
							break;
					}
				}
			}
			if (IsURL(qToken))
			{
				qUrls.AddTail(qToken);
				qLabels.AddTail("");
				lastWasURL = true;
			}
		}
		else if (lastWasURL)
		{
			lastWasURL = false; // clear in all cases, might contain trash between url and possible label
			
			if (qToken.startsWith("[")) // Start of label?
			{
				if (qToken.right(1) == "]") 
					qLabels.Tail() = qToken.mid(1, qToken.length() - 2);
				else if (qToken.find("]") >= 0)
					qLabels.Tail() = qToken.mid(1, qToken.find("]") - 1);
				else
				{
					qLabels.Tail() += qToken.mid(1) + " ";
					inLabel = true;
				}
			}
		}
	}
	
	if (inLabel) 
		qText += "]";
	
	if (qUrls.GetNumItems() > 0)
	{
		QString output = "";
		
		QString qUrl;
		QString qLabel;
		
		while ((qUrls.RemoveHead(qUrl) == B_OK) && (qLabels.RemoveHead(qLabel) == B_OK))
		{
			int urlIndex = qText.find(qUrl); // position in QString
			
			if (urlIndex > 0) // Not in start of string?
			{
				output += qText.left(urlIndex);
				qText = qText.mid(urlIndex);
			}
			
			// now the url...
			QString urltmp = "<a href=\"";
			if ( qUrl.startsWith("www.") )		urltmp += "http://";
			if ( qUrl.startsWith("ftp.") )		urltmp += "ftp://";
			if ( qUrl.startsWith("beshare.") )	urltmp += "server://";
			if ( qUrl.startsWith("irc.") )		urltmp += "irc://";
			urltmp += qUrl;
			urltmp += "\">";
			// Display URL label or link address, if label doesn't exist
			int lb = qText.find("\n"); // check for \n between url and label (Not allowed!!!)
			int le = qText.find(qLabel);
			if (
				(qLabel.length() > 0) && 
				((lb < 0) || (lb > le))
				)
				urltmp += qLabel.stripWhiteSpace(); // remove surrounding spaces before adding
			else
				urltmp += qUrl; 		
			urltmp += "</a>";
			QString urlfmt = WFormat::URL(urltmp);
			output += urlfmt;
			// strip url from original text
			qText = qText.mid(qUrl.length());
			// strip label from original text, if exists
			if (qLabel.length() > 0)
			{
				lb = qText.find("\n");
				le = qText.find("]");
				if ((lb < 0) || (lb > le))
				{
					qText = qText.mid(le + 1);
					if (qText.left(1) == "]")	// Fix for ']' in end of label
						qText = qText.mid(1);
				}
			}
			
		}
		// Still text left?
		if (qText.length() > 0)
			output += qText;
		
		return output;		// <postmaster@raasu.org> 20021107,20021114 -- Return modified string
	}
	else
		return str; 		// <postmaster@raasu.org> 20021107 -- Return unmodified 
}

void
ParseString(QString & str)
{
	bool space = true;
	bool first = false; // make first always be &nbsp;

	// Remove trailing line feeds
	while (str.right(1) == "\n")
		str.truncate(str.length() - 1);

	for (unsigned int i = 0; i < str.length(); i++)
	{
		// go through the string and change newlines to <br> (html)
		if (str[i] == '\n')	
			str.replace(i, 1, "<br>");
		else if (str[i] == ' ')
		{
			if (space)
			{
				// alternate inserting non-breaking space and real space
				if (first)
					first = false;
				else 
				{
					str.replace(i, 1, "&nbsp;");
					i += 5;
					first = true;
				}
			}
		}
		else if (str[i] == '<')
			space = false;
		else if (str[i] == '>')
			space = true;
		else if (str[i] == '\t')
		{
			if (space)
			{
				// <postmaster@raasu.org> 20030623 -- follow 'first' used in spaces here too...
				if (first)
					str.replace(i, 1, " &nbsp; &nbsp;");
				else
					str.replace(i, 1, "&nbsp; &nbsp; ");

				i += 12;
			}
		}
		else
		{	
			// other character
			first = true;	
		}
	}
}

QString
ParseStringStr(const QString & str)
{
	QString s = str;
	ParseString(s);
	return s;
}

void
EscapeHTML(QString & str)
{
	// we don't want to show html...
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (str[i] == '<')
			str.replace(i, 1, "&lt;");
		else if (str[i] == '>')
			str.replace(i, 1, "&gt;");
	}
}

QString
EscapeHTMLStr(const QString & str)
{
	QString s = str;
	EscapeHTML(s);
	return s;
}

QString
FixStringStr(const QString & str)
{
	QString s = str;
	EscapeHTML(s);
	s = ParseChatText(s);
	ParseString(s);
	return s;
}

void
FixString(QString & str)
{
	EscapeHTML(str);
	str = ParseChatText(str);
	ParseString(str);
}
