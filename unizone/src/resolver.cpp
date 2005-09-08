#include "resolver.h"
#include "util.h"

#include "util/Queue.h"
#include "util/NetworkUtilityFunctions.h"

using muscle::Queue;

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
typedef hostent *LPHOSTENT;
#endif

#include <qstringlist.h>

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

			if (byteValue > 255)
				return false;

			if (ok)
				result = (result << 8) + byteValue;
		}
		return true;
    }
    return false;
}

uint32
ResolveAddress(const QString &address)
{
	uint32 res;
	if (ParseIP4(address, res))
		return res;

	NetAddress na;

	if (fAddressCache.GetNumItems() > 0)
	{
		unsigned int i = 0;
		do
		{
			na = fAddressCache[i];
			if (na.address == address)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
					return na.ip;
				else
				{
					uint32 ip = GetHostByName(address);
					if (ip != 0)	// Do not cache failures
					{
						na.ip = ip;
						na.lastcheck = GetCurrentTime64();
						fAddressCache.ReplaceItemAt(i, na);
					}
					return na.ip;
				}
			}
			i++;
		} while (i < fAddressCache.GetNumItems());
	}
	na.address = address;
	na.ip = GetHostByName(address);
	if (na.ip != 0)						// Do not cache failures
		fAddressCache.AddTail(na);	
	return na.ip;
}

uint32
ResolveAddress(const String &address)
{
	return ResolveAddress( QString::fromLocal8Bit( address.Cstr() ) );
}

QString
ResolveHost(uint32 ip)
{
	char host[16];
	QString qhost;
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	
	Inet_NtoA(ip, host);
	qhost = QString::fromLocal8Bit(host);

	//
	NetAddress na;

	if (fAddressCache.GetNumItems() > 0)
	{
		unsigned int i = 0;
		do
		{
			na = fAddressCache[i];
			if (na.ip == ip)
			{
				if ((GetCurrentTime64() - na.lastcheck) < EXPIRETIME) 
					return na.address;
				else
				{
					iaHost.s_addr = inet_addr(host);
					lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
					if (lpHostEntry)
					{
						na.address = QString::fromLocal8Bit(lpHostEntry->h_name);
						na.lastcheck = GetCurrentTime64();
						fAddressCache.ReplaceItemAt(i, na);
					}
					return na.address;
				}
			}
			i++;
		} while (i < fAddressCache.GetNumItems());
	}
	//
	iaHost.s_addr = inet_addr(host);
	lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	if (lpHostEntry)
		return QString::fromLocal8Bit(lpHostEntry->h_name);
	return QString::null;
}
