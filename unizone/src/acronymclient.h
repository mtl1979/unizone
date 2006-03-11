#ifndef UPDATECLIENT
#define UPDATECLIENT

#include <qapplication.h>
#include <qobject.h>

#include "qtsupport/QMessageTransceiverThread.h"
#include "reflector/AbstractReflectSession.h"
#include "util/String.h"

using namespace muscle;

class AcronymClient : public QObject
{
	Q_OBJECT
public:
	AcronymClient(QObject *owner);
	~AcronymClient();

	void SetAcronym(const QString &acronym);
	void SetPage(int);

	void Disconnect();

	// forwarders

	void Reset();
	status_t StartInternalThread(); 
	status_t AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef);

private slots:

	void SessionConnected(const String & sessionID);
	void SessionDetached(const String & sessionID);
	void MessageReceived(const MessageRef & msg, const String & sessionID);

private:

	void ParseLine(const QString &line);
	QMessageTransceiverThread *qmtt;
	QString fAcronym;
	int fPage;
	String fHostName;
};

void QueryAcronym(const QString &q, int page = 1);
#endif
