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

ChatWindow::ChatWindow(ChatType type, QWidget* parent,  const char* name, Qt::WFlags fl)
	: Q3MainWindow(parent, name, fl)
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
			qLabels.Tail() += qToken;
			if (qLabels.Tail().findRev("]") >= 0)
			{
				QString tmp = qLabels.Tail();
				// Match number of [ and ] inside label
				while (tmp.count("[") < tmp.count("]"))
				{
					int pb = tmp.findRev("]");
					if (pb == -1)
						break;
					else
						tmp.truncate(pb);
				}
				if (tmp.count("[") == tmp.count("]"))
				{
					qLabels.Tail() = tmp;
					inLabel = false;
				}
			}
			if (inLabel)
					qLabels.Tail() += " "; // continue
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
						int pos = qToken.findRev("<");
						if (pos != -1)
							qToken.truncate(pos);
						if (!qToken.endsWith(">"))
							bInTag = false;
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
							int pos = qToken.findRev("<");
							if (pos != -1)
								qToken.truncate(pos);
							// another tag?
							if (!qToken.endsWith(">"))
								bInTag = false;
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

					if (qToken.endsWith("]"))
						break;

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
				inLabel = true; // Worst case scenario
				int cb = qToken.findRev("]");
				if (cb != -1)
				{
					// match count of [ and ] inside label
					while (qToken.leftRef(cb + 1).count("[") < qToken.leftRef(cb + 1).count("]"))
					{
						int pb = qToken.findRev("]", cb - 1);
						if (pb == -1)
							break;
						cb = pb;
					}
					if (qToken.leftRef(cb + 1).count("[") == qToken.leftRef(cb + 1).count("]"))
					{
						qLabels.Tail() = qToken.mid(1, cb - 1);
						inLabel = false;
					}
				}
				if (inLabel)
					qLabels.Tail() = qToken.mid(1) + " ";
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
			if ( qUrl.startsWith("beshare.", Qt::CaseInsensitive) )
				urltmp += "server://";
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
				le = qText.find(qLabel + "]");
				if (!muscleInRange(lb, 0, le))
					qText = qText.mid(le + qLabel.length() + 1);
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

	if (len == 0)
		return str;

	// Remove trailing line feeds
	while ((len > 0) && (str[len - 1] == '\n'))
		len--;

	// If text contains only line feeds, return empty string
	if (len == 0)
		return QString::null;

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
		else if (str.midRef(i,2) == "\r\n")
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
#if	__STDC_WANT_SECURE_LIB__
	char buf[26];
	ctime_s(buf, 26, &currentTime);
	return QString::fromLocal8Bit(buf);
#else
	return QString::fromLocal8Bit( ctime(&currentTime) );
#endif
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
		ret = FormatTimeStamp(tr("Date: %1").arg(_day));
		ret += "<br>";
	}

	qCurTime = GetTimeStampAux(stamp);

	ret += FormatTimeStamp(QString("[%1] ").arg(qCurTime));
	return ret;
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
			out = QString("<font color=\"%1\">%2</font>")
				.arg(color, name.left(tag));
			out += FormatUserName(name.mid(tag), color);
		}
		else
		{
			out = QString("<font color=\"%1\">%2</font>").arg(color, name);
		}
	}
	return out;
}

// formatting for:
//	(id) UserName

// <postmaster@raasu.org> 20020930
//
// Change that ID is always in black and colon isn't part of nick ;)

