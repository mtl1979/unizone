#include "user.h"
#include "netclient.h"
#include "debugimpl.h"
#include "global.h"
#include "botitem.h"
#include "userlistitem.h"
#include "regex/PathMatcher.h"
#include "platform.h"			// <postmaster@raasu.org> 20021114

#include <qlistview.h>
#include <qstring.h>

WUser::WUser(QString sid)
{
	fUserName = "?";
	fUserStatus = "?";
	fHostName = "?";
	fUserID = sid;
	fCurUploads = fMaxUploads = 0;
	fBandwidthLabel = "?";
	fBandwidthBPS = 0;
	fFileCount = 0;
	fFirewalled = false;
	fBot = false;
	fPort = 0;
	fInstallID = 0;
	fClient = "?";
	fNeedPing = true;
	fPartial = false;
}

WUser::~WUser()
{
	PRINT("~WUser()\n");
	RemoveFromListView();
}

void
WUser::InitName(const Message * msg)
{
	int32 port;
	const char * name;
	const char * vname, * vnum;	// version
	bool isbot;
	uint64 installID;

	if (msg->FindBool("bot", &isbot) == B_OK)
	{
		fBot = true;
		// is a bot... so no client, files, etc.
		fClient = "Bot";
		fUserStatus = "Bot";
		fBandwidthLabel = "Bot";
	}
	else 
		fBot = false;

	if (msg->FindInt64("installid", (int64 *)&installID) != B_OK)
		fInstallID = 0;
	else
		fInstallID = installID;

	if (msg->FindString("name", &name) == B_OK)
		fUserName = QString::fromUtf8(name);
	else
		fUserName = "???";

	if (msg->FindInt32("port", &port) == B_OK)
		fPort = port;
	else
		fPort = 0;

	if ((msg->FindString("version_name", &vname) == B_OK) && (msg->FindString("version_num", &vnum) == B_OK))
	{
		fClient = QString::fromUtf8(vname);
		fClient += " ";
		fClient += vnum;
		fNeedPing = false;
	}
	else
	{
		// not a *inShare client?
		// ping for a version
		if (fNeedPing)	// only if needed
			gWin->fNetClient->SendPing(fUserID);
	}

	msg->FindBool("supports_partial_hashing", &fPartial);   // NEW 11/7/2002 partial resumes

	// <postmaster@raasu.org> 20020213 -- Fix for download troubles when no files shared
	//
	bool b;

	if (msg->FindBool("firewalled", &b) == B_OK)
	{
		PRINT("WUser: %s firewalled", (b ? "Is" : "Isn't"));
		SetFirewalled(b);
	}

	//

	PRINT("WUser: %S is a %s with installid %d on port %d\n", qStringToWideChar(fUserName), (fBot ? "bot" : "user"), 
																fInstallID, fPort);
}

void
WUser::InitStatus(const Message * msg)
{
	const char * status;
	if (msg->FindString("userstatus", &status) == B_OK)
		fUserStatus = QString::fromUtf8(status);
	else
		fUserStatus = "?";
	PRINT("WUser: %S with status %S\n", qStringToWideChar(fUserName), qStringToWideChar(fUserStatus));
}

void
WUser::InitUploadStats(const Message * msg)
{
	int32 c, m;
	if (msg->FindInt32("cur", (int32 *)&c) == B_OK)
		fCurUploads = c;
	if (msg->FindInt32("max", (int32 *)&m) == B_OK)
		fMaxUploads = m;
	PRINT("WUser: %S with %d of %d uploads going\n", qStringToWideChar(fUserName), fCurUploads, fMaxUploads);
}

void
WUser::InitBandwidth(const Message * msg)
{
	const char * l;
	int32 bps;

	if (msg->FindString("label", &l) == B_OK)
		fBandwidthLabel = l;
	if (msg->FindInt32("bps", (int32 *)&bps) == B_OK)
		fBandwidthBPS = bps;
	PRINT("WUser: %S with label %S and bps %d\n", qStringToWideChar(fUserName), qStringToWideChar(fBandwidthLabel), fBandwidthBPS);
}

