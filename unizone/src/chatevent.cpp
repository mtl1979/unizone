#include "chatevent.h"

WChatEvent::WChatEvent(const QString &sender, const QString & text)
: QCustomEvent(ChatTextType)
{
	fText = text.stripWhiteSpace();
	fSender = sender;
}

WChatEvent::~WChatEvent()
{
}
