#include "wsystemevent.h"

WSystemEvent::WSystemEvent(const QString & msg)
		: QCustomEvent(SystemEvent)
{
	fMsg = msg;
}
