#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "chatwindow.h"
#include "platform.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"
#include "colors.h"
#include "wstring.h"
#include "formatting.h"

ChatWindow::ChatWindow(ChatType type)
{
	_type = type;
}

ChatWindow::~ChatWindow()
{
}

bool 
ChatWindow::NameSaid(QString & msg)
{
	String sname = StripURL((const char *) gWin->GetUserName().utf8()); // <postmaster@raasu.org> -- Don't use latin1 ()
	const unsigned char * orig = (const unsigned char *)sname.Cstr();
	const unsigned char * temp = orig;
	
	while(((*temp >= 'A')&&(*temp <= 'Z'))||
		((*temp >= 'a')&&(*temp <= 'z'))||
		((*temp >= '0')&&(*temp <= '9'))||
		(*temp == '_')||
		(*temp >= 0x80))
		temp++;
	
	if (temp > orig)
		sname = sname.Substring(0, temp - orig);
	
	sname = sname.ToUpperCase();
	
	bool bNameSaid = false;
	bool bTemp;
	while ( (bTemp = NameSaid2(sname, msg)) )
	{
		if (bTemp && !bNameSaid)
			bNameSaid = true;
	}
	if (bNameSaid)
	{
#ifdef WIN32
		int type;
		if (_type == MainType) 
			type = WSettings::FlashMain;
		else
			type = WSettings::FlashPriv;

		if (fWinHandle && !Window()->isActiveWindow() && (Settings()->GetFlash() & type))	// if we have a valid handle AND we are not active AND the user wants to flash
		{
			WFlashWindow(fWinHandle); // flash our window
		}
#endif // WIN32
	}
	return bNameSaid;
}

bool 
ChatWindow::NameSaid2(const String &sname, QString & msg, unsigned long index)
{
	String itxt((const char *) msg.utf8()); // <postmaster@raasu.org> -- Don't use latin1 ()
	itxt = itxt.ToUpperCase();

	int sred;
	
	// Check only on first iteration to avoid infinite loop, when sred = -1 and recursive call adds +1 
	// --> (-1) + 1 = 0 

	if (index == 0)
		sred = (itxt.StartsWith(sname)) ? 0 : -1;
	else
		sred = -1;
	
	if (sred < 0)
	{
		sred = itxt.IndexOf(sname.Prepend(" "), index);
		if (sred >= 0)
			sred++;
	}
	if (sred >= 0)
	{
		unsigned int rlen = sname.Length();

		String temp = (const char *) gWin->GetUserName().utf8(); // <postmaster@raasu.org> 20021005 -- don't use latin1 ()
		temp = temp.ToUpperCase();
		PRINT("Comparing \"%s\" to \"%s\"\n", temp.Cstr(), itxt.Cstr());
		const char * c1 = &itxt.Cstr()[sred + rlen];
		const char * c2 = &temp.Cstr()[rlen];

		char an = c1[0];
		char an2 = c1[1];

        if (muscleInRange(an, 'A', 'Z'))
		{
			if (an != 'S')  // allows pluralization without apostrophes though
			{
				// Oops, don't trigger after all, to avoid the tim/time problem
				return NameSaid2(sname, msg, sred + 1);
			}
			else if (muscleInRange(an2, 'A', 'Z'))
			{
				// 'S' must be last letter in word
				return NameSaid2(sname, msg, sred + 1);
			}
		}

		while (c1 && c2 && (*c1 == *c2))
		{
			c1++;
			c2++;
			rlen++;
			PRINT("Upped rlen\n");
		}

		// yup... we've been mentioned...
		QString output;
		output = "<font color=\"";
		output += WColors::NameSaid;						// <postmaster@raasu.org> 20021005
		output += "\">";
		temp = StripURL((const char *) gWin->GetUserName().utf8());
		if (rlen >= temp.Length()) rlen = temp.Length();
		temp = temp.Substring(0, rlen);
		output += QString::fromUtf8(temp.Cstr());
		String itxt((const char *) msg.utf8());						// <postmaster@raasu.org> 20021005 -- Need to be in original case
		String itxt1 = itxt.Substring(0,sred);
		String itxt2 = itxt.Substring(sred+rlen);
		QString smsg;
		if (sred > 0)
			smsg += QString::fromUtf8(itxt1.Cstr()); // <postmaster@raasu.org> 20021005 -- Don't use latin1 ()
		smsg += output;
		smsg += "</font>";
		smsg += QString::fromUtf8(itxt2.Cstr()); // <postmaster@raasu.org> 20021005

		WString wMessage(smsg);
		PRINT("Name said string: %S\n", wMessage.getBuffer());

		msg = smsg;
		return true;
	}
	return false;
}
void 
ChatWindow::Action(const QString & name, const QString & msg)
{
	QString chat = WFormat::Action().arg(WColors::Action).arg(Settings()->GetFontSize());
	QString nameText = FixStringStr(msg);
	if (NameSaid(nameText) && Settings()->GetSounds())
		QApplication::beep();
	chat += WFormat::Text.arg(WColors::Text).arg(Settings()->GetFontSize()).arg(QObject::tr("%1 %2").arg(FixStringStr(name)).arg(nameText));

	PrintText(chat);
}

