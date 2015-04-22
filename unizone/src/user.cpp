#include "user.h"

#include <qapplication.h>
#include <q3listview.h>
#include <qstring.h>

#include "regex/PathMatcher.h"

#include "netclient.h"
#include "debugimpl.h"
#include "global.h"
#include "botitem.h"
#include "userlistitem.h"
#include "listutil.h"
#include "messageutil.h"
#include "util.h"
#include "wstring.h"
#include "winsharewindow.h"

WUser::WUser(const QString & sid)
{
	fUserName = tr("Unknown");
	fUserStatus = "?";
	fHostName = "?";
	fHostOS = QString::null;
	fUserID = sid;
	fCurUploads = fMaxUploads = 0;
	fBandwidthLabel = qApp->translate("Connection", "Unknown");
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

bool
WUser::CheckName(const QString &name)
{
	QString temp = name.stripWhiteSpace();
	if (temp.isEmpty())
		return false;
	if (temp == qApp->translate("WUser", "Unknown")) // static methods can't use tr()
		return false;
	if (temp == "Unknown")
		return false;
	return true;
}

void
WUser::InitName(MessageRef msg)
{
	QString vname, vnum;	// version
	bool isbot;
	uint64 installID;

        if (msg()->FindBool("bot", isbot) == B_OK)
	{
		fBot = true;
		// is a bot... so no client, files, etc.
		fClient = "Bot";
		fUserStatus = "Bot";
		fBandwidthLabel = "Bot";
	}
	else
		fBot = false;

        if (msg()->FindInt64("installid", installID) != B_OK)
		fInstallID = 0;
	else
		fInstallID = installID;

	GetStringFromMessage(msg, "name", fUserName);

	if (GetUInt32FromMessage(msg, "port", fPort) != B_OK)
		fPort = 0;

	if ((GetStringFromMessage(msg, "version_name", vname) == B_OK) &&
		(GetStringFromMessage(msg, "version_num", vnum) == B_OK))
	{
		QString vt(vname);
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

        msg()->FindBool("supports_partial_hashing", fPartial);			// NEW 11/7/2002 partial resumes
        msg()->FindBool("supports_transfer_tunneling", fTunneling);	// NEW 8th Aug 2004

	// <postmaster@raasu.org> 20020213 -- Fix for download troubles when no files shared
	//
	bool b = false;

        if (msg()->FindBool("firewalled", b) == B_OK)
	{
		PRINT("WUser: %s firewalled\n", (b ? "Is" : "Isn't"));
	}

	if (!b && (fPort > 32767))
	{
		PRINT("WUser: invalid port (%lu), assuming firewalled!\n", fPort);
		SetFirewalled(true);
	}
	else
		SetFirewalled(b);

	//

	GetStringFromMessage(msg, "host_os", fHostOS);

#ifdef _DEBUG
	WString wuser(fUserName);
	PRINT("WUser: %S is a %s with installid " UINT64_FORMAT_SPEC,
		wuser.getBuffer(), (fBot ? "bot" : "user"), fInstallID);
	if (GetFirewalled())
		PRINT("\n");
	else
		PRINT(" on port %lu\n", fPort);
#endif
}

void
WUser::InitStatus(MessageRef msg)
{
	const char * status;
        if (msg()->FindString("userstatus", status) == B_OK)
		fUserStatus = QString::fromUtf8(status);

#ifdef _DEBUG
	WString wuser(fUserName);
	WString wstatus(fUserStatus);
	PRINT("WUser: %S with status %S\n", wuser.getBuffer(), wstatus.getBuffer());
#endif
}

void
WUser::InitUploadStats(MessageRef msg)
{
	GetUInt32FromMessage(msg, "cur", fCurUploads);
	GetUInt32FromMessage(msg, "max", fMaxUploads);

#ifdef _DEBUG
	WString wuser(fUserName);
	PRINT("WUser: %S with %d of %d uploads going\n", wuser.getBuffer(), fCurUploads, fMaxUploads);
#endif
}

void
WUser::InitBandwidth(MessageRef msg)
{
	const char * l;
	fBandwidthBPS = 0;

	GetUInt32FromMessage(msg, "bps", fBandwidthBPS);

	if (fBandwidthBPS != 0)
		fBandwidthLabel = BandwidthToString(fBandwidthBPS);

	if (fBandwidthLabel == "Unknown")
		fBandwidthLabel == qApp->translate("Connection", "Unknown");

	if (fBandwidthLabel == qApp->translate("Connection", "Unknown"))
	{
		if (msg()->FindString("label", l) == B_OK)
		{
			if ((strcmp(l, "?") != 0) && (strcmp(l, "Unknown") != 0))
				fBandwidthBPS = BandwidthToBytes(l);

			if (fBandwidthBPS > 0)
			{
				fBandwidthLabel = qApp->translate("Connection", l);
				AddToList(fBandwidthLabel, QString::number(fBandwidthBPS));
			}
		}
	}
#ifdef _DEBUG
	WString wuser(fUserName);
	WString wlabel(fBandwidthLabel);
	PRINT("WUser: %S with label %S and bps %d\n", wuser.getBuffer(), wlabel.getBuffer(), fBandwidthBPS);
#endif
}

void
WUser::InitFileCount(MessageRef msg)
{
	GetInt32FromMessage(msg, "filecount", fFileCount);

#ifdef _DEBUG
	WString wuser(fUserName);
	PRINT("WUser: %S with filecount %d\n", wuser.getBuffer(), fFileCount); // <postmaster@raasu.org> 20021022 -- Fixed typo
#endif
}

void
WUser::AddToListView(Q3ListView * view)
{
	Q3ListViewItem * item;

	QString qUpload;
	AddToList(qUpload, QString::number(fCurUploads));
	AddToList(qUpload, QString::number(fMaxUploads));
	//
	QString strFileCount = QString::number(fFileCount).rightJustify(6);
	if (fLists.GetValue(view, item) == B_OK)
	{
		// if we've been created... then we should update our list item if needed
		if (!IsBot())
		{
			WUserListItem *uli = dynamic_cast<WUserListItem *>(item);
			if (uli && uli->Firewalled() != fFirewalled)
				uli->SetFirewalled(fFirewalled);
		}

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

		if (item->text(WNickListItem::Files) != strFileCount)
		{
			item->setText(WNickListItem::Files, strFileCount);
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
	else
	{
		// not found? create a new item
		if (fBot)
		{
			item = new WBotItem(view, fUserName, fUserID, fUserStatus, strFileCount,
								fBandwidthLabel, qUpload, fClient, fHostOS);
			Q_CHECK_PTR(item);
		}
		else
		{
			item = new WUserListItem(view, fUserName, fUserID, fUserStatus, strFileCount,
								fBandwidthLabel, qUpload, fClient, fHostOS);
			Q_CHECK_PTR(item);
			((WUserListItem *)(item))->SetFirewalled(fFirewalled);
		}
		fLists.Put(view,item);
	}
}

void
WUser::UpdateListViews()
{
	HashtableIterator<Q3ListView *, Q3ListViewItem *> iter = fLists.GetIterator(HTIT_FLAG_NOREGISTER);
	while (iter.HasData())
	{
		Q3ListView *view = iter.GetKey();
		AddToListView(view);
		iter++;
	}
}

void
WUser::RemoveFromListView(Q3ListView * view)
{
	Q3ListViewItem * item;
	if (view)		// remove from the passed view only
	{
		if (fLists.GetValue(view, item) == B_OK)
		{
			delete item;
			item = NULL; // <postmaster@raasu.org> 20021027
			fLists.Remove(view);
		}
	}
	else
	{
		// remove from all views
		HashtableIterator<Q3ListView *, Q3ListViewItem *> iter = fLists.GetIterator(HTIT_FLAG_BACKWARDS);
		while (iter.HasData())
		{
			item = iter.GetValue();
			PRINT("Deleting item\n");
			delete item;
			PRINT("Done\n");
			fLists.Remove(view);
			iter++;
		}
	}
}

void
WUser::SetFirewalled(bool f)
{
	if (!fBot)
	{
		fFirewalled = f;
	}
}

void
WUser::SetUserName(const QString & name)
{
	fUserName = name;
}

void
WUser::SetStatus(const QString & s)
{
	fUserStatus = s;
}

void
WUser::SetUserHostName(const QString & h)
{
	fHostName = h;
}

void
WUser::SetCurUploads(uint32 c)
{
	fCurUploads = c;
}

void
WUser::SetMaxUploads(uint32 m)
{
	fMaxUploads = m;
}

void
WUser::SetBandwidthLabel(const char * s)
{
	if (strcmp(s, "?") == 0)
		fBandwidthLabel = qApp->translate("Connection", "Unknown");
	else
		fBandwidthLabel = qApp->translate("Connection", s);
}

void
WUser::SetBandwidthBPS(uint32 bps)
{
	fBandwidthBPS = bps;
}

void
WUser::SetFileCount(int32 fc)
{
	fFileCount = fc;
}

QString
WUser::GetLastLine(const QString & channel) const
{
	for (unsigned int n = 0; n < fLastLines.GetNumItems(); n++)
	{
		WLastLines sp = fLastLines[n];
		if (sp.channel == channel)
			return sp.line;
	}
	return QString::null;
}

void
WUser::SetLastLine(const QString & channel, const QString &line)
{
	WLastLines sp;

	// Replace old entry if exists

	for (unsigned int n = 0; n < fLastLines.GetNumItems(); n++)
	{
		sp = fLastLines[n];
		if (sp.channel == channel)
		{
			sp.line = line;
			fLastLines.ReplaceItemAt(n, sp);
			return;
		}
	}

	// No entry yet?

	sp.channel = channel;
	sp.line = line;
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

struct OSPair
{
	const char *id;
	const char *tag;
};

OSPair Systems[] = {
	// Known clients
	{"BeShare", QT_TRANSLATE_NOOP("WUser", "BeOS")},
	{"WinShare", QT_TRANSLATE_NOOP("WUser", "Windows")},
	{"LinShare", QT_TRANSLATE_NOOP("WUser", "Linux")},
	// Known OS tags used by MUSCLE and JavaShare
	{"Windows", QT_TRANSLATE_NOOP("WUser", "Windows")},
	{"Linux", QT_TRANSLATE_NOOP("WUser", "Linux")},
	{"FreeBSD", QT_TRANSLATE_NOOP("WUser", "FreeBSD")},
	{"OpenBSD", QT_TRANSLATE_NOOP("WUser", "OpenBSD")},
	{"NetBSD", QT_TRANSLATE_NOOP("WUser", "NetBSD")},
	{"BeOS", QT_TRANSLATE_NOOP("WUser", "BeOS")},
	{"MacOS", QT_TRANSLATE_NOOP("WUser", "Mac OS")},
	{"Mac OS", QT_TRANSLATE_NOOP("WUser", "Mac OS")},
	{"QNX", QT_TRANSLATE_NOOP("WUser", "QNX")},
	{"OS/2", QT_TRANSLATE_NOOP("WUser", "OS/2")},
	{"SunOS", QT_TRANSLATE_NOOP("WUser", "SunOS")},
	{"AtheOS", QT_TRANSLATE_NOOP("WUser", "AtheOS")},
	{"Tru64", QT_TRANSLATE_NOOP("WUser", "Tru64")},
	{"Irix", QT_TRANSLATE_NOOP("WUser", "Irix")},
	{"AIX", QT_TRANSLATE_NOOP("WUser", "AIX")},
	{"Sequent", QT_TRANSLATE_NOOP("WUser", "Sequent")},
	{"OpenServer", QT_TRANSLATE_NOOP("WUser", "OpenServer")},
	{"HPUX", QT_TRANSLATE_NOOP("WUser", "HPUX")},
	{"Solaris", QT_TRANSLATE_NOOP("WUser", "Solaris")},
	{"UnixWare", QT_TRANSLATE_NOOP("WUser", "UnixWare")},
	// End of list marker ;)
	{NULL, NULL}
};

void
WUser::SetClient(const QString &s)
{
	if (fNeedPing)
		fNeedPing = false;

	fClient = s;
	OSPair p;

	if (s.contains("/Java")) // BeShare 1.xx/Java isn't (always) BeOS
		return;

	for (unsigned int n = 0; (p = Systems[n]).id != NULL; n++)
	{
		int pos;
		if ((pos = s.find(p.id, 0, false)) >= 0)
		{
			if (fHostOS == QString::null)
			{
				fHostOS = tr(p.tag);
			}
			// try to strip host os from client string
			if ((pos > 0) && (fClient[pos-1] == '('))
			{
				fClient.truncate(pos - 1);
				fClient = fClient.stripWhiteSpace();
			}
			break;
		}
	}
}


