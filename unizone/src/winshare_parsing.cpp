#ifdef WIN32		// <postmaster@raasu.org> 20021022 -- Fix to use platform.h, Fixed Window Flashing for older API's
#include <windows.h>
#endif

#include "winsharewindow.h"
#include "global.h"
#include "util/StringTokenizer.h"
#include "debugimpl.h"
#include "settings.h"
#include "formatting.h"
#include "version.h"
#include "platform.h"
#include "tokenizer.h" // <postmaster@raasu.org> 20021128

#include <qapplication.h>

int
WinShareWindow::MatchUserName(QString un, QString & result, const char * filter)
{
	int matchCount = 0;
	WUserIter iter = fNetClient->Users().begin();
	WUserRef user;
	QString res = result;
	//QString qUn = QString::fromUtf8(un);

	while (iter != fNetClient->Users().end())
	{
		user = (*iter).second;
		QString userName = StripURL(user()->GetUserName().lower().stripWhiteSpace());
		if (((filter == NULL) || (MatchUserFilter(user(), filter))) &&
			((userName.startsWith(un)) || (userName.contains(" ["+un) > 0)))
		{
			matchCount++;
			if (matchCount == 1)
			{
				res = StripURL(user()->GetUserName());
			}
			else
			{
				PRINT("WinShareWindow::MatchUserName: Multiple matches\n");
				// multiple matches
				QString temp = res.lower();

				for (uint32 i = 0; i < temp.length(); i++)
				{
					if (temp.at(i) != userName.at(i))
					{
						res = temp.left(i);
						break;
					}
				}
			}
		}
		iter++;
	}
	PRINT("WinShareWindow::MatchUserName: Result %S\n", qStringToWideChar(res));
	result = res;
	return matchCount;
}

bool
WinShareWindow::MatchUserFilter(const WUser * user, const char * filter)
{
	if (user == NULL) 
		return false;
	StringTokenizer idTok(filter, ","); // identifiers may be separated by commas (but not spaces, as those may be parts of the users' names!)
	const char * n;
	while((n = idTok.GetNextToken()) != NULL)
	{
		String next = StripURL(n);
		next = next.Trim();

		// Is this item our user's session ID?
		PRINT("MatchUserFilter: UserID = %s\n", (const char *) user->GetUserID().utf8());
		PRINT("MatchUserFilter: next   = %s\n", next.Cstr());
		PRINT("MatchUserFilter: strcmp = %d\n", strcmp((const char *) user->GetUserID().utf8(), next.Cstr()));
		if (strcmp((const char *) user->GetUserID().utf8(), next.Cstr()) == 0)
			return true;
		else
		{
			 // Does this item (interpreted as a regex) match our user's name?
			 MakeRegexCaseInsensitive(next);
			 StringMatcher sm(next.Cstr());
			 String userName = String(StripURL((const char *) user->GetUserName().utf8())).Trim();
			 PRINT("MatchUserFilter: username = %s\n", userName.Cstr());
			 PRINT("MatchUserFilter: regex = %s\n", next.Cstr());
			 if ((userName.Length() > 0) && sm.Match(userName.Cstr()))
			 {
				 return true;
			 }
		}
	}
	return false;
}

