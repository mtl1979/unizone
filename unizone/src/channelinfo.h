#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include <qstring.h>
#include <qlistview.h>

#include "channelimpl.h"

#include "util/Queue.h"
using muscle::Queue;

typedef Queue<QString> WAdminList;
typedef Queue<QString> WUserList;

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

	bool SetAdmins(QString);
	int NumAdmins() 
	{ 
		return fAdmins.GetNumItems(); 
	}

	void AddAdmin(QString);
	void RemoveAdmin(QString);
	QString GetAdmins();
	bool IsAdmin(QString);

	void AddUser(QString);
	void RemoveUser(QString);
	QString GetUsers();
	int NumUsers(); 

	void SetTopic(QString t); 
	QString GetTopic(); 
	
	void SetPublic(bool p); 
	bool GetPublic(); 
	
	void SetItem(QListViewItem * item); 
	QListViewItem * GetItem(); 
	
	void SetWindow(Channel * win); 
	Channel * GetWindow(); 

	void SetCreated(int64 i);
	int64 GetCreated();

	void SetOwner(QString owner);
	QString GetOwner();
	bool IsOwner(QString);

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