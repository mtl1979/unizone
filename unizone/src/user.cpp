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

	PRINT("WUser: %s is a %s with installid %d on port %d\n", fUserName.latin1(), (fBot ? "bot" : "user"), 
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
	PRINT("WUser: %s with status %s\n", fUserName.latin1(), fUserStatus.latin1());
}

void
WUser::InitUploadStats(const Message * msg)
{
	int32 c, m;
	if (msg->FindInt32("cur", (int32 *)&c) == B_OK)
		fCurUploads = c;
	if (msg->FindInt32("max", (int32 *)&m) == B_OK)
		fMaxUploads = m;
	PRINT("WUser: %s with %d of %d uploads going\n", fUserName.latin1(), fCurUploads, fMaxUploads);
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
	PRINT("WUser: %s with label %s and bps %d\n", fUserName.latin1(), fBandwidthLabel.latin1(), fBandwidthBPS);
}

void
WUser::InitFileCount(const Message * msg)
{
	int32 fc;
	if (msg->FindInt32("filecount", &fc) == B_OK)
		fFileCount = fc;
	PRINT("WUser: %s with filecount %d\n", fUserName.latin1(), fFileCount); // <postmaster@raasu.org> 20021022 -- Fixed typo
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
		if (!fBot)
		{
			pair = MakeListPair(view, new WUserListItem(view, fUserName, fUserID, fUserStatus, QObject::tr("%1").arg(strFileCount), 
								fBandwidthLabel, qUpload, fClient));
			((WUserListItem *)(pair.second))->SetFirewalled(fFirewalled);
		}
		else
		{
			pair = MakeListPair(view, new WBotItem(view, fUserName, fUserID, fUserStatus, QObject::tr("%1").arg(strFileCount), 
								fBandwidthLabel, qUpload, fClient));
		}
		fLists.insert(pair);
	}
	else
	{
		QListViewItem * item = (*it).second;
		// if we've been created... then we should update our list item if needed
		if (item->text(0) != fUserName)
			item->setText(0, fUserName);
		if (item->text(1) != fUserID)
		{
			item->setText(1, fUserID);
		}
		if (item->text(2) != fUserStatus)
			item->setText(2, fUserStatus);

		QString chk = QObject::tr("%1").arg(strFileCount);

		if (item->text(3) != chk)
			item->setText(3, chk);
		if (item->text(4) != fBandwidthLabel)
			item->setText(4, fBandwidthLabel);
		
		chk = qUpload;
		if (item->text(5) != chk)
			item->setText(5, chk);
		if (item->text(6) != fClient)
			item->setText(6, fClient);
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
		if (item->text(6) != fClient)
			item->setText(6, fClient);
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

