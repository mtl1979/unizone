#include "user.h"
#include "netclient.h"
#include "debugimpl.h"
#include "global.h"
#include "botitem.h"
#include "userlistitem.h"
#include "regex/PathMatcher.h"
#include "util.h"
#include "wstring.h"
#include "winsharewindow.h"

#include <qlistview.h>
#include <qstring.h>


WUser::WUser(const QString & sid)
{
	fUserName = "?";
	fUserStatus = "?";
	fHostName = "?";
	fHostOS = QString::null;
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
	fLastLine = QString::null;
}

WUser::~WUser()
{
	PRINT("~WUser()\n");
	RemoveFromListView();
}

void
WUser::InitName(const MessageRef msg)
{
	uint32 port;
	const char * name;
	const char * vname, * vnum;	// version
	const char * hostos;
	bool isbot;
	uint64 installID;

	if (msg()->FindBool("bot", &isbot) == B_OK)
	{
		fBot = true;
		// is a bot... so no client, files, etc.
		fClient = "Bot";
		fUserStatus = "Bot";
		fBandwidthLabel = "Bot";
	}
	else 
		fBot = false;

	if (msg()->FindInt64("installid", (int64 *)&installID) != B_OK)
		fInstallID = 0;
	else
		fInstallID = installID;

	if (msg()->FindString("name", &name) == B_OK)
		fUserName = QString::fromUtf8(name);
	else
		fUserName = "?";

	if (msg()->FindInt32("port", (int32 *) &port) == B_OK)
		fPort = port;
	else
		fPort = 0;

	if ((msg()->FindString("version_name", &vname) == B_OK) && (msg()->FindString("version_num", &vnum) == B_OK))
	{
		String vt(vname);
		vt += " ";
		vt += vnum;
		SetClient(vt);
		fNeedPing = false;
	}
	else
	{
		// not a *inShare client?
		// ping for a version
		if (fNeedPing)	// only if needed
			gWin->fNetClient->SendPing(fUserID);
	}

	msg()->FindBool("supports_partial_hashing", &fPartial);			// NEW 11/7/2002 partial resumes
	msg()->FindBool("supports_transfer_tunneling", &fTunneling);	// NEW 8th Aug 2004 

	// <postmaster@raasu.org> 20020213 -- Fix for download troubles when no files shared
	//
	bool b;

	if (msg()->FindBool("firewalled", &b) == B_OK)
	{
		PRINT("WUser: %s firewalled\n", (b ? "Is" : "Isn't"));
		SetFirewalled(b);
	}
	else if (port > 32767)
	{
		PRINT("WUser: invalid port (%lu), assuming firewalled!\n", port);
		SetFirewalled(true);
	}

	//

#ifdef _DEBUG
	WString wUser(fUserName);
	if (GetFirewalled())
		PRINT("WUser: %S is a %s with installid " UINT64_FORMAT_SPEC "\n",
			wUser.getBuffer(), (fBot ? "bot" : "user"), fInstallID);
	else
		PRINT("WUser: %S is a %s with installid " UINT64_FORMAT_SPEC " on port %lu\n",
			wUser.getBuffer(), (fBot ? "bot" : "user"), fInstallID, fPort);
#endif

	if (msg()->FindString("host_os", &hostos) == B_OK)
		fHostOS = QString::fromUtf8(hostos);
}

void
WUser::InitStatus(const MessageRef msg)
{
	const char * status;
	if (msg()->FindString("userstatus", &status) == B_OK)
		fUserStatus = QString::fromUtf8(status);
	else
		fUserStatus = "?";
#ifdef _DEBUG
	WString wUser(fUserName);
	WString wStatus(fUserStatus);
	PRINT("WUser: %S with status %S\n", wUser.getBuffer(), wStatus.getBuffer());
#endif
}

void
WUser::InitUploadStats(const MessageRef msg)
{
	int32 c, m;
	if (msg()->FindInt32("cur", (int32 *)&c) == B_OK)
		fCurUploads = c;
	if (msg()->FindInt32("max", (int32 *)&m) == B_OK)
		fMaxUploads = m;
#ifdef _DEBUG
	WString wUser(fUserName);
	PRINT("WUser: %S with %d of %d uploads going\n", wUser.getBuffer(), fCurUploads, fMaxUploads);
#endif
}

