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
#include <qregexp.h>

int
WinShareWindow::MatchUserName(const QString & un, QString & result, const QString & filter)
{
	int matchCount = 0;
	QString oldName;

	WUserIter iter = fNetClient->Users().GetIterator();

	while (iter.HasMoreValues())
	{
		WUserRef user;
		iter.GetNextValue(user);
		QString userName = StripURL(user()->GetUserName().lower().stripWhiteSpace());
		if (((filter == QString::null) || (MatchUserFilter(user, filter))) &&
			(userName.startsWith(un.lower())))
		{
			// Only count different nicks
			if (oldName != userName)
				matchCount++;

			if (oldName.isEmpty())
				oldName = userName;

			if (matchCount == 1)
			{
				result = StripURL(user()->GetUserName().stripWhiteSpace());
			}
			else
			{
				// multiple matches
				PRINT("WinShareWindow::MatchUserName: Multiple matches\n");

				for (uint32 i = 0; i < result.length(); i++)
				{
					if (result.at(i).lower() != userName.at(i))
					{
						result.truncate(i);
						break;
					}
				}
			}
		}
	}

#ifdef _DEBUG
	WString wUser(result);
	PRINT("WinShareWindow::MatchUserName: Result %S\n", wUser.getBuffer());
#endif

	return matchCount;
}


