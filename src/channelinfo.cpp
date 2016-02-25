#include "channelinfo.h"
#include "channelimpl.h"
#include "listutil.h"
#include "tokenizer.h"

ChannelInfo::ChannelInfo(const QString &name, const QString &owner)
{
	fName = name;
	fOwner = owner;
	fTopic = QString::null;
	fStrAdmins = QString::null;
	fPublic = true;
	fItem = NULL;
	fWindow = NULL;
}


bool
ChannelInfo::IsOwner(const QString & sid) const
{
	if (sid == fOwner)
		return true;
	else
		return false;
}

bool
ChannelInfo::IsAdmin(const QString & sid) const
{
	if (fAdmins.IsEmpty())		// Allow initial set-up
		return true;
	if (fAdmins.IndexOf(sid) != -1)
		return true;
	else
		return false;
}

bool
ChannelInfo::SetAdmins(const QString & admins)
{
	if (fStrAdmins == admins)
		return false;

	QStringTokenizer tok(admins, ",");
	QString next;
	fAdmins.Clear();
	while ((next = tok.GetNextToken()) != QString::null)
	{
		if ( fAdmins.IndexOf(next) == -1 )
		{
			fAdmins.AddTail(next);
		}
	}
	fStrAdmins = admins;
	return true;
}

ChannelInfo::~ChannelInfo()
{
	// Clean-up
	if (fWindow)
	{
		fWindow = NULL;
	}

	if (fItem)
	{
		delete fItem;
		fItem = NULL;
	}
}

void
ChannelInfo::AddUser(const QString & user)
{
	if ( fUsers.IndexOf(user) == -1 )
	{
		fUsers.AddTail(user);
		if (fWindow)
		{
			fWindow->AddUser(user);
		}
	}
}

void
ChannelInfo::RemoveUser(const QString & user)
{
	int i = fUsers.IndexOf(user);
	if (i != -1)
	{
		fUsers.RemoveItemAt(i);
		if (fWindow)
		{
			fWindow->RemUser(user);
		}
	}
}

bool
ChannelInfo::FindUser(const QString & user)
{
	return fUsers.IndexOf(user) != -1;
}

void
ChannelInfo::AddAdmin(const QString & user)
{
	if ( fAdmins.IndexOf(user) == -1 )
	{
		fAdmins.AddTail(user);
	}
	fStrAdmins = GetAdmins();
}

void
ChannelInfo::RemoveAdmin(const QString & user)
{
	int i = fAdmins.IndexOf(user);
	if (i != -1)
	{
		fAdmins.RemoveItemAt(i);
	}
	fStrAdmins = GetAdmins();
}

QString
ChannelInfo::GetAdmins() const
{
	QString adm;
	int n = fAdmins.GetNumItems();
	if (n > 0)
	{
		int i = 0;
		while (i < n)
		{
			AddToList(adm, fAdmins[i]);
			i++;
		}
	}
	return adm;
}

QString
ChannelInfo::GetUsers() const
{
	QString usr;
	int n = fUsers.GetNumItems();
	if (n > 0)
	{
		int i = 0;
		while (i < n)
		{
			AddToList(usr, fUsers[i]);
			i++;
		}
	}
	return usr;
}

int
ChannelInfo::NumUsers() const
{
	return fUsers.GetNumItems();
}

void
ChannelInfo::SetTopic(const QString & t)
{
	fTopic = t;
}

QString
ChannelInfo::GetTopic() const
{
	return fTopic;
}

void
ChannelInfo::SetPublic(bool p)
{
	fPublic = p;
}

bool
ChannelInfo::GetPublic() const
{
	return fPublic;
}

void
ChannelInfo::SetItem(Q3ListViewItem * item)
{
	fItem = item;
}

Q3ListViewItem *
ChannelInfo::GetItem() const
{
	return fItem;
}

void
ChannelInfo::SetWindow(Channel * win)
{
	fWindow = win;
}

Channel *
ChannelInfo::GetWindow() const
{
	return fWindow;
}

void
ChannelInfo::SetCreated(uint64 i)
{
	fCreated = i;
}

uint64
ChannelInfo::GetCreated() const
{
	return fCreated;
}

void
ChannelInfo::SetOwner(const QString & owner)
{
	fOwner = owner;
}

QString
ChannelInfo::GetOwner() const
{
	return fOwner;
}
