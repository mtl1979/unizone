#ifndef RESOLVER_H
#define RESOLVER_H

#include "util/NetworkUtilityFunctions.h"
#include "util/String.h"

using muscle::String;

class QString;

bool ParseIP4(const QString &address, uint32 &result);

#ifdef MUSCLE_AVOID_IPV6
uint32 ResolveAddress(const QString &address);
uint32 ResolveAddress(const String &address);

QString ResolveHost(uint32 ip);
#if !defined(_MSC_VER) || _MSC_VER < 1800
QString ResolveAliases(uint32 ip);
#endif
#define ConvertIP4(x) x
#else
muscle::ip_address ResolveAddress(const QString &address);
muscle::ip_address ResolveAddress(const String &address);

QString ResolveHost(muscle::ip_address ip);
#if !defined(_MSC_VER) || _MSC_VER < 1800
QString ResolveAliases(muscle::ip_address ip);
#endif
uint32 ConvertIP4(muscle::ip_address ip);
#endif

#endif
