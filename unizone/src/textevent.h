#ifndef TEXTEVENT_H
#define TEXTEVENT_H

#include <qevent.h>

/** This event is send when the user pressed enter in
  *	the text input area.
  * Always call Valid() before sending, as the event makes
  * sure that an empty string is not send
  * this is kinda of generic event.
  */
class WTextEvent : public QCustomEvent
{
public:
	enum Type
	{
		TextType = User + 5000,		// user sent text
		ComboType,					// combo box alert that the text HAS changed
		ResumeType,					// check if user has resumable files
		ChatTextEvent				// printable text
	};

	WTextEvent(const QString & text, int type = TextType);
	virtual ~WTextEvent();

	bool Valid() const;

	QString Text() const;
	void SetText(const QString & str) { fText = str; }

private:
	QString fText;
	bool fValid;
};

#endif	// TEXTEVENT_H
