#include "werrorevent.h"
#include <QCustomEvent>

WErrorEvent::WErrorEvent(const QString & msg)
		: QCustomEvent(ErrorEvent)
{
	fMsg = msg;
}