void
WUser::InitBandwidth(const MessageRef msg)
{
	const char * l;
	uint32 bps = 0;

	fBandwidthLabel = "Unknown"; // Reset

	if (msg()->FindInt32("bps", (int32 *)&bps) == B_OK)
		fBandwidthBPS = bps;

	if (bps != 0)
		fBandwidthLabel = BandwidthToString(bps);

	if (fBandwidthLabel == "Unknown")
	{
		if (msg()->FindString("label", &l) == B_OK)
		{
			fBandwidthLabel = QString::fromUtf8(l);
			fBandwidthLabel.append(",");
			fBandwidthLabel.append(QString::number(bps));
		}
	}
#ifdef _DEBUG
	WString wUser(fUserName);
	WString wBandwidth(fBandwidthLabel);
	PRINT("WUser: %S with label %S and bps %d\n", wUser.getBuffer(), wBandwidth.getBuffer(), fBandwidthBPS);
#endif
}

void
WUser::InitFileCount(const MessageRef msg)
{
	int32 fc;
	if (msg()->FindInt32("filecount", &fc) == B_OK)
		fFileCount = fc;
#ifdef _DEBUG
	WString wUser(fUserName);
	PRINT("WUser: %S with filecount %d\n", wUser.getBuffer(), fFileCount); // <postmaster@raasu.org> 20021022 -- Fixed typo
#endif
}

void
WUser::AddToListView(QListView * view)
{
	WListIter it = fLists.find(view);

	QString qUpload;
	qUpload = QString::number(fCurUploads);
	qUpload += ",";
	qUpload += QString::number(fMaxUploads);
	//
	char strFileCount[10];
	sprintf(strFileCount,"%6lu",fFileCount);
	if (it == fLists.end())	// not found? create a new item
	{
		WListPair pair;
		QListViewItem *item;
		if (!fBot)
		{
			item = new WUserListItem(view, fUserName, fUserID, fUserStatus, QString::fromLatin1(strFileCount), 
								fBandwidthLabel, qUpload, fClient, fHostOS);
			CHECK_PTR(item);
			pair = MakeListPair(view, item);
			((WUserListItem *)(pair.second))->SetFirewalled(fFirewalled);
		}
		else
		{
			item = new WBotItem(view, fUserName, fUserID, fUserStatus, QString::fromLatin1(strFileCount), 
								fBandwidthLabel, qUpload, fClient, fHostOS);
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

		QString chk = QString::fromLatin1(strFileCount);

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

		if (item->text(WNickListItem::HostOS) != fHostOS)
		{
			item->setText(WNickListItem::HostOS, fHostOS);
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
WUser::PingResponse(const MessageRef msg)
{
	fNeedPing = false;
	SetClient(WinShareWindow::GetRemoteVersionString(msg));

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

QString
WUser::GetLastLine(const QString & channel) const
{
	for (unsigned int n = 0; n < fLastLines.GetNumItems(); n++)
	{
		QStringPair sp = fLastLines[n];
		if (sp.first == channel)
			return sp.second;
	}
	return QString::null;
}

void
WUser::SetLastLine(const QString & channel, const QString &line)
{
	QStringPair sp;

	// Replace old entry if exists

	for (unsigned int n = 0; n < fLastLines.GetNumItems(); n++)
	{
		sp = fLastLines[n];
		if (sp.first == channel)
		{
			sp.second = line;
			fLastLines.ReplaceItemAt(n, sp);
			return;
		}
	}
	
	// No entry yet?

	sp.first = channel;
	sp.second = line;
	fLastLines.AddTail(sp);
}

void
WUser::SetClient(const char *c)
{ 
	SetClient(QString::fromUtf8(c));
}

void
WUser::SetClient(const String &s)
{
	SetClient(QString::fromUtf8(s.Cstr()));
}

void
WUser::SetClient(const QString &s)
{
	fClient = s;
	if (fHostOS == QString::null)
	{
		if (s.contains("Windows", false) > 0)
			fHostOS = QObject::tr("Windows");
		else if (s.contains("Linux", false) > 0)
			fHostOS = QObject::tr("Linux");
		else if (s.contains("FreeBSD", false) > 0)
			fHostOS = QObject::tr("FreeBSD");
		else if (s.contains("MacOS", false) > 0)
			fHostOS = QObject::tr("Mac OS");
		else if (s.contains("Mac OS", false) > 0)
			fHostOS = QObject::tr("Mac OS");
		else if (s.contains("QNX", false) > 0)
			fHostOS = QObject::tr("QNX");
		else if (s.contains("BeShare", false) > 0)
			fHostOS = QObject::tr("BeOS");
		else if (s.contains("WinShare", false) > 0)
			fHostOS = QObject::tr("Windows");
		else if (s.contains("LinShare", false) > 0)
			fHostOS = QObject::tr("Linux");
		else if (s.contains("OS/2", false) > 0)
			fHostOS = QObject::tr("OS/2");
	}
}
