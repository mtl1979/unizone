#include "wpwevent.h"
#include "user.h"
#include "util.h"
#include "privatewindowimpl.h"
#include "debugimpl.h"
#include "wstring.h"

WPWEvent::WPWEvent(int type, WUserMap & users, const QString & msg, bool encrypted)
		: QCustomEvent(type)
{
	fWant = false;
	fEncrypted = encrypted;
	if (type == WPWEvent::TextEvent)
	{
		fWant = true;	// this way we'll get an error string in the private window if
		// an invalid command is used
		// everything is stuck into one string
		if (msg[0] == '/' && !startsWith(msg, "//"))
		{
			// just forward the message
			fMsg = msg;
		}
		else
		{
			QString smsg(msg);
			if (startsWith(smsg, "//"))
				smsg.replace(0, 2, "/");
			if (fEncrypted)
				fMsg = "/emsg ";
			else
				fMsg = "/msg ";

			QString tusers;
			WUserIter it = users.GetIterator();
			while (it.HasMoreValues())
			{
				WUserRef uref;
				it.GetNextValue(uref);
				AddToList(tusers, uref()->GetUserID());
			}
			
			fMsg += tusers;
			fMsg += " ";
			fMsg += smsg;
			
#ifdef _DEBUG
			WString wmsg(fMsg);
			PRINT("Sending text: %S\n", wmsg.getBuffer());
#endif
		}
	}
	else
	{
		fMsg = msg;
	}
}

WPWEvent::WPWEvent(int type, const QString & msg)
		: QCustomEvent(type)
{
	fEncrypted = false;
	fMsg = msg;
}

WPWEvent::WPWEvent(int type)
		: QCustomEvent(type)
{
	fEncrypted = false;
}
