#include "events.h"
#include "wsystemevent.h"
#include "werrorevent.h"
#include "wwarningevent.h"

#include <qapplication.h>

void
SystemEvent(QObject *target, const QString &text)
{
	WSystemEvent *wse = new WSystemEvent(text);
	if (wse)
		QApplication::postEvent(target, wse);
}

void
TextEvent(QObject *target, const QString &text, WTextEvent::Type t)
{
	WTextEvent *wte = new WTextEvent(text, t);
	if (wte)
		QApplication::postEvent(target, wte);
}

void
ErrorEvent(QObject *target, const QString &text)
{
	WErrorEvent *wee = new WErrorEvent(text);
	if (wee)
		QApplication::postEvent(target, wee);
}

void
WarningEvent(QObject *target, const QString &text)
{
	WWarningEvent *wwe = new WWarningEvent(text);
	if (wwe)
		QApplication::postEvent(target, wwe);
}
