#include "wsystemevent.h"
#include <QCustomEvent>

WSystemEvent::WSystemEvent(const QString & msg)
		: QCustomEvent(SystemEvent)
{
	fMsg = msg;
}
