#include "updateclient.h"
#include "debugimpl.h"
#include "global.h"
#include "winsharewindow.h"
#include "version.h"
#include "iogateway/PlainTextMessageIOGateway.h"

UpdateClient::UpdateClient(QObject *owner)
: QMessageTransceiverThread(owner)
{
	setName( "UpdateClient" );
}

UpdateClient::~UpdateClient()
{
}

void
UpdateClient::MessageReceived(MessageRef msg, const String &sessionID)
{
	PRINT("Update thread received a message\n");
	String str;

	for (int i = 0; msg()->FindString(PR_NAME_TEXT_LINE, i, str) == B_OK; i++)
	{
		QString s;
		if (CheckVersion(str.Cstr(), &s))
			gWin->PrintSystem(tr("Unizone (English) %1 is available at http://www.raasu.org/tools/windows/.").arg(s));
	}
}

void
UpdateClient::SessionConnected(const String &sessionID)
{
	PRINT("Update thread connected\n");
	MessageRef ref(new Message, NULL);
	if (ref())
	{
		ref()->AddString(PR_NAME_TEXT_LINE, "GET " UPDATE_FILE " HTTP/1.1\nUser-Agent: Unizone/1.2\nHost: " UPDATE_SERVER "\n\n");
		SendMessageToSessions(ref);
	}
}

void
UpdateClient::SessionDetached(const String &sessionID)
{
	PRINT("Update thread disconnected\n");
	Reset();
}

void
UpdateClient::Disconnect()
{
	PRINT("DISCONNECT\n");
	if (IsInternalThreadRunning()) 
	{
		ShutdownInternalThread();
		Reset(); 
	}
}

bool
UpdateClient::CheckVersion(const char * buf, QString * version)
{
	int maj, min, rev, build;
	int ret = sscanf(buf, "%d,%d,%d,%d", &maj, &min, &rev, &build);
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