bool
WinShareWindow::MatchUserFilter(const WUserRef & user, const QString &filter)
{
	if (!filter.isEmpty() && user())
	{
		QStringTokenizer idTok(filter, ","); // identifiers may be separated by commas (but not spaces, as those may be parts of the users' names!)
		QString n;
		while((n = idTok.GetNextToken()) != QString::null)
		{
			QString next = StripURL(n);
			next = next.stripWhiteSpace();
			
			QString userID = user()->GetUserID();
			WString wuid(userID);
			WString wnxt(next);
			PRINT2("MatchUserFilter: UserID = %S\n", wuid.getBuffer());
			PRINT2("MatchUserFilter: next   = %S\n", wnxt.getBuffer());
			
			if (userID.length() > 0)
			{
				// Is this item our user's session ID?
				if (userID == next)
				{
					return true;
				}
			}
			
			QString qUser = StripURL(user()->GetUserName().lower());
			QString userName = qUser.stripWhiteSpace();
			
			if (userName.length() > 0)
			{
				if (userName == next)	// Is this item our user's name?
				{
					return true;
				}
				else 
				{
					// Does this item (interpreted as a regex) match our user's name?
					ConvertToRegex(next);
					QRegExp qr(next, false);
					WString wusr(userName);
					WString wnxt(next);
					PRINT2("MatchUserFilter: UserName = %S\n", wusr.getBuffer());
					PRINT2("MatchUserFilter: next = %S\n", wnxt.getBuffer());
					if (userName.find(qr) >= 0)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool
WinShareWindow::DoTabCompletion(const QString & origText, QString & result)
{
	// Compile a list of indexes to beginnings-of-words in the user's chat string
	Queue<int> words;
	bool inSpace = true;
	unsigned int next = 0;
	while(next < origText.length() )
	{
		if (inSpace)
		{
		 if ((origText.at(next) != ' ')&&(origText.at(next) != '\t'))
		 {
			words.AddTail(next);
			inSpace = false;
		 }
		}
		else if ((origText.at(next) == ' ')||(origText.at(next) == '\t')) inSpace = true;

		next++;
	}

	// Now try matching, starting with the last word.
	// If no match is found, try the last two words, and so on.
	int startAt = -1, backupStartAt = -1;
	QString matchString, backupMatchString;
	for (int i=words.GetNumItems()-1; i>=0; i--)
	{
		int matchAt = words[i];
		QString qres;

		PRINT("Matching\n");
		int numMatches = MatchUserName(origText.mid(words[i]), qres, QString::null);
		PRINT("Match complete\n");
		if (numMatches == 1)
		{
#ifdef _DEBUG
			WString wResult(qres);
			PRINT("Found match %S\n", wResult.getBuffer());
#endif

			matchString = qres;  // found a unique match!  We're done!
			startAt = matchAt;
			break;
		}
		else if (numMatches > 1)
		{
#ifdef _DEBUG
			WString wResult(qres);
			PRINT("Found multiple matches %S\n", wResult.getBuffer());
#endif

			backupMatchString = qres;		// found several matches; keep trying for a single
			backupStartAt = matchAt;        // but we'll use this if nothing else
		}
		matchString.prepend(" ");
	}

	if (startAt == -1)
	{
		startAt = backupStartAt;
		matchString = backupMatchString;
		if (fSettings->GetSounds())
			QApplication::beep();
	}
	if (startAt != -1)
	{
		QString returnCompletedText(origText.left(startAt));
		returnCompletedText += matchString;
		result = returnCompletedText;
		return true;
	}
	return false;
}

void
WinShareWindow::GotUpdateCmd(const QString & key, const QString & value)
{
	if (value.length() > 0)
	{
		QString server = value.lower().stripWhiteSpace();

		// you can also compare against "version", but that is beshare specific
		if (key == "addserver")
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
		else if (key == "removeserver")
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
WinShareWindow::ServerParametersReceived(MessageRef msg)
{
	if (fSettings->GetInfo())
	{
		const char * serverVersion;
		if (msg()->FindString(PR_NAME_SERVER_VERSION, &serverVersion) == B_OK)
			SendSystemEvent(tr("Server version: %1").arg(serverVersion));

		int64 serverUptime;
		if (msg()->FindInt64(PR_NAME_SERVER_UPTIME, &serverUptime) == B_OK)
			SendSystemEvent(tr("Server uptime: %1").arg(MakeHumanTime(serverUptime)));

		// reuse old string
		if (msg()->FindString(PR_NAME_SESSION_ROOT, &serverVersion) == B_OK)
			SendSystemEvent(tr("Session root: %1").arg(serverVersion));

		int64 memA, memU;
		if ((msg()->FindInt64(PR_NAME_SERVER_MEM_AVAILABLE, &memA) == B_OK) &&
			(msg()->FindInt64(PR_NAME_SERVER_MEM_USED, &memU) == B_OK))
		{
			const float one = 1024.0f * 1024.0f;
			float ma = ((float)memA) / one;
			float mu = ((float)memU) / one;
			SendSystemEvent(tr("Memory usage: %1 MB of %2 MB used").arg(mu).arg(ma));
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

/*
bool
WinShareWindow::MatchFilter(const QString & user, const QString & filter)
{
	return MatchFilter(user, (const char *) filter.lower().utf8());
}
*/

bool
WinShareWindow::MatchFilter(const QString & user, const QString & filter)
{
	if (!filter.isEmpty() && (user.length() > 0))
	{
		QStringTokenizer idTok(filter, ","); // identifiers may be separated by commas (but not spaces, as those may be parts of the users' names!)
		QString n;
		while((n = idTok.GetNextToken()) != QString::null)
		{
			QString next(n);
			next = next.stripWhiteSpace();
			
			// Does this item (interpreted as a regex) match our user's name?
			ConvertToRegex(next);
			QRegExp qr(next, false);
			QString userName = user.stripWhiteSpace();
			if ((userName.length() > 0) && userName.find(qr) >= 0)
			{
				return true;
			}
		}
	}
	return false;
}

int
WinShareWindow::FillUserMap(const QString & filter, WUserMap & wmap)
{
	int matchCount = 0;
	WUserIter iter = fNetClient->Users().GetIterator();

	while (iter.HasMoreValues())
	{
		WUserRef user;
		iter.GetNextValue(user);
		if ( MatchUserFilter(user, filter) )
		{
			uint32 uid = user()->GetUserID().toULong();
			wmap.Put(uid, user);
			matchCount++;
		}
	}
	return matchCount;
}
