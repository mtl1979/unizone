#include "netclient.h"

#include "regex/PathMatcher.h"
#include "util/TimeUtilityFunctions.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "iogateway/MessageIOGateway.h"
#include "system/SystemInfo.h"
#include "zlib/ZLibUtilityFunctions.h"
#include <QTimerEvent>
#include <QCustomEvent>
#include <QEvent>

#include "downloadimpl.h"
#include "events.h"
#include "version.h"
#include "debugimpl.h"
#include "settings.h"
#include "messageutil.h"
#include "util.h"
#include "wstring.h"
#include "werrorevent.h"
#include "wmessageevent.h"
#include "wcrypt.h"
#include "resolver.h"

// forward declaration
WinShareWindow * GetWindow(QObject *);

NetClient::NetClient(QObject * owner)
: QObject(owner)
{
	setName( "NetClient" );

	fPort = 0;
	fServerPort = 0;
	fOwner = owner;
	fServer = QString::null;
	fOldID = QString::null;
	fSessionID = QString::null;
	fUserName = QString::null;
	fLocalIP = QString::null;
	timerID = 0;
	hasmessages = false;
	fLoggedIn = false;
	fLoginTime = 0;
	fIdles = 0;
	fChannelLock.Lock();
	fChannels = GetMessageFromPool();
	fChannelLock.Unlock();

	qmtt = NULL;
}

NetClient::~NetClient()
{
	Disconnect();
	WaitForInternalThreadToExit();
}

// <postmaster@raasu.org> -- Add support for port numbers
status_t
NetClient::Connect(const QString & server)
{
	uint16 uiPort = 2960;
	QString host = server;

	int pColon = server.find(":");
	if (pColon >= 0)
	{
		uiPort = server.mid(pColon+1).toUShort();
		host = server.left(pColon);
	}
	return Connect(host, uiPort);
}

status_t
NetClient::Connect(const QString & server, uint16 port)
{
	PRINT("NetClient::Connect()\n");
	Disconnect();

	PRINT("Creating thread\n");
	// QMessageTransceiverThread

	qmtt = new QMessageTransceiverThread(this, "QMessageTransceiverThread");
	Q_CHECK_PTR(qmtt);

	connect(qmtt, SIGNAL(MessageReceived(const MessageRef &, const String &)),
			this, SLOT(MessageReceived(const MessageRef &, const String &)));

	connect(qmtt, SIGNAL(SessionAttached(const String &)),
			this, SLOT(SessionAttached(const String &)));

	connect(qmtt, SIGNAL(SessionDetached(const String &)),
			this, SLOT(SessionDetached(const String &)));

	connect(qmtt, SIGNAL(SessionAccepted(const String &, uint32, const IPAddressAndPort &)),
			this, SLOT(SessionAccepted(const String &, uint32, const IPAddressAndPort &)));

	connect(qmtt, SIGNAL(SessionConnected(const String &, const IPAddressAndPort &)),
			this, SLOT(SessionConnected(const String &, const IPAddressAndPort &)));

	connect(qmtt, SIGNAL(SessionDisconnected(const String &)),
			this, SLOT(SessionDisconnected(const String &)));

	connect(qmtt, SIGNAL(FactoryAttached(uint32)),
			this, SLOT(FactoryAttached(uint32)));

	connect(qmtt, SIGNAL(FactoryDetached(uint32)),
			this, SLOT(FactoryDetached(uint32)));

	connect(qmtt, SIGNAL(ServerExited()),
			this, SLOT(ServerExited()));

	connect(qmtt, SIGNAL(OutputQueuesDrained(const MessageRef &)),
			this, SLOT(OutputQueuesDrained(const MessageRef &)));

	// Setup SSL
#ifdef MUSCLE_ENABLE_SSL
	qmtt->SetSSLPublicKeyCertificate(publicKey);
#endif

	PRINT("Starting thread\n");
	if (qmtt->StartInternalThread() != B_NO_ERROR)
	{
		return B_ERROR;
	}
	PRINT("Internal thread running\n");

	PRINT("Adding new session\n");

	fIdles = 0;
	fServer = server;
	fServerPort = port;

	WinShareWindow *win = GetWindow(this);
	if (win)
	{
		if (win->fSettings->GetChatLimit() == WSettings::LimitNone)
		{
			if (qmtt->AddNewConnectSession(ResolveAddress(server), port) != B_NO_ERROR)
			{
				return B_ERROR;
			}
		}
		else
		{
			ThreadWorkerSessionRef ref(new ThreadWorkerSession());
			ref()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway()));
			ref()->SetOutputPolicy(AbstractSessionIOPolicyRef(new RateLimitSessionIOPolicy(WSettings::ConvertToBytes(
				win->fSettings->GetChatLimit()))));
			ref()->SetInputPolicy(AbstractSessionIOPolicyRef(new RateLimitSessionIOPolicy(WSettings::ConvertToBytes(
				win->fSettings->GetChatLimit()))));
			if (qmtt->AddNewConnectSession(ResolveAddress(server), port, ref) != B_NO_ERROR)
			{
				return B_ERROR;
			}
		}
	}

	PRINT("New session added\n");
	return B_NO_ERROR;
}

