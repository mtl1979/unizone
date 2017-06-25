#ifndef RESOLVER6_H
#define RESOLVER6_H
#ifndef MUSCLE_AVOID_IPV6

#include "util/NetworkUtilityFunctions.h"
#include "util/String.h"

using muscle::String;

class QString;

muscle::IPAddress ResolveAddress6(const QString &address);
muscle::IPAddress ResolveAddress6(const String &address);

QString ResolveHost6(muscle::IPAddress ip);
#if !defined(_MSC_VER) || _MSC_VER < 1800
QString ResolveAliases6(muscle::IPAddress ip);
#endif
#endif
#endif