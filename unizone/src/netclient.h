#ifndef NETCLIENT_H
#define NETCLIENT_H


#include "system/MessageTransceiverThread.h"
#include <qobject.h>
using namespace muscle;

#include "user.h"

class NetClient : public QObject, public MessageTransceiverThread
{
	Q_OBJECT
public:
	NetClient(QObject * owner);
	virtual ~NetClient();

	// <postmaster@raasu.org> -- Add support for port numbers
	status_t Connect(QString server, uint16 port);
	status_t Connect(QString server);
	void Disconnect();
	QString GetServer() { return fServer; } // Get current server hostname or ip address if hostname isn't available


	bool IsConnected() const { return IsInternalThreadRunning(); }

	void AddSubscription(QString str, bool q = false);	// if "q" is true, u won't get an initial response
	void RemoveSubscription(QString str);

	void SendChatText(QString target, QString text);
	void SendPing(QString target);

	void SetUserName(QString user);		// <postmaster@raasu.org> 20021001
	void SetUserStatus(QString status);	//
	void SetConnection(QString connection);
	void SetPort(int32 port) { fPort = port; }
	void SetFileCount(int32 count);
	void SetLoad(int32 num, int32 max);

	void SetNodeValue(const char * node, MessageRef & val);	// set the Message of a node

	// events
	enum
	{
		SignalEvent = QEvent::User + 4000,
		SessionAttached,	// just some constant
		SessionConnected,
		Disconnected,
		MessageReceived
	};

	// client messages
	enum 
	{
	  CONNECTED_TO_SERVER = 0,
	  DISCONNECTED_FROM_SERVER,
	  NEW_CHAT_TEXT,
	  CONNECT_BACK_REQUEST,
	  CHECK_FILE_COUNT,
	  PING,
	  PONG,
	  SCAN_THREAD_REPORT
	}; 

	// path matching
	enum 
	{
		ROOT_DEPTH = 0,         // root node
		HOST_NAME_DEPTH,         
		SESSION_ID_DEPTH,
		BESHARE_HOME_DEPTH,     // used to separate our stuff from other (non-BeShare) data on the same server
		USER_NAME_DEPTH,        // user's handle node would be found here
		FILE_INFO_DEPTH         // user's shared file list is here
	};

	QString LocalSessionID() const;

	//static void Lock() 
	//{ 
	//}
	//static void Unlock() 
	//{ 
	//}

	void HandleParameters(MessageRef & next);
	void HandleResultMessage(MessageRef & ref);

	bool ExistUser(QString sid);
	// this is idential to ExistUser() in that it will return
	// NULL (ExistUser() returns false) if the user is not found, but unlike
	// ExistUser(), it will return a pointer to the found user.
	WUserRef FindUser(QString sid);
	// Find users by IP address
	void FindUsersByIP(WUserMap & umap, QString ip);
	// create a new user
	WUserRef CreateUser(QString sessionID);
	// deletes a user, including removing from the list view
	void RemoveUser(QString sessionID);

	WUserMap & Users() { return fUsers; }

signals:
	void UserDisconnected(QString sid, QString name);
	void UserConnected(QString id);
	void UserNameChanged(QString id, QString oldname, QString newname);
	void DisconnectedFromServer();
	void UserStatusChanged(QString id, QString name, QString status);
	
	void RemoveFile(const QString, const QString);
	void AddFile(const QString, const QString, bool, MessageRef);

protected:
	virtual void SignalOwner();

private:
	int32 fPort;
	QString fSessionID, fServer;
	QObject * fOwner;
	WUserMap fUsers;	// a list of users

	// QMutex fNetLock;

};


#endif	// NETCLIENT_H

