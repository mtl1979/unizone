#include "genericthread.h"
#include "wgenericevent.h"
#include "winsharewindow.h"
#include "global.h"
#include "debugimpl.h"

#include <qapplication.h>

WGenericThread::WGenericThread(QObject * owner, bool * optShutdownFlag)
: WMessenger(owner, NULL), fOwner(owner), fShutdownFlag(optShutdownFlag)
{
	setName( "WGenericThread" );

	wmt = new WMessengerThread(this);
	CHECK_PTR(wmt);

	// Default status

	if (!fShutdownFlag)					// Force use of Shutdown Flag
	{
		fShutdown = false;
		fShutdownFlag = &fShutdown;
	}
	fActive = true;
	fBlocked = false;
	fFinished = false;
	fManuallyQueued = false;
	fLocallyQueued = false;
	fRemotelyQueued = false;
	fPackets = 0;
	fTXRate = 0;
	fTimeLeft = 0;
	fStartTime = 0;
	fPacket = 8;
	InitTransferRate();
	InitTransferETA();

	CTimer = new QTimer(this, "Connect Timer");
	CHECK_PTR(CTimer);

	connect( CTimer , SIGNAL(timeout()), this, SLOT(ConnectTimer()) );
	
	fBlockTimer = new QTimer(this, "Blocked Timer");
	CHECK_PTR(fBlockTimer);

	connect( fBlockTimer, SIGNAL(timeout()), this, SLOT(BlockedTimer()) );
}

WGenericThread::~WGenericThread()
{
	if (fShutdownFlag && !*fShutdownFlag)
	{
		*fShutdownFlag = true;
	}
	if (wmt->IsInternalThreadRunning()) 
	{
		wmt->ShutdownInternalThread();
		wmt->Reset(); 
	}
	wmt->WaitForInternalThreadToExit();
	delete wmt;
	wmt = NULL;
}

bool
WGenericThread::IsManuallyQueued() const
{
	return fManuallyQueued;
}

void
WGenericThread::SetManuallyQueued(bool b)
{
	fManuallyQueued = b;	
}

bool
WGenericThread::IsLocallyQueued() const
{
	return fLocallyQueued;
}

void
WGenericThread::SetLocallyQueued(bool b)
{
	fLocallyQueued = b;
	if (b)
	{
		fStartTime = 0;
	}
	else
	{
		fLastData.restart();
	}
}

bool
WGenericThread::IsRemotelyQueued() const
{
	return fRemotelyQueued;
}

void
WGenericThread::SetRemotelyQueued(bool b)
{
	fRemotelyQueued = b;
	if (!b)
	{
		fLastData.restart();
	}
}

void
WGenericThread::SetActive(bool b)
{
	fActive = b;
}

bool
WGenericThread::IsActive() const
{
	return fActive;
}

void
WGenericThread::SetFinished(bool b)
{
	fFinished = b;
}

bool
WGenericThread::IsFinished() const
{
	return fFinished;
}

void
WGenericThread::SetBlocked(bool b, int64 timeLeft)
{
	fBlocked = b;
	if (b)
	{
		fTimeLeft = timeLeft;
		fStartTime = 0;
	}
	else
	{
		fTimeLeft = 0;
		fLastData.restart();
	}
}

bool
WGenericThread::IsBlocked() const
{
	return fBlocked;
}

void 
WGenericThread::SendReply(MessageRef &m)
{
	// since we're just reading, we don't need to do any locking, etc
	if (!gWin->fDLWindow)	// doesn't exist anymore??
	{
		PRINT("WGenericThread::SendReply() : Invalid fOwner\n");
		if (wmt->IsInternalThreadRunning())
		{
			wmt->Reset();
		}
		return;
	}
	m()->AddPointer("sender", this);
	WGenericEvent * wge = new WGenericEvent(m);
	if (wge)
		QThread::postEvent(fOwner, wge);
}

void 
WGenericThread::SetPacketCount(double bytes)
{
	fPackets += bytes / ((double) fPacket) ;
}

