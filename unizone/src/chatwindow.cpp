#include <qapplication.h>
#include <q3dragobject.h>
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
#include "debugimpl.h"
#include "tokenizer.h"

ChatWindow::ChatWindow(ChatType type)
{
	_type = type;
}

ChatWindow::~ChatWindow()
{
}

QString
ChatWindow::ParseChatText(const QString & str)
{
	// <postmaster@raasu.org> 20021106,20021114 -- Added support for URL labels, uses QStringTokenizer now ;)
	Queue<QString> qUrls;
	Queue<QString> qLabels;  // in this list, null means no label

	QString qText = str;
	bool lastWasURL = false, inLabel = false;
	QStringTokenizer qTok(qText, " \t\n");
	QString qToken;

	while ((qToken = qTok.GetNextToken()) != QString::null)
	{
		bool bInTag = false;	// <postmaster@raasu.org> 20021012,20021114

		if (inLabel)			// label contains space(s) ???
		{
			if (qToken.endsWith("]"))
			{
				qLabels.Tail() += qToken.left(qToken.length() - 1);
				inLabel = false;
			}
			else if (qToken.find("]") >= 0)
			{
				qLabels.Tail() += qToken.left(qToken.find("]") - 1);
				inLabel = false;
			}
			else
				qLabels.Tail() += qToken + " ";
		}
		else if (IsURL(qToken))
		{
			if (
				(qToken.startsWith("beshare:", false)) ||
				(qToken.startsWith("share:", false))
				)
			{
				// Remove html tag after the url...
				if (qToken.endsWith(">"))
				{
					bool bInTag = true;
					while (bInTag)
					{
						if (qToken.endsWith("<"))
							bInTag = false;
						qToken.truncate(qToken.length() - 1);
						if (qToken.endsWith(">"))
							bInTag = true;
					}
				}
				// ...and ensure that URL doesn't end with a dot, comma or colon
				bool cont = true;
				while (!qToken.isEmpty() && cont)
				{
					unsigned int pos = qToken.length() - 1;
					switch (qToken.at(pos).unicode())
					{
					case '.':
					case ',':
					case ':':
							qToken.truncate(pos);
							break;
					default:
							cont = false;
							break;
					}
				}
			}
			else
			{
				while (qToken.length() > 1)
				{
					QChar last = qToken.at(qToken.length() - 1);

					// <postmaster@raasu.org> 20021012,20021114,20030203
					//

					if (last == '>')
					{
						bInTag = true;

						// Skip html tags that are just after the url
						//
						while (bInTag)
						{
							if (qToken.endsWith("<"))
								bInTag = false;

							qToken.truncate(qToken.length() - 1);

							// another tag?
							if (qToken.endsWith(">"))
								bInTag = true;
						}

						last = qToken.at(qToken.length() - 1);
					}

					// <postmaster@raasu.org> Apr 11th 2004
					// Make sure there is same amount of ( and ) characters
					//

					if (qToken.endsWith(")"))
					{
						if (qToken.contains("(") == qToken.contains(")"))
							break;
					}

					if 	(
						muscleInRange(last.unicode(), (unichar) '0', (unichar) '9') ||
						muscleInRange(last.unicode(), (unichar) 'a', (unichar) 'z') ||
						muscleInRange(last.unicode(), (unichar) 'A', (unichar) 'Z') ||
						(last == '/')
						)
					{
						break;
					}
					else
						qToken.truncate(qToken.length() - 1);
				}
			}
			if (IsURL(qToken))
			{
				qUrls.AddTail(qToken);
				qLabels.AddTail(QString::null);
				lastWasURL = true;
			}
		}
		else if (lastWasURL)
		{
			lastWasURL = false; // clear in all cases, might contain trash between url and possible label

			if (qToken.startsWith("[")) // Start of label?
			{
				if (qToken.endsWith("]"))
					qLabels.Tail() = qToken.mid(1, qToken.length() - 2);
				else if (qToken.find("]") >= 0)
					qLabels.Tail() = qToken.mid(1, qToken.find("]") - 1);
				else
				{
					qLabels.Tail() += qToken.mid(1) + " ";
					inLabel = true;
				}
			}
		}
	}

	if (inLabel)
		qText += "]";

	if (qUrls.GetNumItems() > 0)
	{
		QString output = QString::null;

		QString qUrl;
		QString qLabel;

		while ((qUrls.RemoveHead(qUrl) == B_OK) && (qLabels.RemoveHead(qLabel) == B_OK))
		{
			int urlIndex = qText.find(qUrl); // position in QString

			if (urlIndex > 0) // Not in start of string?
			{
				output += qText.left(urlIndex);
				qText = qText.mid(urlIndex);
			}

			// now the url...
			QString urltmp = "<a href=\"";
			if ( qUrl.startsWith("www.") )		urltmp += "http://";
			if ( qUrl.startsWith("ftp.") )		urltmp += "ftp://";
			if ( qUrl.startsWith("beshare.") )	urltmp += "server://";
			if ( qUrl.startsWith("irc.") )		urltmp += "irc://";
			urltmp += qUrl;
			urltmp += "\">";
			// Display URL label or link address, if label doesn't exist
			int lb = qText.find("\n"); // check for \n between url and label (Not allowed!!!)
			int le = qText.find(qLabel);
			if ( qLabel.isEmpty() || muscleInRange(lb, 0, le) )
				urltmp += qUrl;
			else
				urltmp += qLabel.stripWhiteSpace(); // remove surrounding spaces before adding
			urltmp += "</a>";
			QString urlfmt = FormatURL(urltmp);
			output += urlfmt;
			// strip url from original text
			qText = qText.mid(qUrl.length());
			// strip label from original text, if exists
			if (!qLabel.isEmpty())
			{
				lb = qText.find("\n");
				le = qText.find("]");
				if (!muscleInRange(lb, 0, le))
				{
					qText = qText.mid(le + 1);
					if (qText.startsWith("]"))	// Fix for ']' in end of label
						qText = qText.mid(1);
				}
			}

		}
		// Still text left?
		if (!qText.isEmpty())
			output += qText;

		return output;		// <postmaster@raasu.org> 20021107,20021114 -- Return modified string
	}
	else
		return str; 		// <postmaster@raasu.org> 20021107 -- Return unmodified
}

