#include "wmessageevent.h"

WMessageEvent::WMessageEvent(int mt, const String &from)
: QCustomEvent(MessageEventType)
{
	msgtype = mt;
	sender = from;
}

WMessageEvent::WMessageEvent(int mt, const String &from, const MessageRef& msg)
: QCustomEvent(MessageEventType)
{
	msgtype = mt;
	sender = from;
	message = msg;
}

WMessageEvent::WMessageEvent(int mt, const MessageRef& msg)
: QCustomEvent(MessageEventType)
{
	msgtype = mt;
	message = msg;
}

WMessageEvent::~WMessageEvent()
{
}

int
WMessageEvent::MessageType() const
{
	return msgtype;
}

String
WMessageEvent::Sender() const
{
	return sender;
}

MessageRef
WMessageEvent::Message() const
{
	return message;
}
