#include "resolver.h"

#include "resolver4.h"
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

#ifdef MUSCLE_AVOID_IPV6
uint32 
ResolveAddress(const QString &address)
{
	return ResolveAddress4(address);
}

uint32 
ResolveAddress(const String &address)
{
	return ResolveAddress4(address);
}

QString 
ResolveHost(uint32 ip)
{
	return ResolveHost4(ip);
}

QString 
ResolveAliases(uint32 ip)
{
	return ResolveAliases4(ip);
}
#else
muscle::ip_address 
ResolveAddress(const QString &address)
{
	uint32 res;
	muscle::ip_address ip;
	if (ParseIP4(address, res))
	{
		ip.SetLowBits(res);
		return ip;
	}
	ip = ResolveAddress6(address);
	if (!IsValidAddress(ip))
	{
		res = ResolveAddress4(address);
		ip.SetBits(res, 0);
	}
	return ip;
}

muscle::ip_address 
ResolveAddress(const String &address)
{
	return ResolveAddress( QString::fromLocal8Bit( address.Cstr() ) );
}

QString 
ResolveHost(muscle::ip_address ip)
{
	if (IsIPv4Address(ip))
		return ResolveHost4(ConvertIP4(ip));
	return ResolveHost6(ip);
}

QString 
ResolveAliases(muscle::ip_address ip)
{
	if (IsIPv4Address(ip))
		return ResolveAliases4(ConvertIP4(ip));
	return ResolveAliases6(ip);
}

uint32
ConvertIP4(muscle::ip_address ip)
{
	if (IsIPv4Address(ip))
	{
		return ip.GetLowBits() & 0xFFFFFFFF;
	}
	return 0;
}
#endif


