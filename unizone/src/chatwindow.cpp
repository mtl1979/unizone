#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "chatwindow.h"
#include "nicklist.h"
#include "platform.h"
#include "util.h"
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

QString
ChatWindow::FormatNameSaid(const QString &msg)
{
	QString message(msg);
	EscapeHTML(message);
	if (NameSaid(message) && Settings()->GetSounds())
		QApplication::beep();
	message = ParseChatText(message);
	return ParseStringStr(message);
}

bool 
ChatWindow::NameSaid(QString & msg)
{
	// <postmaster@raasu.org> -- Don't use latin1 ()
	QString sname = StripURL(gWin->GetUserName()); 
	int temp = 0;
	
	while (
		muscleInRange(sname.at(temp).unicode(), (unichar) 'A', (unichar) 'Z')||
		muscleInRange(sname.at(temp).unicode(), (unichar) 'a', (unichar) 'z')||
		muscleInRange(sname.at(temp).unicode(), (unichar) '0', (unichar) '9')||
		(sname.at(temp) == '_')||
		(sname.at(temp).unicode() >= 0x80)
		)
		temp++;
	
	if (temp > 0)
		sname.truncate(temp);
	
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
	// <postmaster@raasu.org> -- Don't use latin1 ()
	QString itxt = msg.upper(); 

	int sred = -1;
	
	// Check only on first iteration to avoid infinite loop, when sred = -1 and recursive call adds +1 
	// --> (-1) + 1 = 0 

	if ((index == 0) && (itxt.startsWith(sname)))
		sred = 0;
	else
	{
		sred = itxt.find(" " + sname, index);
		if (sred >= 0)
			sred++;
		else
		{
			// Check for double quoted nick
			sred = itxt.find("\"" + sname, index);
			if (sred >= 0)
				sred++;
		}
	}	

	if (sred >= 0)
	{
		unsigned int rlen = sname.length();

		// <postmaster@raasu.org> 20021005 -- don't use latin1 ()
		QString temp = gWin->GetUserName().upper(); 

#ifdef _DEBUG
		WString wTemp(temp);
		WString wTxt(itxt);
		PRINT("Comparing \"%S\" to \"%S\"\n", wTemp.getBuffer(), wTxt.getBuffer());
#endif

		unsigned int c1 = sred + rlen;	// itxt
		unsigned int c2 = rlen;		// temp

		if (c1 < itxt.length())
		{
			QChar an = itxt.at(c1);
			
			if (muscleInRange(an.unicode(), (unichar) 'A', (unichar) 'Z'))
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
						if (muscleInRange(an.unicode(), (unichar) 'A', (unichar) 'Z'))
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
		temp = StripURL(gWin->GetUserName());
		if (rlen >= temp.length()) 
			rlen = temp.length();
		temp.truncate(rlen);
		QString itxt = msg;						// <postmaster@raasu.org> 20021005 -- Need to be in original case
		QString itxt1 = itxt.left(sred);
		QString itxt2 = itxt.mid(sred+rlen);
		QString smsg;
		if (sred > 0)
			// <postmaster@raasu.org> 20021005 -- Don't use latin1 ()
			smsg += itxt1;	
		smsg += WFormat::NameSaid(temp);
		smsg += itxt2;		// <postmaster@raasu.org> 20021005

#ifdef _DEBUG
		WString wMessage(smsg);
		PRINT("Name said string: %S\n", wMessage.getBuffer());
#endif

		msg = smsg;
		return true;
	}
	return false;
}
void 
ChatWindow::Action(const QString & name, const QString & msg)
{
	QString chat = WFormat::Action(FixStringStr(name), FormatNameSaid(msg));

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

	fChatText->appendText(out);

	if (Settings()->GetLogging())
		LogString(out);
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
	if (Settings()->GetWarning())
	{
		QString e = WFormat::Warning(warning);

		PrintText(e);
	}
}

WSettings *
ChatWindow::Settings()
{
	return gWin->fSettings;
}

#ifdef WIN32
void
ChatWindow::FindWindowHandle(const QString &title)
{
	WString wtitle(title);
	fWinHandle = FindWindow(NULL, wtitle);

	if (!fWinHandle)
	{
		QString temp("[Freeware] - ");
		temp += title;
		wtitle = temp;
		fWinHandle = FindWindow(NULL, wtitle);
	}
	// <postmaster@raasu.org> 20020925
	if (fWinHandle)
	{
		PRINT("ChatWindow: Got Handle!\n");
	}
}
#endif

void
ChatWindow::InitUserList(QListView *lv)
{
	lv->addColumn(tr("Name"));
	lv->addColumn(tr("ID"));
	lv->addColumn(tr("Status"));
	lv->addColumn(tr("Files"));
	lv->addColumn(tr("Connection"));
	lv->addColumn(tr("Load"));
	lv->addColumn(tr("Client"));		// as of now... WinShare specific, WinShare pings all the users and parses the string for client info
	lv->addColumn(tr("OS"));			// as of now... Unizone specific, Unizone parses OS tag from Client tag if not present...

	lv->setColumnAlignment(WNickListItem::ID, Qt::AlignRight);		// <postmaster@raasu.org> 20021005
	lv->setColumnAlignment(WNickListItem::Files, Qt::AlignRight);	// <postmaster@raasu.org> 20021005
	lv->setColumnAlignment(WNickListItem::Load, Qt::AlignRight);	// <postmaster@raasu.org> 20021005

	for (int column = 0; column < 7; column++)
		lv->setColumnWidthMode(column, QListView::Manual);

	// set the sort indicator to show
	lv->setShowSortIndicator(true);

	lv->setAllColumnsShowFocus(true);
}

QString
ChatWindow::tr(const char *s)
{
	return qApp->translate("ChatWindow", s);
}
