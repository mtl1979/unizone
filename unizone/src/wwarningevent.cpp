#include "wwarningevent.h"
//Added by qt3to4:
#include <QCustomEvent>

WWarningEvent::WWarningEvent(const QString & msg)
		: QCustomEvent(WarningEvent)
{
	fMsg = msg;
}
