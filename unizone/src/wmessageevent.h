#ifndef WMESSAGEEVENT_H
#define WMESSAGEEVENT_H

#include <qevent.h>

#include "message/Message.h"
#include "util/String.h"

using namespace muscle;

class WMessageEvent : public QCustomEvent
{
public:
	enum
	{
		MessageEventType = 'wmse',
		HandleMessage,
		ServerParametersMessage,
		BeAddMessage,
		BeRemoveMessage,
		UniAddMessage,
		UniRemoveMessage
	};

	WMessageEvent(int, const String &);
	WMessageEvent(int, const String &, const MessageRef &);
	WMessageEvent(int, const MessageRef &);
	~WMessageEvent();

	int MessageType() const;
	String Sender() const;
	MessageRef Message() const;
private:
	int msgtype;
	String sender;
	MessageRef message;
};
#endif
