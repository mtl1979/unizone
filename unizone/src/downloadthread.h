#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <qapplication.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qtimer.h>

#include "downloadthread.h"
#include "qtsupport/QMessageTransceiverThread.h"
#include "user.h"
#include "wfile.h"

#include <time.h>

#include "message/Message.h"
#include "system/MessageTransceiverThread.h"
#include "qtsupport/QMessageTransceiverThread.h"

using namespace muscle;

#define MD5_DIGEST_SIZE 16

#define MAX_RATE_COUNT 10
#define MAX_ETA_COUNT 10

#define PARTIAL_RESUME_SIZE (64 * 1024)

class QString;
class WDownloadEvent;

class WDownloadThread : public QObject
{
	Q_OBJECT
public:
	// This thread will throw events to this owner to update its GUI
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

	bool IsConnecting() const;

	bool IsTunneled() const { return fTunneled; }

	void Accepted(int64 hisID);
	void Rejected();

	double GetCalculatedRate() const;
	void SetMostRecentRate(double rate);
	void SetPacketCount(double bytes);

	QString GetETA(int64 cur, int64 max, double rate = -1);	// if rate < 0, then call GetCalculatedRate()
	uint64	GetStartTime() { return fStartTime; }

	void SetFile(QString * files, QString * lfiles, QString * lpaths, int32 numFiles, const QString & fromIP, const QString & fromSession,
					const QString & localSession, uint32 remotePort, bool firewalled, bool partial);
	void SetFile(QString * files, QString * lfiles, QString * lpaths, int32 numFiles, const WUserRef & fromUser); // Tunneled
	void NextFile();
	int32 GetCurrentNum() { return fCurFile; }
	int32 GetNumFiles() { return fNumFiles; }

	WFile * GetFile() const { return fFile; }
	QString GetCurrentFile() const;
	QString GetCurrentLocalFile() const;
	QString GetFileName(int i) const;
	QString GetLocalFileName(int i) const;
	QString GetPath(int i) const;
	bool IsLastFile(); 

	QString GetRemoteID() const { return fFromSession; }
	QString GetRemoteUser() const { return fFromUser; }
	QString GetRemoteIP() const;
	uint32 GetRemotePort() const{ return fPort; }

	int GetRate() const { return fTXRate; }
	void SetRate(int rate, ThreadWorkerSessionRef & ref);
	void SetRate(int rate);
	void ResetRate() { SetRate(fTXRate); }
	void ResetRate(ThreadWorkerSessionRef & ref) { SetRate(fTXRate, ref); }

	void SetPacketSize(double s);		// Set/get packet size in kB
	double GetPacketSize();
	// forwarders

	void Reset();
	bool IsInternalThreadRunning();
	status_t RemoveSessions(const char * optDistPath = NULL);
	status_t SendMessageToSessions(const MessageRef & msgRef, const char * optDistPath = NULL);

private slots:
	void ConnectTimer(); // Connection timed out?
	void BlockedTimer(); // Blocking timed out?

	void MessageReceived(const MessageRef & msg, const String & sessionID);

	void SessionAccepted(const String &sessionID, uint32 port);
	void SessionDetached(const String &sessionID);
	void SessionConnected(const String &sessionID);
	void SessionDisconnected(const String &sessionID);

	void ServerExited();

	// --------------------------------------------------------------------------------------------

protected:

	bool event( QEvent * );

//	friend class WDownload;

//	void MessageReceived(const MessageRef &msg) { MessageReceived(msg, _sessionID); }

	mutable Mutex fLockFile;
	WFile * fFile;				// file on the HD
	QString * fFileDl;		// file to dl
	QString * fLocalFileDl; // local filenames for downloaded files
	QString * fPaths;       // remote paths for downloaded files
	QString fIP;				// ip address of remote client
	QString fFromSession;	// session ID of remote client
	QString fFromUser;		// user name of remote client
	QString fLocalSession;	// our session ID
	uint32 fPort;				// port of the remote client (the one it's listening on)
	int32 fAcceptingOn;		// port we're accepting on in case the user is firewalled
	int64 fCurrentOffset;	// current offset in the file
	int64 fFileSize;			// the file size
	time_t fCurrentFileStartTime;
	bool fDownloading;
	bool fFirewalled;
	bool fPartial;
	int32 fNumFiles, fCurFile;
	uint32 fIdles;

	void SendReply(WDownloadEvent *);
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
	bool fNegotiating;
	bool fConnecting;
	bool fTunneled;
	volatile bool fDisconnected;
	double fRate[MAX_RATE_COUNT];	// last 20 rates
	int fRateCount;					// amount we have, 20 max
	uint32 fETA[MAX_ETA_COUNT];		// last 5 ETA's
	int fETACount;					// amount we have, 5 max
	double fPackets;				// amount of ultrafast packets transfered
	int64 fTimeLeft;
	uint64 fStartTime;				// Time elapsed since this session started
	double fPacket;

	void SetMostRecentETA(uint32 eta);
	uint32 ComputeETA() const;

	QString GetUserName(const QString & sid) const;

	int fTXRate; // Current transfer throttling rate

	QTimer * CTimer;					// Connect timer
	QTimer * fBlockTimer;				// Blocked timer

private:
	String _sessionID;

	int64 hisID;

	int timerID;

	QMessageTransceiverThread *qmtt;

	void InitTransferRate();
	void InitTransferETA();

	void InitSessionAux();
};

#endif
