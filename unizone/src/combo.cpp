#include "combo.h"
#include "debugimpl.h"
#include "textevent.h"

#include <qapplication.h>

WComboBox::WComboBox(QObject * owner, QWidget * parent, const char * name)
	: QComboBox(parent, name), fOwner(owner)
{
	if (!name)
		setName("WComboBox");
	setFocusPolicy(QWidget::StrongFocus);
	if (connect(this, SIGNAL(activated(const QString &)), this,
				SLOT(SendEvent(const QString &))))
		PRINT("WComboBox: Connected activated\n");
}

WComboBox::~WComboBox()
{
}

void
WComboBox::focusInEvent(QFocusEvent * /* e */)
{
	PRINT("Focused!\n");
}

void
WComboBox::SendEvent(const QString & str)
{
	WTextEvent *e = new WTextEvent(str, WTextEvent::ComboType); // <postmaster@raasu.org> 20021012
	if (e)
	{
		if (!e->Valid())
		{
			delete e;
			e = NULL;
			return;
		}
		e->setData((void *)this);
		QApplication::postEvent(fOwner, e);
	}
}
