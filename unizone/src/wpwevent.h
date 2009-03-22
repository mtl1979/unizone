#ifndef WPWEVENT_H
#define WPWEVENT_H

#include <qcoreevent.h>
#include "user.h"

class QString;

class WPWEvent : public QCustomEvent
{
public:
	WPWEvent(int type, WUserMap & users, const QString & msg, bool encrypted = false);
	WPWEvent(int type, const QString & msg);
	WPWEvent(int type);

	virtual ~WPWEvent() {}

	QString GetText() const { return fMsg; }
	void SetText(const QString & txt) { fMsg = txt; }
	QObject * SendTo() { return fReply; }
	void SetSendTo(QObject * o) { fReply = o; }
	bool Encrypted() { return fEncrypted; }

	enum
	{
		TextEvent = QEvent::User + 7000,
		TextPosted,			// response to textevent
		TabComplete,		// requesting a tab complete...
		TabCompleted,		// response to tab complete...
		Created,
		Closed
	};

	void SetWantReply(bool t) { fWant = t; }
	bool GetWantReply() const { return fWant; }

private:
	bool fWant, fEncrypted;
	QString fMsg;
	QObject * fReply;

};

#endif
