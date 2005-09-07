#include "resolver.h"
#include "util.h"

#include "util/Queue.h"

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
	uint64 lastcheck;
};

Queue<NetAddress> fAddressCache;

uint32
ResolveAddress(const QString &address)
{
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