QString
ChatWindow::ParseString(const QString & str)
{
	QString s;
	bool space = true;
	bool first = false; // make first always be &nbsp;

	unsigned int len = str.length();
	// Remove trailing line feeds
	while (str[len - 1] == '\n')
		len--;

	for (unsigned int i = 0; i < len; i++)
	{
		if (str[i] == ' ')
		{
			if (space)
			{
				// alternate inserting non-breaking space and real space
				if (first)
					s += " ";
				else
					s += "&nbsp;";
				first = !first;
			}
			else
				s += " ";
		}
		else if (str[i] == '<')
		{
			space = false;
			s += "<";
		}
		else if (str[i] == '>')
		{
			space = true;
			s += ">";
		}
		else if (str[i] == '\n')
		{
			// change newlines to <br> (html)
			if (space)
				s += "<br>";
			else
				s += "\n";
		}
		else if (str.mid(i,2) == "\r\n")
		{
			if (!space)
				s += "\r\n";
		}
		else if (str[i] == '\r') // Only 13
		{
			// change single carriage returns to <br> (html)
			if (space)
				s += "<br>";
			else
				s += "\r";
		}
		else if (str[i] == '\t')
		{
			if (space)
			{
				// <postmaster@raasu.org> 20030623 -- follow 'first' used in spaces here too...
				if (first)
					s += " &nbsp; &nbsp;";
				else
					s += "&nbsp; &nbsp; ";
			}
			else
				s += "\t";
		}
		else if (( (unichar) str[i].unicode() ) < 32) 			// control character?
		{
			// Do Nothing!
		}
		else
		{
			// other character
			first = true;
			s += str[i];
		}
	}
	return s;
}

