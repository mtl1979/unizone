#include "genericthread.h"
#include "wgenericevent.h"
#include "global.h"
#include "platform.h"		// <postmaster@raasu.org> 20021114
#include <qapplication.h>

WGenericThread::WGenericThread(QObject * owner, bool * optShutdownFlag)
	: MessageTransceiverThread(), fOwner(owner), fShutdownFlag(optShutdownFlag)
{
	int i;
	// Default status
	fActive = true;
	fQueued = false;
	fRateCount = 0;
	fETACount = 0;
	fPackets = 0;
	for (i = 0; i < MAX_RATE_COUNT; i++)
		fRate[i] = 0.0f;
	for (i = 0; i < MAX_ETA_COUNT; i++)
		fETA[i] = 0;
}

WGenericThread::~WGenericThread()
{
	if (fShutdownFlag && !*fShutdownFlag)
	{
		*fShutdownFlag = true;
		WaitForInternalThreadToExit();
	}
}

bool
WGenericThread::IsQueued() const
{
	return fQueued;
}

void
WGenericThread::SetQueued(bool b)
{
	fQueued = b;
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
WGenericThread::SetBlocked(bool b)
{
	fBlocked = b;
}

bool
WGenericThread::IsBlocked() const
{
	return fBlocked;
}

void 
WGenericThread::SendReply(Message * m)
{
	// since we're just reading, we don't need to do any locking, etc
	if (!gWin->fDLWindow)	// doesn't exist anymore??
	{
		delete m;
		m = NULL; // <postmaster@raasu.org> 20021027
		return;
	}
	m->AddPointer("sender", this);
	WGenericEvent * wge = new WGenericEvent(m);
	if (wge)
		QThread::postEvent(fOwner, wge); // Don't use QApplication::postEvent() here ;)
}

void 
WGenericThread::SetPacketCount(double bytes)
{
	fPackets += bytes / 8.0f ;
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
	for (int i = 0; i < fRateCount; i++)
		added += fRate[i];

	// <postmaster@raasu.org> 20021024,20021026,20021101 -- Don't try to divide zero or by zero
	return (((added != 0.0f) && (fRateCount != 0)) ? (added / (double)fRateCount) : 0.0f);
}

/*
QString
WGenericThread::ComputeSizeString(int64 offset) const
{
	float size = 0;
	QString pre;
	
	if (offset > 1024)	// > 1 kB?
	{
#ifdef VC7
		size = (float)((double)offset / 1024.0);
#else
		size = (float)((int)offset / 1024);
#endif
		pre = "kB";		// <postmaster@raasu.org> 20021024 -- It's kB, not KB!
		if (size > 1024)	// > 1 MB?
		{
			size /= 1024.0f;
			pre = "MB";

			if (size > 1024) // > 1 GB?
			{
				size /= 1024.0f;
				pre = "GB";
			}
		}

		return QObject::tr("%1 %2").arg(size).arg(pre);
	}
	return QObject::tr("%1 %2").arg((int)offset).arg("bytes");
}
*/

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
/*	
	uint32 min = secs / 60; secs = secs % 60;
	uint32 hours = min / 60; min = min % 60;
*/	
	QString ret;
//	ret.sprintf("%d:%.2d:%.2d", hours, min, secs);
	ret = tr("%1").arg(secs);
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
	for (int i = 0; i < fETACount; i++)
		added += fETA[i];
	return added / fETACount;
}

QString
WGenericThread::ComputePercentString(int64 cur, int64 max)
{
	QString ret;
	ret.sprintf("%.2f", (double)((double)cur / (double)max) * 100.0);
	return ret;
}

QString
WGenericThread::GetUserName(QString sid)
{
	WUserRef uref = gWin->FindUser(sid);
	QString ret;
	if (uref())
		ret = uref()->GetUserName();
	return ret;
}

void
WGenericThread::ConnectTimer()
{
	Reset();
	Message * msg = new Message(WGenericEvent::ConnectFailed);
	msg->AddString("why", "Connection timed out!");
	msg->AddString("file", (const char *) GetCurrentFile().utf8());
	msg->AddBool("retry", true);
	SendReply(msg);
}
