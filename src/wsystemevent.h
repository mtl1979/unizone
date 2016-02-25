#ifndef WSYSTEMEVENT_H
#define WSYSTEMEVENT_H

#include <qstring.h>
#include <qevent.h>

class WSystemEvent : public QCustomEvent
{
public:
	WSystemEvent(const QString & msg);

	virtual ~WSystemEvent() {}

	QString GetText() const { return fMsg; }
	void SetText(const QString & txt) { fMsg = txt; }

	enum
	{
		SystemEvent = QEvent::User + 12000
	};


private:
	QString fMsg;

};

#endif
