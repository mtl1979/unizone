#include "events.h"

#include "debugimpl.h"
#include "wsystemevent.h"
#include "werrorevent.h"
#include "wwarningevent.h"

#include <qapplication.h>

void
SystemEvent(QObject *target, const QString &text)
{
	WASSERT(text.length() > 0, "Invalid text passed to SystemEvent().");
	WSystemEvent *wse = new WSystemEvent(text);
	if (wse)
		QApplication::postEvent(target, wse);
}

void
TextEvent(QObject *target, const QString &text, WTextEvent::Type t)
{
	WASSERT(text.length() > 0, "Invalid text passed to TextEvent().");
	WTextEvent *wte = new WTextEvent(text, t);
	if (wte)
		QApplication::postEvent(target, wte);
}

void
ErrorEvent(QObject *target, const QString &text)
{
	WASSERT(text.length() > 0, "Invalid text passed to ErrorEvent().");
	WErrorEvent *wee = new WErrorEvent(text);
	if (wee)
		QApplication::postEvent(target, wee);
}

void
WarningEvent(QObject *target, const QString &text)
{
	WASSERT(text.length() > 0, "Invalid text passed to WarningEvent().");
	WWarningEvent *wwe = new WWarningEvent(text);
	if (wwe)
		QApplication::postEvent(target, wwe);
}