QString
ChatWindow::FixString(const QString & str)
{
	return ParseString(ParseChatText(EscapeHTML(str.stripWhiteSpace())));
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
	{
		temp++;
		if (temp == sname.length())
			break;
	}
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
#ifdef WIN32
	if (bNameSaid)
	{
		int type;
		if (_type == MainType) 
			type = WSettings::FlashMain;
		else
			type = WSettings::FlashPriv;

		if (!Window()->isActiveWindow() && (Settings()->GetFlash() & type))	// if we have a valid handle AND we are not active AND the user wants to flash
		{
			WFlashWindow(Window()->winId()); // flash our window
		}
	}
#endif // WIN32
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
		int rlen = sname.length();

		// <postmaster@raasu.org> 20021005 -- don't use latin1 ()
		QString temp = gWin->GetUserName().upper(); 

#ifdef _DEBUG
		WString wtemp(temp);
		WString wtxt(itxt);
		PRINT("Comparing \"%S\" to \"%S\"\n", wtemp.getBuffer(), wtxt.getBuffer());
#endif

		int c1 = sred + rlen;	// itxt
		int c2 = rlen;			// temp

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
		smsg += _FormatNameSaid(temp);
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
	QString chat = FormatAction(FixString(name), FormatNameSaid(msg));

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
	QString s = FormatSystemText( ParseChatText(msg) );

	PrintText(s);
}

void 
ChatWindow::PrintError(const QString & error)
{
	if (Settings()->GetError())
	{
		QString e = FormatError(error);

		PrintText(e);
	}
}

void 
ChatWindow::PrintWarning(const QString & warning)
{
	if (Settings()->GetWarning())
	{
		QString e = FormatWarning(warning);

		PrintText(e);
	}
}

WSettings *
ChatWindow::Settings()
{
	return gWin->fSettings;
}

void
ChatWindow::InitUserList(Q3ListView *lv)
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
		lv->setColumnWidthMode(column, Q3ListView::Manual);

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

