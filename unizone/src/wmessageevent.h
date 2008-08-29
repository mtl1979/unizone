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
		MessageEventType = QEvent::User + 28100,
		HandleMessage,
		ServerParametersMessage,
		BeAddMessage,
		BeRemoveMessage,
		UniAddMessage,
		UniRemoveMessage,
		UnknownAddMessage,
		UnknownRemoveMessage
	};

	WMessageEvent(MessageRef);
	WMessageEvent(int, const String &);
	WMessageEvent(int, const String &, MessageRef);
	WMessageEvent(int, MessageRef);
	WMessageEvent(int);
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
