#ifndef LISTUTIL_H
#define LISTUTIL_H

#include "util/String.h"

using namespace muscle;

class QString;

void AddToList(QString &slist, const QString &entry);
void AddToList(String &slist, const String &entry);

void RemoveFromList(QString &slist, const QString &entry);
void RemoveFromList(String &slist, const String &entry);

bool Contains(const QString &slist, const QString &entry);

#endif
