#ifndef RESOLVER_H
#define RESOLVER_H

#include "util/String.h"

using muscle::String;

class QString;

uint32 ResolveAddress(const QString &address);
uint32 ResolveAddress(const String &address);

QString ResolveHost(uint32 ip);
QString ResolveAliases(uint32 ip);

#endif
