#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "util.h"
#include "tokenizer.h"
#include "formatting.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"

#ifdef DEBUG2
#include "wstring.h"
#endif

#include "util/Queue.h"
#include "regex/StringMatcher.h"
using namespace muscle;

#if (QT_VERSION >= 0x030000)
#include <qregexp.h>
#endif

#include <qdatetime.h>
#include <qdns.h>

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

QString 
GetParameterString(const QString & qCommand)
{
	QString qParameters = "";
	int sPos = qCommand.find(" ")+1;
	if (sPos > 0)
	{
		qParameters = qCommand.mid(sPos);
	}
	return qParameters;
}

// <postmaster@raasu.org> 20021103 -- Added GetCommandString() and CompareCommand()
//

QString 
GetCommandString(const QString & qCommand)
{
	QString qCommand2 = qCommand.lower();
	int sPos = qCommand2.find(" ");  // parameters should follow after <space> so they should be stripped off
	int sPos2 = qCommand2.find("/"); // Is / first letter?
	if ((sPos > 0) && (sPos2 == 0))
	{
		qCommand2.truncate(sPos);
	}
	return qCommand2;
}

bool 
CompareCommand(const QString & qCommand, const QString & cCommand)
{
	QString com = GetCommandString(qCommand);
#ifdef DEBUG2
	WString wCommand(com);
	WString wCommand2(cCommand);
	PRINT("Compare String: qCommand=\'%S\'\n", wCommand.getBuffer());
	PRINT("                cCommand=\'%S\'\n", wCommand2.getBuffer());
#endif
	return ((com == cCommand) ? true : false);
}

String
StripURL(const String & strip)
{
	int sp;
	if (IsURL(strip))
	{
		sp = strip.IndexOf(" ");
		if (sp > 0)
		{
			int left = strip.IndexOf('[');	// see if it contains a label..
			if (left == (sp + 1))
			{
				left++;
				int right = strip.IndexOf(']');
				if (right > left)	// make sure right is than greater left :D
				{
					String label = strip.Substring(left, right).Trim();
					if ((right + 1) < (int) strip.Length())
					{
						String rest = strip.Substring(right + 1);
						if (rest.StartsWith(" "))
						{
							label += " ";
							rest = rest.Substring(1);
						}
						return label + StripURL(rest);
					}
					else
						return label;
				}
				else if (right == -1) // ']' is missing?
				{
					String label = strip.Substring(left).Trim();
					return label;
				}
			}
		}
	}
	// Recurse ;)
	sp = strip.IndexOf(" ");
	if (sp > 0)
	{
		String s1 = strip.Substring(0,sp+1); // include space in s1 ;)
		String s2;
		if ((sp + 1) < (int) strip.Length())
		{
			s2 = StripURL(strip.Substring(sp+1));
			return s1 + s2;
		}
		else
			return s1;
	}
	else
		return strip;	// not a url
}

QString
StripURL(const QString & u)
{
	int sp;
	if (IsURL(u))
	{
		sp = u.find(" ");
		if (sp > 0)
		{
			int left = u.find('[');	// see if it contains a label..
			if (left == (sp + 1))
			{
				left++;
				int right = u.find(']');
				if (right > left)	// make sure right is than greater left :D
				{
					QString label = u.mid(left, (right - left)).stripWhiteSpace();
					if ((right + 1) < (int) u.length())
					{
						QString rest = u.mid(right + 1);
						if (rest.startsWith(" "))
						{
							label += " ";
							rest = rest.mid(1);
						}
						return label + StripURL(rest);
					}
					else
						return label;
				}
				else if (right == -1) // ']' is missing?
				{
					QString label = u.mid(left).stripWhiteSpace();
					return label;
				}
			}
		}
	}
	// Recurse ;)
	sp = u.find(" ");
	if (sp > 0)
	{
		QString s1 = u.left(sp+1); // include space in s1 ;)
		QString s2;
		if ((sp + 1) < (int) u.length())
		{
			s2 = StripURL(u.mid(sp+1));
			return s1 + s2;
		}
		else
			return s1;
	}
	else
		return u;	// not a url
}

String 
StripURL(const char * c)
{
	String s(c);
	return StripURL(s);
}

// URL Prefixes

const char * urlPrefix[] = {
	"file://",
	"http://",
	"https://",
	"mailto:",
	"ftp://",
	"audio://",
	"mms://",
	"h323://",
	"callto://",
	"beshare:",		// BeShare Search
	"share:",		//    ----"----
	"server://",	// BeShare Server
	"priv:",		// BeShare Private Chat
	"irc://",		// Internet Relay Chat
	"ttp://",		// Titanic Transfer Protocol
	"ed2k:",		// eDonkey2000
	"magnet:",		// Magnet
	"gnutella:",	// Gnutella
	"mp2p:",		// Piolet
	NULL
};

