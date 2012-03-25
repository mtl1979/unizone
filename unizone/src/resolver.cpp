#include "resolver.h"

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

struct NetAddress
{
#ifdef MUSCLE_AVOID_IPV6
	uint32 ip;
#else
	muscle::ip_address ip;
#endif
	QString address;
	QString aliases;
	uint64 lastcheck;
};

Queue<NetAddress> fAddressCache;


bool
ParseIP4(const QString &address, uint32 &result)
{
	// Borrowed from QHostAddress ;)
	QStringList ipv4 = QStringList::split(".", address, false);
	if (ipv4.count() == 4) {
		bool ok = true;
		for (int i = 0; i < 4; i++) 
		{
			uint byteValue = ipv4[i].toUInt(&ok);

			if (!ok || (byteValue > 255))
				return false;

			result = (result << 8) + byteValue;
		}
		return true;
	}
	return false;
}

void
UpdateEntry(NetAddress &na, LPHOSTENT lpHostEntry)
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

void
#ifdef MUSCLE_AVOID_IPV6
UpdateEntry(NetAddress &na, uint32 ip)
#else
UpdateEntry(NetAddress &na, muscle::ip_address ip)
#endif
{
#ifdef MUSCLE_AVOID_IPV6
	struct in_addr iaHost;	   // Internet address structure
#else
	struct in6_addr iaHost;	   // Internet address structure
#endif
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
	na.ip = ip;					// We need to remember to initialize this

#ifdef MUSCLE_AVOID_IPV6
	iaHost.s_addr = htonl(ip);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
#else
	ip.WriteToNetworkArray((uint8 *)&iaHost, NULL );
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);
#endif
	UpdateEntry(na, lpHostEntry);
}

void
#ifdef MUSCLE_AVOID_IPV6
ResolveAliasesAux(NetAddress &na, uint32 ip)
#else
ResolveAliasesAux(NetAddress &na, muscle::ip_address ip)
#endif
{
	UpdateEntry(na, ip);
	if (na.address == QString::null)
	{
		// Safe defaults
		na.lastcheck = 0;
		na.aliases = QString::null;
	}
}

#ifdef MUSCLE_AVOID_IPV6
uint32
#else
muscle::ip_address
#endif
ResolveAddress(const QString &address)
{
#ifndef MUSCLE_AVOID_IPV6
	{
#endif
	uint32 res;
	if (ParseIP4(address, res))
		return res;
#ifndef MUSCLE_AVOID_IPV6
	}
#endif

#ifdef MUSCLE_AVOID_IPV6
	{
#endif
	muscle::ip_address res2;
	NetAddress na;
	res2 = GetHostByName(address);
#ifdef MUSCLE_AVOID_IPV6
	res = res2;
	}
#endif

	if (fAddressCache.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache.GetNumItems(); i++)
		{
			na = fAddressCache[i];
#ifdef MUSCLE_AVOID_IPV6
			if ((na.address == address) || Contains(na.aliases, address) || (na.ip == res))
#else
			if ((na.address == address) || Contains(na.aliases, address) || (na.ip == res2))
#endif
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
				{
#ifdef MUSCLE_AVOID_IPV6
					if (res != 0)
#else
					if (res2 != 0)
#endif
					{
						if ((address != na.address) && !Contains(na.aliases, address))
						{
							AddToList(na.aliases, address);
							fAddressCache.ReplaceItemAt(i, na);
						}
					}

					return na.ip;
				}
				else
				{
#ifdef MUSCLE_AVOID_IPV6
					if (res != 0)	// Do not cache failures
#else
					if (res2 != 0)	// Do not cache failures
#endif
					{
#ifdef MUSCLE_AVOID_IPV6
						na.address = ResolveHost(res);		// Double check
						ResolveAliasesAux(na, res);
#else
						na.address = ResolveHost(res2);		// Double check
						ResolveAliasesAux(na, res2);
#endif
						if ((address != na.address) && !Contains(na.aliases, address))
							AddToList(na.aliases, address);
						na.lastcheck = GetCurrentTime64();
						fAddressCache.ReplaceItemAt(i, na);
					}
					return na.ip;
				}
			}
		};
	}

	//

#ifdef MUSCLE_AVOID_IPV6
	if (res != 0)						// Do not cache failures
#else
	if (res2 != 0)
#endif
	{
#ifdef MUSCLE_AVOID_IPV6
		ResolveAliasesAux(na, res);
#else
		ResolveAliasesAux(na, res2);
#endif
		if ((na.address != address) && !Contains(na.aliases, address))
			AddToList(na.aliases, address);
		fAddressCache.AddTail(na);
	}
#ifdef MUSCLE_AVOID_IPV6
	return res;
#else
	return res2;
#endif
}

#ifdef MUSCLE_AVOID_IPV6
uint32
#else
muscle::ip_address
#endif
ResolveAddress(const String &address)
{
	return ResolveAddress( QString::fromLocal8Bit( address.Cstr() ) );
}

QString
#ifdef MUSCLE_AVOID_IPV6
ResolveHost(uint32 ip)
#else
ResolveHost(muscle::ip_address ip)
#endif
{
#ifdef MUSCLE_AVOID_IPV6
	struct in_addr iaHost;	   // Internet address structure
#else
	struct in6_addr iaHost;	   // Internet address structure
#endif
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
	//
	NetAddress na;

	if (fAddressCache.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache.GetNumItems(); i++)
		{
			na = fAddressCache[i];
			if (na.ip == ip)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
				{
					if (!na.address.isEmpty())
						return na.address;
				}
#ifdef MUSCLE_AVOID_IPV6
				iaHost.s_addr = htonl(ip);
				lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
#else
				ip.WriteToNetworkArray((uint8 *)&iaHost, NULL);
				lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);
#endif
				
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
					fAddressCache.ReplaceItemAt(i, na);
				}
				return na.address;
			}
		};
	}

	//

#ifdef MUSCLE_AVOID_IPV6
	iaHost.s_addr = htonl(ip);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
#else
	ip.WriteToNetworkArray((uint8 *)&iaHost, NULL);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);
#endif
	if (lpHostEntry)
		return QString::fromLocal8Bit(lpHostEntry->h_name);
	return QString::null;
}

QString
#ifdef MUSCLE_AVOID_IPV6
ResolveAliases(uint32 ip)
#else
ResolveAliases(muscle::ip_address ip)
#endif
{
	//
	NetAddress na;

	if (fAddressCache.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache.GetNumItems(); i++)
		{
			na = fAddressCache[i];
			if (na.ip == ip)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
					return na.aliases;
				else
				{
					UpdateEntry(na, ip);
					if (na.address != QString::null)
					{
						fAddressCache.ReplaceItemAt(i, na);
					}
					return na.aliases;
				}
			}
		};
	}

	//

	ResolveAliasesAux(na, ip);
	fAddressCache.AddTail(na);
	return na.aliases;
}
