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

#include <qapplication.h>

ChatWindow::ChatWindow(ChatType type)
{
	_type = type;
}

ChatWindow::~ChatWindow()
{
}

#define InRange(a, b, c) ((a >= b) && (a <= c))

bool 
ChatWindow::NameSaid(QString & msg)
{
	QString sname = StripURL(gWin->GetUserName()); // <postmaster@raasu.org> -- Don't use latin1 ()
	int temp = 0;
	
	while (
		InRange(sname.at(temp), 'A', 'Z')||
		InRange(sname.at(temp), 'a', 'z')||
		InRange(sname.at(temp), '0', '9')||
		(sname.at(temp) == '_')||
		(sname.at(temp).unicode() >= 0x80)
		)
		temp++;
	
	if (temp > 0)
		sname = sname.left(temp);
	
	sname = sname.upper();
	
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
ChatWindow::NameSaid2(const QString &sname, QString & msg, unsigned long index)
{
	QString itxt = msg.upper(); // <postmaster@raasu.org> -- Don't use latin1 ()

	int sred;
	
	// Check only on first iteration to avoid infinite loop, when sred = -1 and recursive call adds +1 
	// --> (-1) + 1 = 0 

	if (index == 0)
		sred = (itxt.startsWith(sname)) ? 0 : -1;
	else
		sred = -1;
	
	if (sred < 0)
	{
		sred = itxt.find(" " + sname, index);
		if (sred >= 0)
			sred++;
	}
	if (sred >= 0)
	{
		unsigned int rlen = sname.length();

		QString temp = gWin->GetUserName().upper(); // <postmaster@raasu.org> 20021005 -- don't use latin1 ()
		WString wTemp(temp);
		WString wTxt(itxt);
		PRINT("Comparing \"%S\" to \"%S\"\n", wTemp.getBuffer(), wTxt.getBuffer());
		unsigned int c1 = sred + rlen;	// itxt
		unsigned int c2 = rlen;		// temp

		if (c1 < itxt.length())
		{
			QChar an = itxt.at(c1);
			
			if (InRange(an, 'A', 'Z'))
			{
				if (an != 'S')  // allows pluralization without apostrophes though
				{
					// Oops, don't trigger after all, to avoid the tim/time problem
					return NameSaid2(sname, msg, sred + 1);
				}
				else
				{
					if ((c1 + 1) < itxt.length())
					{
						an = itxt.at(c1 + 1);
						if (InRange(an, 'A', 'Z'))
						{
							// 'S' must be last letter in word
							return NameSaid2(sname, msg, sred + 1);
						}
					}
				}
			}
		}

		while (
				(c1 < itxt.length()) && 
				(c2 < temp.length()) && 
				(itxt.at(c1) == temp.at(c2))
			  )
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
		temp = StripURL(gWin->GetUserName());
		if (rlen >= temp.length()) 
			rlen = temp.length();
		temp = temp.left(rlen);
		output += temp;
		QString itxt = msg;						// <postmaster@raasu.org> 20021005 -- Need to be in original case
		QString itxt1 = itxt.left(sred);
		QString itxt2 = itxt.mid(sred+rlen);
		QString smsg;
		if (sred > 0)
			smsg += itxt1;	// <postmaster@raasu.org> 20021005 -- Don't use latin1 ()
		smsg += output;
		smsg += "</font>";
		smsg += itxt2;		// <postmaster@raasu.org> 20021005

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
	QString chat = WFormat::Action();
	QString nameText = FixStringStr(msg);
	if (NameSaid(nameText) && Settings()->GetSounds())
		QApplication::beep();
	QString text = QObject::tr("%1 %2").arg(FixStringStr(name)).arg(nameText);
	chat += WFormat::Text(text);

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
	QString s = WFormat::SystemText( ParseChatText(msg) );

	PrintText(s);
}

void 
ChatWindow::PrintError(const QString & error)
{
	if (Settings()->GetError())
	{
		QString e = WFormat::Error(error);

		PrintText(e);
	}
}

void 
ChatWindow::PrintWarning(const QString & warning)
{
	if (Settings()->GetError())
	{
		QString e = WFormat::Warning(warning);

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
