#include "resolver4.h"
#include "resolver.h"

#ifdef _WIN32
# if defined(_MSC_VER) && _MSC_VER >= 1800
#  include <ws2tcpip.h>
# endif
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
typedef hostent *LPHOSTENT;
#endif

#include <qstringlist.h>

#include "util/Queue.h"
#include "util/NetworkUtilityFunctions.h"

#include "listutil.h"
#include "tokenizer.h"

using muscle::Queue;


// Expire in 1 hour
#ifdef WIN32
# define EXPIRETIME 3600000000UI64
#else
# define EXPIRETIME 3600000000ULL
#endif

struct NetAddress4
{
	uint32 ip;
	QString address;
#if !defined(_MSC_VER) || _MSC_VER < 1800
	QString aliases;
#endif
	uint64 lastcheck;
};

Queue<NetAddress4> fAddressCache4;


#if !defined(_MSC_VER) || _MSC_VER < 1800
void
UpdateEntry(NetAddress4 &na, LPHOSTENT lpHostEntry)
{
	if (lpHostEntry)
	{
		char **p;
		QString aliases;

		na.address = QString::fromLocal8Bit(lpHostEntry->h_name);
		na.lastcheck = GetCurrentTime64();
	
		for (p = lpHostEntry->h_aliases; *p != NULL; p++)
		{
			AddToList(aliases, QString::fromLocal8Bit(*p));
		}
		na.aliases = aliases;
	}
	else
		na.address = QString::null;
}
#endif

void
UpdateEntry4(NetAddress4 &na, uint32 ip)
{
	
	na.ip = ip;					// We need to remember to initialize this

#if defined(_MSC_VER) && _MSC_VER >= 1800
	struct sockaddr_in saGNI;
	WCHAR hostname[NI_MAXHOST];
	DWORD dwRetval;

	saGNI.sin_family = AF_INET;
	saGNI.sin_addr.s_addr = htonl(ip);

	dwRetval = GetNameInfoW((struct sockaddr *) &saGNI, sizeof(struct sockaddr),
		hostname, NI_MAXHOST, NULL, 0, 0);

	if (dwRetval == 0)
	{
		na.address = QString::fromUcs2(hostname);
		na.lastcheck = GetCurrentTime64();
	}
#else
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure

	iaHost.s_addr = htonl(ip);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	UpdateEntry(na, lpHostEntry);
#endif
}

#if !defined(_MSC_VER) || _MSC_VER < 1800
void
ResolveAliasesAux4(NetAddress4 &na, uint32 ip)
{
	UpdateEntry4(na, ip);
	if (na.address == QString::null)
	{
		// Safe defaults
		na.lastcheck = 0;
		na.aliases = QString::null;
	}
}
#endif

uint32
ResolveAddress4(const QString &address)
{
	uint32 res;
	if (ParseIP4(address, res))
		return res;

	{
#ifdef MUSCLE_AVOID_IPV6
		res = GetHostByName(address);
#else
		muscle::ip_address res2;
		res2 = GetHostByName(address);
		res = ConvertIP4(res2);
#endif
	}

	NetAddress4 na;

	if (fAddressCache4.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache4.GetNumItems(); i++)
		{
			na = fAddressCache4[i];
			if ((na.address == address) ||
#if !defined(_MSC_VER) || _MSC_VER < 1800
				Contains(na.aliases, address) || 
#endif
				(na.ip == res))
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
				{
#if !defined(_MSC_VER) || _MSC_VER < 1800
					if (res != 0)
					{
						if ((address != na.address) && !Contains(na.aliases, address))
						{
							AddToList(na.aliases, address);
							fAddressCache4.ReplaceItemAt(i, na);
						}
					}
#endif

					return na.ip;
				}
				else
				{
					if (res != 0)	// Do not cache failures
					{
						na.address = ResolveHost4(res);		// Double check
#if !defined(_MSC_VER) || _MSC_VER < 1800
						ResolveAliasesAux4(na, res);
						if ((address != na.address) && !Contains(na.aliases, address))
							AddToList(na.aliases, address);
#endif
						na.lastcheck = GetCurrentTime64();
						fAddressCache4.ReplaceItemAt(i, na);
					}
					return na.ip;
				}
			}
		};
	}

	//

	if (res != 0)						// Do not cache failures
	{
#if !defined(_MSC_VER) || _MSC_VER < 1800
		ResolveAliasesAux4(na, res);
		if ((na.address != address) && !Contains(na.aliases, address))
			AddToList(na.aliases, address);
#endif
		fAddressCache4.AddTail(na);
	}
	return res;
}

