#ifndef UPDATECLIENT
#define UPDATECLIENT

#include <qobject.h>

#include "qtsupport/QMessageTransceiverThread.h"
#include "reflector/AbstractReflectSession.h"
#include "util/String.h"

using namespace muscle;

class UpdateClient : public QMessageTransceiverThread
{
	Q_OBJECT
public:
	UpdateClient(QObject *owner);
	~UpdateClient();

	void Disconnect();

	// forwarders

	status_t StartInternalThread() 
	{
		return QMessageTransceiverThread::StartInternalThread(); 
	}
	
	status_t AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef)
	{
		return QMessageTransceiverThread::AddNewConnectSession(targetHostName, port, optSessionRef);
	}

	void Reset()
	{
		QMessageTransceiverThread::Reset();
	}

protected:

	void MessageReceived(MessageRef msg, const String & sessionID);

	void SessionConnected(const String & sessionID);
	void SessionDetached(const String & sessionID);

private:

	bool CheckVersion(const char *, QString * = NULL);
};

#endif
