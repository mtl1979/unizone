#ifndef WWARNINGEVENT_H
#define WWARNINGEVENT_H

#include "user.h"

#include <qstring.h>
#include <qevent.h>

class WWarningEvent : public QCustomEvent
{
public:
	WWarningEvent(const QString & msg);

	virtual ~WWarningEvent() {}

	QString GetText() const { return fMsg; }
	void SetText(const QString & txt) { fMsg = txt; }

	enum
	{
		WarningEvent = QEvent::User + 12500
	};


private:
	QString fMsg;

};

#endif