bool
WinShareWindow::DoTabCompletion(QString origText, QString & result, const char * filter)
{
	// Do it all in lower case, for case insensitivity
	String text((const char *) origText.lower().utf8());

	// Compile a list of pointers to beginnings-of-words in the user's chat string
	Queue<const char *> words;
	bool inSpace = true;
	const char * next = text.Cstr();
	while(*next)
	{
		if (inSpace)
		{
		 if ((*next != ' ')&&(*next != '\t'))
		 {
			words.AddTail(next);
			inSpace = false;
		 }
		}
		else if ((*next == ' ')||(*next == '\t')) inSpace = true;

		next++;
	}

	// Now try matching, starting with the last word.
	// If no match is found, try the last two words, and so on.
	const char * startAt = NULL, * backupStartAt = NULL;
	String matchString, backupMatchString;
	for (int i=words.GetNumItems()-1; i>=0; i--)
	{
		const char * matchAt = words[i];
		QString qres;

		PRINT("Matching\n");
		int numMatches = MatchUserName(QString::fromUtf8(words[i]), qres, filter);
		PRINT("Match complete\n");
		if (numMatches == 1)
		{
			PRINT("Found match %S\n", qStringToWideChar(qres));
			matchString = (const char *) qres.utf8();  // found a unique match!  We're done!
			startAt = matchAt;
			break;
		}
		else if (numMatches > 1)
		{
			PRINT("Found multiple matches %S\n", qStringToWideChar(qres));
			backupMatchString = (const char *) qres.utf8();  // found several matches; keep trying for a single
			backupStartAt = matchAt;         // but we'll use this if nothing else
		}
		matchString.Prepend(" ");
	}

	if (startAt == NULL)
	{
		startAt = backupStartAt;
		matchString = backupMatchString;
		QApplication::beep();
	}
	if (startAt)
	{
		String returnCompletedText = (const char *) origText.utf8();
		returnCompletedText = returnCompletedText.Substring(0, startAt-text.Cstr());
		returnCompletedText += matchString;
		result = QString::fromUtf8(returnCompletedText.Cstr());
		return true;
	}
	return false;
}

bool
WinShareWindow::NameSaid(QString & msg)
{
	String sname = StripURL((const char *) GetUserName().utf8()); // <postmaster@raasu.org> -- Don't use latin1 ()
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
		if (fWinHandle && !this->isActiveWindow() && (fSettings->GetFlash() & WSettings::FlashMain))	// if we have a valid handle AND we are not active AND the user wants to flash
		{
			WFlashWindow(fWinHandle); // flash our window
		}
#endif // WIN32
	}
	return bNameSaid;
}

bool
WinShareWindow::NameSaid2(String sname, QString & msg, unsigned long index)
{
	String itxt((const char *) msg.utf8()); // <postmaster@raasu.org> -- Don't use latin1 ()
	itxt = itxt.ToUpperCase();

	int sred = (itxt.StartsWith(sname)) ? 0 : -1;
	if (sred < 0)
	{
		sred = itxt.IndexOf(sname.Prepend(" "), index);
		if (sred >= 0)
			sred++;
	}
	if (sred >= 0)
	{
		int rlen = sname.Length();

		String temp = (const char *) GetUserName().utf8(); // <postmaster@raasu.org> 20021005 -- don't use latin1 ()
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
		temp = StripURL((const char *) GetUserName().utf8());
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
		PRINT("Name said string: %S\n", qStringToWideChar(smsg));
		msg = smsg;
		return true;
	}
	return false;
}

void
WinShareWindow::GotUpdateCmd(const char * key, QString value)
{
	if (value.length()>0)
	{
		QString server = value.lower();
		// you can also compare against "version", but that is beshare specific
		if (!strcmp(key, "addserver"))
		{
			bool exists = false;
			// see if the server exists yet...
			for (int i = 0; i < fServerList->count(); i++)
			{
				QString s = fServerList->text(i).lower().stripWhiteSpace();
				if (s == server)
				{
					exists = true;
					break;
				}
			}
			if (!exists)
				fServerList->insertItem(value);
		}
		else if (!strcmp(key, "removeserver"))
		{
			// try to find the server
			for (int i = 0; i < fServerList->count(); i++)
			{
				QString s = fServerList->text(i).lower().stripWhiteSpace();
				if (s == server)
				{
					// wipe the guy out
					fServerList->removeItem(i);
					i--;	// go down one...
				}
			}
		}
	}
}