void
NetClient::Disconnect()
{
	PRINT("DISCONNECT\n");
	Cleanup();
	fLoggedIn = false;
	fLoginTime = 0;
	fServerVersion = 0.0;

	if (IsConnected())
	{
		WinShareWindow *win = GetWindow(this);
		if (win)
		{
			win->setWindowTitle("Unizone");
			win->setStatus(QString::null, 3);
		}
		// Reset() implies ShutdownInternalThread();
		//
		PRINT("RESETING\n");
		Reset();
		PRINT("DELETING USERS\n");
		WUserIter it = fUsers.GetIterator(HTIT_FLAG_BACKWARDS);
		while (it.HasData())
		{
			WUserRef uref = it.GetValue();
			it++;
			RemoveUser(uref);
		}
		PRINT("EMITTING DisconnectedFromServer()\n");
		emit DisconnectedFromServer();
		PRINT("DONE\n");
	}
}

QString
NetClient::LocalSessionID() const
{
	return fSessionID;
}

void
NetClient::AddSubscription(const QString & str, bool q)
{
	MessageRef ref(GetMessageFromPool(PR_COMMAND_SETPARAMETERS));
	if (ref())
	{
		ref()->AddBool((const char *) str.utf8(), true);		// true doesn't mean anything
		if (q)
			ref()->AddBool(PR_NAME_SUBSCRIBE_QUIETLY, true);		// no initial response
		SendMessageToSessions(ref);
	}
}

void
NetClient::AddSubscriptionList(const String * slist, bool q)
{
	MessageRef ref(GetMessageFromPool(PR_COMMAND_SETPARAMETERS));
	if (ref())
	{
		int n = 0;
		while (slist[n] != NULL)
			ref()->AddBool(slist[n++], true);		// true doesn't mean anything
		if (q)
			ref()->AddBool(PR_NAME_SUBSCRIBE_QUIETLY, true);		// no initial response
		SendMessageToSessions(ref);
	}
}

void
NetClient::RemoveSubscription(const QString & str)
{
	MessageRef ref(GetMessageFromPool(PR_COMMAND_REMOVEPARAMETERS));
	if (ref())
	{
		AddStringToMessage(ref, PR_NAME_KEYS, str);
		SendMessageToSessions(ref);
	}
}

bool
NetClient::ExistUser(const QString & sessionID)
{
	bool ok;
	uint32 uid = sessionID.toULong(&ok);
	if (ok)
		return fUsers.ContainsKey(uid);
	else
		return false;
}

WUserRef
NetClient::FindUser(const QString & sessionID)
{
	bool ok;
	uint32 uid = sessionID.toULong(&ok);
	if (ok)
	{
		// Make sure it's an exact match...
		if (QString::number(uid) == sessionID)
		{
			WUserRef found;
			if (fUsers.GetValue(uid, found) == B_NO_ERROR)
				return found;
		}
	}
	return WUserRef(NULL);
}

void
NetClient::FindUsersByIP(WUserMap & umap, const QString & ip)
{
	WUserIter iter = fUsers.GetIterator(HTIT_FLAG_NOREGISTER);
	while ( iter.HasData())
	{
		WUserRef uref = iter.GetValue();
		iter++;
		if (uref()->GetUserHostName() == ip)
		{
			uint32 uid = uref()->GetUserID().toULong();
			umap.Put(uid, uref);
		}
	}
}

WUserRef
NetClient::FindUserByIPandPort(const QString & ip, uint32 port)
{
	WUserIter iter = fUsers.GetIterator(HTIT_FLAG_NOREGISTER);
	while (iter.HasData())
	{
		WUserRef found = iter.GetValue();
		if (found()->GetUserHostName() == ip)
		{
			if (
				(port == 0) ||
				(found()->GetPort() == port)
			)
			{
				return found;
			}
		}
		iter++;
	}
	return WUserRef(NULL);
}

// will insert into list if successful
WUserRef
NetClient::CreateUser(const QString & sessionID)
{
	bool ok;
	uint32 uid = sessionID.toULong(&ok);
	if (ok)
	{
		WUser * n = new WUser(sessionID);
		if (n)
		{
			WUserRef nref(n);
			fUsers.Put(uid, nref);
			emit UserConnected(nref);
			return nref;
		}
	}
	return WUserRef(NULL);
}

void
NetClient::RemoveUser(const QString & sessionID)
{
	bool ok;
	uint32 uid = sessionID.toULong(&ok);
	if (ok)
	{
		WUserRef uref;
		if (fUsers.GetValue(uid, uref) == B_NO_ERROR)
		{
			RemoveUser(uref);
		}
	}
}

void
NetClient::RemoveUser(const WUserRef & user)
{
	if (user())
	{
		bool ok;
		uint32 uid = user()->GetUserID().toULong(&ok);
		if (ok)
		{
			PRINT("NetClient::RemoveUser: Signaling...\n");
			emit UserDisconnected(user);
			PRINT("NetClient::RemoveUser: Erasing\n");
			fUsers.Remove(uid);
			PRINT("NetClient::RemoveUser: Done\n");
		}
	}
}

