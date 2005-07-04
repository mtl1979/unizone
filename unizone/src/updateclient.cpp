#include "updateclient.h"
#include "debugimpl.h"
#include "global.h"
#include "winsharewindow.h"
#include "version.h"
#include "wsystemevent.h"

#include "iogateway/PlainTextMessageIOGateway.h"

#include <qapplication.h>

UpdateClient::UpdateClient(QObject *owner)
: QObject(owner)
{
	setName( "UpdateClient" );

	// QMessageTransceiverThread

	qmtt = new QMessageTransceiverThread(this);
	CHECK_PTR(qmtt);

	connect(qmtt, SIGNAL(MessageReceived(MessageRef, const String &)),
			this, SLOT(MessageReceived(MessageRef, const String &)));

	connect(qmtt, SIGNAL(SessionConnected(const String &)),
			this, SLOT(SessionConnected(const String &)));

	connect(qmtt, SIGNAL(SessionDetached(const String &)),
			this, SLOT(SessionDetached(const String &)));
}

UpdateClient::~UpdateClient()
{
	qmtt->ShutdownInternalThread();
}

void
UpdateClient::MessageReceived(MessageRef msg, const String & /* sessionID */)
{
	PRINT("Update thread received a message\n");
	String str;

	for (int i = 0; msg()->FindString(PR_NAME_TEXT_LINE, i, str) == B_OK; i++)
	{
		QString s;
		if (CheckVersion(str, &s))
		{
			gWin->SendSystemEvent(tr("Unizone (English) %1 is available at http://www.raasu.org/tools/windows/.").arg(s));
		}
	}
}

void
UpdateClient::SessionConnected(const String & /* sessionID */)
{
	PRINT("Update thread connected\n");
	MessageRef ref(GetMessageFromPool());
	if (ref())
	{
		String cmd("GET " UPDATE_FILE " HTTP/1.1\nUser-Agent: Unizone/");
		cmd += UZ_MajorVersion();
		cmd += ".";
		cmd += UZ_MinorVersion();
		cmd += "\nHost: ";
		cmd += fHostName;
		cmd += "\n\n";
		ref()->AddString(PR_NAME_TEXT_LINE, cmd);
		qmtt->SendMessageToSessions(ref);
	}
}

void
UpdateClient::SessionDetached(const String & /* sessionID */)
{
	PRINT("Update thread disconnected\n");
	fHostName = "";
	Reset();
}

void
UpdateClient::Disconnect()
{
	PRINT("DISCONNECT\n");
	if (qmtt->IsInternalThreadRunning()) 
	{
		fHostName = "";
		Reset(); 
	}
}

bool
UpdateClient::CheckVersion(const String & buf, QString * version)
{
	int maj, min, rev, build;
	int kMajor, kMinor, kPatch, kBuild;
	kMajor = UZ_MajorVersion();
	kMinor = UZ_MinorVersion();
	kPatch = UZ_Patch();
	kBuild = UZ_Build();
	int ret = sscanf(buf.Cstr(), "%d,%d,%d,%d", &maj, &min, &rev, &build);
	PRINT("CheckVersion: ret == %d\n", ret);
	if (ret == 4)	// we want 4 return values
	{
		PRINT("Checking version %d.%d.%d.%d\n", maj, min, rev, build);
		if (maj == kMajor)
		{
			if (min == kMinor)
			{
				if (rev == kPatch)
				{
					if (build <= kBuild)
						return false;
				}
				else if (rev < kPatch)
				{
					return false;
				}
			}
			else if (min < kMinor)
			{
				return false;
			}
		}
		else if (maj < kMajor)
		{
			return false;
		}
		if (version)
			*version = tr("%1.%2.%3 build %4").arg(maj).arg(min).arg(rev).arg(build);
		return true;
	}
	return false;
}

status_t 
UpdateClient::StartInternalThread() 
{
	return qmtt->StartInternalThread(); 
}
	
status_t 
UpdateClient::AddNewConnectSession(const String & targetHostName, uint16 port, AbstractReflectSessionRef optSessionRef)
{
	fHostName = targetHostName;
	return qmtt->AddNewConnectSession(targetHostName, port, optSessionRef);
}

void 
UpdateClient::Reset()
{
	qmtt->Reset();
}

