#ifndef WPWEVENT_H
#define WPWEVENT_H
#include "user.h"
#include "privatewindowimpl.h"
#include "debugimpl.h"

#include <qstring.h>
#include <qevent.h>

class WPWEvent : public QCustomEvent
{
public:
	WPWEvent(int type, WUserMap & users, const QString & msg)
		: QCustomEvent(type)
	{
		fWant = false;
		if (type == WPWEvent::TextEvent)
		{
			fWant = true;	// this way we'll get an error string in the private window if
							// an invalid command is used
			// everything is stuck into one string
			if (msg[0] == '/' && !msg.startsWith("//"))
			{
				// just forward the message
				fMsg = msg;
			}
			else
			{
				QString smsg = msg;
				if (smsg.startsWith("//"))
					smsg.replace(0, 2, "/");
				fMsg = "/msg ";
				for (WUserIter it = users.begin(); it != users.end(); it++)
				{
					fMsg += (*it).second()->GetUserID();
					fMsg += ",";
				}
				fMsg.truncate(fMsg.length() - 1);
				fMsg += " ";
				fMsg += smsg;

				wchar_t * wText = qStringToWideChar(fMsg);
				PRINT("Sending text: %S\n", wText);
				delete [] wText;
			}
		}
		else
		{
			fMsg = msg;
		}
	}

	WPWEvent(int type, const QString & msg)
		: QCustomEvent(type)
	{
		fMsg = msg;
	}

	virtual ~WPWEvent() {}

	QString GetText() const { return fMsg; }
	void SetText(const QString & txt) { fMsg = txt; }
	QObject * SendTo() { return fReply; }
	void SetSendTo(QObject * o) { fReply = o; }

	enum
	{
		TextEvent = QEvent::User + 7000,
		TextPosted,			// response to textevent
		TabComplete,		// requesting a tab complete...
		TabCompleted,		// response to tab complete...
		Closed
	};

	void SetWantReply(bool t) { fWant = t; }
	bool GetWantReply() const { return fWant; }

private:
	bool fWant;
	QString fMsg;
	QObject * fReply;

};

#endif