void
NetClient::HandleBeRemoveMessage(const String & nodePath)
{
	int pd = GetPathDepth(nodePath.Cstr());
	String sid = GetPathClauseString(SESSION_ID_DEPTH, nodePath.Cstr());
	QString qsid(sid.Cstr());
#ifdef DEBUG2
	if (pd == SESSION_ID_DEPTH)
#else
	if (pd == BESHARE_HOME_DEPTH)
#endif
	{
		RemoveUser(qsid);
	}
	else if (pd == FILE_INFO_DEPTH)
	{
		String fileName = GetPathClauseString(FILE_INFO_DEPTH, nodePath.Cstr());
		QString qfile = QString::fromUtf8(fileName.Cstr());
		WUserRef uref = FindUser(qsid);
		emit RemoveFile(uref, qfile);
	}
}

void
NetClient::HandleUniRemoveMessage(const String & nodePath)
{
	int pd = GetPathDepth(nodePath.Cstr());
	if (pd >= USER_NAME_DEPTH)
	{
		String sid = GetPathClauseString(SESSION_ID_DEPTH, nodePath.Cstr());
		QString qsid(sid.Cstr());

		switch (pd)
		{
		case CHANNEL_DEPTH:
			{
				String cdata = GetPathClauseString(CHANNELDATA_DEPTH, nodePath.Cstr());
				if (cdata == "channeldata")
				{
					String channel = GetPathClauseString(CHANNEL_DEPTH, nodePath.Cstr());
					QString qchannel = QString::fromUtf8( channel.Cstr() );
					if (!qchannel.isEmpty())
					{
						// user parted channel
						RemoveChannel(qsid, qchannel);
					}
				}
				break;
			}

		}
	}
}

void
NetClient::HandleUniAddMessage(const String & nodePath, MessageRef ref)
{
	String cdata;
	PRINT("UniShare: AddMessage - node = %s\n", nodePath.Cstr());
	int pd = GetPathDepth(nodePath.Cstr());
	if (pd >= USER_NAME_DEPTH)
	{
		MessageRef tmpRef;
		if (ref()->FindMessage(nodePath.Cstr(), tmpRef) == B_OK)
		{
			String sid = GetPathClauseString(SESSION_ID_DEPTH, nodePath.Cstr());
			QString qsid(sid.Cstr());

			switch (pd)
			{
			case USER_NAME_DEPTH:
				{
					PRINT("UniShare: PathDepth == USER_NAME_DEPTH\n");
					String nodeName = GetPathClauseString(USER_NAME_DEPTH, nodePath.Cstr());
					if (nodeName.EqualsIgnoreCase("serverinfo"))
					{
						int64 rtime;
						QString user, newid, oldid;
                                                tmpRef()->FindInt64("registertime", rtime);
						GetStringFromMessage(tmpRef, "user", user);
						GetStringFromMessage(tmpRef, PR_NAME_SESSION, newid);
						if (GetStringFromMessage(tmpRef, "oldid", oldid) == B_OK)
						{
							emit UserIDChanged(oldid, newid);
						}
						WinShareWindow *win = GetWindow(this);
						if (win)
						{
							if (
								( win->GetUserName() == user ) &&
								( win->GetRegisterTime(user) <= rtime )
								)
							{
								// Collide nick
								MessageRef col(GetMessageFromPool(RegisterFail));
								if (col())
								{
									String to("/*/");
									to += sid;
									to += "/unishare";
									col()->AddString(PR_NAME_KEYS, to);
									AddStringToMessage(col, "name", win->GetUserName() );
									col()->AddInt64("registertime", win->GetRegisterTime() );
									SendMessageToSessions(col);
								}
							}
						}
					}
					break;
				}
			case CHANNEL_DEPTH:
				{
					PRINT("UniShare: PathDepth == CHANNEL_DEPTH\n");
					cdata = GetPathClauseString(CHANNELDATA_DEPTH, nodePath.Cstr());
					if (cdata == "channeldata")
					{
						QString channel = QString::fromUtf8( GetPathClauseString(CHANNEL_DEPTH, nodePath.Cstr()).Cstr() );
						if (!channel.isEmpty())
						{
							// user joined channel
							AddChannel(qsid, channel);
							QString topic, owner, admins;
							bool pub;
							if (GetStringFromMessage(tmpRef, "owner", owner) == B_OK)
								emit ChannelOwner(channel, qsid, owner);
							if (GetStringFromMessage(tmpRef, "admins", admins) == B_OK)
								emit ChannelAdmins(channel, qsid, admins);
							if (GetStringFromMessage(tmpRef, "topic", topic) == B_OK)
								emit ChannelTopic(channel, qsid, topic);
							if (tmpRef()->FindBool("public", &pub) == B_OK)
								emit ChannelPublic(channel, qsid, pub);
						}
					}

					break;
				}
			case CHANNELINFO_DEPTH:
				{
					PRINT("UniShare: PathDepth == CHANNELINFO_DEPTH\n");
					break;
				}
			}
		}
	}
}

