#include "wwarningevent.h"

WWarningEvent::WWarningEvent(const QString & msg)
		: QCustomEvent(WarningEvent)
{
	fMsg = msg;
}
