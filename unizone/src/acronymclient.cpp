#include "acronymclient.h"
#include "debugimpl.h"
#include "events.h"
#include "global.h"
#include "winsharewindow.h"
#include "version.h"
#include "wsystemevent.h"
#include "resolver.h"
#include "util.h"
#include "wstring.h"

#ifdef WIN32
#include "windows\vsscanf.h"
#endif

#include "iogateway/PlainTextMessageIOGateway.h"

AcronymClient * GetAcronymFromPool();
void PutToAcronymPool(AcronymClient *);

AcronymClient::AcronymClient(QObject *owner)
: QObject(owner)
{
	setName( "AcronymClient" );

	fPage = 1;

	// QMessageTransceiverThread

	qmtt = new QMessageTransceiverThread(this);
	CHECK_PTR(qmtt);

	connect(qmtt, SIGNAL(MessageReceived(const MessageRef &, const String &)),
			this, SLOT(MessageReceived(const MessageRef &, const String &)));

	connect(qmtt, SIGNAL(SessionConnected(const String &)),
			this, SLOT(SessionConnected(const String &)));

	connect(qmtt, SIGNAL(SessionDetached(const String &)),
			this, SLOT(SessionDetached(const String &)));
}

AcronymClient::~AcronymClient()
{
	qmtt->ShutdownInternalThread();
}

void
AcronymClient::SetAcronym(const QString &acronym)
{
	fAcronym = acronym;
}

void
AcronymClient::SetPage(int page)
{
	fPage = page;
}

void
AcronymClient::MessageReceived(const MessageRef & msg, const String & /* sessionID */)
{
	PRINT("Update thread received a message\n");
	QString str;

	for (int i = 0; GetStringFromMessage(msg, PR_NAME_TEXT_LINE, i, str) == B_OK; i++)
	{
		ParseLine(str);
	}
}

void
AcronymClient::SessionConnected(const String & /* sessionID */)
{
	PRINT("Acronym thread connected\n");
	MessageRef ref(GetMessageFromPool());
	if (ref())
	{
		String query("http://www.acronymfinder.com/af-query.asp?String=exact&Acronym=");
		query << (const char *) fAcronym.utf8();
		if (fPage > 1)
		{
			query << "&page=";
			query << fPage;
		}
		query << "&Find=Find";
		String cmd("GET ");
		cmd << query;
		cmd << " HTTP/1.1\nUser-Agent: Unizone/";
		cmd << UZ_MajorVersion();
		cmd << ".";
		cmd << UZ_MinorVersion();
		cmd << "\nHost: ";
		cmd << fHostName;
		cmd << "\n\n";
		ref()->AddString(PR_NAME_TEXT_LINE, cmd);
		qmtt->SendMessageToSessions(ref);
	}
}

void
AcronymClient::SessionDetached(const String & /* sessionID */)
{
	PRINT("Update thread disconnected\n");
	fHostName = "";
	Reset();
}

void
AcronymClient::Disconnect()
{
	PRINT("DISCONNECT\n");
	if (qmtt->IsInternalThreadRunning()) 
	{
		fHostName = "";
		Reset(); 
	}
}



status_t 
AcronymClient::StartInternalThread() 
{
	return qmtt->StartInternalThread(); 
}
	
status_t 
AcronymClient::AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef)
{
	fHostName = targetHostName;
	return qmtt->AddNewConnectSession(ResolveAddress(targetHostName), port, optSessionRef);
}

void 
AcronymClient::Reset()
{
	qmtt->Reset();
	PutToAcronymPool(this);
}

void
AcronymClient::ParseLine(const QString &line)
{
	if (line.contains("<td valign=\"middle\" width=\"70"))
	{
		QString out(fAcronym);
		out += ": ";
		out += line.mid(line.find(">") + 1);
		SystemEvent(gWin,out);
	} else if (line.contains("exact&s=r&page"))
	{
		int s = line.find("exact&s=r&page");
		QString pg = line.mid(s + 15);
		pg.truncate(pg.find(">"));
		int p = pg.toInt(NULL);
		if (p > fPage)
		{
			QueryAcronym(fAcronym, p);
		}
		else
		{
			int np = line.find("exact&s=r&page", s + 1); 
			if (np > 0)
			{
				ParseLine(line.mid(np));
			}
		}
	}
}

void
QueryAcronym(const QString &q, int page)
{
	AbstractReflectSessionRef acref(new ThreadWorkerSession());
	acref()->SetGateway(AbstractMessageIOGatewayRef(new PlainTextMessageIOGateway));
	AcronymClient *ac = GetAcronymFromPool();
	ac->SetAcronym(q);
	ac->SetPage(page);
	ac->StartInternalThread();
	ac->AddNewConnectSession("www.acronymfinder.com", 80, acref);
}

Queue<AcronymClient *> AcronymPool;
Mutex PoolMutex;

AcronymClient *
GetAcronymFromPool()
{
	PoolMutex.Lock();
	AcronymClient * ac = NULL;
	if (AcronymPool.GetNumItems() > 0)
	{
		AcronymPool.RemoveHead(ac);
	}
	PoolMutex.Unlock();
	if (ac == NULL)
	{
		ac = new AcronymClient(NULL);
	}
	return ac;
}

void
PutToAcronymPool(AcronymClient * ac)
{
	PoolMutex.Lock();
	AcronymPool.AddTail(ac);
	PoolMutex.Unlock();
}
