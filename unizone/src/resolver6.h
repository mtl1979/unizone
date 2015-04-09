#ifndef RESOLVER6_H
#define RESOLVER6_H
#ifndef MUSCLE_AVOID_IPV6

#include "util/NetworkUtilityFunctions.h"
#include "util/String.h"

using muscle::String;

class QString;

muscle::ip_address ResolveAddress6(const QString &address);
muscle::ip_address ResolveAddress6(const String &address);

QString ResolveHost6(muscle::ip_address ip);
#if !defined(_MSC_VER) || _MSC_VER < 1800
QString ResolveAliases6(muscle::ip_address ip);
#endif
#endif
#endif