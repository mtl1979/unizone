#include "platform.h"
#include "wstring.h"
#include "debugimpl.h"
#include "colors.h"
#include "formatting.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"

#include <qstring.h>
#include <qdatetime.h>

#if (QT_VERSION >= 0x030000)
#include <qregexp.h>
#endif

#ifdef WIN32
#include <windows.h>

long 
GetRegKey( HKEY key, wchar_t *subkey, wchar_t *retdata, wchar_t value )
	{
    long retval;
    HKEY hkey;

    retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if(retval == ERROR_SUCCESS)
        {
        long datasize = MAX_PATH;
        wchar_t data[MAX_PATH];

        if (RegQueryValue(hkey, (wchar_t *)value,(wchar_t *)data,&datasize) == ERROR_SUCCESS)
	        lstrcpy(retdata,data);
    
		RegCloseKey(hkey);
        }

    return retval;
    }

void 
WFlashWindow(HWND fWinHandle)
{
#ifdef BUILD_WIN98	// flash our window
								FLASHWINFO finf;
								finf.cbSize = sizeof(finf);
								finf.hwnd = fWinHandle;
								finf.dwFlags = FLASHW_ALL;
								finf.uCount = 3;
								finf.dwTimeout = 400;
								FlashWindowEx(&finf);
#elif (WINVER < 0x0500)
								FlashWindow(fWinHandle,true);
#else
								FlashWindow(fWinHandle,FLASHW_ALL);
#endif // BUILD_WIN98
}

#endif

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
		qCommand2 = qCommand2.left(sPos);
	}
	return qCommand2;
}

bool 
CompareCommand(const QString & qCommand, const char * cCommand)
{
	QString com = GetCommandString(qCommand);
#ifdef DEBUG2
	WString wCommand = com;
	PRINT("Compare String: qCommand=\'%S\'\n", wCommand.getBuffer());
	PRINT("                cCommand=\'%s\'\n", cCommand);
#endif
	return (strcmp(com.latin1(), cCommand) ? false : true);
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
	QCString qu = u.utf8();
	const char * cu = (const char *) qu;
	return QString::fromUtf8(StripURL(cu).Cstr());
}

String 
StripURL(const char * c)
{
	String s(c);
	return StripURL(s);
}

bool
IsURL(const String & url)
{
	String u = url.ToLowerCase();

	// Add default protocol prefixes

	if (u.StartsWith("www."))		u = u.Prepend("http://");
	if (u.StartsWith("ftp."))		u = u.Prepend("ftp://");
	if (u.StartsWith("beshare."))	u = u.Prepend("server://");
	if (u.StartsWith("irc."))		u = u.Prepend("irc://");

	if (
		(u.StartsWith("file://")) || 
		(u.StartsWith("http://")) || 
		(u.StartsWith("https://")) ||
		(u.StartsWith("mailto:")) ||
		(u.StartsWith("ftp://")) ||
		(u.StartsWith("audio://")) || 
		(u.StartsWith("mms://")) || 
		(u.StartsWith("beshare:")) || 
		(u.StartsWith("share:")) || 
		(u.StartsWith("server://")) ||
		(u.StartsWith("priv:")) ||
		(u.StartsWith("irc://")) ||
		(u.StartsWith("ttp://"))
		)
	{
		if (
			!u.EndsWith("://") &&
			(u.Length() > 9)
			)
		{
			return true;
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
	QCString ur = url.utf8();
	const char * u = (const char *) ur;
	return IsURL( u );
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
   switch(c)
   {
     case '[': case ']': case '|': case '(': case ')':
        return true;

     case '<': case '~':   // these chars are only special if they are the first character in the string
        return isFirstCharInString; 

     default:
        return false;
   }
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
			ret += *str;
			const char * n = (str + 1);
			str++;
			if (n)
			{
				ret += *n;
				str++;
			}
			isFirst = false;	
		}
		else
		{
			if (IsRegexToken2(*str, isFirst)) ret += '\\';
			isFirst = false;
			ret += *str;
			str++;
		}
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
	return QObject::tr(m.latin1());
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
	return QObject::tr(d.latin1());
}

QString
GetTimeStamp()
{
	static QString _day = "";
	QString qCurTime;

	qCurTime = QDateTime::currentDateTime().toString();

	// Strip off year
	QString qYear = qCurTime.mid(qCurTime.findRev(" ") + 1);
	qCurTime.truncate(qCurTime.findRev(" "));			
	
	// ... and day of week
	QString qDOW = qCurTime.left(qCurTime.find(" "));
	qDOW = TranslateDay(qDOW);

	qCurTime = qCurTime.mid(qCurTime.find(" ") + 1);	
	
	// Linux ;)
	QChar q(160,0);
	qCurTime.replace(QRegExp(q), "");
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
		ret = WFormat::TimeStamp.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(qDate);
		ret += "<br>";
	}
	
	qCurTime.prepend("[");
	qCurTime.append("] ");

	ret += WFormat::TimeStamp.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(qCurTime);
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
			file.Prepend("fires/");
		}
		else
		{
			file.Prepend("files/");
		}
		file.Prepend("beshare/");
	}
}

