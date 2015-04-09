#ifndef RESOLVER4_H
#define RESOLVER4_H

#include "util/String.h"

using muscle::String;

class QString;

uint32 ResolveAddress4(const QString &address);
uint32 ResolveAddress4(const String &address);

QString ResolveHost4(uint32 ip);
#if !defined(_MSC_VER) || _MSC_VER < 1800
QString ResolveAliases4(uint32 ip);
#endif
#endif