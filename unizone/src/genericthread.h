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

#define MD5_DIGEST_SIZE 16

#define MAX_RATE_COUNT 10
#define MAX_ETA_COUNT 10

#define PARTIAL_RESUME_SIZE (64 * 1024)

// ------------------------------------------------------------------------------------

class WGenericThread : public QObject, public MessageTransceiverThread
{
	Q_OBJECT
public:
	WGenericThread(QObject * owner, bool * optShutdownFlag = NULL);
	virtual ~WGenericThread();

	QTime fLastData;		// public time to keep control of the last time we got some data

	bool IsQueued() const;
	virtual void SetQueued(bool b);

	// The active state signifies whether the download is active.
	// IE: It has not been canceled, and is still in downloading, etc.
	bool IsActive() const;
	virtual void SetActive(bool b);

	bool IsBlocked() const;
	virtual void SetBlocked(bool b);

	double GetCalculatedRate() const;
	void SetMostRecentRate(double rate);
	void SetPacketCount(double bytes);

	QString GetETA(uint64 cur, uint64 max, double rate = -1);	// if rate < 0, then call GetCalculatedRate()


//	QString ComputeSizeString(int64 offset) const;
	QString ComputePercentString(int64 cur, int64 max);

	virtual QString GetRemoteID() { return QString::null; }
	virtual QString GetRemoteUser() { return QString::null; }
	virtual QString GetCurrentFile() { return QString::null; }

	virtual long GetCurrentNum() { return -1; }
	virtual long GetNumFiles() { return 0; }
	virtual bool IsLastFile() { return ((GetCurrentNum() + 1) == GetNumFiles()); }

public slots:
	void ConnectTimer(); // Connection timed out?

protected:
	QObject * fOwner;
	bool * fShutdownFlag;
	bool fQueued;
	bool fActive;
	bool fBlocked;
	double fRate[MAX_RATE_COUNT];	// last 20 rates
	int fRateCount;					// amount we have, 20 max
	uint32 fETA[MAX_ETA_COUNT];		// last 5 ETA's
	int fETACount;					// amount we have, 5 max
	double fPackets;				// amount of ultrafast 8 kB packets transfered

	void SetMostRecentETA(uint32 eta);
	uint32 ComputeETA() const;

	virtual void SignalOwner() = 0;
	virtual void SendReply(Message * m);
	QString GetUserName(QString sid);

};

#endif

