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
	muscle::ip_address ip;
	QString address;
	QString aliases;
	uint64 lastcheck;
};

Queue<NetAddress6> fAddressCache6;

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

void
UpdateEntry6(NetAddress6 &na, muscle::ip_address ip)
{
	struct in6_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
	na.ip = ip;					// We need to remember to initialize this

	ip.WriteToNetworkArray((uint8 *)&iaHost, NULL );
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);
	UpdateEntry6(na, lpHostEntry);
}

void
ResolveAliasesAux6(NetAddress6 &na, muscle::ip_address ip)
{
	UpdateEntry6(na, ip);
	if (na.address == QString::null)
	{
		// Safe defaults
		na.lastcheck = 0;
		na.aliases = QString::null;
	}
}

muscle::ip_address
ResolveAddress6(const QString &address)
{
	muscle::ip_address res;
	NetAddress6 na;
	res = GetHostByName(address);

	if (fAddressCache6.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache6.GetNumItems(); i++)
		{
			na = fAddressCache6[i];
			if ((na.address == address) || Contains(na.aliases, address) || (na.ip == res))
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
				{
					if (res != 0)
					{
						if ((address != na.address) && !Contains(na.aliases, address))
						{
							AddToList(na.aliases, address);
							fAddressCache6.ReplaceItemAt(i, na);
						}
					}

					return na.ip;
				}
				else
				{
					if (res != 0)	// Do not cache failures
					{
						na.address = ResolveHost6(res);		// Double check
						ResolveAliasesAux6(na, res);
						if ((address != na.address) && !Contains(na.aliases, address))
							AddToList(na.aliases, address);
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
		ResolveAliasesAux6(na, res);
		if ((na.address != address) && !Contains(na.aliases, address))
			AddToList(na.aliases, address);
		fAddressCache6.AddTail(na);
	}
	return res;
}

muscle::ip_address
ResolveAddress6(const String &address)
{
	return ResolveAddress6( QString::fromLocal8Bit( address.Cstr() ) );
}

QString
ResolveHost6(muscle::ip_address ip)
{
	struct in6_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
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
				return na.address;
			}
		};
	}

	//

	ip.WriteToNetworkArray((uint8 *)&iaHost, NULL);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in6_addr), AF_INET6);

	if (lpHostEntry)
		return QString::fromLocal8Bit(lpHostEntry->h_name);
	return QString::null;
}

QString
ResolveAliases6(muscle::ip_address ip)
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