QString
ChatWindow::FormatLocalText(const QString &session, const QString &name,
	const QString &text)
{
	QString temp = QString("<font size=\"%1\"><b>(%2)</b> <b>")
		.arg(QString::number(GetFontSize()), session);
	temp += FormatUserName(name.stripWhiteSpace(), GetColor(WColors::LocalName));
	temp += QString("</b>: <font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::Text), text);
	return temp;
}

QString
ChatWindow::FormatRemoteName(const QString &session, const QString &name)
{
	QString temp = QString("<font size=\"%1\"><b>(%2)</b>")
		.arg(QString::number(GetFontSize()), session);
	if (WUser::CheckName(name))
	{
		QString uname = FormatUserName(name.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
		temp += QString(" <b>%1</b>").arg(uname);
	}
	temp += ": </font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatRemoteText(const QString &session, const QString &name,
	const QString &text)
{
	QString temp = QString("<font size=\"%1\"><b>(%2)</b>")
		.arg(QString::number(GetFontSize()), session);
	if (WUser::CheckName(name))
	{
		temp += QString(" <b>%1</b>").arg(FormatUserName(name.stripWhiteSpace(),
			GetColor(WColors::RemoteName)));
	}
	temp += QString(": <font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::Text), text);
	return temp;
}

QString
ChatWindow::FormatRemoteWatch(const QString &session, const QString &name,
	const QString &text)
{
	QString temp = QString("<font size=\"%1\"><b>(%2)</b>")
		.arg(QString::number(GetFontSize()), session);
	if (WUser::CheckName(name))
	{
		QString uname = FormatUserName(name.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
		temp += QString(" <b>%1</b>").arg(uname);
	}
	temp += QString(": <font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::Watch), text);
	return temp;
}

// text color...
QString
ChatWindow::FormatText(const QString &text)
{
	return QString("<font color=\"%1\"><font size=\"%2\">"
		"%3</font></font>").arg(GetColor(WColors::Text),
		QString::number(GetFontSize()), text);
}

// watch color...
QString
ChatWindow::FormatWatch(const QString &text)
{
	return QString("<font color=\"%1\"><font size=\"%2\">"
		"%3</font></font>").arg(GetColor(WColors::Watch),
		QString::number(GetFontSize()), text);
}

QString
ChatWindow::_FormatNameSaid(const QString &text)
{
	return QString("<font color=\"%1\">%2</font>")
		.arg(GetColor(WColors::NameSaid), text);
}

//    |
//   \ /
//	System: User #?? is now connected.
QString
ChatWindow::FormatSystemText(const QString &text)
{
	QString temp = QString("<font size=\"%1\"><font color=\"%2\"><b>")
		.arg(QString::number(GetFontSize()), GetColor(WColors::System));
	temp += tr("System:");
	temp += QString("</b></font> <font color=\"%1\">").arg(GetColor(WColors::Text));
	if (text.contains("<br>"))
		temp += "<br>";
	temp += text;
	temp += "</font></font>";
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
	if (WUser::CheckName(user))
	{
		QString username = FormatUserName(user.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
		return tr("User #%1 (a.k.a %2) has disconnected.").arg(session, username);
	}
	else
		return tr("User #%1 has disconnected.").arg(session);
}

QString
ChatWindow::FormatUserNameChangedNoOld(const QString &session, const QString &name)
{
	QString username = FormatUserName(name.stripWhiteSpace(),
		GetColor(WColors::RemoteName));
	return tr("User #%1 is now known as %2.").arg(session, username);
}

QString
ChatWindow::FormatUserNameChangedNoNew(const QString &session)
{
	return tr("User #%1 is now nameless.").arg(session);
}

QString
ChatWindow::FormatUserNameChanged(const QString &session, const QString &oldname, const QString &newname)
{
	QString oname = FormatUserName(oldname.stripWhiteSpace(),
		GetColor(WColors::RemoteName));
	QString nname = FormatUserName(newname.stripWhiteSpace(),
		GetColor(WColors::RemoteName));
	return tr("User #%1 (a.k.a %2) is now known as %3.")
		.arg(session, oname, nname);
}

QString
ChatWindow::FormatUserStatusChanged(const QString &session, const QString &user,
	const QString &status)
{
	if (WUser::CheckName(user))
	{
		QString username = FormatUserName(user.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
		return tr("User #%1 (a.k.a %2) is now %3.").arg(session, username,
			status);
	}
	else
		return tr("User #%1 is now %2.").arg(session, status);
}

QString
ChatWindow::FormatUserIPAddress(const QString &user, const QString &ip)
{
	QString temp;
	QString username = FormatUserName(user.stripWhiteSpace(),
		GetColor(WColors::RemoteName));
	temp = tr("%1's IP address is %2.").arg(username, ip);
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatUserIPAddress2(const QString &session, const QString &ip)
{
	return tr("User #%1's IP address is %2.").arg(session).arg(ip);
}

// ping formatting
QString
ChatWindow::FormatPingText(uint32 time, const QString &version)
{
	QString temp = QString("<font size=\"%1\"><font color=\"%2\">")
		.arg(QString::number(GetFontSize()), GetColor(WColors::Ping));
	temp += tr("Ping returned in %1 milliseconds").arg(time);
	temp += QString(" (%1)</font></font>").arg(version);
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatPingUptime(const QString &uptime, const QString &logged)
{
	QString temp = QString("<font size=\"%1\"><font color=\"%2\">")
		.arg(QString::number(GetFontSize()), GetColor(WColors::Ping));
	temp += tr("Uptime: %1, Logged on for %2").arg(uptime).arg(logged);
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

// error format
QString
ChatWindow::FormatError(const QString &text)
{
	QString temp = QString("<font size=\"%1\"><font color=\"%2\"><b>")
		.arg(QString::number(GetFontSize()), GetColor(WColors::Error));
	temp += tr("Error:");
	temp += QString("</b></font> <font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::ErrorMsg), text);
	return temp.stripWhiteSpace();
}

// warning format
QString
ChatWindow::FormatWarning(const QString &text)
{
	QString temp = QString("<font size=\"%1\"><font color=\"%2\"><b>")
		.arg(QString::number(GetFontSize()), GetColor(WColors::Warning));
	temp += tr("Warning:");
	temp += QString("</b></font> <font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::WarningMsg), text);
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatStatusChanged(const QString &status)
{
	return tr("You are now %1.").arg(status);
}

QString
ChatWindow::FormatNameChanged(const QString &name)
{
	QString username = FormatUserName(name.stripWhiteSpace(),
		GetColor(WColors::LocalName));
	return tr("Name changed to %1.").arg(username);
}

// priv messages

// <postmaster@raasu.org> 20020930,20030127
// Changed that ID is always black, uid in --

QString
ChatWindow::FormatSendPrivMsg(const QString &session, const QString &myname,
	const QString &othername, const QString &text)
{
	QString mname = FormatUserName(myname.stripWhiteSpace(),
	GetColor(WColors::LocalName));
	QString temp = QString("<font size=\"%1\">-<b>%2</b>- <b>%3</b> -> <b>")
		.arg(QString::number(GetFontSize()), session, mname);

	if (WUser::CheckName(othername))
		temp += FormatUserName(othername.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
	else
		temp += FormatUserName(qApp->translate("WUser", "Unknown"),
			GetColor(WColors::RemoteName));

	temp += QString("</b>: <font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::Text), text);

	return temp;
}

QString
ChatWindow::FormatReceivePrivMsg(const QString &session,
	const QString &othername, const QString &text)
{
	QString temp = QString("<font size=\"%1\">-<b>%2</b>- <b>")
		.arg(QString::number(GetFontSize()), session);

	if (WUser::CheckName(othername))
		temp += FormatUserName(othername.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
	else
		temp += FormatUserName(qApp->translate("WUser", "Unknown"),
			GetColor(WColors::RemoteName));
	temp += QString("</b>: <font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::PrivText), text);
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatAction(const QString &msg)
{
	QString temp = QString("<font size=\"%1\">").arg(GetFontSize());
	temp += QString("<font color=\"%1\"><b>").arg(GetColor(WColors::Action));
	temp += tr("Action:");
	temp += "</b></font> ";
	temp += QString("<font color=\"%1\">%2</font></font>")
		.arg(GetColor(WColors::Text), msg);
	return temp;
}

QString
ChatWindow::FormatAction(const QString &name, const QString &msg)
{
	QString temp = QString("%1 %2").arg(name.stripWhiteSpace(), msg);
	return FormatAction(temp);
}

QString
ChatWindow::FormatURL(const QString &url)
{
	return QString("<font color=\"%1\"><u>%2</u></font>")
		.arg(GetColor(WColors::URL), url);
}

QString
ChatWindow::FormatGotPinged(const QString &session, const QString &name)
{
	QString temp = QString("<font size=\"%1\"><font color=\"%2\">")
		.arg(QString::number(GetFontSize()), GetColor(WColors::Ping));
	if (WUser::CheckName(name))
	{
		QString username = FormatUserName(name.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
		temp += tr("User #%1 (a.k.a %2) pinged you.").arg(session, username);
	}
	else
		temp += tr("User #%1 pinged you.").arg(session);
	temp += "</font></font>";
	return temp.stripWhiteSpace();
}

QString
ChatWindow::FormatPingSent(const QString &session, const QString &name)
{

	if (WUser::CheckName(name))
	{
		QString username = FormatUserName(name.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
		return tr("Ping sent to user #%1 (a.k.a %2).").arg(session, username);
	}
	else
		return tr("Ping sent to user #%1.").arg(session);
}

QString
ChatWindow::FormatTimeStamp(const QString &stamp)
{
	return QString("<font size=\"%1\"><font color=\"%2\">"
		"<b>%3</b></font> </font>").arg(QString::number(GetFontSize()),
		GetColor(WColors::Text), stamp);
}

QString
ChatWindow::FormatTimeRequest(const QString &session, const QString &name)
{
	if (WUser::CheckName(name))
	{
		QString username = FormatUserName(name.stripWhiteSpace(),
			GetColor(WColors::RemoteName));
		return tr("Time request sent to user #%1 (a.k.a %2).")
			.arg(session, username);
	}
	else
		return tr("Time request sent to user #%1.").arg(session);
}

QString
ChatWindow::FormatPrivateIsBot(const QString &session, const QString &name)
{
	if (WUser::CheckName(name))
	{
		QString username = FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		return tr("User #%1 (a.k.a %2) is a bot!").arg(session, username);
	}
	else
		return tr("User #%1 is a bot!").arg(session);
}

QString
ChatWindow::FormatPrivateRemoved(const QString &session, const QString &name)
{
	if (WUser::CheckName(name))
	{
		QString username = FormatUserName(name.stripWhiteSpace(), GetColor(WColors::RemoteName));
		return tr("User #%1 (a.k.a %2) was removed from the private chat window.").arg(session, username);
	}
	else
		return tr("User #%1 was removed from the private chat window.").arg(session);
}

