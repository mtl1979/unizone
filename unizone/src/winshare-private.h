#ifndef WINSHARE_PRIVATE_H
#define WINSHARE_PRIVATE_H

#include "textevent.h"
#include "wsystemevent.h"

void TextEvent(QObject *target, const QString &text, WTextEvent::Type t);
void SystemEvent(QObject *target, const QString &text);

void AddToList(QString &slist, const QString &entry);

#endif
