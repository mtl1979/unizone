#ifndef NETCLIENT_H
#define NETCLIENT_H


// #include "system/MessageTransceiverThread.h"
#include "qtsupport/QMessageTransceiverThread.h"

#include "support/MuscleSupport.h"

using namespace muscle;

#include <qobject.h>

#include "user.h"
class NetClient : public QMessageTransceiverThread 
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
	QString GetServerIP();					// Get current server IP address
	uint32 GetServerPort() { return fServerPort; } // Get current server port number
	
	bool IsConnected() const;
	
	void AddSubscription(QString str, bool q = false);	// if "q" is true, u won't get an initial response
	void RemoveSubscription(QString str);
	
	void SendChatText(QString target, QString text);
	void SendPing(QString target);
	
	void SetUserName(QString user); 	// <postmaster@raasu.org> 20021001
	void SetUserStatus(QString status); //
	void SetConnection(QString connection);
	void SetPort(uint32 port) { fPort = port; }
	void SetFileCount(int32 count);
	void SetLoad(int32 num, int32 max);
	
	status_t SendMessageToSessions(MessageRef msgRef, const char * optDistPath = NULL);
	void SetNodeValue(const char * node, MessageRef & val); // set the Message of a node
	
	QString * GetChannelList();
	int GetChannelCount();
	
	QString * GetChannelUsers(QString channel);
	int GetUserCount(QString channel);
	
	// events
	enum
	{
		/* SignalEvent = QEvent::User + 4000, */
			SESSION_ATTACHED = QEvent::User + 4000,	// just some constant
			SESSION_CONNECTED,
			DISCONNECTED,
			MESSAGE_RECEIVED
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
	
	// path matching -- BeShare
	enum 
	{
		ROOT_DEPTH = 0, 		// root node
			HOST_NAME_DEPTH,		 
			SESSION_ID_DEPTH,
			BESHARE_HOME_DEPTH, 	// used to separate our stuff from other (non-BeShare) data on the same server
			USER_NAME_DEPTH,		// user's handle node would be found here
			FILE_INFO_DEPTH 		// user's shared file list is here
	};
	
	// path matching -- UniShare
	enum
	{
		UNISHARE_HOME_DEPTH = BESHARE_HOME_DEPTH,
			CHANNELDATA_DEPTH,
			CHANNEL_DEPTH,
			CHANNELINFO_DEPTH
	};
	
	// UniShare event codes
	enum
	{
		ClientConnected = 'UsCo',
			RegisterFail,
			ClientDisconnected,
			ChannelCreated,
			ChannelText,
			ChannelData,
			ChannelInvite,
			ChannelKick,
			ChannelMaster,
			ChannelJoin,
			ChannelPart,
			ChannelSetTopic,
			ChannelSetPublic
	};
	
	QString LocalSessionID() const;
	
	
	void HandleParameters(MessageRef & next);
	void HandleResultMessage(MessageRef & ref);
	
	bool ExistUser(QString sid);
	// this is idential to ExistUser() in that it will return
	// NULL (ExistUser() returns false) if the user is not found, but unlike
	// ExistUser(), it will return a pointer to the found user.
	WUserRef FindUser(QString sid);
	// Find users by IP address
	void FindUsersByIP(WUserMap & umap, QString ip);
	WUserRef FindUserByIPandPort(QString ip, uint32 port);
	// create a new user
	WUserRef CreateUser(QString sessionID);
	// deletes a user, including removing from the list view
	void RemoveUser(QString sessionID);
	
	WUserMap & Users() { return fUsers; }
	
	// forwarders

	void Reset();
	status_t SetOutgoingMessageEncoding(int32 encoding, const char * optDistPath = NULL);
	status_t WaitForInternalThreadToExit();

signals:
	void UserDisconnected(const QString &, const QString &);
	void UserConnected(const QString &);
	void UserNameChanged(const QString &, const QString &, const QString &);
	void DisconnectedFromServer();
	void UserStatusChanged(const QString &, const QString &, const QString &);
	void UserIDChanged(const QString &, const QString &);
	void UserHostName(const QString &, const QString &);
	
	void RemoveFile(const QString &, const QString &);
	void AddFile(const QString &, const QString &, bool, MessageRef);
	
	void ChannelTopic(const QString &, const QString &, const QString &);
	void ChannelAdmins(const QString &, const QString &, const QString &);
	void ChannelAdded(const QString &, const QString &, int64);
	void ChannelPublic(const QString &, const QString &, bool);
	void ChannelOwner(const QString &, const QString &, const QString &);
	
private:
	uint32 fPort, fServerPort;
	QString fSessionID, fServer;
	QString fOldID; 		// Old id for persistent channel admin/owner state
	QString fUserName;
	QObject * fOwner;
	WUserMap fUsers;		// a list of users
	MessageRef fChannels;	// channel database
	// QMessageTransceiverThread *qmtt;
	
	void HandleBeRemoveMessage(String nodePath);
	void HandleBeAddMessage(String nodePath, MessageRef ref);
	
	void HandleUniRemoveMessage(String nodePath);
	void HandleUniAddMessage(String nodePath, MessageRef ref);
	
	void AddChannel(QString sid, QString channel);
	void RemoveChannel(QString sid, QString channel);

	void SendSignal(int signal);
	
	mutable QMutex fChannelLock;

	int timerID;

protected:
	virtual void timerEvent(QTimerEvent *);

	// --------------------------------------------------------------------------------------------

	void BeginMessageBatch();
	void MessageReceived(MessageRef msg, const String & sessionID);
	void EndMessageBatch();

	void SessionAttached(const String & sessionID);
	void SessionDetached(const String & sessionID);
   
	void SessionAccepted(const String & sessionID, uint16 port);

	void SessionConnected(const String & sessionID);
	void SessionDisconnected(const String & sessionID);

	void FactoryAttached(uint16 port);
	void FactoryDetached(uint16 port);

	void ServerExited();

	void OutputQueuesDrained(MessageRef ref);

};


#endif	// NETCLIENT_H

