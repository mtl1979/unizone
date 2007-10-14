#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "serverclient.h"
#include "debugimpl.h"
#include "global.h"
#include "winsharewindow.h"
#include "version.h"
#include "resolver.h"
#include "settings.h"
#include "tokenizer.h"
#include "util.h"
#include "wstring.h"

#include "iogateway/PlainTextMessageIOGateway.h"
#include "qtsupport/QMessageTransceiverThread.h"

using namespace muscle;

ServerClient::ServerClient(QObject *owner)
: QObject(owner)
{
	setName( "ServerClient" );

	// QMessageTransceiverThread

	qmtt = new QMessageTransceiverThread(this, "QMessageTransceiverThread");
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
		QString nstr;
		for (int i = 0; GetStringFromMessage(msg, PR_NAME_TEXT_LINE, i, nstr) == B_OK; i++)
		{
#ifdef _DEBUG
			WString wstr(nstr);
			PRINT("UPDATESERVER: %S\n", wstr.getBuffer());
#endif
			int hind = nstr.find("#");
			if (hind >= 0)
				nstr.truncate(hind);
			if (nstr.startsWith("beshare_"))
			{
				QStringTokenizer tok(nstr.mid(8), "=");
				QString param = tok.GetNextToken();
				if (!param.isEmpty())
				{
					QString val = tok.GetRemainderOfString();
					QString qkey = param.stripWhiteSpace();
					QString qval = val.isEmpty() ? QString::null : val.stripWhiteSpace();
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
		if (fHostPort != 80)
		{
			cmd << ":";
			cmd << (int) fHostPort;
		}
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
ServerClient::AddNewConnectSession(const String & targetHostName, uint16 port, ThreadWorkerSessionRef optSessionRef)
{
	fHostName = targetHostName;
	fHostPort = port;
	uint32 _port;
	if ((_port = gWin->Settings()->GetHTTPPort()) == 0)
	{
		return qmtt->AddNewConnectSession(ResolveAddress(targetHostName), port, optSessionRef);
	}
	else
	{
		QString proxy = gWin->Settings()->GetHTTPProxy();
		return qmtt->AddNewConnectSession(ResolveAddress(proxy), _port, optSessionRef);
	}
}

void 
ServerClient::Reset()
{
	qmtt->Reset();
}
