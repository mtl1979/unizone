#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

// #include "genericthread.h"
#include "downloadthread.h"
// #include "uploadthread.h"
#include "qtsupport/QMessageTransceiverThread.h"

#include <qfile.h>
#include <qstring.h>
#include <qthread.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <time.h>

#include "message/Message.h"
#include "system/MessageTransceiverThread.h"
#include "qtsupport/QMessageTransceiverThread.h"

using namespace muscle;

#define MD5_DIGEST_SIZE 16

#define MAX_RATE_COUNT 10
#define MAX_ETA_COUNT 10

#define PARTIAL_RESUME_SIZE (64 * 1024)


class WDownloadThread : public QObject
{
	Q_OBJECT
public:
	// This thread will throw events to this owner to udpate it's GUI
	WDownloadThread(QObject * owner, bool * optShutdownFlag = NULL);
	~WDownloadThread();

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

	void SetFile(QString * files, QString * lfiles, int32 numFiles, QString fromIP, QString fromSession,
					QString localSession, uint32 remotePort, bool firewalled, bool partial);
	void NextFile();
	int32 GetCurrentNum() { return fCurFile; }
	int32 GetNumFiles() { return fNumFiles; }

	QFile * GetFile() const { return fFile; }
	QString GetCurrentFile();
	QString GetCurrentLocalFile();
	QString GetFileName(int i);
	QString GetLocalFileName(int i);
	bool IsLastFile(); 

	static QString FixFileName(const QString & fixMe);

	QString GetRemoteID() { return fFromSession; }
	QString GetRemoteUser() { return fFromUser; }
	QString GetRemoteIP() { return fIP; }
	uint32 GetRemotePort() { return fPort; }

	int GetRate() { return fTXRate; }
	void SetRate(int rate, AbstractReflectSessionRef & ref);
	void SetRate(int rate);
	void ResetRate() { SetRate(fTXRate); }
	void ResetRate(AbstractReflectSessionRef & ref) { SetRate(fTXRate, ref); }

	// forwarders

	void Reset();
	bool IsInternalThreadRunning();
	status_t RemoveSessions(const char * optDistPath = NULL);
	status_t SendMessageToSessions(MessageRef msgRef, const char * optDistPath = NULL);

public slots:
	void ConnectTimer(); // Connection timed out?
	void BlockedTimer(); // Blocking timed out?

	void MessageReceived(MessageRef msg, const String & sessionID);
	void SessionAccepted(const String &sessionID, uint16 port);
	void SessionConnected(const String &sessionID);
	void ServerExited();
	void SessionDisconnected(const String &sessionID);

protected:
	mutable QMutex fLockFile;
	QFile * fFile;			// file on the HD
	QString * fFileDl;		// file to dl
	QString * fLocalFileDl; // local filenames for downloaded files
	QString fIP;			// ip address of remote client
	QString fFromSession;	// session ID of remote client
	QString fFromUser;		// user name of remote client
	QString fLocalSession;	// our session ID
	uint32 fPort;			// port of the remote client (the one it's listening on)
	int32 fAcceptingOn;		// port we're accepting on in case the user is firewalled
	uint64 fCurrentOffset;	// current offset in the file
	uint64 fFileSize;		// the file size
	time_t fCurrentFileStartTime;
	bool fDownloading;
	bool fFirewalled;
	bool fPartial;
	int32 fNumFiles, fCurFile;

	void SendReply(MessageRef &m);
	void timerEvent(QTimerEvent *);

	QObject * fOwner;
	bool fShutdown;
	bool * fShutdownFlag;
	bool fManuallyQueued;
	bool fLocallyQueued;
	bool fRemotelyQueued;			// only usable in downloads
	bool fActive;
	bool fBlocked;
	bool fFinished;
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

	QString GetUserName(QString sid);

	int fTXRate; // Current transfer throttling rate

	QTimer * CTimer;					// Connect timer
	QTimer * fBlockTimer;				// Blocked timer

	QMessageTransceiverThread * qmtt;

private:
	QString UniqueName(QString file, int index); // build up unique name using 'file' and 'index'
	String _sessionID;

	int timerID;

	void InitTransferRate();
	void InitTransferETA();
};

// subclass ThreadWorkerSessionFactory to do throttling
class WDownloadThreadWorkerSessionFactory : public ThreadWorkerSessionFactory
{
public:
	WDownloadThreadWorkerSessionFactory(int limit);
	AbstractReflectSession * CreateSession(const String &);
	
private:
	int fLimit;
};

#endif