QString
ChatWindow::tr(const char *s, const char *c)
{
	return qApp->translate("ChatWindow", s, c);
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

int 
ChatWindow::GetFontSize()
{
	return Settings()->GetFontSize();
}

QString
ChatWindow::GetColor(int c)
{
	return Settings()->GetColorItem(c);
}

QString
InitTimeStamp()
{
	time_t currentTime = time(NULL);
	return QString::fromLocal8Bit( ctime(&currentTime) );
}

QString
ChatWindow::GetTimeStampAux(const QString &stamp)
{
	QString qCurTime;

	qCurTime = stamp;
	qCurTime = qCurTime.left(qCurTime.findRev(" ", -1));
	qCurTime = qCurTime.mid(qCurTime.findRev(" ", -1) + 1);

	return qCurTime;
}

QString
ChatWindow::GetDateStampAux(const QString &stamp)
{
	QString qCurTime = stamp;

	// Strip off year
	QString qYear = qCurTime.mid(qCurTime.findRev(" ") + 1);
	qYear.truncate(4);
	qCurTime.truncate(qCurTime.findRev(" "));

	// ... and day of week
	QString qDOW = qCurTime.left(qCurTime.find(" "));
	qDOW = TranslateDay(qDOW);
	qCurTime = qCurTime.mid(qCurTime.find(" ") + 1);

	// Strip Month and translate it
	QString qMonth = qCurTime.left(qCurTime.find(" "));
	qMonth = TranslateMonth(qMonth);

	qCurTime = qCurTime.mid(qCurTime.find(" ") + 1);

	// Strip Day
	if (qCurTime.startsWith(" "))
		qCurTime = qCurTime.mid(1);
	QString qDay = qCurTime.left(qCurTime.find(" "));
	return qDOW + " " + qMonth + " " + qDay + " " + qYear;
}

QString
ChatWindow::GetTimeStamp2()
{
	QString stamp = InitTimeStamp();
	return GetDateStampAux(stamp) + " " + GetTimeStampAux(stamp);
}

QString
ChatWindow::GetTimeStamp()
{
	static QString _day = QString::null;

	QString ret;
	QString qCurTime;
	// Is this first time today?

	QString stamp = InitTimeStamp();
	QString qDate = GetDateStampAux(stamp);
	if (qDate != _day)
	{
		_day = qDate;
		qDate.prepend(" ");
		qDate.prepend(qApp->translate("Date", "Date:"));
		ret = FormatTimeStamp(qDate);
		ret += "<br>";
	}

	qCurTime = GetTimeStampAux(stamp);
	qCurTime.prepend("[").append("] ");

	ret += FormatTimeStamp(qCurTime);
	return ret;
}

QString
ChatWindow::tr2(const char *s)
{
	QString temp = tr(s).stripWhiteSpace();

	if (!temp.isEmpty())
	{
		temp += " ";
	}
	return temp;
}

QString 
ChatWindow::tr3(const QString &s)
{
	QString temp = s.stripWhiteSpace();
	if (temp != QString("."))
		return temp.prepend(" ");
	return temp;
}

QString
ChatWindow::FormatUserName(const QString &name, const QString &color)
{
	QString out;
	if (name.startsWith("<font"))
	{
		int e = name.find("</font>");
		if (e == (int) name.length() - 7)
			return name;
		out = name.left(e + 7);
		out += FormatUserName(name.mid(e + 7), color);
	}
	else
	{
		int tag = name.find("<font");
		if (tag > 0)
		{
			out = tr("<font color=\"%1\">").arg(color);
			out += name.left(tag);
			out += "</font>";
			out += FormatUserName(name.mid(tag), color);
		}
		else
		{
			out = tr("<font color=\"%1\">").arg(color);
			out += name;
			out += "</font>";
		}
	}
	return out;
}

// formatting for:
//	(id) UserName

// <postmaster@raasu.org> 20020930
//
// Change that ID is always in black and colon isn't part of nick ;)

// <postmaster@raasu.org> 20040511
//
// We can't use tr() with URLs, it just doesn't work!!!
// This includes user names and status texts

QString
ChatWindow::FormatLocalText(const QString &session, const QString &name, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "<b>(";
	temp += session;
	temp += ")</b> ";
	temp += "<b>";
	temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::LocalName));
	temp += "</b>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString 
ChatWindow::FormatRemoteName(const QString &session, const QString &name)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<b>(%1)</b>").arg(session);
	if (WUser::CheckName(name))
	{
		temp += " ";
		temp += "<b>";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += "</b>";
	}
	temp += ": </font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatRemoteText(const QString &session, const QString &name, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "<b>(";
	temp += session;
	temp += ")</b>";
	if (WUser::CheckName(name))
	{
		temp += " ";
		temp += "<b>";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += "</b>";
	}
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString
ChatWindow::FormatRemoteWatch(const QString &session, const QString &name, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "<b>(";
	temp += session;
	temp += ")</b>";
	if (WUser::CheckName(name))
	{
		temp += " ";
		temp += "<b>";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += "</b>";
	}
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Watch));
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

// text color...
QString 
ChatWindow::FormatText(const QString &text)
{
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += tr("<font size=\"%1\">").arg(GetFontSize());
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp.stripWhiteSpace();
}

// watch color...
QString
ChatWindow::FormatWatch(const QString &text)
{
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::Watch));
	temp += tr("<font size=\"%1\">").arg(GetFontSize());
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::_FormatNameSaid(const QString &text)
{
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::NameSaid));
	temp += text;
	temp += "</font>";
	return temp.stripWhiteSpace();
}

