#ifndef EVENTS_H
#define EVENTS_H

#include "textevent.h"

void TextEvent(QObject *target, const QString &text, WTextEvent::Type t);
void SystemEvent(QObject *target, const QString &text);
void ErrorEvent(QObject *target, const QString &text);
void WarningEvent(QObject *target, const QString &text);

#endif
