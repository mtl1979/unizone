#include "netclient.h"
#include "global.h"
#include "debugimpl.h"
#include "version.h"
#include "settings.h"
#include "lang.h"				// <postmaster@raasu.org> 20020924
#include "platform.h"			// <postmaster@raasu.org> 20021114

#include <qapplication.h>
#include "regex/PathMatcher.h"
#include "util/TimeUtilityFunctions.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "iogateway/MessageIOGateway.h"

NetClient::NetClient(QObject * owner)
{
	fPort = 0;
	fOwner = owner;
	fServer = QString::null;
}

NetClient::~NetClient()
{
	Disconnect();
}

// <postmaster@raasu.org> -- Add support for port numbers
status_t
NetClient::Connect(QString server)
{
	uint16 uiPort = 2960;
	int pColon = server.find(":");
	if (pColon >= 0)
	{
		uiPort = server.mid(pColon+1).toUShort();
		server = server.left(pColon);
	}
	return Connect(server,uiPort);
}

status_t
NetClient::Connect(QString server, uint16 port)
{
	Disconnect();
	PRINT("Starting thread\n");
	if (StartInternalThread() != B_NO_ERROR)
	{
		return B_ERROR;
	}
	PRINT("Internal thread running\n");
	PRINT("Adding new session\n");

	if (gWin->fSettings->GetChatLimit() == WSettings::LimitNone)
	{
		if (AddNewConnectSession((const char *) server.utf8(), port) != B_NO_ERROR)
		{
			return B_ERROR;
		}
	}
	else
	{
		AbstractReflectSessionRef ref(new ThreadWorkerSession(), NULL);
		ref()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway(), NULL));
		ref()->SetOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(WSettings::ConvertToBytes(
								gWin->fSettings->GetChatLimit())), NULL));
		ref()->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(WSettings::ConvertToBytes(
								gWin->fSettings->GetChatLimit())), NULL));
		if (AddNewConnectSession((const char *) server.utf8(), port, ref) != B_NO_ERROR)
			return B_ERROR;
	}
	PRINT("New session added\n");
	fServer = server;
	return B_NO_ERROR;
}

void
NetClient::Disconnect()
{
	PRINT("DISCONNECT\n");
	gWin->setCaption("Unizone");
	if (IsInternalThreadRunning()) 
	{
		ShutdownInternalThread();
		Reset(); 
		emit DisconnectedFromServer();
		PRINT("DELETING\n");
		WUserIter it = fUsers.begin();
		while (it != fUsers.end())
		{
			(*it).second()->RemoveFromListView();
			fUsers.erase(it);
			it = fUsers.begin();
		}
		PRINT("DONE\n");
	}
}

void
NetClient::SignalOwner()
{
	QCustomEvent *e = new QCustomEvent(NetClient::SignalEvent);
	if (e)
		QApplication::postEvent(fOwner, e);
}

QString
NetClient::LocalSessionID() const
{
	return fSessionID;
}

void
NetClient::AddSubscription(QString str, bool q)
{
	MessageRef ref(new Message(PR_COMMAND_SETPARAMETERS), NULL);
	if (ref())
	{
		ref()->AddBool((const char *) str.utf8(), true);		// true doesn't mean anything
		if (q)
			ref()->AddBool(PR_NAME_SUBSCRIBE_QUIETLY, true);		// no initial response
		SendMessageToSessions(ref);
	}
}

void
NetClient::RemoveSubscription(QString str)
{
	MessageRef ref(new Message(PR_COMMAND_REMOVEPARAMETERS), NULL);
	if (ref())
	{
		ref()->AddString(PR_NAME_KEYS, (const char *) str.utf8());
		SendMessageToSessions(ref);
	}
}

bool
NetClient::ExistUser(QString sessionID)
{
	QString id = sessionID;
	if (fUsers.find(id) != fUsers.end())
		return true;
	return false;
}

WUserRef
NetClient::FindUser(QString sessionID)
{
	QString id = sessionID;
	WUserIter iter = fUsers.find(id);
	if (iter != fUsers.end())
		return (*iter).second;
	return WUserRef(NULL, NULL);
}

void 
NetClient::FindUsersByIP(WUserMap & umap, QString ip)
{
	for (WUserIter iter = fUsers.begin(); iter != fUsers.end(); iter++)
	{
		if ((*iter).second()->GetUserHostName() == ip)
		{
			WUserPair p = MakePair((*iter).second()->GetUserID(), (*iter).second);
			umap.insert(p);
			//return (*iter).second;
		}
	}
	return;
	// return WUserRef(NULL, NULL);
}

