#include "winsharewindow.h"
#include "wstring.h"
#include "netclient.h"
#include "tokenizer.h"
#include "settings.h"

bool
WinShareWindow::IsIgnoredIP(const QString & ip)
{
	WString wIP(ip);
	PRINT("IsIgnoredIP(%S)\n", wIP.getBuffer());

#ifdef DEBUG2
	wIP = fIgnoreIP;
	PRINT("IP IGNORE MASK: %S\n", wIP.getBuffer());
#endif

	return MatchFilter(ip, (const char *) fIgnoreIP.utf8());
}

bool
WinShareWindow::AddIPIgnore(const QString & ip)
{
	WString wIP(ip);
	PRINT("AddIPIgnore(%S)\n", wIP.getBuffer());

	if ( IsIgnoredIP(ip) )
		return false;

	// Append to ignore list
	//
	if (fIgnoreIP.isEmpty())
		fIgnoreIP = ip;
	else
	{
		fIgnoreIP += ",";
		fIgnoreIP += ip;
	}

#ifdef DEBUG2
	wIP = fIgnoreIP;
	PRINT("IP IGNORE MASK: %S\n", wIP.getBuffer());
#endif

	return true;

}

bool
WinShareWindow::RemoveIPIgnore(const QString & ip)
{
	WString wIP(ip);
	PRINT("RemoveIPIgnore(%S)\n", wIP.getBuffer());

	if ( !IsIgnoredIP(ip) )
		return false;

	if (fIgnoreIP == ip) // First and only?
	{
		fIgnoreIP = "";
#ifdef DEBUG2
		PRINT("IP IGNORE MASK CLEARED\n");
#endif

		return true;
	}
	// First in list?

	int pos = fIgnoreIP.startsWith(ip + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fIgnoreIP.find("," + ip + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = ip.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fIgnoreIP.right(len) == ("," + ip))
		{
			// Find last occurance (there is no endsWith in QString class)
			pos = fIgnoreIP.findRev("," + ip, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not ignored!
		return false;
	}

	if (pos == 0)
	{
		fIgnoreIP = fIgnoreIP.mid(len);
	}
	else
	{
		fIgnoreIP = fIgnoreIP.left(pos)+fIgnoreIP.mid(len+pos);
	}

#ifdef DEBUG2
	wIP = fIgnoreIP;
	PRINT("IP IGNORE MASK: %S\n", wIP.getBuffer());
#endif

	return true;
}

bool
WinShareWindow::IsBlackListedIP(const QString & ip)
{
	WUserMap umap;
	fNetClient->FindUsersByIP(umap, ip);

	for (WUserIter iter = umap.begin(); iter != umap.end(); iter++)
	{
		if ((*iter).second() != NULL)
		{
			if ( IsBlackListed((*iter).second) )
				return true;
		}
	}
	return false;
}

bool
WinShareWindow::IsBlackListed(const WUserRef & user)
{
	if (user() == NULL) // Is the user still connected?
		return false;

	if (fBlackList.isEmpty()) // No users in blacklist?
		return false;

	return MatchUserFilter(user, (const char *) fBlackList.utf8());
}

bool
WinShareWindow::IsBlackListed(const QString & user)
{
	// Find the user record
	//

	WUserRef uref = fNetClient->FindUser(user);

	// Valid user reference?
	//

	if (uref() != NULL)	
	{
		// Valid reference!
	
		// Check user reference against blacklist
		
		return IsBlackListed(uref);
	}
	else
	{
		// Invalid reference!

		return MatchFilter(user, (const char *) fBlackList.utf8());
	}
}

bool
WinShareWindow::IsWhiteListed(const WUserRef & user)
{
	if (user() == NULL) // Is the user still connected?
		return false;

	if (fWhiteList.isEmpty()) // No users in whitelist?
		return false;

	return MatchUserFilter(user, (const char *) fWhiteList.utf8());
}

bool
WinShareWindow::IsWhiteListed(const QString & user)
{
	// Find the user record
	//

	WUserRef uref = fNetClient->FindUser(user);

	// Valid user reference?
	//

	if (uref() != NULL)	
	{
		// Valid reference!
	
		// Check user reference against whitelist
		
		return IsWhiteListed(uref);
	}
	else
	{
		// Invalid reference!

		return MatchFilter(user, (const char *) fWhiteList.utf8());
	}
}

bool
WinShareWindow::IsFilterListed(const QString & pattern)
{
	if (fFilterList.isEmpty())	// empty?
		return false;
	
	QStringTokenizer tok(fFilterList);
	QString t;
	while ((t = tok.GetNextToken()) != QString::null)
	{
		QRegExp qr(t);
		if ((pattern.find(qr) >= 0) || (t == pattern))
			return true;
	}
	return false;
}

bool
WinShareWindow::IsAutoPrivate(const QString & user)
{
	// Find the user record
	//

	WUserRef uref = fNetClient->FindUser(user);

	// Valid user reference?
	//

	if (uref() != NULL)	
	{
		// Valid reference!
	
		// Check user reference against auto-private list
		
		return IsAutoPrivate(uref);
	}
	else
	{
		// Invalid reference!

		return MatchFilter(user, (const char *) fAutoPriv.utf8());
	}
}

bool
WinShareWindow::IsIgnored(const WUserRef & user)
{
	if (user() == NULL) // Is the user still connected?
		return false;

	if (fIgnore.isEmpty()) // No users in ignore list?
		return false;

	return MatchUserFilter(user, (const char *) fIgnore.utf8());
}

bool
WinShareWindow::IsIgnored(const QString & user, bool bTransfer)
{
	bool bDisconnected = false;

	// default for bDisconnected is true, if bTransfer is true
	if (bTransfer)
		bDisconnected = true;

	return IsIgnored(user, bTransfer, bDisconnected);
}

bool
WinShareWindow::IsIgnored(const QString & user, bool bTransfer, bool bDisconnected)
{
	// Find the user record
	//

	WUserRef uref = fNetClient->FindUser(user);

	// Valid user reference?
	//

	if (uref() != NULL)	
	{
		// Valid reference!

		// Nuke the binkys?

		if (fSettings->GetBinkyNuke() && bTransfer)
		{	
			if (uref()->GetUserName().lower().find("binky") > -1) 
				return true;
		}
	
		// Check user reference against ignore list
		
		return IsIgnored(uref);
	}
	else
	{
		// Invalid reference!

		// Nuke disconnected users?

		if (fSettings->GetBlockDisconnected() && bTransfer && bDisconnected)
			return true;
		else
			return MatchFilter(user, (const char *) fIgnore.utf8());
	}
}

// Append to blacklist
//

bool
WinShareWindow::BlackList(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Already blacklisted?
	//
	if (IsBlackListed(user))
		return false;

	// Append to blacklist
	//
	if (fBlackList.isEmpty())
		fBlackList = user;
	else
	{
		fBlackList += ",";
		fBlackList += user;
	}
	return true;
}

bool
WinShareWindow::WhiteList(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Already whitelisted?
	//
	if (IsWhiteListed(user))
		return false;

	// Append to whitelist
	//
	if (fWhiteList.isEmpty())
		fWhiteList = user;
	else
	{
		fWhiteList += ",";
		fWhiteList += user;
	}
	return true;
}

bool
WinShareWindow::FilterList(const QString & pattern)
{
	// Is pattern specified?
	//
	if (pattern.isEmpty())
		return false;

	// Already in filter list?
	//
	if (IsFilterListed(pattern))
		return false;

	// Append to filter list
	//
	if (fFilterList.isEmpty())
		fFilterList = pattern;
	else
	{
		fFilterList += ",";
		fFilterList += pattern;
	}
	return true;
}

// Remove from blacklist
//

bool
WinShareWindow::UnBlackList(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Is really blacklisted?
	//
	if (fBlackList == user) // First and only?
	{
		fBlackList = "";
		return true;
	}

	// First in list?

	int pos = fBlackList.lower().startsWith(user.lower() + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fBlackList.find("," + user + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = user.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fBlackList.lower().right(len) == ("," + user.lower()))
		{
			// Find last occurance (there is no endsWith in QString class)

			pos = fBlackList.findRev("," + user, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not blacklisted!

		return false;
	}

	if (pos == 0)
	{
		fBlackList = fBlackList.mid(len);
	}
	else
	{
		fBlackList = fBlackList.left(pos)+fBlackList.mid(len+pos);
	}
	return true;
}

bool
WinShareWindow::UnWhiteList(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Is really whitelisted?
	//
	if (fWhiteList == user) // First and only?
	{
		fWhiteList = "";
		return true;
	}

	// First in list?

	int pos = fWhiteList.lower().startsWith(user.lower() + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fWhiteList.find("," + user + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = user.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fWhiteList.lower().right(len) == ("," + user.lower()))
		{
			// Find last occurance (there is no endsWith in QString class)

			pos = fWhiteList.findRev("," + user, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not whitelisted!

		return false;
	}

	if (pos == 0)
	{
		fWhiteList = fWhiteList.mid(len);
	}
	else
	{
		fWhiteList = fWhiteList.left(pos)+fWhiteList.mid(len+pos);
	}
	return true;
}

bool
WinShareWindow::UnFilterList(const QString & pattern)
{
	// Is pattern specified?
	//
	if (pattern.isEmpty())
		return false;

	// Is really in filter list?
	//
	if (fFilterList == pattern) // First and only?
	{
		fFilterList = "";
		return true;
	}

	// First in list?

	int pos = fFilterList.lower().startsWith(pattern.lower() + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fFilterList.find("," + pattern + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = pattern.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fFilterList.lower().right(len) == ("," + pattern.lower()))
		{
			// Find last occurance (there is no endsWith in QString class)

			pos = fFilterList.findRev("," + pattern, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not in filter list!

		return false;
	}

	if (pos == 0)
	{
		fFilterList = fFilterList.mid(len);
	}
	else
	{
		fFilterList = fFilterList.left(pos)+fFilterList.mid(len+pos);
	}
	return true;
}

// Append to ignore list
//

bool
WinShareWindow::Ignore(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Already ignored?
	//
	if (IsIgnored(user, true, false))
		return false;

	// Append to ignore list
	//
	if (fIgnore == "")
		fIgnore = user;
	else
	{
		fIgnore += ",";
		fIgnore += user;
	}
	return true;
}


// Remove from ignore list
//

bool
WinShareWindow::UnIgnore(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Is really ignored?
	//
	if (fIgnore == user) // First and only?
	{
		fIgnore = "";
		return true;
	}

	// First in list?

	int pos = fIgnore.lower().startsWith(user.lower() + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fIgnore.find("," + user + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = user.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fIgnore.lower().right(len) == ("," + user.lower()))
		{
			// Find last occurance (there is no endsWith in QString class)

			pos = fIgnore.findRev("," + user, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not ignored!

		return false;
	}

	if (pos == 0)
	{
		fIgnore = fIgnore.mid(len);
	}
	else
	{
		fIgnore = fIgnore.left(pos)+fIgnore.mid(len+pos);
	}
	return true;
}

bool
WinShareWindow::IsAutoPrivate(const WUserRef & user)
{
	if (user() == NULL) // Is the user still connected?
		return false;

	if (fAutoPriv.isEmpty()) // No users in auto-private list?
		return false;

	return MatchUserFilter(user, (const char *) fAutoPriv.utf8());
}

// Append to auto-private list
//

bool
WinShareWindow::AutoPrivate(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Already in auto-private list?
	//
	if (IsAutoPrivate(user))
		return false;

	// Append to auto-private list
	//
	if (fAutoPriv.isEmpty())
		fAutoPriv = user;
	else
	{
		fAutoPriv += ",";
		fAutoPriv += user;
	}
	return true;
}

// Remove from auto-private list
//

bool
WinShareWindow::UnAutoPrivate(const QString & user)
{
	// Is user specified?
	//
	if (user.isEmpty())
		return false;

	// Is really in auto-private list?
	//
	if (fAutoPriv == user) // First and only?
	{
		fAutoPriv = "";
		return true;
	}

	// First in list?

	int pos = fAutoPriv.lower().startsWith(user.lower() + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fAutoPriv.find("," + user + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = user.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fAutoPriv.lower().right(len) == ("," + user.lower()))
		{
			// Find last occurance (there is no endsWith in QString class)

			pos = fAutoPriv.findRev("," + user, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not in auto-private list!

		return false;
	}

	if (pos == 0)
	{
		fAutoPriv = fAutoPriv.mid(len);
	}
	else
	{
		fAutoPriv = fAutoPriv.left(pos)+fAutoPriv.mid(len+pos);
	}
	return true;

}