void 
ChatWindow::PrintText(const QString & str)
{
	QString out("");
	if (Settings()->GetTimeStamps())
		out = GetTimeStamp();

	if (!str.isEmpty())
		out += str;

#if (QT_VERSION < 0x030000)
	if (fChatText->text().isEmpty())
		fChatText->setText(out);
	else
#endif
	{
		CheckScrollState();
		fChatText->append(
#if (QT_VERSION < 0x030000)
					"\t" +
#endif
 					out);
	}
	if (Settings()->GetLogging())
		LogString(out);
	UpdateTextView();
}

void 
ChatWindow::PrintSystem(const QString & msg)
{
	QString s = WFormat::SystemText().arg(WColors::System).arg(Settings()->GetFontSize());
	s += WFormat::Text.arg(WColors::Text).arg(Settings()->GetFontSize()).arg(ParseChatText(msg));

	PrintText(s);
}

void 
ChatWindow::PrintError(const QString & error)
{
	if (Settings()->GetError())
	{
		QString e = WFormat::Error().arg(WColors::Error).arg(Settings()->GetFontSize());
		e += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(Settings()->GetFontSize()).arg(error);

		PrintText(e);
	}
}

void 
ChatWindow::PrintWarning(const QString & warning)
{
	if (Settings()->GetError())
	{
		QString e = WFormat::Warning().arg(WColors::Error).arg(Settings()->GetFontSize());
		e += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(Settings()->GetFontSize()).arg(warning);

		PrintText(e);
	}
}

void
ChatWindow::CheckScrollState()
{
	QScrollBar * scroll = fChatText->verticalScrollBar();
#ifdef DEBUG2
	PRINT("CHECKSCROLLSTATE: value = %d, maxValue = %d, minValue = %d\n", scroll->value(), scroll->maxValue(), scroll->minValue());
#endif
	fScrollX = fChatText->contentsX();
	fScrollY = fChatText->contentsY();
	if (scroll->value() >= scroll->maxValue())
		fScrollDown = true;
	else
		fScrollDown = false;
}

void
ChatWindow::UpdateTextView()
{
	// <postmaster@raasu.org> 20021021 -- Fixed too long line in debug output
#ifdef DEBUG2
	PRINT("UPDATETEXTVIEW: ContentsX = %d, ContentsY = %d\n", fChatText->contentsX(),		fChatText->contentsY());
	PRINT("              : ContentsW = %d, ContentsH = %d\n", fChatText->contentsWidth(),	fChatText->contentsHeight());
#endif
	if (fScrollDown)
	{
		fScrollY = fChatText->contentsHeight();
	}
	if (fScrollX != fChatText->contentsX() || fScrollY != fChatText->contentsY())
	{
		fChatText->setContentsPos(fScrollX, fScrollY);
#ifndef WIN32	// linux only... (FreeBSD???)
		fChatText->repaintContents(
									fChatText->contentsX(), fChatText->contentsY(),
									fChatText->contentsWidth(), fChatText->contentsHeight(),
									false);
#endif
	}
}

void
ChatWindow::BeforeShown()
{
	CheckScrollState();
}

void
ChatWindow::GotShown(const QString & txt)
{
	fChatText->setText(ParseForShown(txt));
	UpdateTextView();
}

WSettings *
ChatWindow::Settings()
{
	return gWin->fSettings;
}
