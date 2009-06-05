#ifndef MESSAGEUTIL_H
#define MESSAGEUTIL_H

class QString;

#include "message/Message.h"
#include "util/String.h"

using namespace muscle;

status_t GetStringFromMessage(const MessageRef &msg, const String key, QString &value);
status_t GetStringFromMessage(const MessageRef &msg, const String key, uint32 index, QString &value);
status_t GetInt32FromMessage(const MessageRef &msg, const String key, int32 &value);
status_t GetUInt32FromMessage(const MessageRef &msg, const String key, uint32 &value);

status_t AddStringToMessage(const MessageRef &msg, const String key, const QString &value);

status_t ReplaceStringInMessage(const MessageRef &msg, bool okayToAdd, const String key, const QString &value);
status_t ReplaceStringInMessage(const MessageRef &msg, bool okayToAdd, const String key, uint32 index, const QString &value);

#endif