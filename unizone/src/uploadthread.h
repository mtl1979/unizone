#ifndef UPLOADTHREAD_H
#define UPLOADTHREAD_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

// #include "genericthread.h"
// #include "filethread.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "message/Message.h"
#include "system/MessageTransceiverThread.h"
#include "qtsupport/QMessageTransceiverThread.h"

using namespace muscle;

#include <qobject.h>
#include <qstring.h>
#include <qfile.h>
#include <qdatetime.h>

class WFileThread;

/*
 *
 *  This is my wonderful file upload thread. It handles starting the
 *	connection remote peer and transfering a file(s) to him/her.
 *
 */

#define MD5_DIGEST_SIZE 16

#define MAX_RATE_COUNT 10
#define MAX_ETA_COUNT 10

#define PARTIAL_RESUME_SIZE (64 * 1024)

class WUploadThread : public QObject
{
	Q_OBJECT
public:
	WUploadThread(QObject * owner, bool * optShutdownFlag = NULL);
	~WUploadThread();

	void SetUpload(int socket, uint32 remoteIP, WFileThread * ft);
	void SetUpload(const QString & remoteIP, uint32 remotePort, WFileThread * ft);

	bool InitSession();

	QTime fLastData;		// public time to keep control of the last time we got some data

	bool IsManuallyQueued() const;
	void SetManuallyQueued(bool b);

	bool IsLocallyQueued() const;
	void SetLocallyQueued(bool b);

	bool IsRemotelyQueued() const;
	void SetRemotelyQueued(bool b);

	// The active state signifies whether the download is active.
	// IE: It has not been canceled, and is still in downloading, etc.
	bool IsActive() const;
	void SetActive(bool b);

	bool IsBlocked() const;
	void SetBlocked(bool b, int64 timeLeft = -1);

	bool IsFinished() const;
	void SetFinished(bool b);

	double GetCalculatedRate() const;
	void SetMostRecentRate(double rate);
	void SetPacketCount(double bytes);

	QString GetETA(uint64 cur, uint64 max, double rate = -1);	// if rate < 0, then call GetCalculatedRate()
	uint64	GetStartTime() { return fStartTime; }

	QString GetRemoteID() const {return fRemoteSessionID;}
	QString GetRemoteUser() const; 
	QString GetRemoteIP() const {return fStrRemoteIP;}
	uint32 GetRemotePort() const {return fPort;}
	QString GetCurrentFile() const {return fFileUl;}
	QString GetFileName(int i) const;

	int32 GetCurrentNum() const { return fCurFile; }
	int32 GetNumFiles() const { return fNumFiles; }
	bool IsLastFile(); 

	int GetRate() { return fTXRate; }
	virtual void SetRate(int rate, AbstractReflectSessionRef & ref); 
	virtual void SetRate(int rate); 
	virtual void ResetRate() { SetRate(fTXRate); }
	virtual void ResetRate(AbstractReflectSessionRef & ref) { SetRate(fTXRate, ref); }

	void SetPacketSize(int s);		// Set/get packet size in kB
	int GetPacketSize();

	int GetBanTime();

	// forwarders

	void Reset();
	bool IsInternalThreadRunning();
	status_t RemoveSessions(const char * optDistPath = NULL);
	status_t SendMessageToSessions(MessageRef msgRef, const char * optDistPath = NULL);

private slots:

	void ConnectTimer(); // Connection timed out?
	void BlockedTimer(); // Blocking timed out?

	// --------------------------------------------------------------------------------------------

	void MessageReceived(MessageRef msg, const String &sessionID);
	void OutputQueuesDrained(MessageRef msg);
	
	void SessionConnected(const String &sessionID);
	void SessionDisconnected(const String &sessionID);

    void SessionAttached(const String & sessionID);
	void SessionDetached(const String & sessionID);

	void ServerExited();

protected:

	void SendReply(MessageRef &m);
	void SendQueuedNotification();
	void SendRejectedNotification(bool);
	void timerEvent(QTimerEvent *);
	bool event(QEvent *);
	QObject * fOwner;
	bool fShutdown;
	bool * fShutdownFlag;
	bool fManuallyQueued;
	bool fLocallyQueued;
	bool fRemotelyQueued;			// only usable in downloads
	bool fActive;
	bool fBlocked;
	bool fFinished;
	bool fConnecting;
	volatile bool fDisconnected;
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

	QString GetUserName(const QString & sid) const;

	int fTXRate; // Current transfer throttling rate

	QTimer * CTimer;					// Connect timer
	QTimer * fBlockTimer;				// Blocked timer

private:
	Queue<MessageRef> fUploads;
	Queue<QString> fNames;
	QFile * fFile;
	uint32 fRemoteIP;
	QString fStrRemoteIP;				// the accept version gets a string IP
	uint32 fPort;						// port for accept version
	uint32 fIdles;						// idle packets sent between real packets
	int fSocket;
	uint64 fFileSize;
	uint64 fCurrentOffset;
	int fMungeMode;
	QString fRemoteSessionID;
	QString fRemoteUser;
	QString fFileUl;
	WFileThread * fFileThread;
	MessageRef fCurrentRef;
	bool fWaitingForUploadToFinish;
	bool fAccept;						// is this the accept version?
	bool fForced;						// did this transfer bypass queue?
//	bool fInit;							// has InitSession() been postponed due file scan in progress

	int timerID;

	int32 fCurFile;
	uint32 fNumFiles;

	String _sessionID;

	QMessageTransceiverThread *qmtt;

	void DoUpload();
	void TransferFileList(const MessageRef &);
	void NextFile();
	void SignalUpload();

	enum
	{
		UploadEvent = 'tsue'
	};

	MessageRef fSavedFileList;

	void InitTransferRate();
	void InitTransferETA();
};


#endif
