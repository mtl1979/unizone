#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qapplication.h>
#include <qdragobject.h>
#include <qsound.h>

#include "chatwindow.h"
#include "nicklist.h"
#include "platform.h"
#include "util.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"
#include "colors.h"
#include "wfile.h"
#include "wstring.h"
#include "formatting.h"

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
	QString message = EscapeHTML(msg);
	if (NameSaid(message) && Settings()->GetSounds())
		beep();
	return ParseString(ParseChatText(message));
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

		if (!Window()->isActiveWindow() && (Settings()->GetFlash() & type))	// if we have a valid handle AND we are not active AND the user wants to flash
		{
			WFlashWindow(Window()->winId()); // flash our window
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

	if ((index == 0) && (startsWith(itxt, sname)))
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
		WString wtemp(temp);
		WString wtxt(itxt);
		PRINT("Comparing \"%S\" to \"%S\"\n", wtemp.getBuffer(), wtxt.getBuffer());
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
		if (rlen > temp.length()) 
			rlen = temp.length();
		temp.truncate(rlen);
		// <postmaster@raasu.org> 20021005 -- Need to be in original case
		QString smsg;
		if (sred > 0)
			// <postmaster@raasu.org> 20021005 -- Don't use latin1 ()
			smsg += msg.left(sred);	
		smsg += WFormat::NameSaid(temp);
		smsg += msg.mid(sred+rlen);		// <postmaster@raasu.org> 20021005

#ifdef _DEBUG
		WString wmsg(smsg);
		PRINT("Name said string: %S\n", wmsg.getBuffer());
#endif

		msg = smsg;
		return true;
	}
	return false;
}
void 
ChatWindow::Action(const QString & name, const QString & msg)
{
	QString chat = WFormat::Action(FixString(name), FormatNameSaid(msg));

	PrintText(chat);
}

void 
ChatWindow::PrintText(const QString & str)
{
	QString out;
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

	lv->setAcceptDrops(true);
}

QString
ChatWindow::tr(const char *s)
{
	return qApp->translate("ChatWindow", s);
}

void
ChatWindow::beep()
{
	QString fn = Settings()->GetSoundFile();
#ifdef _DEBUG
	WString wfn(fn);
	PRINT("Sound file: %S\n", wfn.getBuffer());
#endif
	if (QSound::available()) 
    {
		PRINT("Sound available\n");
       	if (!fn.isEmpty())
		{
			if (WFile::Exists(fn))
			{
				PRINT("Sound file exists.\n");
				QSound::play(fn);
				return;
			}
			else
			{
				PRINT("Sound file doesn't exist!\n");
			}
		}
	}
	else
	{
		PRINT("Sound services not available!\n");
	}
	QApplication::beep();
}

void
ChatWindow::Clear()
{
	fChatText->clear();
}
