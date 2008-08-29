#include "werrorevent.h"
//Added by qt3to4:
#include <QCustomEvent>

WErrorEvent::WErrorEvent(const QString & msg)
		: QCustomEvent(ErrorEvent)
{
	fMsg = msg;
}
