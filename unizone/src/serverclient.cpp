#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "serverclient.h"
#include "debugimpl.h"
#include "global.h"
#include "winsharewindow.h"
#include "version.h"
#include "resolver.h"

#include "iogateway/PlainTextMessageIOGateway.h"
#include "qtsupport/QMessageTransceiverThread.h"
#include "util/StringTokenizer.h"

using namespace muscle;

ServerClient::ServerClient(QObject *owner)
: QObject(owner)
{
	setName( "ServerClient" );

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

ServerClient::~ServerClient()
{
	qmtt->ShutdownInternalThread();
}

void
ServerClient::MessageReceived(const MessageRef & msg, const String & /* sessionID */)
{
	if (msg())
	{
		String nstr;
		for (int i = 0; msg()->FindString(PR_NAME_TEXT_LINE, i, nstr) == B_OK; i++)
		{
			PRINT("UPDATESERVER: %s\n", nstr.Cstr());
			int hind = nstr.IndexOf("#");
			if (hind >= 0)
				nstr = nstr.Substring(0, hind);
			if (nstr.StartsWith("beshare_"))
			{
				StringTokenizer tok(nstr() + 8, "=");
				const char * param = tok.GetNextToken();
				if (param)
				{
					const char * val = tok.GetRemainderOfString();
					QString qkey = QString::fromUtf8(param).stripWhiteSpace();
					QString qval = val ? QString::fromUtf8(val).stripWhiteSpace() : "";
					gWin->GotUpdateCmd(qkey, qval);
				}
			}
		}
	}
}

void
ServerClient::SessionConnected(const String & /* sessionID */)
{
	MessageRef msgref(GetMessageFromPool());
	if (msgref())
	{
		String cmd("GET /servers.txt HTTP/1.1\nUser-Agent: Unizone/");
		cmd << UZ_MajorVersion();
		cmd << ".";
		cmd << UZ_MinorVersion();
		cmd << "\nHost: ";
		cmd << fHostName;
		cmd << "\n\n";
		msgref()->AddString(PR_NAME_TEXT_LINE, cmd);
		qmtt->SendMessageToSessions(msgref);
	}
}

void
ServerClient::SessionDetached(const String & /* sessionID */)
{
	fHostName = "";
	Reset();
}

void
ServerClient::Disconnect()
{
	PRINT("DISCONNECT\n");
	if (qmtt->IsInternalThreadRunning()) 
	{
		fHostName = "";
		Reset(); 
	}
}

status_t 
ServerClient::StartInternalThread() 
{
	return qmtt->StartInternalThread(); 
}

status_t 
ServerClient::AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef)
{
	fHostName = targetHostName;
	return qmtt->AddNewConnectSession(ResolveAddress(targetHostName), port, optSessionRef);
}

void 
ServerClient::Reset()
{
	qmtt->Reset();
}
