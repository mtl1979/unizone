#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include <q3listview.h>

#include "util/Queue.h"

using namespace muscle;

class QString;

typedef Queue<QString> WAdminList;
typedef Queue<QString> WUserList;

class Channel;
class Q3ListViewItem;

class ChannelInfo
{
public:
	ChannelInfo(const QString &name, const QString &owner);
	~ChannelInfo();

	bool SetAdmins(const QString &);
	int NumAdmins() 
	{ 
		return fAdmins.GetNumItems(); 
	}

	void AddAdmin(const QString &);
	void RemoveAdmin(const QString &);
	QString GetAdmins() const ;
	bool IsAdmin(const QString &) const;

	void AddUser(const QString &);
	void RemoveUser(const QString &);
	bool FindUser(const QString &);
	QString GetUsers() const;
	int NumUsers() const; 

	void SetTopic(const QString & t); 
	QString GetTopic() const; 
	
	void SetPublic(bool p); 
	bool GetPublic() const; 
	
	void SetItem(Q3ListViewItem * item); 
	Q3ListViewItem * GetItem() const; 
	
	void SetWindow(Channel * win); 
	Channel * GetWindow() const; 

	void SetCreated(uint64 i);
	uint64 GetCreated() const;

	void SetOwner(const QString & owner);
	QString GetOwner() const;
	bool IsOwner(const QString &) const;

private:
	WAdminList fAdmins;
	WUserList fUsers;
	QString fOwner, fName;
	Q3ListViewItem * fItem;
	Channel * fWindow;
	QString fTopic;
	QString fStrAdmins;
	bool fPublic;
	uint64 fCreated;
};

#endif
