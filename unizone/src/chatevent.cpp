#include "chatevent.h"
//Added by qt3to4:
#include <QCustomEvent>

WChatEvent::WChatEvent(const QString &sender, const QString & text)
: QCustomEvent(ChatTextType)
{
	fText = text.stripWhiteSpace();
	fSender = sender;
}

WChatEvent::~WChatEvent()
{
}
