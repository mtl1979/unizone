#include "textevent.h"

WTextEvent::WTextEvent(const QString & text, int type)
: QCustomEvent(type), fValid(false)
{
	fText = text.stripWhiteSpace();
	fEncrypted = false;
	// <postmaster@raasu.org> 20021024 
	if (fText.length() == 0) 
		return; // empty string
	
	for (unsigned int i = 0; i < fText.length(); i++)
	{
		// go through the text and make sure it contains
		// some data other then line feeds, etc
		switch ((QChar)fText.at(i))
		{
			case '\n':
			case '\r':
			case '\t':
			case ' ':
				break;
			default:
			{
				fValid = true;
				i = fText.length();	// set i to the length of the text to drop out of the loop
				break;
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