bool
IsURL(const String & url)
{
	String u = url.ToLowerCase();
	
	// Add default protocol prefixes
	
	if (u.StartsWith("www."))		
		u = u.Prepend("http://");
	if (u.StartsWith("ftp."))		
		u = u.Prepend("ftp://");
	if (u.StartsWith("beshare.") && u.Length() > 12)	
		u = u.Prepend("server://");
	if (u.StartsWith("irc."))		
		u = u.Prepend("irc://");
	
	if (u.Length() > 9)
	{
		const char *prefix;
		for (unsigned int i = 0; (prefix = urlPrefix[i]) != NULL; i++)
		{
			if (u.StartsWith(prefix))
			{
				if (!u.EndsWith("://"))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool 
IsURL(const char * url)
{
	String u(url);
	return IsURL( u );
}

bool
IsURL(const QString & url)
{
	QString u = url.lower();

	// Add default protocol prefixes

	if (u.startsWith("www."))		
		u = u.prepend("http://");
	if (u.startsWith("ftp."))		
		u = u.prepend("ftp://");
	if (u.startsWith("beshare.") && u.length() > 12)	
		u = u.prepend("server://");
	if (u.startsWith("irc."))		
		u = u.prepend("irc://");

	if (u.length() > 9)
	{
		const char *prefix;
		for (unsigned int i = 0; (prefix = urlPrefix[i]) != NULL; i++)
		{
			if (u.startsWith(prefix))
			{
				if (u.right(3) != "://")
				{
					return true;
				}
			}
		}
	}
	return false;
}

QString 
MakeSizeString(uint64 s)
{
	QString result, postFix;
	double n = (int64) s;
	postFix = "B";
	if (n > 1024.0f)	// > 1 kB?
	{
		n /= 1024.0f;
		postFix = "kB"; // we're in kilobytes now, <postmaster@raasu.org> 20021024 KB -> kB
		
		if (n > 1024.0f)	// > 1 MB?
		{
			n /= 1024.0f;
			postFix = "MB";
			
			if (n > 1024.0f)	// > 1 GB?
			{
				n /= 1024.0f;
				postFix = "GB";
			}
		}
	}
	result.sprintf("%.2f ", n);
	result += postFix;
	return result;	
}

uint32
BandwidthToBytes(const QString & connection)
{
	uint32 bps = 0;
	if (connection.find(",") > 0)
	{
		QString spd = connection.mid(connection.find(",") + 1);
		bps = spd.toULong();
	}
	else if (connection == "300 baud")
	{
		bps = 75;
	}
	else if ( ( connection == "14.4 kbps" ) || ( connection == QObject::tr( "14.4 kbps" ) ) )
	{
		bps = 14400;
	}
	else if ( ( connection == "28.8 kbps" ) || ( connection == QObject::tr( "28.8 kbps" ) ) ) 
	{
		bps = 28800;
	}
	else if (
		( connection == "33.6 kbps" ) || ( connection == QObject::tr( "33.6 kbps" ) ) ||
		( connection == "36.6 kbps" ) || ( connection == QObject::tr( "36.6 kbps" ) )
		)
	{
		bps = 33600;
	}
	else if ( ( connection == "57.6 kbps" ) || ( connection == QObject::tr( "57.6 kbps" ) ) )
	{
		bps = 57600;
	}
	else if ( ( connection == "ISDN-64k" ) || ( connection == QObject::tr( "ISDN-64k" ) ) )
	{
		bps = 64000;
	}
	else if ( ( connection == "ISDN-128k" ) || ( connection == QObject::tr( "ISDN-128k" ) ) )
	{
		bps = 128000;
	}
	else if ( ( connection == "DSL-256k" ) || ( connection == QObject::tr( "DSL-256k" ) ) )
	{
		bps = 256000;
	}
	else if ( 
		( connection == "DSL" ) || ( connection == QObject::tr( "DSL" ) ) ||
		( connection == "DSL-384k" ) || (connection == QObject::tr( "DSL-384k" ) ) 
		)
	{
		bps = 384000;
	}
	else if ( ( connection == "DSL-512k" ) || ( connection == QObject::tr( "DSL-512k" ) ) )
	{
		bps = 512000;
	}
	else if ( ( connection == "Cable" ) || ( connection == QObject::tr( "Cable" ) ) )
	{
		bps = 768000;
	}
	else if ( ( connection == "T1" ) || ( connection == QObject::tr( "T1" ) ) )
	{
		bps = 1500000;
	}
	else if ( ( connection == "T3" ) || ( connection == QObject::tr( "T3" ) ) )
	{
		bps = 4500000;
	}
	else if ( ( connection == "OC-3" ) || ( connection == QObject::tr( "OC-3" ) ) )
	{
		bps = 3 * 51840000;
	}
	else if ( ( connection == "OC-12" ) || ( connection == QObject::tr( "OC-12" ) ) )
	{
		bps = 12 * 51840000;
	}
	return bps;
}

QString
BandwidthToString(uint32 bps)
{
	switch (bps)
	{
	case 75:			return "300 baud";
	case 300:			return "300 baud";
	case 14400: 		return "14.4 kbps";
	case 28800: 		return "28.8 kbps";
	case 33600: 		return "33.6 kbps";
	case 57600: 		return "57.6 kbps";
	case 64000: 		return "ISDN-64k";
	case 128000:		return "ISDN-128k";
	case 256000:		return "DSL-256k";
	case 384000:		return "DSL-384k";
	case 512000:		return "DSL-512k";
	case 768000:		return "Cable";
	case 1500000:		return "T1";
	case 4500000:		return "T3";
	case 3 * 51840000:	return "OC-3";
	case 12 * 51840000: return "OC-12";
	default:			return "Unknown";
	}
}

QString
GetServerName(const QString & server)
{
	int pColon = server.find(":");
	if (pColon >= 0)
	{
		return server.left(pColon);
	}
	else
	{
		return server;
	}
}

uint16
GetServerPort(const QString & server)
{
	int pColon = server.find(":");
	if (pColon >= 0)
	{
		return server.mid(pColon+1).toUShort();
	}
	else
	{
		return 2960;
	}
}

// Is reserved regex token, but isn't wildcard token (* or ? or ,)
// Skip \ now, it's a special case
bool IsRegexToken2(char c, bool isFirstCharInString)
{
   if ((c == '\\') || (c == '*') || (c == '?') || (c == ','))
	   return false;
   return IsRegexToken(c, isFirstCharInString);
}

// Converts basic wildcard pattern to valid regex
void ConvertToRegex(String & s)
{
	const char * str = s.Cstr();
	
	String ret;
	
	bool isFirst = true;
	while(*str)
	{
		if (*str == '\\')			// skip \c
		{
			const char * n = (str + 1);
			if (n)
			{
				if (*n != '@')	// convert \@ to @
					ret += *str;
				str++;
				ret += *n;
				str++;
			}
		}
		else
		{
			if (IsRegexToken2(*str, isFirst)) ret += '\\';
			ret += *str;
			str++;
		}

		// reset
		if (isFirst)
			isFirst = false;
	}
	s = ret;
}

const char * MonthNames[12] = {
								QT_TRANSLATE_NOOP( "Date", "Jan" ),
								QT_TRANSLATE_NOOP( "Date", "Feb" ),
								QT_TRANSLATE_NOOP( "Date", "Mar" ),
								QT_TRANSLATE_NOOP( "Date", "Apr" ),
								QT_TRANSLATE_NOOP( "Date", "May" ),
								QT_TRANSLATE_NOOP( "Date", "Jun" ),
								QT_TRANSLATE_NOOP( "Date", "Jul" ),
								QT_TRANSLATE_NOOP( "Date", "Aug" ),
								QT_TRANSLATE_NOOP( "Date", "Sep" ),
								QT_TRANSLATE_NOOP( "Date", "Oct" ),
								QT_TRANSLATE_NOOP( "Date", "Nov" ),
								QT_TRANSLATE_NOOP( "Date", "Dec" )
};

QString TranslateMonth(const QString & m)
{
	return QObject::tr(m.local8Bit());
}

const char * DayNames[7] = {
								QT_TRANSLATE_NOOP( "Date", "Mon" ),
								QT_TRANSLATE_NOOP( "Date", "Tue" ),
								QT_TRANSLATE_NOOP( "Date", "Wed" ),
								QT_TRANSLATE_NOOP( "Date", "Thu" ),
								QT_TRANSLATE_NOOP( "Date", "Fri" ),
								QT_TRANSLATE_NOOP( "Date", "Sat" ),
								QT_TRANSLATE_NOOP( "Date", "Sun" )
};

QString TranslateDay(const QString & d)
{
	return QObject::tr(d.local8Bit());
}

QString
GetTimeStamp()
{
	static QString _day = "";
	QString qCurTime;

	qCurTime = QDateTime::currentDateTime().toString();
#ifndef WIN32
	qCurTime = QString::fromLocal8Bit(qCurTime);
#endif

	// Strip off year
	QString qYear = qCurTime.mid(qCurTime.findRev(" ") + 1);
	qCurTime.truncate(qCurTime.findRev(" "));			
	
	// ... and day of week
	QString qDOW = qCurTime.left(qCurTime.find(" "));
	qDOW = TranslateDay(qDOW);

	qCurTime = qCurTime.mid(qCurTime.find(" ") + 1);	
	
	// Linux ;)
	// QChar q(160,0);
	// qCurTime.replace(QRegExp(q), "");
	//

	// Strip Month and translate it
	QString qMonth = qCurTime.left(qCurTime.find(" "));
	qMonth = TranslateMonth(qMonth);
	
	qCurTime = qCurTime.mid(qCurTime.find(" ") + 1);

	// Strip Day
	QString qDay = qCurTime.left(qCurTime.find(" "));
	
	qCurTime = qCurTime.mid(qCurTime.find(" ") + 1);
	
	QString ret = "";
	// Is this first time today?

	QString qDate = qDOW + " " + qMonth + " " + qDay + " " + qYear;
	if (qDate != _day)
	{
		_day = qDate;
		qDate.prepend(" ");
		qDate.prepend(QObject::tr("Date:", "Date"));
		ret = WFormat::TimeStamp(qDate);
		ret += "<br>";
	}
	
	qCurTime.prepend("[");
	qCurTime.append("] ");

	ret += WFormat::TimeStamp(qCurTime);
	return ret;
}


QString
ComputePercentString(int64 cur, int64 max)
{
	QString ret;
	double p = 0.0f;
	
	if ( (cur > 0) && (max > 0) )
	{
		p = ((double)cur / (double)max) * 100.0f;
	}
	
	ret.sprintf("%.2f", p);
	return ret;
}


void
Reverse(QString &text)
{
	int start = 0;
	int end = text.length() - 1;
	while (start < end)
	{
		QChar c = text[end];
		text[end] = text[start];
		text[start] = c;
		start++;
		end--;
	}
}

void MakeNodePath(String &file)
{
	if (gWin->fSettings)
	{
		if (gWin->fSettings->GetFirewalled())
		{
			file = file.Prepend("fires/");
		}
		else
		{
			file = file.Prepend("files/");
		}
		file = file.Prepend("beshare/");
	}
}

String MakePath(const String &dir, const String &file)
{
	String ret(dir);
	if (!ret.EndsWith("/"))
		ret += "/";
				
	ret += file;
	
	return ret; 
}

/*
 * Is received text action text or normal text
 *
 */

bool
IsAction(const QString &text, const QString &user)
{
	bool ret = false;

	if (text.startsWith(user + " "))
		ret = true;

	if (text.startsWith(user + "'s "))
		ret = true;

	return ret;
}


QString 
FixFileName(const QString & fixMe)
{
	// bad characters in Windows:
	//	/, \, :, *, ?, ", <, >, |
#ifdef WIN32
	QString ret(fixMe);
	for (unsigned int i = 0; i < ret.length(); i++)
	{
		switch ((QChar) ret.at(i))
		{
		case '/':
		case '\\':
		case ':':
		case '*':
		case '?':
		case '\"':
		case '<':
		case '>':
		case '|':
			ret.replace(i, 1, "_");
			break;
		}
	}
	return ret;
#else
	return fixMe;
#endif
}

const QString &
CheckIfEmpty(const QString & str, const QString & str2)
{
	if (str.isEmpty())
	{
		return str2;
	}
	else
	{
		return str;
	}
}

uint32
CalculateChecksum(const uint8 * data, size_t bufSize)
{
   uint32 sum = 0L;
   for (size_t i=0; i<bufSize; i++) sum += (*(data++)<<(i%24));
   return sum;
}

uint32
GetHostByName(const QString &name)
{
	QDns query(name);
	QValueList<QHostAddress> ips = query.addresses();
	QValueList<QHostAddress>::ConstIterator ipiter = ips.begin();
	while (ipiter != ips.end())
	{
		if ((*ipiter).isIp4Addr())
			return (*ipiter).ip4Addr();
		ipiter++;
	}
	return 0;
}

QString
UniqueName(const QString & file, int index)
{
	QString tmp, base, ext;
	int sp = file.findRev("/", -1); // Find last /
	if (sp > -1)
	{
		tmp = file.left(sp + 1);			// include slash
		base = file.mid(sp + 1);			// filename
		QString out(tmp);
		out += QString::number(index);
		out += " ";
		out += base;
		return out;
	}
	WASSERT(true, "Invalid download path!");
	return QString::null;
}

void
SavePicture(QString &file, const ByteBufferRef &buf)
{
	int n = 1;
	QString path("downloads/");
	path += FixFileName(file);
	QString nf = path;
	while (QFile::exists(nf)) 
	{
		nf = UniqueName(path, n++);
	}
	QFile fFile(nf);
	if (fFile.open(IO_WriteOnly))
	{
		unsigned int bytes = fFile.writeBlock((char *) buf()->GetBuffer(), buf()->GetNumBytes());
		fFile.close();
		if  (bytes == buf()->GetNumBytes())
		{
			file = nf;
			return;
		}
	}
	file = QString::null;
}
