#include "resolverthread.h"
#include "global.h"
#include "winsharewindow.h" // For FillUserMap
#include "resolver.h"		// For ResolveAddress
#include "settings.h"		// For GetError
#include "events.h"			// For ErrorEvent
#include "util.h"			// For FixString
#include "netclient.h"		// For FindUsersByIP

ResolverThread::ResolverThread(bool * shutdownflag)
: QThread()
{
	fShutdownFlag = shutdownflag;
	if (*shutdownflag)
		*shutdownflag = false;
}

ResolverThread::~ResolverThread()
{
	fQueueLock.Lock();
	fQueue.Clear();
	fQueueLock.Unlock();
}

void
ResolverThread::Query(const QString &user, bool verbose)
{
	fQueueLock.Lock();
	ResolverEntry ent;
	ent.user = user;
	ent.verbose = verbose;
	fQueue.AddTail(ent);
	fQueueLock.Unlock();	
	wakeup();
}

void
ResolverThread::PrintAddressInfo(const WUserRef &user, bool verbose)
{
	QString addr, uname, uid;
	QString out;
	uint32 address = 0;
	char host[16];

	if (user() != NULL)
	{
		address = ResolveAddress(user()->GetUserHostName());
		addr = user()->GetUserHostName();
		uname = user()->GetUserName();
		uid = user()->GetUserID();
	}

	if (address > 0)
	{
		Inet_NtoA(address, host);
					
		out += tr("Address info for user #%1:").arg(uid);
		out += "\n" + tr("User Name: %1").arg(uname);
					
		out += "\n" + tr("IP Address: %1").arg(host);
		
		if (verbose)
		{
			if (user()->GetPort() != 0)
				out += "\n" + tr("Port: %1").arg( user()->GetPort() );
					
			QString qhost = ResolveHost(address);
			if (!qhost.isEmpty())
			{
				out += "\n" + tr("Host Name: %1").arg(qhost);
			}

			QString aliases = ResolveAliases(address);
			if (!aliases.isEmpty())
			{
				aliases.replace(QRegExp(","), " ");
				out += "\n" + tr("Aliases: %1").arg(aliases);
			}
		}
		SystemEvent(gWin, FixString(out));
	}					
	else if (gWin->fSettings->GetError())
	{
		ErrorEvent(gWin, tr("No address info for %1").arg(uname));
	}
}

bool
ResolverThread::PrintAddressInfo(uint32 address, bool verbose)
{
	char host[16];
	QString out;
	bool found = false;

	if (address > 0)
	{
		Inet_NtoA(address, host);
					
		if (verbose)
		{
			out += tr("Address info for %1:").arg(host);
				
			QString qhost = ResolveHost(address);
			if (qhost != QString::null)
			{
				out += "\n" + tr("Host Name: %1").arg(qhost);
				found = true;
			}

			QString aliases = ResolveAliases(address);

			if (!aliases.isEmpty())
			{	
				aliases.replace(QRegExp(","), " ");

				out += "\n" + tr("Aliases: %1").arg(aliases);
				found = true;
			}
					
			// List all users from this ip
						
			WUserMap cmap;
			gWin->fNetClient->FindUsersByIP(cmap, host);
			if (cmap.GetNumItems() > 0)
			{
				out += "\n" + tr("Connected users:");

				WUserIter it = cmap.GetIterator(HTIT_FLAG_NOREGISTER);
				while ( it.HasMoreValues() )
				{
					WUserRef uref;
					it.GetNextValue(uref);
					if ( uref() )
					{
						QString uid = uref()->GetUserID();
						QString uname = uref()->GetUserName();
						uint32 port = uref()->GetPort();
						out += "\n" + tr("#%1 - %2").arg(uid).arg(uname);
						if (port != 0)
						{	
							out += " ";
							out += tr("(port: %1)").arg(port);
						}
						found = true;
					}
				}
			}
		}
		else
		{
			out += tr("IP Address: %1").arg(host);
			found = true;
		}
		if (found)
		{
			SystemEvent(gWin, FixString(out));
		}
	}
	return found;
}

void
ResolverThread::QueryEntry(const QString &user, bool verbose)
{
	int numMatches;
	WUserMap wmap;
	numMatches = gWin->FillUserMap(user, wmap);
	uint32 address = 0;
	if (numMatches > 0)	
	{
		// Found atleast one match in users
		WUserIter uiter = wmap.GetIterator(HTIT_FLAG_NOREGISTER);
		while (uiter.HasMoreValues())
		{
			WUserRef uref;
			uiter.GetNextValue(uref);
			PrintAddressInfo(uref, verbose);
		}
	}
	else				
	{
		// Try as hostname or ip address
		address = ResolveAddress(user);
		if (!PrintAddressInfo(address, verbose) && gWin->fSettings->GetError())
		{
			ErrorEvent(gWin, tr("No address info for %1").arg(user));
		}
	}
}

void
ResolverThread::run()
{
	while (*fShutdownFlag == false)
	{
		fWaitLock.lock();
		cond.wait(&fWaitLock);
		fWaitLock.unlock();
		ResolverEntry ent;
		fQueueLock.Lock();
		status_t ret = fQueue.RemoveHead(ent);
		fQueueLock.Unlock();
		if (ret == B_NO_ERROR)
			QueryEntry(ent.user, ent.verbose);
	}
}

void
ResolverThread::wakeup()
{
//	fWaitLock.lock();
	cond.wakeOne();
//	fWaitLock.unlock();
}
