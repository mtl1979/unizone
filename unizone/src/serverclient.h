#ifndef SERVERCLIENT
#define SERVERCLIENT

#include <qobject.h>

#include "qtsupport/QMessageTransceiverThread.h"
#include "reflector/AbstractReflectSession.h"
#include "util/String.h"

using namespace muscle;

class ServerClient : public QObject
{
	Q_OBJECT
public:
	ServerClient(QObject *owner);
	~ServerClient();

	void Disconnect();

	// forwarders

	status_t StartInternalThread() 
	{
		return qmtt->StartInternalThread(); 
	}
	
	status_t AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef)
	{
		return qmtt->AddNewConnectSession(targetHostName, port, optSessionRef);
	}

	void Reset()
	{
		qmtt->Reset();
	}

public slots:
   /** Emitted when a new Message has been received by one of the sessions being operated by our internal thread.
     * @param msg Reference to the Message that was received.
     * @param sessionID Session ID string of the session that received the message
     */
   void MessageReceived(MessageRef msg, const String & sessionID);

   /** Emitted when a session object connects to its remote peer (only used by sessions that were
     * created using AddNewConnectSession())
     * @param sessionID Session ID string of the newly connected Session object.
     */
   void SessionConnected(const String & sessionID);

   /** Emitted when a session object is removed from the internal thread's ReflectServer 
     * @param sessionID Session ID string of the newly disconnected Session object.
     */
   void SessionDetached(const String & sessionID);

private:
	QMessageTransceiverThread * qmtt;
};

#endif
