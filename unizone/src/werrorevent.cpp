#include "werrorevent.h"

WErrorEvent::WErrorEvent(const QString & msg)
		: QCustomEvent(ErrorEvent)
{
	fMsg = msg;
}
