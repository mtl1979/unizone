#include "wsystemevent.h"
//Added by qt3to4:
#include <QCustomEvent>

WSystemEvent::WSystemEvent(const QString & msg)
		: QCustomEvent(SystemEvent)
{
	fMsg = msg;
}
