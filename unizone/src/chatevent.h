#ifndef CHATEVENT_H
#define CHATEVENT_H

#include <qevent.h>

/**
  * This event is sent when remote user sends private message
  */
class WChatEvent : public QCustomEvent
{
public:
	enum Type
	{
		ChatTextType = QEvent::User + 9000	// user sent text
	};

	WChatEvent(const QString & sender, const QString & text);
	virtual ~WChatEvent();

	QString Text() const { return fText; }
	void SetText(const QString & str) { fText = str; }
	QString Sender() const { return fSender; }

private:
	QString fSender, fText;
};

#endif	// TEXTEVENT_H
