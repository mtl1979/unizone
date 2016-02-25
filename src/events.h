#ifndef EVENTS_H
#define EVENTS_H

#include "textevent.h"

#include "message/Message.h"

using namespace muscle;

void TextEvent(QObject *target, const QString &text, WTextEvent::Type t);
void SystemEvent(QObject *target, const QString &text);
void ErrorEvent(QObject *target, const QString &text);
void WarningEvent(QObject *target, const QString &text);

// from NetClient
void SendEvent(QObject *target, int type, const String &from,  MessageRef msg);
void SendEvent(QObject *target, int type, const String &from);
void SendEvent(QObject *target, int type, MessageRef msg);

#endif
