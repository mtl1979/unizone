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
using namespace muscle;

#include "messenger.h"

#define MD5_DIGEST_SIZE 16

#define MAX_RATE_COUNT 10
#define MAX_ETA_COUNT 10

#define PARTIAL_RESUME_SIZE (64 * 1024)

// ------------------------------------------------------------------------------------

class WGenericThread : public WMessenger 
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


	// Forwarders for WMessengerThread

	void Reset()
	{
		wmt->Reset();
	}

public slots:
	void ConnectTimer(); // Connection timed out?
	void BlockedTimer(); // blocking timed out?

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

	virtual void SignalOwner() = 0;
	virtual void SendReply(MessageRef &m);
	QString GetUserName(QString sid);

	int fTXRate; // Current transfer throttling rate

	QTimer * CTimer;					// Connect timer
	QTimer * fBlockTimer;				// Blocked timer

	WMessengerThread * wmt;

private:
	void InitTransferRate();
	void InitTransferETA();
};

#endif

