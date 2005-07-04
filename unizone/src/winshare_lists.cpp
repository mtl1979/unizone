#include "winsharewindow.h"
#include "debugimpl.h"
#include "wstring.h"
#include "netclient.h"
#include "tokenizer.h"
#include "settings.h"
#include "util.h"

#include <qregexp.h>

bool
WinShareWindow::IsIgnoredIP(const QString & ip)
{
	{
		WString wIP(ip);
		PRINT("IsIgnoredIP(%S)\n", wIP.getBuffer());
	}

	{
		WString wIP(fIgnoreIP);
		PRINT2("IP IGNORE MASK: %S\n", wIP.getBuffer());
	}

	return MatchFilter(ip, fIgnoreIP);
}

bool
WinShareWindow::AddIPIgnore(const QString & ip)
{
	{
		WString wIP(ip);
		PRINT("AddIPIgnore(%S)\n", wIP.getBuffer());
	}

	if ( IsIgnoredIP(ip) )
		return false;

	// Append to ignore list
	//

	AddToList(fIgnoreIP, ip);

	{
		WString wIP(fIgnoreIP);
		PRINT2("IP IGNORE MASK: %S\n", wIP.getBuffer());
	}

	return true;

}

bool
WinShareWindow::RemoveIPIgnore(const QString & ip)
{
	{
		WString wIP(ip);
		PRINT("RemoveIPIgnore(%S)\n", wIP.getBuffer());
	}

	if ( !IsIgnoredIP(ip) )
		return false;

	if (fIgnoreIP == ip) // First and only?
	{
		fIgnoreIP = QString::null;
		PRINT2("IP IGNORE MASK CLEARED\n");

		return true;
	}

	RemoveFromList(fIgnoreIP, ip);

	{
		WString wIP(fIgnoreIP);
		PRINT2("IP IGNORE MASK: %S\n", wIP.getBuffer());
	}

	return true;
}

bool
WinShareWindow::IsBlackListedIP(const QString & ip)
{
	WUserMap umap;
	fNetClient->FindUsersByIP(umap, ip);

	WUserIter iter = umap.GetIterator();
	while (iter.HasMoreValues())
	{
		WUserRef uref;
		iter.GetNextValue(uref);
		if (uref())
		{
			if ( IsBlackListed(uref) )
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

	return MatchUserFilter(user, fBlackList);
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

		return MatchFilter(user, fBlackList);
	}
}

bool
WinShareWindow::IsWhiteListed(const WUserRef & user)
{
	if (user() == NULL) // Is the user still connected?
		return false;

	if (fWhiteList.isEmpty()) // No users in whitelist?
		return false;

	return MatchUserFilter(user, fWhiteList);
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

		return MatchFilter(user, fWhiteList);
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
		t = t.lower();
		QString p = pattern.lower();
		if (t != p)
		{
			QRegExp qr(t);
			if (p.find(qr) >= 0)
				return true;
		}
		else
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

		return MatchFilter(user, fAutoPriv);
	}
}

bool
WinShareWindow::IsIgnored(const WUserRef & user)
{
	if (user() == NULL) // Is the user still connected?
		return false;

	if (fIgnore.isEmpty()) // No users in ignore list?
		return false;

	return MatchUserFilter(user, fIgnore);
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
			if (uref()->GetUserName().find(QString("binky"), 0, false) >= 0) 
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
			return MatchFilter(user, fIgnore);
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

	AddToList(fBlackList, user);

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

	AddToList(fWhiteList, user);

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

	AddToList(fFilterList, pattern);

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

	if (!IsBlackListed(user))
		return false;

	RemoveFromList(fBlackList, user);

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

	if (!IsWhiteListed(user))
		return false;

	RemoveFromList(fWhiteList, user);

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

	if (!IsFilterListed(pattern))
		return false;

	RemoveFromList(fFilterList, pattern);

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

	AddToList(fIgnore, user);

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

	if (!IsIgnored(user, true, false))
		return false;

	RemoveFromList(fIgnore, user);

	return true;
}

bool
WinShareWindow::IsAutoPrivate(const WUserRef & user)
{
	if (user() == NULL) // Is the user still connected?
		return false;

	if (fAutoPriv.isEmpty()) // No users in auto-private list?
		return false;

	return MatchUserFilter(user, fAutoPriv);
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

	AddToList(fAutoPriv, user);

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

	if (!IsAutoPrivate(user))
		return false;

	RemoveFromList(fAutoPriv, user);

	return true;
}