void
NetClient::AddChannel(const QString &sid, const QString &channel)
{
	MessageRef mChannel;
	fChannelLock.Lock();
	if ( fChannels()->FindMessage((const char *) channel.utf8(), mChannel) == B_OK)
	{
		mChannel()->AddBool((const char *) sid.utf8(), true);
	}
	else
	{
		MessageRef mChannel(GetMessageFromPool());
		if (mChannel())
		{
			emit ChannelAdded(channel, sid, GetCurrentTime64());
			mChannel()->AddBool((const char *) sid.utf8(), true);
			fChannels()->AddMessage((const char *) channel.utf8(), mChannel);
		}
	}
	fChannelLock.Unlock();
}

void
NetClient::RemoveChannel(const QString &sid, const QString &channel)
{
	MessageRef mChannel;
	fChannelLock.Lock();
	if ( fChannels()->FindMessage((const char *) channel.utf8(), mChannel) == B_OK)
	{
		mChannel()->RemoveName((const char *) sid.utf8());
		if (mChannel()->GetNumNames(B_MESSAGE_TYPE) == 0)
		{
			// Last user parted, remove channel entry
			fChannels()->RemoveName((const char *) channel.utf8());
		}
	}
	fChannelLock.Unlock();
}

QString *
NetClient::GetChannelList()
{
	fChannelLock.Lock();
	int n = fChannels()->GetNumNames(B_MESSAGE_TYPE);
	QString * qChannels = new QString[n];
	int i = 0;
	String channel;
	MessageFieldNameIterator iter = fChannels()->GetFieldNameIterator(B_MESSAGE_TYPE);
	while (iter.HasData())
	{
		channel = iter.GetFieldName();
		iter++;
		qChannels[i++] = QString::fromUtf8(channel.Cstr());
	}
	fChannelLock.Unlock();
	return qChannels;
}

QString *
NetClient::GetChannelUsers(const QString & channel)
{
	MessageRef mChannel;
	fChannelLock.Lock();
	QString * users = NULL;
	if (fChannels()->FindMessage((const char *) channel.utf8(), mChannel) == B_OK)
	{
		int n = mChannel()->GetNumNames(B_BOOL_TYPE);
		users = new QString[n];
		int i = 0;
		String user;
		MessageFieldNameIterator iter = mChannel()->GetFieldNameIterator(B_BOOL_TYPE);
		while (iter.HasData())
		{
			user = iter.GetFieldName();
			iter++;
			users[i++] = QString::fromUtf8(user.Cstr());
		}
	}
	fChannelLock.Unlock();
	return users;
}

int
NetClient::GetChannelCount()
{
	fChannelLock.Lock();
	int n = fChannels()->GetNumNames(B_MESSAGE_TYPE);
	fChannelLock.Unlock();
	return n;
}

int
NetClient::GetUserCount(const QString & channel)
{
	fChannelLock.Lock();
	int n = 0;
	MessageRef mChannel;
	if (fChannels()->FindMessage((const char *) channel.utf8(), mChannel) == B_OK)
	{
		n = mChannel()->GetNumNames(B_BOOL_TYPE);
	}
	fChannelLock.Unlock();
	return n;
}

void
NetClient::HandleBeAddMessage(const String & nodePath, MessageRef ref)
{
	int pd = GetPathDepth(nodePath.Cstr());
	String sid = GetPathClauseString(SESSION_ID_DEPTH, nodePath.Cstr());
	QString qsid(sid.Cstr());

	WUserRef user = FindUser(qsid);
	if (!user())	// doesn't exist
		user = CreateUser(qsid);

#ifdef DEBUG2
	if (pd >= BESHARE_HOME_DEPTH)
	{
		String ct = GetPathClauseString(NetClient::BESHARE_HOME_DEPTH, nodePath.Cstr());
		if (ct != "beshare")
			PRINT("Unknown protocol: %s, node path = %s\n", ct.Cstr(), nodePath.Cstr());
	}
#endif

	if (pd >= HOST_NAME_DEPTH)
	{
		String host = GetPathClauseString(NetClient::HOST_NAME_DEPTH, nodePath.Cstr());
		QString hostName = QString::fromUtf8(host.Cstr());

		if (hostName != user()->GetUserHostName())
		{
			user()->SetUserHostName(hostName);
			emit UserHostName(user, hostName);
		}
	}

	if (pd >= USER_NAME_DEPTH)
	{
		MessageRef tmpRef;

		if (ref()->FindMessage(nodePath.Cstr(), tmpRef) == B_OK)
		{
			switch (pd)
			{
			case USER_NAME_DEPTH:
				{
					String nodeName = GetPathClauseString(USER_NAME_DEPTH, nodePath.Cstr());
					if (nodeName.EqualsIgnoreCase("name"))
					{
						QString oldname = user()->GetUserName();
						user()->InitName(tmpRef);
						if (oldname != user()->GetUserName())
							emit UserNameChanged(user, oldname, user()->GetUserName());
					}
					else if (nodeName.EqualsIgnoreCase("userstatus"))
					{
						QString oldstatus = user()->GetStatus();
						user()->InitStatus(tmpRef);
						if (oldstatus != user()->GetStatus())
							emit UserStatusChanged(user, user()->GetUserName(), user()->GetStatus());
					}
					else if (nodeName.EqualsIgnoreCase("uploadstats"))
					{
						user()->InitUploadStats(tmpRef);
					}
					else if (nodeName.EqualsIgnoreCase("bandwidth"))
					{
						user()->InitBandwidth(tmpRef);
					}
					else if (nodeName.EqualsIgnoreCase("filecount"))
					{
						user()->InitFileCount(tmpRef);
					}
					else if (nodeName.EqualsIgnoreCase("fires"))
					{
						user()->SetFirewalled(true);
					}
					else if (nodeName.EqualsIgnoreCase("files"))
					{
						user()->SetFirewalled(false);
					}

					TextEvent(fOwner, qsid, WTextEvent::UserUpdateEvent);
				}
				break;

			case FILE_INFO_DEPTH:
				{
					String fileName = GetPathClauseString(FILE_INFO_DEPTH, nodePath.Cstr());
					QString qfile = QString::fromUtf8(fileName.Cstr());

					MessageRef unpacked = InflateMessage(tmpRef);
					if (unpacked())
						emit AddFile(user, qfile, (GetPathClause(USER_NAME_DEPTH, nodePath.Cstr())[2] == 'r')? true : false, unpacked);
					break;
				}
			}
		}
	}
}

