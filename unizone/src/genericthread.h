#ifndef GENERICTHREAD_H
#define GENERICTHREAD_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qevent.h>
#include <qthread.h>
#include <qdatetime.h>
#include <qobject.h>

#include "message/Message.h"
#include "system/MessageTransceiverThread.h"
#include "qtsupport/QMessageTransceiverThread.h"
using namespace muscle;

#define MD5_DIGEST_SIZE 16

#define MAX_RATE_COUNT 10
#define MAX_ETA_COUNT 10

#define PARTIAL_RESUME_SIZE (64 * 1024)

// ------------------------------------------------------------------------------------

class WGenericThread : public QObject
{
	Q_OBJECT
public:
	WGenericThread(QObject * owner, bool * optShutdownFlag = NULL);
	virtual ~WGenericThread();

	QTime fLastData;		// public time to keep control of the last time we got some data

	bool IsManuallyQueued() const;
	virtual void SetManuallyQueued(bool b);

	bool IsLocallyQueued() const;
	virtual void SetLocallyQueued(bool b);

	bool IsRemotelyQueued() const;
	virtual void SetRemotelyQueued(bool b);

	// The active state signifies whether the download is active.
	// IE: It has not been canceled, and is still in downloading, etc.
	bool IsActive() const;
	virtual void SetActive(bool b);

	bool IsBlocked() const;
	virtual void SetBlocked(bool b, int64 timeLeft = -1);

	bool IsFinished() const;
	virtual void SetFinished(bool b);

	double GetCalculatedRate() const;
	void SetMostRecentRate(double rate);
	void SetPacketCount(double bytes);

	QString GetETA(uint64 cur, uint64 max, double rate = -1);	// if rate < 0, then call GetCalculatedRate()
	uint64	GetStartTime() { return fStartTime; }

	virtual QString GetRemoteID() { return QString::null; }
	virtual QString GetRemoteUser() { return QString::null; }
	virtual QString GetRemoteIP() { return QString::null; }
	virtual uint32 GetRemotePort() { return 0; }
	virtual QString GetCurrentFile() { return QString::null; }
	virtual QString GetCurrentLocalFile() { return QString::null; }

	virtual long GetCurrentNum() { return -1; }
	virtual long GetNumFiles() { return 0; }
	virtual bool IsLastFile() { return ((GetCurrentNum() + 1) == GetNumFiles()); }
	virtual QString GetFileName(int i) { return QString::null; }
	virtual QString GetLocalFileName(int i) { return GetFileName(i); }

	int GetRate() { return fTXRate; }
	virtual void SetRate(int rate, AbstractReflectSessionRef & ref) { fTXRate = rate; }
	virtual void SetRate(int rate) { fTXRate = rate; }
	virtual void ResetRate() { SetRate(fTXRate); }
	virtual void ResetRate(AbstractReflectSessionRef & ref) { SetRate(fTXRate, ref); }

	virtual void SetPacketSize(int s);		// Set/get packet size in kB
	virtual int GetPacketSize();

	int GetBanTime();

	// forwarders

	void Reset()
	{
		qmtt->Reset();
	}

	bool IsInternalThreadRunning()
	{
		return qmtt->IsInternalThreadRunning();
	}

public slots:
	void ConnectTimer(); // Connection timed out?
	void BlockedTimer(); // blocking timed out?

   /** Emitted when a new Message has been received by one of the sessions being operated by our internal thread.
     * @param msg Reference to the Message that was received.
     * @param sessionID Session ID string of the session that received the message
     */
   virtual void MessageReceived(MessageRef msg, const String & sessionID) {}

   /** Emitted when a new Session object is accepted by one of the factories being operated by our internal thread
     * @param sessionID Session ID string of the newly accepted Session object.
     * @param port Port number that the accepting factory was listening to
     */ 
   virtual void SessionAccepted(const String & sessionID, uint16 port) {}

   /** Emitted when a session object is attached to the internal thread's ReflectServer */
   virtual void SessionAttached(const String & sessionID) {}

   /** Emitted when a session object connects to its remote peer (only used by sessions that were
     * created using AddNewConnectSession())
     * @param sessionID Session ID string of the newly connected Session object.
     */
   virtual void SessionConnected(const String & sessionID) {}

   /** Emitted when a session object is disconnected from its remote peer
     * @param sessionID Session ID string of the newly disconnected Session object.
     */
   virtual void SessionDisconnected(const String & sessionID) {}

   /** Emitted when a session object is removed from the internal thread's ReflectServer 
     * @param sessionID Session ID string of the newly disconnected Session object.
     */
   virtual void SessionDetached(const String & sessionID) {}

   /** Emitted when a factory object is attached to the internal thread's ReflectServer.
     * @param port Port ID that the factory object is attached on.
     */
   virtual void FactoryAttached(uint16 port) {}

   /** Emitted when a factory object is removed from the internal thread's ReflectServer.
     * @param port Port ID that the factory object was removed from.
     */
   virtual void FactoryDetached(uint16 port) {}

   /** Emitted when the thread's internal ReflectServer object exits. */
   virtual void ServerExited() {}

   /** Signal emitted when a TCP connection is completed.  */
   virtual void SessionConnected() {}

   /** Emitted when the output-queues of the sessions specified in a previous call to 
     * RequestOutputQueuesDrainedNotification() have drained.  Note that this signal only 
     * gets emitted once per call to RequestOutputQueuesDrainedNotification();
     * it is not emitted spontaneously.
     * @param ref MessageRef that you previously specified in RequestOutputQueuesDrainedNotification().
     */
   virtual void OutputQueuesDrained(MessageRef ref) {}

   /** This signal is called for all events send by the internal thread.  You can use this
     * to catch custom events that don't have their own signal defined above, or if you want to
     * receive all thread events via a single slot.
     * @param code the MTT_EVENT_* code of the new event.
     * @param optMsg If a Message is relevant, this will contain it; else it's a NULL reference.
     * @param optFromSession If a session ID is relevant, this is the session ID; else it will be "".
     * @param optFromFactory If a factory is relevant, this will be the factory's port number; else it will be zero.
     */
   virtual void InternalThreadEvent(uint32 code, MessageRef optMsg, const String & optFromSession, uint16 optFromfactory) {}

protected:
	QObject * fOwner;
	bool fShutdown;
	bool * fShutdownFlag;
	//bool fQueued;
	bool fManuallyQueued;
	bool fLocallyQueued;
	bool fRemotelyQueued;			// only usable in downloads
	bool fActive;
	bool fBlocked;
	bool fFinished;
	double fRate[MAX_RATE_COUNT];	// last 20 rates
	int fRateCount;					// amount we have, 20 max
	uint32 fETA[MAX_ETA_COUNT];		// last 5 ETA's
	int fETACount;					// amount we have, 5 max
	double fPackets;				// amount of ultrafast 8 kB packets transfered
	int64 fTimeLeft;
	uint64 fStartTime;				// Time elapsed since this session started
	int fPacket;

	void SetMostRecentETA(uint32 eta);
	uint32 ComputeETA() const;

	virtual void SendReply(MessageRef &m);
	QString GetUserName(QString sid);

	int fTXRate; // Current transfer throttling rate

	QTimer * CTimer;					// Connect timer
	QTimer * fBlockTimer;				// Blocked timer

	QMessageTransceiverThread * qmtt;

private:
	void InitTransferRate();
	void InitTransferETA();
};

#endif

