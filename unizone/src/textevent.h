#ifndef TEXTEVENT_H
#define TEXTEVENT_H

#include <qcoreevent.h>

class QString;

/** This event is sent when the user pressed enter in
  *	the text input area.
  * Always call Valid() before sending, as the event makes
  * sure that an empty string is not sent
  * this is kinda of generic event.
  */
class WTextEvent : public QCustomEvent
{
public:
	enum Type
	{
		TextType = QEvent::User + 5000,		// user sent text
		ComboType,							// combo box alert that the text HAS changed
		ResumeType,							// check if user has resumable files
		ChatTextEvent,						// printable text
		UserUpdateEvent
	};

	WTextEvent(int type = TextType);
	WTextEvent(const QString & text, int type = TextType);
	virtual ~WTextEvent();

	bool Valid() const;

	QString Text() const;
	void SetText(const QString & str);
	bool Encrypted() const { return fEncrypted; }
	void SetEncrypted(bool e) { fEncrypted = e; } 

private:
	QString fText;
	bool fValid, fEncrypted;
};

#endif	// TEXTEVENT_H