void
NetClient::HandleResultMessage(const MessageRef & ref)
{
	String nodePath;
	// remove all the items that need to be removed
	for (int i = 0; (ref()->FindString(PR_NAME_REMOVED_DATAITEMS, i, nodePath) == B_OK); i++)
	{
		if (GetPathDepth(nodePath.Cstr()) >= BESHARE_HOME_DEPTH)
		{
			String prot = GetPathClauseString(BESHARE_HOME_DEPTH, nodePath.Cstr());
			if (prot == "beshare")
			{
				SendEvent(this, WMessageEvent::BeRemoveMessage, nodePath);
			}
			else if (prot == "unishare")
			{
				SendEvent(this, WMessageEvent::UniRemoveMessage, nodePath);
			}
		}
		else
			SendEvent(this, WMessageEvent::UnknownRemoveMessage, nodePath);
	}

	// look for addition messages
	MessageFieldNameIterator iter = ref()->GetFieldNameIterator(B_MESSAGE_TYPE);
	while (iter.HasData())
	{
		nodePath = iter.GetFieldName();
		iter++;
		if (GetPathDepth(nodePath.Cstr()) >= BESHARE_HOME_DEPTH)
		{
			String prot = GetPathClauseString(BESHARE_HOME_DEPTH, nodePath.Cstr());
			if (prot == "beshare")
			{
				SendEvent(this, WMessageEvent::BeAddMessage, nodePath, ref);
			}
			else if (prot == "unishare")
			{
				SendEvent(this, WMessageEvent::UniAddMessage, nodePath, ref);
			}
		}
		else
			SendEvent(this, WMessageEvent::UnknownAddMessage, nodePath);
	}
}

void
NetClient::HandleParameters(const MessageRef & next)
{
	PRINT("PR_RESULT_PARAMETERS received\n");

	PRINT("Extracting server version\n");
	const char * serverVersion;
	if (next()->FindString(PR_NAME_SERVER_VERSION, &serverVersion) == B_OK)
		fServerVersion = atof(serverVersion);
	else
		fServerVersion = 0.0;

	PRINT("Extracting session id\n");
	const char * sessionRoot;

	if (next()->FindString(PR_NAME_SESSION_ROOT, &sessionRoot) == B_OK)
	{
		// returns something like
		// "ip.ip.ip.ip/number" (eg: "/127.172.172.172/1234")
		String myip = GetPathClauseString(1, sessionRoot);
		String id = GetPathClauseString(2, sessionRoot);
		if (id.Length() > 0)
		{
			fOldID = fSessionID;
			fSessionID = id.Cstr();
			fLocalIP = myip.Cstr();
			fLoggedIn = true;
			fLoginTime = GetCurrentTime64();

			WinShareWindow *win = GetWindow(this);
			if (win)
			{
				if (win->fDLWindow)
				{
					// Update Local Session ID in Download Window
					win->fDLWindow->SetLocalID(fSessionID);
				}

				MessageRef uc(GetMessageFromPool());
				if (uc())
				{
					uc()->AddInt64("registertime", win->GetRegisterTime(fUserName));
					AddStringToMessage(uc, PR_NAME_SESSION, fSessionID);
					if ((fOldID != QString::null) && (fOldID != fSessionID))
					{
						AddStringToMessage(uc, "oldid", fOldID);
					}
					AddStringToMessage(uc, "name", fUserName);

					SetNodeValue("unishare/serverinfo", uc);
				}
			}

			if ((fOldID != QString::null) && (fOldID != fSessionID))
			{
				emit UserIDChanged(fOldID, fSessionID);
				fOldID = fSessionID;
			}

#ifdef _DEBUG
			WString wsession(fSessionID);
			PRINT("My ID is: %S\n", wsession.getBuffer());
#endif

			if (win)
			{
				if (win->fSettings->GetIPAddresses())
				{
					QString temp(myip.Cstr());
					temp += " : ";
					temp += GetServerIP();
					win->setStatus( temp , 2);
				}
				win->setWindowTitle( tr("Unizone - User #%1 on %2").arg(fSessionID).arg(GetServer()) );
			}
		}
	}
}