void
WinShareWindow::ServerParametersReceived(const Message * msg)
{
	if (fSettings->GetInfo())
	{
		const char * serverVersion;
		if (msg->FindString(PR_NAME_SERVER_VERSION, &serverVersion) == B_OK)
			PrintSystem(tr("Server version: %1").arg(serverVersion));

		int64 serverUptime;
		if (msg->FindInt64(PR_NAME_SERVER_UPTIME, &serverUptime) == B_OK)
			PrintSystem(tr("Server uptime: %1").arg(MakeHumanTime(serverUptime)));

		// reuse old string
		if (msg->FindString(PR_NAME_SESSION_ROOT, &serverVersion) == B_OK)
			PrintSystem(tr("Session root: %1").arg(serverVersion));

		int64 memA, memU;
		if ((msg->FindInt64(PR_NAME_SERVER_MEM_AVAILABLE, &memA) == B_OK) &&
			(msg->FindInt64(PR_NAME_SERVER_MEM_USED, &memU) == B_OK))
		{
			const float one = 1024.0f * 1024.0f;
			float ma = ((float)memA) / one;
			float mu = ((float)memU) / one;
			PrintSystem(tr("Memory usage: %1 MB of %2 MB used").arg(mu).arg(ma));
		}
	}
}

void
WinShareWindow::SetWatchPattern(QString pattern)
{
	fWatch = pattern;
	if (fSettings->GetInfo())
	{
		if (fWatch == "")	// no pattern?
			PrintSystem(tr("Watch pattern cleared."));
		else
			PrintSystem(tr("Watch pattern set to %1.").arg(pattern));
	}
}

QString
WinShareWindow::ParseForShown(const QString & txt)
{
	// <postmaster@raasu.org> 20021005,20021128 -- Don't use latin1(), use QStringTokenizer ;)
	QStringTokenizer tk(txt, "\t");
	QString next;
	QString out;
	while ((next = tk.GetNextToken()) != QString::null)
	{
#if (QT_VERSION < 0x030100)
		out += "<br>";	// replace our TAB
#endif
		out += next; 
	}
	return out;
}

bool
WinShareWindow::CheckVersion(const char * buf, QString * version)
{
	int maj, min, rev, build;
	int ret = sscanf(buf, "%d,%d,%d,%d", &maj, &min, &rev, &build);
	PRINT("CheckVersion: ret == %d\n", ret);
	if (ret == 4)	// we want 4 return values
	{
		PRINT("Checking version %d.%d.%d.%d\n", maj, min, rev, build);
		if (maj == kMajor)
		{
			if (min == kMinor)
			{
				if (rev == kPatch)
				{
					if (build <= kBuild)
						return false;
				}
				else if (rev < kPatch)
				{
					return false;
				}
			}
			else if (min < kMinor)
			{
				return false;
			}
		}
		else if (maj < kMajor)
		{
			return false;
		}
		if (version)
			*version = tr("%1.%2.%3 build %4").arg(maj).arg(min).arg(rev).arg(build);
		return true;
	}
	return false;
}

QString
WinShareWindow::GetTimeStamp()
{
	char sCurTime[50];
	time_t curTime;
	struct tm * curTimeTm;

	curTime = time(NULL);
	curTimeTm = localtime(&curTime);
	sprintf(sCurTime, "[%d/%d %.2d:%.2d:%.2d] ", curTimeTm->tm_mon + 1, curTimeTm->tm_mday, 
				curTimeTm->tm_hour, curTimeTm->tm_min, curTimeTm->tm_sec);

	QString cl = WColors::Text;
	QString ret = WFormat::TimeStamp.arg(cl).arg(gWin->fSettings->GetFontSize()).arg(sCurTime);
	return ret;
}

bool
WinShareWindow::MatchFilter(const QString user, const char * filter)
{
	if (user == NULL) 
		return false;
	StringTokenizer idTok(filter, ","); // identifiers may be separated by commas (but not spaces, as those may be parts of the users' names!)
	const char * n;
	while((n = idTok.GetNextToken()) != NULL)
	{
		String next(n);
		next = next.Trim();
		
		// Does this item (interpreted as a regex) match our user's name?
		MakeRegexCaseInsensitive(next);
		StringMatcher sm(next.Cstr());
		String userName = String((const char *) user.utf8()).Trim();
		if ((userName.Length() > 0) && sm.Match(userName.Cstr()))
		{
			return true;
		}
	}
	return false;
}