// will insert into list if successful
WUserRef
NetClient::CreateUser(QString sessionID)
{
	WUser * n = new WUser(sessionID);
	if (n)
	{
		n->SetUserID(sessionID);
		WUserRef nref(n, NULL);
		WUserPair pair = MakePair(sessionID, nref);

		fUsers.insert(pair);
		emit UserConnected(pair.first);
	}
	return WUserRef(n, NULL);	// NULL, or a valid user
}

void
NetClient::RemoveUser(QString sessionID)
{
	WUserIter iter = fUsers.find(sessionID);
	if (iter != fUsers.end())
	{
		emit UserDisconnected((*iter).first, (*iter).second()->GetUserName());
		PRINT("NetClient::RemoveUser: Removing from list\n");
		(*iter).second()->RemoveFromListView();
		PRINT("NetClient::RemoveUser: Erasing\n");
		fUsers.erase(iter);
		PRINT("NetClient::RemoveUser: Done\n");
	}
}


void
NetClient::HandleResultMessage(MessageRef & ref)
{
	String nodePath;
	// remove all the items that need to be removed
	for (int i = 0; (ref()->FindString(PR_NAME_REMOVED_DATAITEMS, i, nodePath) == B_OK); i++)
	{
		int pd = GetPathDepth(nodePath.Cstr());
		if (pd >= USER_NAME_DEPTH)
		{
			QString sid = QString::fromUtf8(GetPathClause(SESSION_ID_DEPTH, nodePath.Cstr()));
			sid = sid.mid(0, sid.find('/') );

			switch (pd)
			{
				case USER_NAME_DEPTH:
				{
					if (!strncmp(GetPathClause(USER_NAME_DEPTH, nodePath.Cstr()), "name", 4))
					{
						// user removed
						RemoveUser(sid);
					}
					break;
				}

				case FILE_INFO_DEPTH: 
				{
					const char * fileName = GetPathClause(FILE_INFO_DEPTH, nodePath.Cstr());
					emit RemoveFile(sid, fileName);
					break;
				}
			}
		}
	}

	// look for addition messages
	MessageFieldNameIterator iter = ref()->GetFieldNameIterator(B_MESSAGE_TYPE);
	while (iter.GetNextFieldName(nodePath) == B_OK)
	{
		int pd = GetPathDepth(nodePath.Cstr());
		if (pd >= USER_NAME_DEPTH)
		{
			MessageRef tmpRef;
			if (ref()->FindMessage(nodePath.Cstr(), tmpRef) == B_OK)
			{
				const Message * pmsg = tmpRef.GetItemPointer();
				QString sid = GetPathClause(SESSION_ID_DEPTH, nodePath.Cstr());
				sid = sid.left(sid.find('/'));
				switch (pd)
				{
					case USER_NAME_DEPTH:
					{
						QString hostName = QString::fromUtf8(GetPathClause(NetClient::HOST_NAME_DEPTH, nodePath.Cstr()));
						hostName = hostName.left(hostName.find('/'));

						WUserRef user = FindUser(sid);
						if (!user())	// doesn't exist
						{
							user = CreateUser(sid);
							if (!user())	// couldn't create?
								break;	// oh well
							user()->SetUserHostName(hostName);
						}
						
						QString nodeName = QString::fromUtf8(GetPathClause(USER_NAME_DEPTH, nodePath.Cstr()));
						if (nodeName.lower().left(4) == "name")
						{
							QString oldname = user()->GetUserName();
							user()->InitName(pmsg); 
							emit UserNameChanged(sid, oldname, user()->GetUserName());
						}
						else if (nodeName.lower().left(10) == "userstatus")
						{
							user()->InitStatus(pmsg); 
							emit UserStatusChanged(sid, user()->GetUserName(), user()->GetStatus());
						}
						else if (nodeName.lower().left(11) == "uploadstats")
						{
							user()->InitUploadStats(pmsg);
						}
						else if (nodeName.lower().left(9) == "bandwidth")
						{
							user()->InitBandwidth(pmsg);
						}
						else if (nodeName.lower().left(9) == "filecount")
						{
							user()->InitFileCount(pmsg);
						}
						else if (nodeName.lower().left(5) == "fires")
						{
							user()->SetFirewalled(true);
						}
						else if (nodeName.lower().left(5) == "files")
						{
							user()->SetFirewalled(false);
						}
					}
					break;
	
					case FILE_INFO_DEPTH:
					{
						QString fileName = QString::fromUtf8(GetPathClause(FILE_INFO_DEPTH, nodePath.Cstr()));
						
						emit AddFile(sid, fileName, (GetPathClause(USER_NAME_DEPTH, nodePath.Cstr())[2] == 'r')? true : false, tmpRef);
						break;
					}
				}
			}
		}
	}
}

