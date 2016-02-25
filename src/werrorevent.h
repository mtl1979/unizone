#ifndef WERROREVENT_H
#define WERROREVENT_H

#include <qstring.h>
#include <qevent.h>

class WErrorEvent : public QCustomEvent
{
public:
	WErrorEvent(const QString & msg);

	virtual ~WErrorEvent() {}

	QString GetText() const { return fMsg; }
	void SetText(const QString & txt) { fMsg = txt; }

	enum
	{
		ErrorEvent = QEvent::User + 13000
	};


private:
	QString fMsg;

};

#endif
