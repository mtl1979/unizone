#include "resolver4.h"
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

struct NetAddress4
{
	uint32 ip;
	QString address;
	QString aliases;
	uint64 lastcheck;
};

Queue<NetAddress4> fAddressCache4;


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

void
UpdateEntry4(NetAddress4 &na, uint32 ip)
{
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
	na.ip = ip;					// We need to remember to initialize this

	iaHost.s_addr = htonl(ip);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	UpdateEntry(na, lpHostEntry);
}

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


uint32
ResolveAddress4(const QString &address)
{
	uint32 res;
	if (ParseIP4(address, res))
		return res;

	{
		muscle::ip_address res2;
		res2 = GetHostByName(address);
		res = res2.GetLowBits() & 0xFFFFFFF;
	}

	NetAddress4 na;

	if (fAddressCache4.GetNumItems() > 0)
	{
		for (unsigned int i = 0; i < fAddressCache4.GetNumItems(); i++)
		{
			na = fAddressCache4[i];
			if ((na.address == address) || Contains(na.aliases, address) || (na.ip == res))
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
				{
					if (res != 0)
					{
						if ((address != na.address) && !Contains(na.aliases, address))
						{
							AddToList(na.aliases, address);
							fAddressCache4.ReplaceItemAt(i, na);
						}
					}

					return na.ip;
				}
				else
				{
					if (res != 0)	// Do not cache failures
					{
						na.address = ResolveHost4(res);		// Double check
						ResolveAliasesAux4(na, res);
						if ((address != na.address) && !Contains(na.aliases, address))
							AddToList(na.aliases, address);
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
		ResolveAliasesAux4(na, res);
		if ((na.address != address) && !Contains(na.aliases, address))
			AddToList(na.aliases, address);
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
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
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
