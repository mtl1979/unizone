#ifdef WIN32
#include <windows.h>
#include <qstring.h>
#include "platform.h"
#include "debugimpl.h"

QString 
wideCharToQString(const wchar_t *wide)
{
    QString result;
    result.setUnicodeCodes(wide, lstrlenW(wide));
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
		PRINT("qStringToWideChar: result = %S\n",result);
		return result;
	}
	else
		return NULL;
}

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
	PRINT("Compare String: qCommand=\'%s\'\n",GetCommandString(qCommand).latin1());
	PRINT("                cCommand=\'%s\'\n",cCommand);
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
		(url.StartsWithIgnoreCase("https://")) ||
		(url.StartsWithIgnoreCase("mailto:")) ||
		(url.StartsWithIgnoreCase("ftp://")) || 
		(url.StartsWithIgnoreCase("audio://")) ||
		(url.StartsWithIgnoreCase("beshare:")) || 
		(url.StartsWithIgnoreCase("priv:")) ||
		(url.StartsWithIgnoreCase("share:")) || 
		(url.StartsWithIgnoreCase("mms://"))
		)
		return true;
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
	result.sprintf("%.2f %s", n, postFix.latin1());
	return result;	
}
