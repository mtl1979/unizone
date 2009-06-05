#include "resolver.h"

#ifndef _WIN32
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
	uint32 ip;
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
UpdateEntry(NetAddress &na, uint32 ip)
{
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
	na.ip = ip;					// We need to remember to initialize this

	iaHost.s_addr = htonl(ip);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	UpdateEntry(na, lpHostEntry);
}

void
ResolveAliasesAux(NetAddress &na, uint32 ip)
{
	UpdateEntry(na, ip);
	if (na.address == QString::null)
	{
		// Safe defaults
		na.lastcheck = 0;
		na.aliases = QString::null;
	}
}

uint32
ResolveAddress(const QString &address)
{
	uint32 res;
	if (ParseIP4(address, res))
		return res;

	NetAddress na;
	res = GetHostByName(address);

	if (fAddressCache.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache.GetNumItems(); i++)
		{
			na = fAddressCache[i];
			if ((na.address == address) || Contains(na.aliases, address) || (na.ip == res))
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
				{
					if (res != 0)
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
					if (res != 0)	// Do not cache failures
					{
						na.address = ResolveHost(res);		// Double check
						ResolveAliasesAux(na, res);
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

	if (res != 0)						// Do not cache failures
	{
		ResolveAliasesAux(na, res);
		if ((na.address != address) && !Contains(na.aliases, address))
			AddToList(na.aliases, address);
		fAddressCache.AddTail(na);
	}
	return res;
}

uint32
ResolveAddress(const String &address)
{
	return ResolveAddress( QString::fromLocal8Bit( address.Cstr() ) );
}

QString
ResolveHost(uint32 ip)
{
	struct in_addr iaHost;	   // Internet address structure
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
					fAddressCache.ReplaceItemAt(i, na);
				}
				return na.address;
			}
		};
	}

	//

	iaHost.s_addr = htonl(ip);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	if (lpHostEntry)
		return QString::fromLocal8Bit(lpHostEntry->h_name);
	return QString::null;
}

QString
ResolveAliases(uint32 ip)
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
