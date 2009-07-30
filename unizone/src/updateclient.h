#ifndef UPDATECLIENT
#define UPDATECLIENT

#include <qapplication.h>
#include <qobject.h>

#include "qtsupport/QMessageTransceiverThread.h"
#include "reflector/AbstractReflectSession.h"
#include "util/String.h"

using namespace muscle;

class UpdateClient : public QObject
{
	Q_OBJECT
public:
	UpdateClient(QObject *owner);
	~UpdateClient();

	void Disconnect();

	// forwarders

	status_t StartInternalThread(); 
	status_t AddNewConnectSession(const String & targetHostName, uint16 port, ThreadWorkerSessionRef optSessionRef);

	void Reset();

private slots:

	void MessageReceived(const MessageRef & msg, const String & sessionID);

	void SessionConnected(const String & sessionID, const IPAddressAndPort & connectedTo);
	void SessionDetached(const String & sessionID);

private:

	bool CheckVersion(const QString &, QString &);

	QMessageTransceiverThread *qmtt;
	String fHostName;
	uint32 fHostPort;
};

#endif
