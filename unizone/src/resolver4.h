#ifndef RESOLVER4_H
#define RESOLVER4_H

#include "util/String.h"

using muscle::String;

class QString;

uint32 ResolveAddress4(const QString &address);
uint32 ResolveAddress4(const String &address);

QString ResolveHost4(uint32 ip);
QString ResolveAliases4(uint32 ip);
#endif