uint32
ResolveAddress4(const String &address)
{
	return ResolveAddress4( QString::fromLocal8Bit( address.Cstr() ) );
}

QString
ResolveHost4(uint32 ip)
{
#if !defined(_MSC_VER) || _MSC_VER < 1800
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
#endif
	
	//
	NetAddress4 na;

	if (fAddressCache4.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache4.GetNumItems(); i++)
		{
			na = fAddressCache4[i];
			if (na.ip == ip)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME)
				{
					if (!na.address.isEmpty())
						return na.address;
				}
#if defined(_MSC_VER) && _MSC_VER >= 1800
				struct sockaddr_in saGNI;
				WCHAR hostname[NI_MAXHOST];
				DWORD dwRetval;

				saGNI.sin_family = AF_INET;
				saGNI.sin_addr.s_addr = htonl(ip);

				dwRetval = GetNameInfoW((struct sockaddr *) &saGNI, sizeof(struct sockaddr),
					hostname, NI_MAXHOST, NULL, 0, 0);

				if (dwRetval == 0)
				{
					na.address = QString::fromUcs2(hostname);
					na.lastcheck = GetCurrentTime64();
				}
#else
				iaHost.s_addr = htonl(ip);
				lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
				
				if (!lpHostEntry)
				{
					QStringTokenizer tok(na.aliases, ",");
					QString alias;
					while (!lpHostEntry && (alias = tok.GetNextToken()) != QString::null)
						lpHostEntry = gethostbyname((const char *) alias.local8Bit());
				}
		
				if (lpHostEntry)
				{
					UpdateEntry(na, lpHostEntry);
					fAddressCache4.ReplaceItemAt(i, na);
				}
#endif
				return na.address;
			}
		};
	}

	//

#if defined(_MSC_VER) && _MSC_VER >= 1800
	struct sockaddr_in saGNI;
	WCHAR hostname[NI_MAXHOST];
	DWORD dwRetval;

	saGNI.sin_family = AF_INET;
	saGNI.sin_addr.s_addr = htonl(ip);

	dwRetval = GetNameInfoW((struct sockaddr *) &saGNI, sizeof(struct sockaddr),
		hostname, NI_MAXHOST, NULL, 0, 0);

	if (dwRetval == 0)
	{
		na.address = QString::fromUcs2(hostname);
		na.lastcheck = GetCurrentTime64();
	}
	return na.address;
#else
	iaHost.s_addr = htonl(ip);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);

	if (lpHostEntry)
		return QString::fromLocal8Bit(lpHostEntry->h_name);
#endif
	return QString::null;
}


#if !defined(_MSC_VER) || _MSC_VER < 1800
QString
ResolveAliases4(uint32 ip)
{
	//
	NetAddress4 na;

	if (fAddressCache4.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache4.GetNumItems(); i++)
		{
			na = fAddressCache4[i];
			if (na.ip == ip)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
					return na.aliases;
				else
				{
					UpdateEntry4(na, ip);
					if (na.address != QString::null)
					{
						fAddressCache4.ReplaceItemAt(i, na);
					}
					return na.aliases;
				}
			}
		};
	}

	//

	ResolveAliasesAux4(na, ip);
	fAddressCache4.AddTail(na);
	return na.aliases;
}
#endif