void
WGenericThread::SetMostRecentRate(double rate)
{
	if (fPackets != 0.0f)
	{
		rate *= 1 + fPackets;
	}
	if (fRateCount == MAX_RATE_COUNT)
	{
		// remove the oldest rate
		for (int i = 1; i < MAX_RATE_COUNT; i++)
			fRate[i - 1] = fRate[i];
		fRate[MAX_RATE_COUNT - 1] = rate;
	}
	else
		fRate[fRateCount++] = rate;
	fPackets = 0.0f; // reset packet count
}

double
WGenericThread::GetCalculatedRate() const
{
	double added = 0.0f;
	double rate = 0.0f;

	for (int i = 0; i < fRateCount; i++)
		added += fRate[i];

	// <postmaster@raasu.org> 20021024,20021026,20021101 -- Don't try to divide zero or by zero

	if ( (added > 0.0f) && (fRateCount > 0) )
		rate = added / (double)fRateCount;

	return rate;
}


QString
WGenericThread::GetETA(uint64 cur, uint64 max, double rate)
{
	if (rate < 0)
		rate = GetCalculatedRate();
	// d = r * t
	// t = d / r
	uint64 left = max - cur;	// amount left
	uint32 secs = (uint32)((double)(int64)left / rate);

	SetMostRecentETA(secs);
	secs = ComputeETA();

	QString ret;
	ret.setNum(secs);
	return ret;
}

// Do the same averaging for ETA's that we do for rates
void
WGenericThread::SetMostRecentETA(uint32 eta)
{
	if (fETACount == MAX_ETA_COUNT)
	{
		// remove the oldest eta
		for (int i = 1; i < MAX_ETA_COUNT; i++)
			fETA[i - 1] = fETA[i];
		fETA[MAX_ETA_COUNT - 1] = eta;
	}
	else
		fETA[fETACount++] = eta;

}

uint32
WGenericThread::ComputeETA() const
{
	uint32 added = 0;
	uint32 eta = 0;
	for (int i = 0; i < fETACount; i++)
		added += fETA[i];

	if ( (added > 0) && (fETACount > 0 ) )
	{
		eta = added / fETACount;
	}

	return eta;
}

QString
WGenericThread::GetUserName(QString sid)
{
	WUserRef uref = gWin->FindUser(sid);
	QString ret = sid;
	if (uref())
		ret = uref()->GetUserName();
	else
	{
		uref = gWin->FindUserByIPandPort(GetRemoteIP(), GetRemotePort());
		if (uref())
		{
			ret = uref()->GetUserName();
		}
		else
		{
			uref = gWin->FindUserByIPandPort(GetRemoteIP(), 0);
			if (uref())
			{
				ret = uref()->GetUserName();
			}
		}
	}
	return ret;
}

void
WGenericThread::ConnectTimer()
{
	Reset();
	MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectFailed));
	if (msg())
	{
		msg()->AddString("why", "Connection timed out!");
		SendReply(msg);
	}
}

void
WGenericThread::BlockedTimer()
{
	SetBlocked(false);
	fTimeLeft = 0;
}

int
WGenericThread::GetBanTime()
{
	if (fTimeLeft == 0)
		return 0;
	else if (fTimeLeft == -1)
		return -1;
	else
		return (fTimeLeft / 60000000);
}

void
WGenericThread::SetPacketSize(int s)
{
	// Clear Rate and ETA counts because changing the Packet Size between two estimate recalculations causes miscalculation
	if (fPacket != s)
	{
		fRateCount = 0;
		fETACount = 0;
	}
	fPacket = s;
}

int
WGenericThread::GetPacketSize()
{
	return fPacket;
}

void
WGenericThread::InitTransferRate()
{
	for (int i = 0; i < MAX_RATE_COUNT; i++)
	{
		fRate[i] = 0.0f;
	}

	fRateCount = 0;
}

void
WGenericThread::InitTransferETA()
{
	for (int i = 0; i < MAX_ETA_COUNT; i++)
	{
		fETA[i] = 0;
	}

	fETACount = 0;
}
