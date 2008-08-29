#ifdef WIN32
#pragma warning(disable: 4512 4786)
#endif

#include "wmessageevent.h"
//Added by qt3to4:
#include <QCustomEvent>

WMessageEvent::WMessageEvent(MessageRef msg)
: QCustomEvent(MessageEventType)
{
	msgtype = MessageEventType;
	message = msg;
}

WMessageEvent::WMessageEvent(int mt, const String &from)
: QCustomEvent(MessageEventType)
{
	msgtype = mt;
	sender = from;
}

WMessageEvent::WMessageEvent(int mt, const String &from, MessageRef msg)
: QCustomEvent(MessageEventType)
{
	msgtype = mt;
	sender = from;
	message = msg;
}

WMessageEvent::WMessageEvent(int mt, MessageRef msg)
: QCustomEvent(MessageEventType)
{
	msgtype = mt;
	message = msg;
}

WMessageEvent::WMessageEvent(int mt)
: QCustomEvent(MessageEventType)
{
	msgtype = mt;
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
