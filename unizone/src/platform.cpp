
#if defined(__LINUX__)
#include <wchar.h>
#endif

#include "platform.h"
#include "debugimpl.h"

#include <qstring.h>

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
wideCharToQString(const wchar_t *wide)
{
    QString result;
#ifdef WIN32
    result.setUnicodeCodes(wide, lstrlenW(wide));
#else
    result.setUnicodeCodes((const ushort *) wide, wcslen(wide));
#endif
    return result;
}

wchar_t *
qStringToWideChar(const QString &str)
{
    	if (str.isNull())
        	return NULL;
	wchar_t *result = new wchar_t[str.length() + 1];
	if (result)
	{
		for (unsigned int i = 0; i < str.length(); ++i)
			result[i] = str.at(i).unicode();
		result[str.length()] = 0;
//		PRINT("qStringToWideChar: result = %S\n", result);
		return result;
	}
	else
		return NULL;
}

QString 
GetParameterString(QString qCommand)
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
GetCommandString(QString qCommand)
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
CompareCommand(QString qCommand, const char * cCommand)
{
	PRINT("Compare String: qCommand=\'%S\'\n", qStringToWideChar(GetCommandString(qCommand)));
	PRINT("                cCommand=\'%s\'\n", cCommand);
	return (strcmp(GetCommandString(qCommand).latin1(),cCommand) ? false : true);
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
					String label = Trim(strip.Substring(left, right));
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
					String label = Trim(strip.Substring(left));
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
	String s((const char *) u.utf8());
	return QString::fromUtf8(StripURL(s).Cstr());
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
	if (
		(url.StartsWithIgnoreCase("file://")) || 
		(url.StartsWithIgnoreCase("http://")) ||
		(url.StartsWithIgnoreCase("www.")) ||
		(url.StartsWithIgnoreCase("https://")) ||
		(url.StartsWithIgnoreCase("mailto:")) ||
		(url.StartsWithIgnoreCase("ftp://")) ||
		(url.StartsWithIgnoreCase("ftp.")) ||
		(url.StartsWithIgnoreCase("audio://")) ||
		(url.StartsWithIgnoreCase("beshare:")) ||
		(url.StartsWithIgnoreCase("beshare.")) ||
		(url.StartsWithIgnoreCase("server://")) ||
		(url.StartsWithIgnoreCase("priv:")) ||
		(url.StartsWithIgnoreCase("share:")) || 
		(url.StartsWithIgnoreCase("mms://")) ||
		(url.StartsWithIgnoreCase("irc://")) ||
		(url.StartsWithIgnoreCase("irc."))
		)
	{
		if (
			!url.EndsWith("://") &&
			(url.Length() > 9)
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

String 
Trim(String orig)
{ 
   int32 len = (int32) orig.Length();
   const char * s = orig.Cstr();
   int32 startIdx; for (startIdx = 0;     startIdx<len;    startIdx++) if (s[startIdx] != ' ') break; 
   int32 endIdx;   for (endIdx   = len-1; endIdx>startIdx; endIdx--)   if (s[endIdx] != ' ')   break; 
   return orig.Substring((uint32)startIdx, (uint32)(endIdx+1)); 
}


int32
BandwidthToBytes(QString connection)
{
	int32 bps = 0;
	if (connection == "300 baud")
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
BandwidthToString(int32 bps)
{
	switch (bps)
	{
	case 75:			return "300 baud";
//	case 300:			return "300 baud";
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
GetServerName(QString server)
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
GetServerPort(QString server)
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