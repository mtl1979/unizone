#include "messageutil.h"

status_t
GetStringFromMessage(const MessageRef &msg, const String key, QString &value)
{
	return GetStringFromMessage(msg, key, 0, value);
}

status_t
GetStringFromMessage(const MessageRef &msg, const String key, uint32 index, QString &value)
{
	const char * val;
        status_t ret = msg()->FindString(key, index, val);
	if (ret == B_OK)
		value = QString::fromUtf8(val);
	return ret;
}

status_t
AddStringToMessage(const MessageRef &msg, const String key, const QString &value)
{
	QByteArray val = value.utf8();
	return msg()->AddString(key, (const char *) val);
}

status_t
ReplaceStringInMessage(const MessageRef &msg, bool okayToAdd, const String key, const QString &value)
{
	return ReplaceStringInMessage(msg, okayToAdd, key, 0, value);
}

status_t
ReplaceStringInMessage(const MessageRef &msg, bool okayToAdd, const String key, uint32 index, const QString &value)
{
	QByteArray val = value.utf8();
	return msg()->ReplaceString(okayToAdd, key, index, (const char *) val);
}

status_t
GetInt32FromMessage(const MessageRef &msg, const String key, int32 &value)
{
	int32 val;
        status_t ret = msg()->FindInt32(key, val);
	if (ret == B_OK)
		value = val;
	return ret;
}

status_t
GetUInt32FromMessage(const MessageRef &msg, const String key, uint32 &value)
{
	uint32 val;
        status_t ret = msg()->FindInt32(key, val);
	if (ret == B_OK)
		value = val;
	return ret;
}

