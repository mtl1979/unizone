#ifdef WIN32		// <postmaster@raasu.org> 20021022 -- Fix to use platform.h, Fixed Window Flashing for older API's
#include <windows.h>
#endif

#include "winsharewindow.h"
#include "global.h"
#include "util/StringTokenizer.h"
#include "debugimpl.h"
#include "settings.h"
#include "formatting.h"
#include "netclient.h"
#include "util.h"
#include "wstring.h"
#include "tokenizer.h" // <postmaster@raasu.org> 20021128

#include <qapplication.h>

int
WinShareWindow::MatchUserName(const QString & un, QString & result, const char * filter)
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
		if (((filter == NULL) || (MatchUserFilter(user, filter))) &&
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

	WString wUser(res);
	PRINT("WinShareWindow::MatchUserName: Result %S\n", wUser.getBuffer());

	result = res;
	return matchCount;
}

bool
WinShareWindow::MatchUserFilter(const WUserRef & user, const char * filter)
{
	if (user() == NULL) 
		return false;

	StringTokenizer idTok(filter, ","); // identifiers may be separated by commas (but not spaces, as those may be parts of the users' names!)
	const char * n;
	while((n = idTok.GetNextToken()) != NULL)
	{
		String next = StripURL(n);
		next = next.Trim();

		PRINT("MatchUserFilter: UserID = %s\n", (const char *) user()->GetUserID().utf8());
		PRINT("MatchUserFilter: next   = %s\n", next.Cstr());
		String userID = (const char *) user()->GetUserID().utf8();
		if (userID.Length() > 0)
		{
			// Is this item our user's session ID?
			if (userID == next)
			{
				return true;
			}
		}

		String userName = StripURL((const char *) user()->GetUserName().utf8());
		userName = userName.Trim();

		if (userName.Length() > 0)
		{
			if (userName == next)	// Is this item our user's name?
			{
				return true;
			}
			else 
			{
				// Does this item (interpreted as a regex) match our user's name?
				ConvertToRegex(next);
				MakeRegexCaseInsensitive(next);
				StringMatcher sm(next.Cstr());
				PRINT("MatchUserFilter: UserName = %s\n", userName.Cstr());
				PRINT("MatchUserFilter: next = %s\n", next.Cstr());
				if (sm.Match(userName.Cstr()))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool
WinShareWindow::DoTabCompletion(const QString & origText, QString & result, const char * filter)
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
			WString wResult(qres);
			PRINT("Found match %S\n", wResult.getBuffer());

			matchString = (const char *) qres.utf8();  // found a unique match!  We're done!
			startAt = matchAt;
			break;
		}
		else if (numMatches > 1)
		{
			WString wResult(qres);
			PRINT("Found multiple matches %S\n", wResult.getBuffer());

			backupMatchString = (const char *) qres.utf8();  // found several matches; keep trying for a single
			backupStartAt = matchAt;         // but we'll use this if nothing else
		}
		matchString.Prepend(" ");
	}

	if (startAt == NULL)
	{
		startAt = backupStartAt;
		matchString = backupMatchString;
		if (fSettings->GetSounds())
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

void
WinShareWindow::GotUpdateCmd(const char * key, const char * value)
{
	if (strlen(value) > 0)
	{
		QString server = value;
		server = server.lower().stripWhiteSpace();
		// you can also compare against "version", but that is beshare specific
		if (strcmp(key, "addserver") == 0)
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
				fServerList->insertItem(server);
		}
		else if (strcmp(key, "removeserver") == 0)
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
WinShareWindow::ServerParametersReceived(const MessageRef msg)
{
	if (fSettings->GetInfo())
	{
		const char * serverVersion;
		if (msg()->FindString(PR_NAME_SERVER_VERSION, &serverVersion) == B_OK)
			PrintSystem(tr("Server version: %1").arg(serverVersion));

		int64 serverUptime;
		if (msg()->FindInt64(PR_NAME_SERVER_UPTIME, &serverUptime) == B_OK)
			PrintSystem(tr("Server uptime: %1").arg(MakeHumanTime(serverUptime)));

		// reuse old string
		if (msg()->FindString(PR_NAME_SESSION_ROOT, &serverVersion) == B_OK)
			PrintSystem(tr("Session root: %1").arg(serverVersion));

		int64 memA, memU;
		if ((msg()->FindInt64(PR_NAME_SERVER_MEM_AVAILABLE, &memA) == B_OK) &&
			(msg()->FindInt64(PR_NAME_SERVER_MEM_USED, &memU) == B_OK))
		{
			const float one = 1024.0f * 1024.0f;
			float ma = ((float)memA) / one;
			float mu = ((float)memU) / one;
			PrintSystem(tr("Memory usage: %1 MB of %2 MB used").arg(mu).arg(ma));
		}
	}
}

void
WinShareWindow::SetWatchPattern(const QString &pattern)
{
	fWatch = pattern;
	if (fSettings->GetInfo())
	{
		if (fWatch.isEmpty())	// no pattern?
			PrintSystem(tr("Watch pattern cleared."));
		else
			PrintSystem(tr("Watch pattern set to %1.").arg(pattern));
	}
}

bool
WinShareWindow::MatchFilter(const QString & user, const char * filter)
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
		ConvertToRegex(next);
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

int
WinShareWindow::FillUserMap(const QString & filter, WUserMap & wmap)
{
	int matchCount = 0;
	WUserIter iter = fNetClient->Users().begin();
	WUserRef user;

	while (iter != fNetClient->Users().end())
	{
		user = (*iter).second;
		if ( MatchUserFilter(user, (const char *) filter.utf8()) )
		{
			WUserPair wpair = MakePair(user()->GetUserID(), user);
			wmap.insert(wmap.end(), wpair);
			matchCount++;
		}
		iter++;
	}
	return matchCount;
}
