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

// formatting for:
//	(id) UserName

// <postmaster@raasu.org> 20020930
// Change that ID is always in black and colon isn't part of nick ;)

QString WFormat::LocalName  = "<font size=\"%2\"><b>(%3)</b> <font color=\"%1\"><b>%4</b></font>: </font>";
QString WFormat::RemoteName = "<font size=\"%2\"><b>(%3)</b> <font color=\"%1\"><b>%4</b></font>: </font>";

// text color...
QString WFormat::Text = "<font color=\"%1\" size=\"%2\">%3</font>";

//    |
//   \ /
//	System: User #?? is now connected.
QString WFormat::SystemText()
{
	return QObject::tr("<font color=\"%1\" size=\"%2\"><b>System:</b> </font>");
}

QString WFormat::UserConnected()
{
	return QObject::tr("User #%1 is now connected.");
}

QString WFormat::UserDisconnected()
{
	return QObject::tr("User #%1 (a.k.a. <font color=\"%3\">%2</font>) has disconnected.");
}

QString WFormat::UserNameChangedNoOld()
{
	return QObject::tr("User #%1 is now known as <font color=\"%3\">%2</font>.");
}

QString WFormat::UserNameChanged()
{
	return QObject::tr("User #%1 (a.k.a. <font color=\"%4\">%2</font>) is now known as <font color=\"%5\">%3</font>.");
}

QString WFormat::UserStatusChanged()
{
	return QObject::tr("User #%1 (a.k.a. <font color=\"%4\">%2</font>) is now %3.");
}

QString WFormat::UserStatusChanged2()
{
	return QObject::tr("User #%1 is now %2.");
}

// ping formatting
QString WFormat::PingText()
{
	return QObject::tr("<font color=\"%1\" size=\"%2\">Ping returned in %3 milliseconds (%4)</font>");
}

QString WFormat::PingUptime()
{
	return QObject::tr("<font color=\"%1\" size=\"%2\"> (Uptime: %3, Logged on for %4)</font>");
}


// error format
QString WFormat::Error()
{
	return QObject::tr("<font color=\"%1\" size=\"%2\"><b>Error:</b></font> ");
}

// error text color (just regular text)
QString WFormat::ErrorMsg = "<font color=\"%1\" size=\"%2\">%3</font>";

// warning format
QString WFormat::Warning()
{
	return QObject::tr("<font color=\"%1\" size=\"%2\"><b>Warning:</b></font> ");
}

QString WFormat::StatusChanged()
{
	return QObject::tr("You are now %1.");
}

// priv messages

// <postmaster@raasu.org> 20020930,20030127 
// Changed that ID is always black, uid in --

QString WFormat::SendPrivMsg	= "<font size=\"%2\"><b>-%3-</b> <font color=\"%1\"><b>%4 -> (%5)</b></font>: </font>";	
QString WFormat::ReceivePrivMsg = "<font size=\"%2\"><b>-%3-</b> <font color=\"%1\"><b>%4</b></font>: </font><font color=\"%5\" size=\"%6\">%7</font>";

QString WFormat::Action()
{
	return QObject::tr("<font color=\"%1\" size=\"%2\"><b>Action:</b></font> ");
}

// <postmaster@raasu.org> 20020930
// WFormat::URL Doesn't work because % is a valid character in URLs

QString WFormat::URL1 = "<font color=\"%1\" size=\"%2\"><u>";
QString WFormat::URL2 = "</u></font>";

QString WFormat::GotPinged()
{
	return QObject::tr("<font color=\"%1\" size=\"%2\">User #%3 (a.k.a. <font color=\"%5\">%4</font>) pinged you.</font>");
}

QString WFormat::TimeStamp = "<font color=\"%1\" size=\"%2\"><b>%3</b></font> ";



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
				// Remove html tag after the url and ensure that URL doesn't end with a dot, comma or colon
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
				if (
					(qToken.right(1) == ".") ||
					(qToken.right(1) == ",") ||
					(qToken.right(1) == ":")
					)
				{
					qToken.truncate(qToken.length() - 1);
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
			QString urlfmt = WFormat::URL1.arg(WColors::URL).arg(gWin->fSettings->GetFontSize());
			urlfmt += "<a href=\"";
			if ( qUrl.startsWith("www.") )		urlfmt += "http://";
			if ( qUrl.startsWith("ftp.") )		urlfmt += "ftp://";
			if ( qUrl.startsWith("beshare.") )	urlfmt += "server://";
			if ( qUrl.startsWith("irc.") )		urlfmt += "irc://";
			urlfmt += qUrl;
			urlfmt += "\">";
			// Display URL label or link address, if label doesn't exist
			int lb = qText.find("\n"); // check for \n between url and label (Not allowed!!!)
			int le = qText.find(qLabel);
			if (
				(qLabel.length() > 0) && 
				((lb < 0) || (lb > le))
				)
				urlfmt += qLabel.stripWhiteSpace(); // remove surrounding spaces before adding
			else
				urlfmt += qUrl; 		
			urlfmt += "</a>";
			urlfmt += WFormat::URL2;
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
	bool first = true;
	for (int i = 0; i < (int)str.length(); i++)
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
	for (int i = 0; i < (int)str.length(); i++)
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