void
NetClient::SendChatText(const QString & target, const QString & text, bool enc)
{
	if (IsConnected())
	{
		MessageRef chat(GetMessageFromPool(NEW_CHAT_TEXT));
		if (chat())
		{
			QString tostr = "/*/";
			tostr += target;
			tostr += "/beshare";
			AddStringToMessage(chat, PR_NAME_KEYS, tostr);
			AddStringToMessage(chat, PR_NAME_SESSION, fSessionID);
			if (enc)
			{
				QString tmp = wencrypt2(text);
				AddStringToMessage(chat, "enctext", tmp);
			}
			else
				AddStringToMessage(chat, "text", text);
			if (target != "*")
				chat()->AddBool("private", true);
			SendMessageToSessions(chat);
		}
	}
}

void
NetClient::SendPicture(const QString & target, const ByteBufferRef &buffer, const QString &name)
{
	if (IsConnected())
	{
		MessageRef pic(GetMessageFromPool(NEW_PICTURE));
		if (pic())
		{
			QString tostr = "/*/";
			tostr += target;
			tostr += "/beshare";
			AddStringToMessage(pic, PR_NAME_KEYS, tostr);
			AddStringToMessage(pic, PR_NAME_SESSION, fSessionID);
			pic()->AddData("picture", B_RAW_TYPE, buffer()->GetBuffer(), buffer()->GetNumBytes());
			pic()->AddInt32("chk", CalculateFileChecksum(buffer));
			AddStringToMessage(pic, "name", name);
			if (target != "*")
				pic()->AddBool("private", true);
			SendMessageToSessions(pic);
		}
	}
}

void
NetClient::SendPing(const QString & target)
{
	if (IsConnected())
	{
		MessageRef ping(GetMessageFromPool(PING));
		if (ping())
		{
			QString to("/*/");
			to += target;
			to += "/beshare";
			AddStringToMessage(ping, PR_NAME_KEYS, to);
			AddStringToMessage(ping, PR_NAME_SESSION, LocalSessionID());
			ping()->AddInt64("when", (int64) GetCurrentTime64());
			SendMessageToSessions(ping);
		}
	}
}

void
NetClient::SetUserName(const QString & user)
{
	fUserName = user;
	// change the user name
	if (IsConnected())
	{
		WinShareWindow *win = GetWindow(this);
		if (win)
		{
			MessageRef ref(GetMessageFromPool());
			if (ref())
			{
				AddStringToMessage(ref, "name", user); // <postmaster@raasu.org> 20021001
				ref()->AddInt32("port", fPort);
				ref()->AddInt64("installid", win->fSettings->GetInstallID());
				AddStringToMessage(ref, "version_name", tr("Unizone (English)"));	// "secret" WinShare version data (so I don't have to ping Win/LinShare users
				AddStringToMessage(ref, "version_num", WinShareVersionString());
				ref()->AddString("host_os", GetOSName());
				ref()->AddBool("supports_partial_hashing", true);		// 64kB hash sizes
#ifndef DISABLE_TUNNELING
				ref()->AddBool("supports_transfer_tunneling", true);
#endif
				ref()->AddBool("firewalled", win->fSettings->GetFirewalled()); // is firewalled user, needed if no files shared

				SetNodeValue("beshare/name", ref);
			}
		}
	}
}

void
NetClient::SetUserStatus(const QString & status)
{
	if (IsConnected())
	{
		MessageRef ref(GetMessageFromPool());
		if (ref())
		{
			AddStringToMessage(ref, "userstatus", status); // <postmaster@raasu.org> 20021001
			SetNodeValue("beshare/userstatus", ref);
		}
	}
}

void
NetClient::SetConnection(const QString & connection)
{
	if (IsConnected())
	{
		MessageRef ref(GetMessageFromPool());
		if (ref())
		{
			int32 bps = BandwidthToBytes(connection);

			AddStringToMessage(ref, "label", connection);
			ref()->AddInt32("bps", bps);

			SetNodeValue("beshare/bandwidth", ref);
		}
	}
}

void
NetClient::SetNodeValue(const char * node, const MessageRef & val, int priority)
{
	if (IsConnected())
	{
		MessageRef ref(GetMessageFromPool(PR_COMMAND_SETDATA));
		if (ref())
		{
			ref()->AddMessage(node, val);
			SendMessageToSessions(ref, priority);
		}
	}
}

void
NetClient::SetFileCount(int32 count)
{
	if (IsConnected())
	{
		MessageRef fc(GetMessageFromPool());
		if (fc())
		{
			fc()->AddInt32("filecount", count);
			SetNodeValue("beshare/filecount", fc);
		}
	}
}

void
NetClient::SetLoad(int32 num, int32 max)
{
	if (IsConnected())
	{
		MessageRef load(GetMessageFromPool());
		if (load())
		{
			load()->AddInt32("cur", num);
			load()->AddInt32("max", max);
			SetNodeValue("beshare/uploadstats", load);
		}
	}
}

