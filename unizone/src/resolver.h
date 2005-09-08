#ifndef RESOLVER_H
#define RESOLVER_H

#include "util/String.h"
#include <qstring.h>

using muscle::String;

uint32 ResolveAddress(const QString &address);
uint32 ResolveAddress(const String &address);

QString ResolveHost(uint32 ip);

#endif
