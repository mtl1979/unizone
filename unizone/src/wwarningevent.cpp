#include "wwarningevent.h"
#include <QCustomEvent>

WWarningEvent::WWarningEvent(const QString & msg)
		: QCustomEvent(WarningEvent)
{
	fMsg = msg;
}
