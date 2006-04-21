#ifndef SERVERCLIENT
#define SERVERCLIENT

#include <qapplication.h>
#include <qobject.h>

#include "util/String.h"
#include "message/Message.h"
#include "qtsupport/QMessageTransceiverThread.h"
#include "reflector/AbstractReflectSession.h"

using namespace muscle;

class ServerClient : public QObject
{
	Q_OBJECT
public:
	ServerClient(QObject *owner);
	~ServerClient();

	void Disconnect();

	// forwarders

	status_t StartInternalThread(); 
	status_t AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef);
	void Reset();

private:

	QMessageTransceiverThread *qmtt;
	String fHostName;
	uint32 fHostPort;

private slots:

	void MessageReceived(const MessageRef & msg, const String & sessionID);

	void SessionConnected(const String & sessionID);
	void SessionDetached(const String & sessionID);
};

#endif
