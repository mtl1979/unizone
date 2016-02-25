#include <QString>

#include "textevent.h"
#include <QCustomEvent>

WTextEvent::WTextEvent(int type)
: QCustomEvent(type), fValid(false)
{
	// empty
	fEncrypted = false;
}

WTextEvent::WTextEvent(const QString & text, int type)
: QCustomEvent(type), fValid(false)
{
	fText = text.stripWhiteSpace();
	fEncrypted = false;
	// <postmaster@raasu.org> 20021024
	if (fText.isEmpty())
		return; // empty string

	for (int i = 0; i < fText.length(); i++)
	{
		// go through the text and make sure it contains
		// some data other than line feeds, etc
		switch (fText.at(i).unicode())
		{
			case '\n':
			case '\r':
			case '\t':
			case ' ':
				break;
			default:
			{
				fValid = true;
				return;
			}
		}
	}
}

WTextEvent::~WTextEvent()
{
}

bool
WTextEvent::Valid() const
{
	return fValid;
}

QString
WTextEvent::Text() const
{
	return fText;
}

void
WTextEvent::SetText(const QString & str)
{
	fText = str;
}
