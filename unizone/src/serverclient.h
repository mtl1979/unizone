#ifndef SERVERCLIENT
#define SERVERCLIENT

#include <qobject.h>

#include "util/String.h"
#include "message/message.h"
#include "qtsupport/QMessageTransceiverThread.h"
#include "reflector/AbstractReflectSession.h"

using namespace muscle;

class ServerClient : public QMessageTransceiverThread
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

protected:

	void MessageReceived(MessageRef msg, const String & sessionID);

	void SessionConnected(const String & sessionID);
	void SessionDetached(const String & sessionID);

private:
//	QMessageTransceiverThread * qmtt;
};

#endif