QString
NetClient::GetServerIP() const
{
	QString ip = "0.0.0.0";
#ifdef MUSCLE_AVOID_IPV6
	uint32 address;
#else
	muscle::IPAddress address;
#endif
	String host;;
	address = ResolveAddress(fServer);
	if (address > 0)
	{
		host = Inet_NtoA(address, true);
		ip = host.Cstr();
	}
	return ip;
}

// ----

void
NetClient::MessageReceived(const MessageRef &msg, const String & /* sessionID */)
{
	PRINT2("MTT_EVENT_INCOMING_MESSAGE\n");
	if (msg())
	{
		switch (msg()->what)
		{
			case PR_RESULT_PARAMETERS:
			{
				WinShareWindow *win = GetWindow(this);
				if (win)
				{
					if (win->GotParams())
					{
						// a /serverinfo was sent
						::SendEvent(fOwner, WMessageEvent::ServerParametersMessage, msg);
					}
					else
					{
						HandleParameters(msg);
						win->GotParams(msg);
					}
				}
				break;
			}

			case PR_RESULT_DATAITEMS:
			{
				PRINT2("PR_RESULT_DATAITEMS\n");
				HandleResultMessage(msg);

				break;
			}

			case PR_RESULT_ERRORACCESSDENIED:
			{
				PRINT("PR_RESULT_ERRORACCESSDENIED\n");
				WinShareWindow *win = GetWindow(this);
				if (win)
					win->SendErrorEvent( tr ( "Access Denied!!!" ) );

				MessageRef subMsg;
				QString action = tr( "do that to" );

				QString who;
				if (msg()->FindMessage(PR_NAME_REJECTED_MESSAGE, subMsg) == B_NO_ERROR)
				{
					if (subMsg())
					{
						switch(subMsg()->what)
						{
							case PR_COMMAND_KICK:
							{
								action = tr( "kick" );
								break;
							}

							case PR_COMMAND_ADDBANS:
							{
								action = tr( "ban" );
								break;
							}

							case PR_COMMAND_REMOVEBANS:
							{
								action = tr( "unban" );
								break;
							}

							case PR_COMMAND_ADDREQUIRES:
							{
								action = tr( "require" );
								break;
							}

							case PR_COMMAND_REMOVEREQUIRES:
							{
								action = tr( "unrequire" );
								break;
							}
						}

						if (GetStringFromMessage(subMsg, PR_NAME_KEYS, who) == B_NO_ERROR)
						{
							WinShareWindow *win = GetWindow(this);
							if (win)
								win->SendErrorEvent( tr("You are not allowed to %1 [%2]").arg(action).arg(who) );
						}
					}
				}
				break;
			}

			default:
			{
				PRINT2("Handling message\n");
				::SendEvent(fOwner, WMessageEvent::HandleMessage, msg);
				break;
			}
		}
	}
}

void
NetClient::SessionAccepted(const String & /* sessionID */, uint32 /* factoryID */, const IPAddressAndPort & /*iap*/)
{
	PRINT("MTT_EVENT_SESSION_ACCEPTED\n");
}

void
NetClient::SessionAttached(const String & /* sessionID */)
{
	PRINT("MTT_EVENT_SESSION_ATTACHED\n");
	SendSignal(NetClient::SESSION_ATTACHED);
}

void
NetClient::SessionConnected(const String & /* sessionID */, const IPAddressAndPort & /* connectedTo */)
{
	PRINT("MTT_EVENT_SESSION_CONNECTED\n");

	timerID = startTimer(15000);

	emit ConnectedToServer();
	SendSignal(NetClient::SESSION_CONNECTED);
	PRINT("Returning\n");
}

void
NetClient::SessionDisconnected(const String & /* sessionID */)
{
	PRINT("MTT_EVENT_SESSION_DISCONNECTED\n");

	Cleanup();

	SendSignal(NetClient::DISCONNECTED);
}

void
NetClient::SessionDetached(const String & /* sessionID */)
{
	PRINT("MTT_EVENT_SESSION_DETACHED\n");

	Cleanup();

	SendSignal(NetClient::DISCONNECTED);
}

void
NetClient::FactoryAttached(uint32 /* port */)
{
	PRINT("MTT_EVENT_FACTORY_ATTACHED\n");
}

void
NetClient::FactoryDetached(uint32 /* port */)
{
	PRINT("MTT_EVENT_FACTORY_DETACHED\n");
}

void
NetClient::OutputQueuesDrained(const MessageRef &/* ref */)
{
	PRINT2("MTT_EVENT_OUTPUT_QUEUES_DRAINED\n");

	fIdles = 0;
	if (IsConnected())
	{
		NetPacket np;
		const char * optDistPath = NULL;
		bool found = false;

		if (packetbuf.GetNumItems() > 0)
		{
			fPacketLock.Lock();
			PRINT2("Messages in High Priority Queue: %lu\n", packetbuf.GetNumItems());
			packetbuf.RemoveHead(np);
			fPacketLock.Unlock();

			found = true;
		}
		else if (lowpacketbuf.GetNumItems() > 0)
		{
			fLowPacketLock.Lock();
			PRINT2("Messages in Low Priority Queue: %lu\n", lowpacketbuf.GetNumItems());
			lowpacketbuf.RemoveHead(np);
			fLowPacketLock.Unlock();
			found = true;
		}

		if (np.path.Length() > 0)
			optDistPath = np.path.Cstr();

		if (found)
		{
			qmtt->SendMessageToSessions(np.mref, optDistPath);
			qmtt->RequestOutputQueuesDrainedNotification(GetMessageFromPool());
		}
		else
		{
			hasmessages = false;
			PRINT("NO MESSAGES IN QUEUE!\n");
		}
	}
}

