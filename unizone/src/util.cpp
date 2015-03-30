#include "util.h"

#include <qapplication.h>
#include <qregexp.h>
#include <q3dns.h>
#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>
#include <QByteArray>
#include <fcntl.h>

#include "util/Queue.h"
#include "util/StringTokenizer.h"
using namespace muscle;

#include "debugimpl.h"
#include "tokenizer.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"
#include "utypes.h"
#include "wfile.h"
#include "wstring.h"


#ifdef _WIN32
extern QString gDataDir;
#endif


QString
EscapeHTML(const QString & str)
{
	QString s;
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == '<')
			s += "&lt;";
		else if (str[i] == '>')
			s += "&gt;";
		else
			s += str[i];
	}
	return s;
}

QString
GetParameterString(const QString & qCommand)
{
	QString qParameters;
	int sPos = qCommand.find(" ")+1;
	if (sPos > 0)
	{
		while (qCommand[sPos].unicode() == 10)
			sPos++;
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
	if ((sPos > 0) && qCommand2.startsWith("/")) // Is / first letter?
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
	WString c1(com);
	WString c2(cCommand);
	PRINT2("Compare String: qCommand=\'%S\'\n", c1.getBuffer());
	PRINT2("                cCommand=\'%S\'\n", c2.getBuffer());
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

		while (strip.Substring(sp + 1, sp + 2) == " ")
			sp++;

		if (sp > 0)
		{
			int left = strip.IndexOf('[');	// see if it contains a label..
			if (left == (sp + 1))
			{
				int right = strip.IndexOf(']', left + 1);
				if (right != -1)	// ']' does exist
				{
					String label;
					while (strip.Substring(left, right).GetNumInstancesOf("[") > 
							strip.Substring(left, right).GetNumInstancesOf("]")
							)
					{
						int next = strip.IndexOf("]", right + 1);
						if (next == -1)
							break;
						right = next;
					}
					left++;
					label = strip.Substring(left, right).Trim();
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
				else // ']' is missing?
				{
					String label = strip.Substring(left + 1).Trim();
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

		while (u.at(sp + 1) == ' ')
			sp++;

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
	"mic://",		// Microsoft Comic Chat
	NULL
};

bool
IsURL(const String & url)
{
	QString u = QString::fromUtf8( url.Cstr() );
	return IsURL( u );
}

bool
IsURL(const char * url)
{
	QString u = QString::fromUtf8(url);
	return IsURL( u );
}

bool
IsURL(const QString & url)
{
	QString u = url;

	// Add default protocol prefixes

	if (u.startsWith("www.", false) && !u.startsWith("www..", false))
		u.prepend("http://");
	if (u.startsWith("ftp.", false) && !u.startsWith("ftp..", false))
		u.prepend("ftp://");
	if ((u.startsWith("beshare.", false) && !u.startsWith("beshare..", false)) && u.length() > 12)
		u.prepend("server://");
	if (u.startsWith("irc.", false) && !u.startsWith("irc..", false))
		u.prepend("irc://");

	if (u.length() > 9)
	{
		const char *prefix;
		for (unsigned int i = 0; (prefix = urlPrefix[i]) != NULL; i++)
		{
			if (u.startsWith(prefix, false))
			{
				if (!u.endsWith("://"))
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
	double n = (double) (int64) s;
	postFix = qApp->translate("MakeSizeString","B");
	if (n > 1024.0f)	// > 1 kB?
	{
		n /= 1024.0f;
		postFix = qApp->translate("MakeSizeString","kB"); // we're in kilobytes now, <postmaster@raasu.org> 20021024 KB -> kB

		if (n > 1024.0f)	// > 1 MB?
		{
			n /= 1024.0f;
			postFix = qApp->translate("MakeSizeString","MB");

			if (n > 1024.0f)	// > 1 GB?
			{
				n /= 1024.0f;
				postFix = qApp->translate("MakeSizeString","GB");
			}
		}
		result.sprintf("%.2f ", n);
	}
	else
	{
		result.sprintf(UINT64_FORMAT_SPEC " ", s);
	}
	result += postFix;

	return result;
}

struct ConPair
{
	const char *id;
	uint32 bw;
};

ConPair Bandwidths[] = {
	// For reverse lookup
	{QT_TRANSLATE_NOOP("Connection", "75 baud"),	75},		
	// 300 down / 75 up
	{QT_TRANSLATE_NOOP("Connection", "300 baud"),	75},		
	{QT_TRANSLATE_NOOP("Connection", "9.6 kbps"),	9600},
	{QT_TRANSLATE_NOOP("Connection", "14.4 kbps"),	14400},
	{QT_TRANSLATE_NOOP("Connection", "28.8 kbps"),	28800},
	{QT_TRANSLATE_NOOP("Connection", "33.6 kbps"),	33600},
	// Misspelled 33.6 kbps
	{QT_TRANSLATE_NOOP("Connection", "36.6 kbps"),	33600},
	{QT_TRANSLATE_NOOP("Connection", "57.6 kbps"),	57600},
	{QT_TRANSLATE_NOOP("Connection", "ISDN-64k"),	64000},
	{QT_TRANSLATE_NOOP("Connection", "ISDN-128k"),	128000},
	{QT_TRANSLATE_NOOP("Connection", "DSL-256k"),	256000},
	// 4 timeslots
	{QT_TRANSLATE_NOOP("Connection", "EDGE"),	236800},
	{QT_TRANSLATE_NOOP("Connection", "DSL-384k"),	384000},
	// Backwards compatibility
	{QT_TRANSLATE_NOOP("Connection", "DSL"),	384000},
	{QT_TRANSLATE_NOOP("Connection", "HSDPA"),	384000},
	{QT_TRANSLATE_NOOP("Connection", "DSL-512k"),	512000},
	{QT_TRANSLATE_NOOP("Connection", "Cable"),	768000},
	// Was 1000000
	{QT_TRANSLATE_NOOP("Connection", "DSL-1M"),	1024000},
	// For reverse lookup
	{QT_TRANSLATE_NOOP("Connection", "DSL-1M"),	1000000},
	{QT_TRANSLATE_NOOP("Connection", "DSL-2M"),	2048000},
	{QT_TRANSLATE_NOOP("Connection", "T1"),		1500000},
	{QT_TRANSLATE_NOOP("Connection", "T3"),		4500000},
	// 3  * 51840000
	{QT_TRANSLATE_NOOP("Connection", "OC-3"),	155520000},
	// 5.76 Mbit/s
	{QT_TRANSLATE_NOOP("Connection", "HSUPA"),	576000000},
	// 12 * 51840000
	{QT_TRANSLATE_NOOP("Connection", "OC-12"),	622080000},
	// Unknown speed
	{QT_TRANSLATE_NOOP("Connection", "Unknown"),	0},
	// Dummy entry
	{"?", 0},
	{NULL, UINT32_MAX}
};

uint32
BandwidthToBytes(const QString & connection)
{
	uint32 bps = 0;
	int p = connection.find(",");
	QString conn;
	if (p > 0)
	{
		conn = connection.left(p);
		QString spd = connection.mid(p + 1);
		bps = spd.toULong();
	}
	else
	{
		conn = connection;
	}
	if (bps == 0)
	{
		ConPair bw;
		int n = 0;
		while ((bw = Bandwidths[n++]).bw != UINT32_MAX)
		{
			if (
				( conn == bw.id ) ||
				( conn == qApp->translate("Connection", bw.id) )
				)
			{
				bps = bw.bw;
				break;
			}
		};
	}
#ifdef DEBUG2
	WString wconn(conn);
	PRINT2("Connection = '%S', bps = %lu\n", wconn.getBuffer(), bps);
#endif
	return bps;
}

QString
BandwidthToString(uint32 bps)
{
	ConPair bw;
	int n = 0;
	while ((bw = Bandwidths[n++]).bw != UINT32_MAX)
	{
		if (bps == bw.bw)
		{
			return qApp->translate("Connection", bw.id);
		}
	};
	
	return qApp->translate("Connection", "Unknown");
}

QString
GetServerName(const QString & server)
{
	int pSquare = server.find("[");
	if (pSquare == 0)
	{
		pSquare = server.find("]");
		return server.mid(1, pSquare - 1);
	}
	int pColon = server.find(":");
	if (pColon >= 0 && server.count(":") == 1)
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
	int pSquare = server.find("]:");
	if (pSquare > 0)
	{
		return server.mid(pSquare+2).toUShort();
	}
	int pColon = server.find(":");
	if (pColon >= 0 && server.count(":") == 1)
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
bool IsRegexToken2(QChar c, bool isFirstCharInString)
{
   switch(c.unicode())
   {
      case '[': case ']': case '|': case '(': case ')': case '=': case '^': case '+': case '$': case '{':  case '}':
	  case ':': case '-': case '.':
        return true;

      case '<': case '~':   // these chars are only special if they are the first character in the string
         return isFirstCharInString;

      default:
         return false;
   }
}

bool IsRegexToken2(char c, bool isFirstCharInString)
{
	return IsRegexToken2(QChar(c), isFirstCharInString);
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
			if (IsRegexToken2(*str, false)) ret += '\\';
			ret += *str;
			str++;
		}

		// reset
		if (isFirst)
			isFirst = false;
	}
	s = ret;
}

QString ConvertToRegexInternal(const QString & s, bool simple, bool isFirst)
{
	int x = 0;
	QString ret;
	while(x < s.length())
	{
		if (s[x] == '\\')			// skip \c
		{
			if (x + 1 < s.length())
			{
				if (s[x + 1] != '@')	// convert \@ to @
					ret += s[x];
				x++;
				ret += s[x];
			}
		}
		else if (s[x] == '*')
		{
			ret += simple ? "*" : ".*";
		}
		else if (s[x] == '?')
		{
			ret += simple ? "?" : ".";
		}
		else if (s[x] == ',')
		{
			ret += "|";
		}
		else
		{
			if (IsRegexToken2(s[x], false)) ret += '\\';
			ret += s[x];
		}

		x++;
		// reset
		if (isFirst)
			isFirst = false;
	}
	return ret;
}

void ConvertToRegex(QString & s, bool simple)
{
	QString ret;
	if (!simple)
	{
		ret = "^";
		if (s.count(",") > 0)
		{
			int pos = 0;
			int pos2 = 0;
			QStringList l = s.split(",");
			while (pos < l.at(0).length()) // Check start of strings
			{
				for (int i = 1; i < l.size(); i++)
				{
					if (pos == min(l.at(i).length(), l.at(0).length()) || l.at(i)[pos] != l.at(0)[pos])
						goto step2;
				}
				pos++;
			}
step2:
			while (pos2 < l.at(0).length()) // Check end of strings
			{
				for (int i = 1; i < l.size(); i++)
				{
					if (pos2 == min(l.at(i).length(), l.at(0).length()) || l.at(i)[l.at(i).length() - pos2 - 1] != l.at(0)[l.at(0).length() - pos2 - 1])
						goto step3;
				}
				pos2++;
			}
step3:
			if (pos > 0 || pos2 > 0)
			{
				QStringList retlist;
				if (pos > 0)
				{
					ret += ConvertToRegexInternal(l.at(0).left(pos), true, true);
				}
				ret += "(";
				for (int i = 0; i < l.size(); i++)
				{
					QString temp = l.at(i).mid(pos, l.at(i).length() - pos - pos2);
					retlist.append(ConvertToRegexInternal(temp, true, false));
				}
				ret += retlist.join("|");
				ret += ")";
                if (pos2 > 0)
				{
					ret += ConvertToRegexInternal(l.at(0).right(pos2), true, false);
				}
				ret += "$";
				s = ret;
				return;
			}
			// Fall through
		}
	}

	ret += ConvertToRegexInternal(s, simple, true);
	if (!simple)
		ret += "$";
	s = ret;
}

bool HasRegexTokens(const QString & str)
{
   bool isFirst = true;
   int x = 0;
   while(x < str.length())
   {
      if (IsRegexToken2(str[x], isFirst)) return true;
	  else if (str[x] == '*') return true;
	  else if (str[x] == '?') return true;
      else
      {
         x++;
         isFirst = false;
      }
   }
   return false;
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
	return qApp->translate("Date", m.utf8());
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
	return qApp->translate("Date", d.utf8());
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

void MakeNodePath(QString &file)
{
	if (gWin->fSettings)
	{
		if (gWin->fSettings->GetFirewalled())
		{
			file = file.prepend("fires/");
		}
		else
		{
			file = file.prepend("files/");
		}
		file = file.prepend("beshare/");
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

QString MakePath(const QString &dir, const QString &file)
{
	QString ret = QDir::convertSeparators(dir);
	if (!ret.endsWith(QDir::separator()))
		ret += QDir::separator();

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
	for (int i = 0; i < ret.length(); i++)
	{
		switch (ret.at(i).unicode())
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
CalculateFileChecksum(const uint8 *data, size_t size)
{
	uint32 sum = 0L;
	for (size_t i=0; i<size; i++) 
		sum += (*(data++)<<(i%24));
	return sum;
}

uint32
CalculateFileChecksum(const ByteBufferRef &buf)
{
	const uint8 * data = buf()->GetBuffer();
	size_t bufsize = buf()->GetNumBytes();
	return CalculateFileChecksum(data, bufsize);
}

muscle::ip_address
GetHostByName(const QString &name)
{
	return GetHostByName((const char *) name.local8Bit());
}

QString
UniqueName(const QString & file, int index)
{
	QString tmp, base, ext;
	int sp = file.findRev(QDir::separator(), -1); // Find last /
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
	WASSERT(false, "Invalid download path!");
	return QString::null;
}

void
SavePicture(QString &file, const uint8 * buf, size_t bufsize)
{
	int n = 1;
	QString path = MakePath(downloadDir(), FixFileName(file));
	QString nf(path);
	while (WFile::Exists(nf))
	{
		nf = UniqueName(path, n++);
	}
	WFile fFile;
	WString wfile(nf);
	PRINT("SavePicture: file=%ls, buf=%p, size=%ld\n", wfile.getBuffer(), buf, bufsize);
	if (fFile.Open(wfile, 
#ifdef WIN32
		O_WRONLY | O_CREAT | O_BINARY
#else
		O_WRONLY | O_CREAT
#endif
		))
	{
		uint64 bytes = fFile.WriteBlock(buf, bufsize);
		fFile.Close();
		if (bytes == bufsize)
		{
			file = nf;
			return;
		}
	}
	file = QString::null;
}

void
SavePicture(QString &file, const ByteBufferRef &buf)
{
	const uint8 * data = buf()->GetBuffer();
	size_t bufsize = buf()->GetNumBytes();
	SavePicture(file, data, bufsize);
}

uint64
toULongLong(const QStringRef &in, bool *ok)
{
	uint64 out = 0;
	bool o = true;
	for (int x = 0; x < in.length(); x++)
	{
		QChar c = in.at(x);
		if (!muscleInRange(c.unicode(), (unichar) '0', (unichar) '9'))
		{
			o = false;
			break;
		}
		out *= 10;
		out += c.unicode() - '0';
	}
	if (ok)
		*ok = o;
	return out;
}

QString
fromULongLong(const uint64 &in)
{
	uint64 tmp;
	QString out;
	int mlen = 20;

	if (in < 10)
		return QString(QChar(((int) in) + '0'));

	tmp = in;
	out.resize(mlen--);
	while (tmp > 0)
	{
		char n = (char) (tmp % 10);
		out[mlen--] = QChar(n + '0');
		tmp /= 10;
	}
	return out.mid(++mlen);
}

QString
hexFromULongLong(const uint64 &in, int length)
{
	uint64 tmp;
	QString out;
	if (length == 1) // 1 == no padding
	{
		if (in < 10)
			return QChar(((int) in) + '0');
		else if (in < 16)
			return QChar(((int) in) + 55);
		return QString::null; // Not enough space...
	}

	out.fill('0', length--);
	tmp = in;
	while (tmp > 0)
	{
		char n = (char) (tmp % 16);
		out[length--] = QChar(n + ((n < 10) ? '0' : 55));
		tmp /= 16;
		if (length == -1) return QString::null; // Not enough space
	}
	return out;
}

int64 toLongLong(const QStringRef &in, bool *ok)
{
	int64 out = 0;
	bool o = true;
	bool negate = false;

	for (int x = 0; x < in.length(); x++)
	{
		QChar c = in.at(x);
		if ((x == 0) && (c.unicode() == '-'))
			negate = true;
		else if (!muscleInRange(c.unicode(), (unichar) '0', (unichar) '9'))
		{
			o = false;
			break;
		}
		out *= 10;
		out += c.unicode() - '0';
	}

	if (negate)
		out = -out;

	if (ok)
		*ok = o;
	return out;
}

QString fromLongLong(const int64 &in)
{
	uint64 tmp;
	bool negate = false;
	QString out;
	int mlen = 20;

	if (in < 0)
	{
		if (in > -10)
			return QString("-%1").arg(QChar('0' - (int) in));
		tmp = -in;
		negate = true;
	}
	else if (in < 10)
		return QString(QChar(((int) in) + '0'));
	else
		tmp = in;

	out.resize(mlen--);
	while (tmp > 0)
	{
		char n = (char) (tmp % 10);
		out[mlen--] = QChar(n + '0');
		tmp /= 10;
	}

	if (negate)
		out[mlen--] = '-';

	return out.mid(++mlen);
}

QString hexFromLongLong(const int64 &in, int length)
{
	uint64 i;
	memcpy(&i, &in, sizeof(int64));
	return hexFromULongLong(i, length);
}

void HEXClean(QString &in)
{
	QString tmp;
	for (int x = 0; x < in.length(); x++)
	{
		QChar c = in[x].lower();
		if (
			muscleInRange(c.unicode(), (unichar) '0', (unichar) '9') ||
			muscleInRange(c.unicode(), (unichar) 'a', (unichar) 'f')
		   )
			tmp += c;
	}
	in = tmp;
}

void BINClean(QString &in)
{
	QString tmp, part;
	int s = 0, p;
	while (s < in.length())
	{
		// Skip initial garbage
		while (!muscleInRange(in[s].unicode(), (unichar) '0', (unichar) '1'))
		{
			s++;
			// avoid looping out of string...
			if (s == in.length())
			{
				if (tmp.isEmpty())
				{
					in = QString::null;
					return;
				}
				else
					break;
			}
		}
		part = QString::null;
		p = 0;
		while (p < 8)
		{
			if (s == in.length())
			  break;
			QChar c = in[s++];
			if (muscleInRange(c.unicode(), (unichar) '0', (unichar) '1'))
				part += c;
			else // garbage?
				break;
			p++;
		}
		if (p > 0)
		{
			tmp += part.rightJustified(8, '0');
		}
	}
	in = tmp;
}

void OCTClean(QString &in)
{
	QString tmp, part;
	int s = 0, p;
	while (s < in.length())
	{
		// Skip initial garbage
		while (!muscleInRange(in[s].unicode(), (unichar) '0', (unichar) '6'))
		{
			s++;
			// avoid looping out of string...
			if (s == in.length())
			{
				if (tmp.isEmpty())
				{
					in = QString::null;
					return;
				}
				else
					break;
			}
		}
		part = QString::null;
		p = 0;
		while (p < 3)
		{
			if (s == in.length())
			  break;
			QChar c = in[s++];
			if (muscleInRange(c.unicode(), (unichar) '0', (unichar) '6'))
				part += c;
			else // garbage?
				break;
			p++;
		}
		if (p > 0)
		{
			tmp += part.rightJustified(3, '0');
		}
	}
	in = tmp;
}

QString BINDecode(const QString &in)
{
	QByteArray out;

	if (in.length() % 8 != 0)
		return QString::null;

	for (int x = 0; x < in.length(); x += 8)
	{
		QStringRef part = in.midRef(x, 8);
		int xx = 1;
		int c = 0;
		for (int y = 7; y > -1; y--)
		{
			if (part.at(y) == '1')
				c = c + xx;
			xx *= 2;
		}
		out += (char) c;
	}
	return QString::fromUtf8(out);
}

QString BINEncode(const QString &in)
{
	QByteArray temp = in.utf8();
	QString out, part;
	for (int x = 0; x < temp.length(); x++)
	{
		unsigned char c = temp.at(x);
		part = "00000000";
		for (int xx = 0; xx < 8; xx++)
		{
			if (c % 2 == 1)
				part[7-xx] = '1';
			c /= 2;
		}
		out += part;
	}
	return out;
}

QString OCTDecode(const QString &in)
{
	QByteArray out;

	if (in.length() % 3 != 0)
		return QString::null;

	for (int x = 0; x < in.length(); x += 3)
	{
		QStringRef part = in.midRef(x, 3);
		int xx = 1;
		int c = 0;
		for (int y = 2; y > -1; y--)
		{
			c = c + (xx * (part.at(y).unicode() - '0'));
			xx *= 7;
		}
		out += (char) c;
	}
	return QString::fromUtf8(out);
}

QString OCTEncode(const QString &in)
{
	QByteArray temp = in.utf8();
	QString out, part;
	for (int x = 0; x < temp.length(); x++)
	{
		unsigned char c = temp.at(x);
		part = "000";
		for (int xx = 0; xx < 3; xx++)
		{
			part[2-xx] = ((char) ('0' + (c % 7)));
			c /= 7;
		}
		out += part;
	}
	return out;
}

int
Match(const QString &string, const QRegExp &exp)
{
	if (string.isEmpty())
		return -1;

	QString str;
	QRegExp e;
	if (exp.caseSensitive())
	{
		str = string;
		e = exp;
	}
	else
	{
		str = string.lower();
		e.setPattern(exp.pattern().lower());
	}
	return str.find(e);
}

int64
ConvertPtr(void *ptr)
{
		return (int64) ((intptr_t) ptr);
}

bool BinkyCheck(const QString &user)
{
	if (user.find(QString("binky"), 0, false) >= 0)
		return true;
	if (user.find(QString("yknib"), 0, false) >= 0)
		return true;
	return false;
}

QString FixPath(const QString & fixMe)
{
	// bad characters in Windows:
	//	/, :, *, ?, ", <, >, |
#ifdef WIN32
	QString ret(fixMe);
	for (int i = 0; i < ret.length(); i++)
	{
		switch (ret.at(i).unicode())
		{
		case '/':
			ret.replace(i, 1, "\\");
			break;
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

QString downloadDir(const QString &subdir)
{
	static QString out;
	if (out.isEmpty())
	{
		out = "downloads";
		out += QDir::separator();
	}
	if (gWin->fSettings->GetPreservePaths())
	{
		// Make sure we don't get path starting with "downloads/downloads"
		if (subdir == "downloads")
			return out;

		QString d = QDir::convertSeparators(subdir);
		if (d.startsWith(out))
			d = d.mid(out.length());

		if (!d.isEmpty())
		{
			QDir dir(out);
			if (d.startsWith("/"))
				d = d.mid(1);
			if (d[1] == ':')
				d = d.mid(3);
			d = FixPath(d);
			QStringTokenizer tok(d, "/\\");
			// traverse path to check every directory exists
			QString t;
			while ((t = tok.GetNextToken()) != QString::null)
			{
				if (!dir.exists(t))
				{
					dir.mkdir(t);
				}
				dir.cd(t);
			}
			d.prepend(out);
			d += QDir::separator();
#ifdef _DEBUG
			WString wd(d);
			PRINT("Path: %S", wd.getBuffer());
#endif
			return d;
		}
	}
	return out;
}

QString _imageFormats = "*.png;*.bmp;*.xbm;*.xpm;*.pbm;*.pgm;*.ppm;*.jpg;*.jpeg;*.mng;*.gif;*.tiff";

QString imageFormats()
{
	return _imageFormats;
}

QString WikiEscape(const QString &page)
{
	QByteArray out;
	QByteArray in = page.utf8();
	for (int x = 0; x < in.length(); x++)
	{
		const char c = in.at(x);
		     if ((c >= '0') && (c <= '9')) out += c;
		else if ((c >= 'a') && (c <= 'z')) out += c;
		else if ((c >= 'A') && (c <= 'Z')) out += c;
		else if ((c == '_') || (c == ' ')) out += '_';
		else    out += "%" + chartohex(c);
	}
	return out;
}

QString URLEscape(const QString &page)
{
	QByteArray out;
	QByteArray in = page.utf8();
	for (int x = 0; x < in.length(); x++)
	{
		const char c = in.at(x);
		     if ((c >= '0') && (c <= '9')) out += c;
		else if ((c >= 'a') && (c <= 'z')) out += c;
		else if ((c >= 'A') && (c <= 'Z')) out += c;
		else if (c == '_') out += '_';
		else    out += "%" + chartohex(c);
	}
	return out;
}

QString SimplifyPath(const QString &path)
{
	QString old = QDir::convertSeparators(path);
#ifdef _WIN32
	QString data = QDir::convertSeparators(gDataDir);
	if (!data.endsWith(QDir::separator()))
		data += QDir::separator();
	if (old.startsWith(data, false))
		old = old.mid(data.length());
#else
	QString app = QDir::convertSeparators(gAppDir);
	if (!app.endsWith(QDir::separator()))
		app += QDir::separator();
	if (old.startsWith(app))
		old = old.mid(app.length());
#endif
	return old;
}