void
NetClient::HandleParameters(MessageRef & next)
{
	PRINT("PR_RESULT_PARAMETERS received\n");
	PRINT("Extracting session id\n");
	const char * sessionRoot;

	if (next()->FindString(PR_NAME_SESSION_ROOT, &sessionRoot) == B_OK)
	{
		// returns something like
		// "ip.ip.ip.ip/number" (eg: "/127.172.172.172/1234")
		const char * id = strrchr(sessionRoot, '/');	// get last slash
		if (id)
		{
			fSessionID = id + 1;
			PRINT("My ID is: %s\n", fSessionID.latin1());
			gWin->setCaption( QObject::tr("Unizone - User #%1 on %2").arg(fSessionID).arg(GetServer()) );
		}
	}
}

void
NetClient::SendChatText(QString target, QString text)
{
	if (IsInternalThreadRunning())
	{
		MessageRef chat(new Message(NEW_CHAT_TEXT), NULL);
		if (chat())
		{
			QString tostr = "/*/";
			tostr += target;
			tostr += "/beshare";
			chat()->AddString(PR_NAME_KEYS, (const char *) tostr.utf8());
			chat()->AddString("session", (const char *) fSessionID.utf8());
			chat()->AddString("text", (const char *) text.utf8());
			if (target != "*")
				chat()->AddBool("private", true);
			SendMessageToSessions(chat);
		}
	}
}

void
NetClient::SendPing(QString target)
{
	if (IsInternalThreadRunning())
	{
		MessageRef ping(new Message(PING), NULL);
		if (ping())
		{
			QString to("/*/");
			to += target;
			to += "/beshare";
			ping()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			ping()->AddString("session", (const char *) LocalSessionID().utf8());
			ping()->AddInt64("when", GetCurrentTime64());
			SendMessageToSessions(ping);
		}
	}
}

void
NetClient::SetUserName(QString user)
{
	// change the user name
	MessageRef ref(new Message(), NULL);
	if (ref())
	{
		ref()->AddString("name", (const char *) user.utf8()); // <postmaster@raasu.org> 20021001
		ref()->AddInt32("port", fPort);
		ref()->AddInt64("installid", 0);
		ref()->AddString("version_name", NAME);	// "secret" WinShare version data (so I don't have to ping Win/LinShare users
		ref()->AddString("version_num", WinShareVersionString());
		ref()->AddBool("supports_partial_hashing", true);		// 64kB hash sizes
		ref()->AddBool("firewalled", gWin->fSettings->GetFirewalled()); // is firewalled user, needed if no files shared

		SetNodeValue("beshare/name", ref);
	}
}

void
NetClient::SetUserStatus(QString status)
{
	MessageRef ref(new Message(), NULL);
	if (ref())
	{
		ref()->AddString("userstatus", (const char *) status.utf8()); // <postmaster@raasu.org> 20021001
		SetNodeValue("beshare/userstatus", ref);
	}
}

void
NetClient::SetConnection(QString connection)
{
	MessageRef ref(new Message(), NULL);
	if (ref())
	{
		ref()->AddString("label", (const char *) connection.utf8());
		ref()->AddInt32("bps", 0);

		SetNodeValue("beshare/bandwidth", ref);
	}
}

void
NetClient::SetNodeValue(const char * node, MessageRef & val)
{
	MessageRef ref(new Message(PR_COMMAND_SETDATA), NULL);
	if (ref())
	{
		ref()->AddMessage(node, val);
		SendMessageToSessions(ref);
	}
}

void
NetClient::SetFileCount(int32 count)
{
	MessageRef fc(new Message(), NULL);
	fc()->AddInt32("filecount", count);
	SetNodeValue("beshare/filecount", fc);
}

void
NetClient::SetLoad(int32 num, int32 max)
{
	MessageRef load(new Message(), NULL);
	if (load())
	{
		load()->AddInt32("cur", num);
		load()->AddInt32("max", max);
		SetNodeValue("beshare/uploadstats", load);
	}
}