void
NetClient::ServerExited()
{
	PRINT("MTT_EVENT_SERVER_EXITED\n");

	Cleanup();

	SendSignal(NetClient::DISCONNECTED);
}

void
NetClient::Cleanup()
{
	PRINT("NetClient::Cleanup()\n");
	if (timerID != 0)
	{
		killTimer(timerID);
		timerID = 0;
	}

	fPacketLock.Lock();
	packetbuf.Clear();
	fPacketLock.Unlock();

	fLowPacketLock.Lock();
	lowpacketbuf.Clear();
	fLowPacketLock.Unlock();

	hasmessages = false;
}

bool
NetClient::IsConnected() const
{
	return qmtt ? qmtt->IsInternalThreadRunning() : false;
}

void
NetClient::Reset()
{
	if (IsConnected())
	{
		qmtt->Reset();
		qmtt->WaitForInternalThreadToExit();
		QCustomEvent *qce = new QCustomEvent(NetClient::THREAD_EXITED);
		if (qce)
		{
			qce->setData(qmtt);
			QApplication::postEvent(this, qce);
		}
		qmtt = NULL;
	}
}

status_t
NetClient::SendMessageToSessions(const MessageRef & msgRef, int priority, const char * optDistPath)
{
	status_t ret = B_ERROR;

	if (IsConnected())
	{
		NetPacket np;
		np.mref = msgRef;
		if (optDistPath)
			np.path = optDistPath;

		if (priority == 0) // low
		{
			fLowPacketLock.Lock();
			ret = lowpacketbuf.AddTail(np);
			fLowPacketLock.Unlock();
		}
		else
		{
			fPacketLock.Lock();
			ret = packetbuf.AddTail(np);
			fPacketLock.Unlock();
		}

		if (!hasmessages)
			qmtt->RequestOutputQueuesDrainedNotification(GetMessageFromPool());

		if (ret == B_OK)
			hasmessages = true;
	}
	return ret;
}

status_t
NetClient::SetOutgoingMessageEncoding(int32 encoding, const char * optDistPath)
{
	return qmtt ? qmtt->SetOutgoingMessageEncoding(encoding, optDistPath) : B_ERROR;
}

status_t
NetClient::WaitForInternalThreadToExit()
{
	return qmtt ? qmtt->WaitForInternalThreadToExit() : B_ERROR;
}

void
NetClient::SendSignal(int signal)
{
	QCustomEvent *e = new QCustomEvent(signal);
	if (e)
		QApplication::postEvent(fOwner, e);
}

void
NetClient::timerEvent(QTimerEvent * /* e */)
{
	if (IsConnected())
	{
		// 1 minute maximum
		fIdles++;
		if (fIdles == 1)
		{
			MessageRef nop(GetMessageFromPool(PR_COMMAND_NOOP));
			if ( nop() )
			{
				SendMessageToSessions(nop);
			}
		}
		if (fIdles < 3)
			return;

		// fall through
		ErrorEvent(parent(), tr("Send Queue Overflow!"));
		Disconnect();
	}
}

bool
NetClient::event(QEvent * e)
{
	if (e->type() == (QEvent::Type) NetClient::THREAD_EXITED)
	{
		QCustomEvent *qce = dynamic_cast<QCustomEvent *>(e);
		if (qce)
			delete (QMessageTransceiverThread *) qce->data();
		return true;
	}
	else if (e->type() == (QEvent::Type) WMessageEvent::MessageEventType)
	{
		bool ret = false;
		WMessageEvent *wme = dynamic_cast<WMessageEvent *>(e);
		if (wme)
		{
			switch (wme->MessageType())
			{
			case WMessageEvent::BeRemoveMessage:
			case WMessageEvent::UnknownRemoveMessage:
				HandleBeRemoveMessage(wme->Sender());
				ret = true;
				break;
			case WMessageEvent::UniRemoveMessage:
				HandleUniRemoveMessage(wme->Sender());
				ret = true;
				break;
			case WMessageEvent::BeAddMessage:
			case WMessageEvent::UnknownAddMessage:
				HandleBeAddMessage(wme->Sender(), wme->Message());
				ret = true;
				break;
			case WMessageEvent::UniAddMessage:
				HandleUniAddMessage(wme->Sender(), wme->Message());
				ret = true;
				break;
			}
		}
		return ret;
	}
	else
		return QObject::event(e);
}

WinShareWindow *
GetWindow(QObject *obj)
{
	return dynamic_cast<WinShareWindow *>(obj->parent());
}
