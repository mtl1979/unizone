#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include <qstring.h>
#include <qlistview.h>

#include "util/Queue.h"

using namespace muscle;

typedef Queue<QString> WAdminList;
typedef Queue<QString> WUserList;

class Channel;

class ChannelInfo
{
public:
	ChannelInfo(QString name, QString owner)
	{
		fName = name;
		fOwner = owner;
		fTopic = QString::null;
		fStrAdmins = QString::null;
		fPublic = true;
		fItem = NULL;
		fWindow = NULL;
	}

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
	QString GetUsers() const;
	int NumUsers() const; 

	void SetTopic(const QString & t); 
	QString GetTopic() const; 
	
	void SetPublic(bool p); 
	bool GetPublic() const; 
	
	void SetItem(QListViewItem * item); 
	QListViewItem * GetItem() const; 
	
	void SetWindow(Channel * win); 
	Channel * GetWindow() const; 

	void SetCreated(int64 i);
	int64 GetCreated() const;

	void SetOwner(const QString & owner);
	QString GetOwner() const;
	bool IsOwner(const QString &) const;

private:
	WAdminList fAdmins;
	WUserList fUsers;
	QString fOwner, fName;
	QListViewItem * fItem;
	Channel * fWindow;
	QString fTopic;
	QString fStrAdmins;
	bool fPublic;
	int64 fCreated;
};

#endif