//    |
//   \ /
//	System: User #?? is now connected.
QString 
ChatWindow::FormatSystemText(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::System));
	temp += tr("<b>System:</b>");
	temp += "</font>";
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	if (text.contains("<br>"))
		temp += "<br>";
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatUserConnected(const QString &session)
{
	return tr("User #%1 is now connected.").arg(session);
}

QString 
ChatWindow::FormatUserDisconnected(const QString &session, const QString &user)
{
	QString temp;
	if (WUser::CheckName(user))
	{
		temp  = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(user.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp  = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("has disconnected.");
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatUserNameChangedNoOld(const QString &session, const QString &name)
{
	QString temp = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("is now known as");
	temp += " ";
	temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
	temp += tr3(tr(".", "'is now known as' suffix"));
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatUserNameChangedNoNew(const QString &session)
{
	QString temp = tr("User #%1 is now nameless.").arg(session);
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatUserNameChanged(const QString &session, const QString &oldname, const QString &newname)
{
	QString temp = tr("User #%1 (a.k.a").arg(session);
	temp += " ";
	temp += FormatUserName(oldname.stripWhiteSpace(), GetColor(WColors::RemoteName));
	temp += tr(")", "aka suffix");
	temp += " ";
	temp += tr("is now known as");
	temp += " ";
	temp += FormatUserName(newname.stripWhiteSpace(), GetColor(WColors::RemoteName));
	temp += tr3(tr(".", "'is now known as' suffix"));
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatUserStatusChanged(const QString &session, const QString &user, const QString &status)
{
	QString temp;
	if (WUser::CheckName(user))
	{
		temp  = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(user.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp  = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("is now");
	temp += " ";
	temp += status;		// No need to strip spaces, it was done before translating status...
	temp += tr3(tr(".", "'is now' suffix"));
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatUserIPAddress(const QString &user, const QString &ip)
{
	QString temp = tr2(QT_TRANSLATE_NOOP("WFormat", "ip_prefix"));
	temp += FormatUserName(user.stripWhiteSpace(), GetColor(WColors::RemoteName));
	if (tr("ip_space","Need space after username in IP address string?") == QString("yes")) 
		temp += " ";
	temp += tr("'s IP address is %1.").arg(ip);
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatUserIPAddress2(const QString &session, const QString &ip)
{
	QString temp = tr("User #%1's IP address is %2.").arg(session).arg(ip);
	return temp.stripWhiteSpace();
}

// ping formatting
QString 
ChatWindow::FormatPingText(uint32 time, const QString &version)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Ping));
	temp += tr("Ping returned in %1 milliseconds").arg(time);
	temp += " (";
	temp += version;
	temp += ")</font></font>";
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatPingUptime(const QString &uptime, const QString &logged)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Ping));
	temp += tr("Uptime: %1, Logged on for %2").arg(uptime).arg(logged);
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

// error format
QString 
ChatWindow::FormatError(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Error));
	temp += tr("<b>Error:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::ErrorMsg));
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

// warning format
QString 
ChatWindow::FormatWarning(const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Warning));
	temp += tr("<b>Warning:</b>");
	temp += "</font> ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::WarningMsg));
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatStatusChanged(const QString &status)
{
	QString temp = tr2(QT_TRANSLATE_NOOP("WFormat", "You are now"));
	temp += status;
	temp += tr3(tr(".", "'You are now' suffix"));
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatNameChanged(const QString &name)
{
	QString temp = tr2(QT_TRANSLATE_NOOP("WFormat", "Name changed to"));
	temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::LocalName));
	temp += tr3(tr(".", "'Name changed to' suffix"));
	return temp.stripWhiteSpace();
}

// priv messages

// <postmaster@raasu.org> 20020930,20030127 
// Changed that ID is always black, uid in --

QString
ChatWindow::FormatSendPrivMsg(const QString &session, const QString &myname, const QString &othername, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "-";
	temp += "<b>";
	temp += session;
	temp += "</b>";
	temp += "- ";
	temp += "<b>";
	temp += FormatUserName(myname.stripWhiteSpace(), GetColor(WColors::LocalName));
	temp += "</b>";
	temp += " -> ";
	temp += "<b>";
	if (WUser::CheckName(othername))
		temp += FormatUserName(othername.stripWhiteSpace(), GetColor(WColors::RemoteName));
	else
		temp += FormatUserName(qApp->translate("WUser", "Unknown"), GetColor(WColors::RemoteName));
	temp += "</b>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += text;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString 
ChatWindow::FormatReceivePrivMsg(const QString &session, const QString &othername, const QString &text)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += "-";
	temp += "<b>";
	temp += session;
	temp += "</b>";
	temp += "- ";
	temp += "<b>";
	if (WUser::CheckName(othername))
		temp += FormatUserName(othername.stripWhiteSpace(), GetColor(WColors::RemoteName));
	else
		temp += FormatUserName(qApp->translate("WUser", "Unknown"), GetColor(WColors::RemoteName));
	temp += "</b>";
	temp += ": ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::PrivText));
	temp += text;
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatAction(const QString &msg)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Action));
	temp += tr("<b>Action:</b>");
	temp += "</font>";
	temp += " ";
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += msg;
	temp += "</font>";
	temp += "</font>";
	return temp;
}

QString
ChatWindow::FormatAction(const QString &name, const QString &msg)
{
	QString temp(name.stripWhiteSpace());
	temp += " ";
	temp += msg;
	return FormatAction(temp);
}

QString 
ChatWindow::FormatURL(const QString &url)
{ 
	QString temp = tr("<font color=\"%1\">").arg(GetColor(WColors::URL));
	temp += "<u>";
	temp += url;
	temp += "</u></font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatGotPinged(const QString &session, const QString &name)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Ping));
	if (WUser::CheckName(name))
	{
		temp += tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp += tr("User #%1").arg(session);
	temp += " ";
	temp += tr("pinged you.");
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatPingSent(const QString &session, const QString &name)
{
	QString temp = tr("Ping sent to");
	temp += " ";
	if (WUser::CheckName(name))
	{
		temp += tr("user #%1 (a.k.a","Ping sent to user...").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		if (tr(")", "aka suffix") != QString(","))
			temp += tr(")", "aka suffix");
	}
	else
		temp += tr("user #%1", "Ping sent to user...").arg(session);
	temp += ".";
	return temp.stripWhiteSpace();
}

QString 
ChatWindow::FormatTimeStamp(const QString &stamp)
{
	QString temp = tr("<font size=\"%1\">").arg(GetFontSize());
	temp += tr("<font color=\"%1\">").arg(GetColor(WColors::Text));
	temp += "<b>";
	temp += stamp;
	temp += "</b></font> </font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatTimeRequest(const QString &session, const QString &username)
{
	QString temp = tr("Time request sent to");
	temp += " ";
	if (WUser::CheckName(username))
	{
		temp += tr("user #%1 (a.k.a","Ping sent to user...").arg(session);
		temp += " ";
		temp += FormatUserName(username.stripWhiteSpace(), GetColor(WColors::RemoteName));
		if (tr(")", "aka suffix") != QString(","))
			temp += tr(")", "aka suffix");
	}
	else
		temp += tr("user #%1", "Ping sent to user...").arg(session);
	temp += "."; 
	return temp.stripWhiteSpace();						
}

QString
ChatWindow::FormatPrivateIsBot(const QString &session, const QString &name)
{
	QString temp;
	if (WUser::CheckName(name))
	{
		temp = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("is a bot!");
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatPrivateRemoved(const QString &session, const QString &name)
{
	QString temp;
	if (WUser::CheckName(name))
	{
		temp = tr("User #%1 (a.k.a").arg(session);
		temp += " ";
		temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		temp += tr(")", "aka suffix");
	}
	else
		temp = tr("User #%1").arg(session);
	temp += " ";
	temp += tr("was removed from the private chat window.");
	return temp.stripWhiteSpace();
}