void
WUser::InitFileCount(const Message * msg)
{
	int32 fc;
	if (msg->FindInt32("filecount", &fc) == B_OK)
		fFileCount = fc;
	PRINT("WUser: %S with filecount %d\n", qStringToWideChar(fUserName), fFileCount); // <postmaster@raasu.org> 20021022 -- Fixed typo
}

void
WUser::AddToListView(QListView * view)
{
	WListIter it = fLists.find(view);

	QString qUpload;
	qUpload = QObject::tr("%1,%2").arg(fCurUploads).arg(fMaxUploads);
	//
	char strFileCount[10];
	sprintf(strFileCount,"%6lu",fFileCount);
	if (it == fLists.end())	// not found? create a new item
	{
		WListPair pair;
		QListViewItem *item;
		if (!fBot)
		{
			item = new WUserListItem(view, fUserName, fUserID, fUserStatus, QObject::tr("%1").arg(strFileCount), 
								fBandwidthLabel, qUpload, fClient);
			CHECK_PTR(item);
			pair = MakeListPair(view, item);
			((WUserListItem *)(pair.second))->SetFirewalled(fFirewalled);
		}
		else
		{
			item = new WBotItem(view, fUserName, fUserID, fUserStatus, QObject::tr("%1").arg(strFileCount), 
								fBandwidthLabel, qUpload, fClient);
			CHECK_PTR(item);
			pair = MakeListPair(view, item);
		}
		fLists.insert(pair);
	}
	else
	{
		QListViewItem * item = (*it).second;
		// if we've been created... then we should update our list item if needed
		if (item->text(WNickListItem::Name) != fUserName)
		{
			item->setText(WNickListItem::Name, fUserName);
		}

		if (item->text(WNickListItem::ID) != fUserID)
		{
			item->setText(WNickListItem::ID, fUserID);
		}

		if (item->text(WNickListItem::Status) != fUserStatus)
		{
			item->setText(WNickListItem::Status, fUserStatus);
		}

		QString chk = QObject::tr("%1").arg(strFileCount);

		if (item->text(WNickListItem::Files) != chk)
		{
			item->setText(WNickListItem::Files, chk);
		}

		if (item->text(WNickListItem::Connection) != fBandwidthLabel)
		{
			item->setText(WNickListItem::Connection, fBandwidthLabel);
		}
		
		if (item->text(WNickListItem::Load) != qUpload)
		{
			item->setText(WNickListItem::Load, qUpload);
		}

		if (item->text(WNickListItem::Client) != fClient)
		{
			item->setText(WNickListItem::Client, fClient);
		}
	}
}

void
WUser::RemoveFromListView(QListView * view)
{
	if (view)		// remove from the passed view only
	{
		WListIter it = fLists.find(view);
		if (it != fLists.end())
		{
			QListViewItem * item = (*it).second;
			delete item;
			item = NULL; // <postmaster@raasu.org> 20021027
			fLists.erase(it);
		}
	}
	else
	{
		// remove from all views
		WListIter it = fLists.begin();
		while (it != fLists.end())
		{
			PRINT("Deleting item\n");
			delete (*it).second;
			PRINT("Done\n");
			fLists.erase(it);
			it = fLists.begin();
		}
	}
}

void
WUser::PingResponse(const Message * msg)
{
	fNeedPing = false;
	fClient = WinShareWindow::GetRemoteVersionString(msg);

	// go through each item in the list and update the client
	for (WListIter it = fLists.begin(); it != fLists.end(); it++)
	{
		QListViewItem * item = (*it).second;
		if (item->text(WNickListItem::Client) != fClient)
		{
			item->setText(WNickListItem::Client, fClient);
		}
	}
}

void
WUser::SetFirewalled(bool f)
{
	fFirewalled = f;
	if (!fBot)
	{
		for (WListIter it = fLists.begin(); it != fLists.end(); it++)
			((WUserListItem *)((*it).second))->SetFirewalled(f);
	}
}

