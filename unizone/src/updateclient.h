#ifndef UPDATECLIENT
#define UPDATECLIENT

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
	status_t AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef);

	void Reset();

private slots:

	void MessageReceived(MessageRef msg, const String & sessionID);

	void SessionConnected(const String & sessionID);
	void SessionDetached(const String & sessionID);

private:

	bool CheckVersion(const String &, QString * = NULL);

	QMessageTransceiverThread *qmtt;
};

#endif
