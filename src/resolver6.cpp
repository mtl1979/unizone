#ifndef MUSCLE_AVOID_IPV6

#include "resolver6.h"

#ifdef _WIN32
#include <ws2tcpip.h>
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

struct NetAddress6
{
	muscle::IPAddress ip;
	QString address;
#if !defined(_MSC_VER) || _MSC_VER < 1800
	QString aliases;
#endif
	uint64 lastcheck;
};

Queue<NetAddress6> fAddressCache6;

#if !defined(_MSC_VER) || _MSC_VER < 1800
void
UpdateEntry6(NetAddress6 &na, LPHOSTENT lpHostEntry)
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
UpdateEntry6(NetAddress6 &na, muscle::IPAddress ip)
{

	na.ip = ip;					// We need to remember to initialize this

#if defined(_MSC_VER) && _MSC_VER >= 1800
	struct sockaddr_in6 saGNI;
	WCHAR hostname[NI_MAXHOST];
	DWORD dwRetval;

	ip.WriteToNetworkArray((uint8 *)&saGNI.sin6_addr.u.Byte, NULL);

	dwRetval = GetNameInfoW((struct sockaddr *) &saGNI, sizeof(struct sockaddr_in6),
		hostname, NI_MAXHOST, NULL, 0, 0);

	if (dwRetval == 0)
	{
		na.address = QString::fromUcs2(hostname);
		na.lastcheck = GetCurrentTime64();
	}
#else
	struct in6_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure

	ip.WriteToNetworkArray((uint8 *)&iaHost, NULL );
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);
	UpdateEntry6(na, lpHostEntry);
#endif
}

#if !defined(_MSC_VER) || _MSC_VER < 1800
void
ResolveAliasesAux6(NetAddress6 &na, muscle::IPAddress ip)
{
	UpdateEntry6(na, ip);
	if (na.address == QString::null)
	{
		// Safe defaults
		na.lastcheck = 0;
		na.aliases = QString::null;
	}
}
#endif

muscle::IPAddress
ResolveAddress6(const QString &address)
{
	muscle::IPAddress res;
	NetAddress6 na;
	res = GetHostByName(address);

	if (fAddressCache6.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache6.GetNumItems(); i++)
		{
			na = fAddressCache6[i];
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
							fAddressCache6.ReplaceItemAt(i, na);
						}
					}
#endif

					return na.ip;
				}
				else
				{
					if (res != 0)	// Do not cache failures
					{
						na.address = ResolveHost6(res);		// Double check
#if !defined(_MSC_VER) || _MSC_VER < 1800
						ResolveAliasesAux6(na, res);
						if ((address != na.address) && !Contains(na.aliases, address))
							AddToList(na.aliases, address);
#endif
						na.lastcheck = GetCurrentTime64();
						fAddressCache6.ReplaceItemAt(i, na);
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
		ResolveAliasesAux6(na, res);
		if ((na.address != address) && !Contains(na.aliases, address))
			AddToList(na.aliases, address);
#endif
		fAddressCache6.AddTail(na);
	}
	return res;
}

muscle::IPAddress
ResolveAddress6(const String &address)
{
	return ResolveAddress6( QString::fromLocal8Bit( address.Cstr() ) );
}

QString
ResolveHost6(muscle::IPAddress ip)
{
#if !defined(_MSC_VER) || _MSC_VER < 1800
	struct in6_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
#endif

	//
	NetAddress6 na;

	if (fAddressCache6.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache6.GetNumItems(); i++)
		{
			na = fAddressCache6[i];
			if (na.ip == ip)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME)
				{
					if (!na.address.isEmpty())
						return na.address;
				}
#if defined(_MSC_VER) && _MSC_VER >= 1800
				struct sockaddr_in6 saGNI;
				WCHAR hostname[NI_MAXHOST];
				DWORD dwRetval;

				ip.WriteToNetworkArray((uint8 *)&saGNI.sin6_addr.u.Byte, (uint32 *)&saGNI.sin6_scope_id);

				dwRetval = GetNameInfoW((struct sockaddr *) &saGNI, sizeof(struct sockaddr_in6),
					hostname, NI_MAXHOST, NULL, 0, 0);

				if (dwRetval == 0)
				{
					na.address = QString::fromUcs2(hostname);
					na.lastcheck = GetCurrentTime64();
				}
#else
				ip.WriteToNetworkArray((uint8 *)&iaHost, NULL);
				lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);

				if (!lpHostEntry)
				{
					QStringTokenizer tok(na.aliases, ",");
					QString alias;
					while (!lpHostEntry && (alias = tok.GetNextToken()) != QString::null)
						lpHostEntry = gethostbyname((const char *) alias.local8Bit());
				}

				if (lpHostEntry)
				{
					UpdateEntry6(na, lpHostEntry);
					fAddressCache6.ReplaceItemAt(i, na);
				}
#endif
				return na.address;
			}
		};
	}

	//

#if defined(_MSC_VER) && _MSC_VER >= 1800
	struct sockaddr_in6 saGNI;
	WCHAR hostname[NI_MAXHOST];
	DWORD dwRetval;

	ip.WriteToNetworkArray((uint8 *)&saGNI.sin6_addr.u.Byte, (uint32 *)&saGNI.sin6_scope_id);

	dwRetval = GetNameInfoW((struct sockaddr *) &saGNI, sizeof(struct sockaddr_in6),
		hostname, NI_MAXHOST, NULL, 0, 0);

	if (dwRetval == 0)
	{
		na.address = QString::fromUcs2(hostname);
		na.lastcheck = GetCurrentTime64();
	}
	return na.address;
#else
	ip.WriteToNetworkArray((uint8 *)&iaHost, NULL);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);

	if (lpHostEntry)
		return QString::fromLocal8Bit(lpHostEntry->h_name);
#endif
	return QString::null;
}


#if !defined(_MSC_VER) || _MSC_VER < 1800
QString
ResolveAliases6(muscle::IPAddress ip)
{
	//
	NetAddress6 na;

	if (fAddressCache6.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache6.GetNumItems(); i++)
		{
			na = fAddressCache6[i];
			if (na.ip == ip)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME)
					return na.aliases;
				else
				{
					UpdateEntry6(na, ip);
					if (na.address != QString::null)
					{
						fAddressCache6.ReplaceItemAt(i, na);
					}
					return na.aliases;
				}
			}
		};
	}

	//

	ResolveAliasesAux6(na, ip);
	fAddressCache6.AddTail(na);
	return na.aliases;
}
#endif
#endif