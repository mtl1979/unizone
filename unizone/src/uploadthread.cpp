#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "uploadthread.h"
#include "wuploadevent.h"
#include "global.h"
#include "downloadimpl.h"
#include "settings.h"
#include "md5.h"
#include "wstring.h"
#include "filethread.h"
#include "debugimpl.h"

WUploadThread::WUploadThread(QObject * owner, bool * optShutdownFlag)
	: QMessageTransceiverThread(owner), fOwner(owner), fShutdownFlag(optShutdownFlag) 
{ 
	PRINT("WUploadThread ctor\n");
	setName( "WUploadThread" );
	fFile = NULL; 
	fFileUl = QString::null;
	fRemoteSessionID = QString::null;
	// qmtt = new QMessageTransceiverThread(this);
	// CHECK_PTR(qmtt);

	// Default status

	if (!fShutdownFlag)					// Force use of Shutdown Flag
	{
		fShutdown = false;
		fShutdownFlag = &fShutdown;
	}

	fCurFile = -1;
	fNumFiles = -1;
	fPort = 0;
	fSocket = 0;
	fForced = false;
	timerID = 0;
	fActive = false;
	fBlocked = false;
	fFinished = false;
	fManuallyQueued = false;
	fLocallyQueued = false;
	fRemotelyQueued = false;
	fDisconnected = false;
	fConnecting = true;
	fPackets = 0;
	fTXRate = 0;
	fTimeLeft = 0;
	fStartTime = 0;
	fPacket = 8;

	InitTransferRate();
	InitTransferETA();

	CTimer = new QTimer(this, "Connect Timer");
	CHECK_PTR(CTimer);

	connect( CTimer, SIGNAL(timeout()), this, SLOT(ConnectTimer()) );
	
	fBlockTimer = new QTimer(this, "Blocked Timer");
	CHECK_PTR(fBlockTimer);

	connect( fBlockTimer, SIGNAL(timeout()), this, SLOT(BlockedTimer()) );
/*
	connect(qmtt, SIGNAL(MessageReceived(MessageRef, const String &)), 
			this, SLOT(MessageReceived(MessageRef, const String &)));
	connect(qmtt, SIGNAL(SessionAttached(const String &)),
			this, SLOT(SessionAttached(const String &)));
	connect(qmtt, SIGNAL(SessionDetached(const String &)),
			this, SLOT(SessionDetached(const String &)));
	connect(qmtt, SIGNAL(SessionConnected(const String &)),
			this, SLOT(SessionConnected(const String &)));
	connect(qmtt, SIGNAL(ServerExited()),
			this, SLOT(ServerExited()));
	connect(qmtt, SIGNAL(SessionDisconnected(const String &)),
			this, SLOT(SessionDisconnected(const String &)));
	connect(qmtt, SIGNAL(OutputQueuesDrained(MessageRef)),
			this, SLOT(OutputQueuesDrained(MessageRef)));
*/
	PRINT("WUploadThread ctor OK\n");
}

WUploadThread::~WUploadThread()
{
	PRINT("WUploadThread dtor\n");

	if (timerID != 0) 
	{
		killTimer(timerID);
		timerID = 0;
	}

	if (fShutdownFlag && !*fShutdownFlag)
	{
		*fShutdownFlag = true;
	}

	if (fFile)
	{
		PRINT("\tCleaning up files...\n");
		fFile->close();
		delete fFile;
		fFile = NULL;
	}

//	qApp->sendPostedEvents( fOwner, WUploadEvent::Type );

	// Reset();

	PRINT("WUploadThread dtor OK\n");
}

void
WUploadThread::SetUpload(int socket, uint32 remoteIP, WFileThread * ft)
{
	char host[16];
	fAccept = false;
	fRemoteIP = remoteIP;
	fSocket = socket;
	fFileThread = ft;
	// Set string ip too
	uint32 _ip = GetPeerIPAddress(fSocket);
	Inet_NtoA(_ip, host);
	fStrRemoteIP = host;
}

void 
WUploadThread::SetUpload(QString remoteIP, uint32 remotePort, WFileThread * ft)
{
	fFileThread = ft;
	fAccept = true;
	fStrRemoteIP = remoteIP;
	fPort = remotePort;
}

bool 
WUploadThread::InitSession()
{
	PRINT("WUploadThread::InitSession\n");

	AbstractReflectSessionRef limit;

	// First check if IP is blacklisted or ignored
	//

	if (gWin->IsIgnoredIP(fStrRemoteIP))
	{
		SetBlocked(true);
	}

	if (gWin->IsBlackListedIP(fStrRemoteIP) && (gWin->fSettings->GetBLLimit() != WSettings::LimitNone))
	{
		AbstractReflectSessionRef ref(new ThreadWorkerSession, NULL);
		ref()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway(), NULL));
		SetRate(WSettings::ConvertToBytes(gWin->fSettings->GetBLLimit()), ref);

		limit = ref;
	}
	else if (gWin->fSettings->GetULLimit() != WSettings::LimitNone)
	{
		AbstractReflectSessionRef ref(new ThreadWorkerSession, NULL);
		ref()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway(), NULL));
		SetRate(WSettings::ConvertToBytes(gWin->fSettings->GetULLimit()), ref);
		limit = ref;
	}

	if (!fAccept)
	{
		if (AddNewSession(fSocket, limit) == B_OK && StartInternalThread() == B_OK)
		{
			MessageRef mref(GetMessageFromPool(WUploadEvent::ConnectInProgress));
			if (mref())
			{
				SendReply(mref);
			}
			CTimer->start(60000, true);
		}
		else
		{
			MessageRef fail(GetMessageFromPool(WUploadEvent::ConnectFailed));
			if (fail())
			{
				fail()->AddString("why", QT_TR_NOOP( "Could not init session!" ));
				SendReply(fail);
			}
			return false;
		}
	}
	else
	{
		const String sRemoteIP = (const char *) fStrRemoteIP.utf8(); // <postmaster@raasu.org> 20021026
		if (AddNewConnectSession(sRemoteIP, (uint16)fPort, limit) == B_OK && StartInternalThread() == B_OK)
		{
			MessageRef mref(GetMessageFromPool(WUploadEvent::ConnectInProgress));
			if (mref())
			{
				SendReply(mref);
			}
			CTimer->start(60000, true);
		}
		else
		{
			MessageRef fail(GetMessageFromPool(WUploadEvent::ConnectFailed));
			if (fail())
			{
				fail()->AddString("why", QT_TR_NOOP( "Couldn't create new connect session!" ));
				SendReply(fail);
			}
			return false;
		}
	}
	return true;
}

void
WUploadThread::SetLocallyQueued(bool b)
{
	fLocallyQueued = b;
	if (b)
	{
		fStartTime = 0;
	}
	else
	{
		fLastData.restart();
		if (fSavedFileList())
		{
			MessageRef fFileList = fSavedFileList;
			fSavedFileList.Neutralize();
			WUploadEvent *wue = new WUploadEvent(fFileList);
			if (wue) QApplication::postEvent(this, wue);
			// TransferFileList(fFileList);
			return;
		}
		else if (IsInternalThreadRunning())  // Still connected?
		{
			// Don't need to start forced or finished uploads ;)
			if (!fForced && !fFinished)
			{
				// we can start now!
				SignalUpload();
			}				
		}
		else if (fConnecting)
		{
			MessageRef con(GetMessageFromPool(WUploadEvent::ConnectInProgress));
			if (con())
			{
				SendReply(con);
			}
		}
		else if (!fFinished)
		{
			// fActive = false;
			fFinished = true;
			Reset();
			MessageRef fail(GetMessageFromPool(WUploadEvent::ConnectFailed));
			if (fail())
			{
				fail()->AddString("why", QT_TR_NOOP( "Connection reset by peer!" ));
				SendReply(fail);
			}
		}
	}
}

void
WUploadThread::SetBlocked(bool b, int64 timeLeft)
{
	SetActive(!b);
	fBlocked = b;
	if (b)
	{
		fTimeLeft = timeLeft;
		fStartTime = 0;
		if (timeLeft != -1)
			fBlockTimer->start(timeLeft/1000, true);
	}
	else if (!b)
	{
		fTimeLeft = 0;
		fLastData.restart();
		if (fBlockTimer->isActive())
		{
			fBlockTimer->stop();
		}
	}
	if (IsInternalThreadRunning())
	{
		if (b)
		{
			SendRejectedNotification(true);
		}
		else
		{
			SetLocallyQueued(true); // put in queue ;)
			// we can start now!
			// SignalUpload();
		}
	}
	else
	{
		if (b)
		{
			SendRejectedNotification(false);
		}
	}
}

void
WUploadThread::SetManuallyQueued(bool b)
{
	fManuallyQueued = b;	
	if (IsInternalThreadRunning() && b)
		SendQueuedNotification();
}


void 
WUploadThread::SendReply(MessageRef &m)
{
	if (m())
	{
		if (fShutdownFlag && *fShutdownFlag)
		{
			Reset();
//			return;
		}

//		m()->AddBool("upload", true);
		m()->AddPointer("sender", this);
		WUploadEvent * wue = new WUploadEvent(m);
		if (wue)
			QThread::postEvent(fOwner, wue);
	}
}

void
WUploadThread::SessionAttached(const String &sessionID)
{
	// If you aren't firewalled
	fConnecting = false;
	_sessionID = sessionID;
	CTimer->stop();

	timerID = startTimer(10000);

	MessageRef con(GetMessageFromPool(WUploadEvent::Connected));
	if (con())
	{
		SendReply(con);
	}
}
void
WUploadThread::SessionDetached(const String &sessionID)
{
	SessionDisconnected(sessionID);
}

void
WUploadThread::SessionConnected(const String &sessionID)
{
	// If you are firewalled
	fConnecting = false;
	_sessionID = sessionID;
	CTimer->stop();

	timerID = startTimer(10000);

	MessageRef con(GetMessageFromPool(WUploadEvent::Connected));
	if (con())
	{
		SendReply(con);
	}
}

void
WUploadThread::ServerExited()
{
	SessionDisconnected(_sessionID);
}

void
WUploadThread::SessionDisconnected(const String &sessionID)
{
	PRINT("WUploadThread::SessionDisconnected\n");

	if (timerID != 0) 
	{
		killTimer(timerID);
		timerID = 0;
	}

	*fShutdownFlag = true;
	fDisconnected = true;

	if (fActive) // Do it only once...
	{
		fActive = false;

		if (fFile)
		{
			fFile->close();
			delete fFile; 
			fFile = NULL;
		}
		
		MessageRef dis(GetMessageFromPool(WUploadEvent::Disconnected));
		if (dis())
		{
			if (fCurrentOffset < fFileSize || fUploads.GetNumItems() > 0)
			{
				dis()->AddBool("failed", true);
			}
			else
			{
				dis()->AddBool("failed", false);
			}
			SendReply(dis);
		}
	}
}

void
WUploadThread::MessageReceived(MessageRef msg, const String &sessionID)
{
	PRINT("WUploadThread::MessageReceived\n");
	switch (msg()->what)
	{
		case WDownload::TransferCommandPeerID:
		{
			PRINT("WDownload::TransferCommandPeerID\n");
			const char *name = NULL;
			const char *id = NULL;
			if (msg()->FindString("beshare:FromSession", &id) == B_OK)
			{
				fRemoteSessionID = QString::fromUtf8(id);
			}

			if (msg()->FindString("beshare:FromUserName", &name) ==  B_OK)
			{							
				QString user = QString::fromUtf8(name);
				if (user.isEmpty())
					fRemoteUser = GetUserName(fRemoteSessionID);
				else
					fRemoteUser = user; 
			}
			else
			{
				fRemoteUser = GetUserName(fRemoteSessionID);
			}

			if (gWin->IsIgnored(fRemoteSessionID, true))
			{
				SetBlocked(true);
			}

			MessageRef ui(GetMessageFromPool(WUploadEvent::UpdateUI));
			if (ui())
			{
				if (name) 
					ui()->AddString("name", name);
				if (id) 
					ui()->AddString("id", id);

				SendReply(ui);
			}

			bool c = false;
			
			if (msg()->FindBool("unishare:supports_compression", &c) == B_OK)
			{
				SetOutgoingMessageEncoding(MUSCLE_MESSAGE_ENCODING_ZLIB_9);
			}
			break;
		}
	
		case WDownload::TransferFileList:
		{
			// TransferFileList(msg);
			WUploadEvent *qce = new WUploadEvent(msg);
			if (qce) 
			{
				QApplication::postEvent(this, qce);
			}
			break;
		}
	}
}

void
WUploadThread::OutputQueuesDrained(MessageRef msg)
{
	PRINT("\tMTT_EVENT_OUTPUT_QUEUES_DRAINED\n");

	if (fWaitingForUploadToFinish)
	{
		PRINT("\tfWaitingForUploadToFinish\n");
		// qmtt->Reset();
		fFinished = true;
		// fActive = false;
		fWaitingForUploadToFinish = false;

		PRINT("\t\tSending message\n");

		MessageRef msg(GetMessageFromPool(WUploadEvent::FileDone));
		if (msg())
		{
			msg()->AddBool("done", true);
			msg()->AddString("file", (const char *) fFileUl.utf8());
			PRINT("\t\tSending...\n");
			SendReply(msg);
			PRINT("\t\tSent...\n"); // <postmaster@raasu.org> 20021023 -- Fixed typo
			PRINT("\t\tDisconnecting...\n");
			MessageRef dis(GetMessageFromPool(WUploadEvent::Disconnected));
			if (dis())
			{
				dis()->AddBool("done", true);
				SendReply(dis);
			}
			PRINT("\t\tDisconnected...\n");
		}
		else
		{
			PRINT("\t\tNot sent!\n");
		}
		PRINT("\tfWaitingForUploadToFinish OK\n");
	}
	else if (!fFinished)
	{
		DoUpload();
	}
}

void 
WUploadThread::SendQueuedNotification()
{
	MessageRef q(GetMessageFromPool(WDownload::TransferNotifyQueued));
	if (q())
	{
		SendMessageToSessions(q);
	}
	MessageRef qf(GetMessageFromPool(WUploadEvent::FileQueued));
	if (qf())
	{
		SendReply(qf);
	}
}

void
WUploadThread::SendRejectedNotification(bool direct)
{
	MessageRef q(GetMessageFromPool(WDownload::TransferNotifyRejected));

	if (!q())
		return;

	if (fTimeLeft != -1)
		q()->AddInt64("timeleft", fTimeLeft);
	if (direct || (fPort == 0))
	{
		SendMessageToSessions(q);
	}
	else
	{
		String node;
		if (fRemoteSessionID != QString::null)
		{
			node = "/*/";
			node += (const char *) fRemoteSessionID.latin1();
		}
		else
		{
			// use /<ip>/* instead of /*/<sessionid>, because session id isn't yet known at this point
			node = "/";
			node += (const char *) fStrRemoteIP.latin1();
			node += "/*";
		}
		if (
            (q()->AddString(PR_NAME_SESSION, "") == B_NO_ERROR) &&
            (q()->AddString(PR_NAME_KEYS, node) == B_NO_ERROR) &&
			(q()->AddInt32("port", (int32) fPort) == B_NO_ERROR)
			) 
		{
				gWin->SendRejectedNotification(q);
		}
	}
	MessageRef b(GetMessageFromPool(WUploadEvent::FileBlocked));
	if (b())
	{
		if (fTimeLeft != -1)
			b()->AddInt64("timeleft", fTimeLeft);
		SendReply(b);
	}
}

void 
WUploadThread::DoUpload()
{
	PRINT("WUploadThread::DoUpload\n");
	if (fShutdownFlag && *fShutdownFlag)	// Do we need to interrupt?
	{
		fFinished = true;
		MessageRef fail(GetMessageFromPool(WUploadEvent::ConnectFailed));
		if (fail())
		{
			fail()->AddString("why", QT_TR_NOOP( "Connection reset by peer!" ));
			SendReply(fail);
		}
		return;
	}

	// Still connected?

	if (!IsInternalThreadRunning())
	{
		fFinished = true;
		MessageRef fail(GetMessageFromPool(WUploadEvent::ConnectFailed));
		if (fail())
		{
			fail()->AddString("why", QT_TR_NOOP( "Connection reset by peer!" ));
			SendReply(fail);
		}
//		fActive = false;
		return;
	}

	fActive = true;

	// Small files get to bypass queue
	if (IsLocallyQueued())
	{
		if (
			(fFile && (fFileSize >= gWin->fSettings->GetMinQueuedSize())) || 
			IsManuallyQueued()
			)		
		{
			// not yet
			fForced = false;
			MessageRef lq(GetMessageFromPool(WUploadEvent::FileQueued));
			if (lq())
			{
				SendReply(lq);
			}
			return;
		}
		fForced = true;		// Set this here to avoid duplicate call to DoUpload()
	}

	// Recheck if IP is ignored or not
	//

	if (gWin->IsIgnoredIP(fStrRemoteIP) && !IsBlocked())
	{
		SetBlocked(true);
	}

	if (IsBlocked())
	{
		MessageRef msg(GetMessageFromPool(WUploadEvent::FileBlocked));
		if (msg())
		{
			if (fTimeLeft != -1)
				msg()->AddInt64("timeleft", fTimeLeft);
			SendReply(msg);
		}
		return;
	}

	if (fStartTime == 0)
		fStartTime = GetRunTime64();

	if (fFile)
	{
		MessageRef uref(GetMessageFromPool(WDownload::TransferFileData));
		if (uref())
		{
			// think about doing this in a dynamic way (depending on connection)
			uint32 bufferSize = GetPacketSize() * 1024;	
			uint8 * scratchBuffer;
			if (uref()->AddData("data", B_RAW_TYPE, NULL, bufferSize) == B_OK &&
				uref()->FindDataPointer("data", B_RAW_TYPE, (void **)&scratchBuffer, NULL) == B_OK)
			{
				int32 numBytes = fFile->readBlock((char *)scratchBuffer, bufferSize);
				if (numBytes > 0)
				{
					// munge mode
					if (fMungeMode != WDownload::MungeModeNone)
					{
						bool unknown = false;
						switch (fMungeMode)
						{
						case WDownload::MungeModeXOR:
							{
								for (int32 x = 0; x < numBytes; x++)
									scratchBuffer[x] ^= 0xFF;
								break;
							}
							
						default:
							unknown = true;
							break;
						}
						if (!unknown)
							uref()->AddInt32("mm", fMungeMode);
					}
					
					// possibly do checksums here
					
					if ((uint32)numBytes < bufferSize)
					{
						// aha, extra bytes must be removed!
						uref()->AddData("temp", B_RAW_TYPE, scratchBuffer, numBytes);
						uref()->Rename("temp", "data");
					}
					SendMessageToSessions(uref);

					// NOTE: RequestOutputQueuesDrainedNotification() can recurse, so we need to update the offset before
					//       calling it!
					fCurrentOffset += numBytes;
					MessageRef drain(GetMessageFromPool());
					if (drain())
						RequestOutputQueuesDrainedNotification(drain);

					// we'll use this event for sending as well
					MessageRef update(GetMessageFromPool(WUploadEvent::FileDataSent));	
					if (update())
					{
						update()->AddInt64("offset", fCurrentOffset);
						update()->AddInt64("size", fFileSize);
						update()->AddInt32("sent", numBytes);
						
						if (fCurrentOffset >= fFileSize)
						{
							update()->AddBool("done", true);	// file done!
							update()->AddString("file", (const char *) fFileUl.utf8());
						}
						SendReply(update);
					}
					return;
				}
				
				if (numBytes <= 0)
				{
					// next file
					fFile->close();
					delete fFile; 
					fFile = NULL;
					fCurrentOffset = fFileSize = 0;
					DoUpload();
					return;
				}
			}
		}
	}
	else
	{
		while (!fFile)
		{
			if (fUploads.GetNumItems() != 0)
			{
				// grab the ref and remove it from the list
				fUploads.RemoveHead(fCurrentRef);
				Message * m = fCurrentRef();
				String path, filename;
				m->FindString("winshare:Path", path);
				m->FindString("beshare:File Name", filename);
				String filePath = path;

				if (!filePath.EndsWith("/"))
					filePath += "/";
				
				filePath += filename;

				fFileUl = QString::fromUtf8(filePath.Cstr());

				// <postmaster@raasu.org> 20021023, 20030702 -- Add additional debug message
				WString wFileUl = fFileUl; 
				PRINT("WUploadThread::DoUpload: filePath = %S\n", wFileUl.getBuffer()); 
				
				fFile = new QFile(fFileUl);
				CHECK_PTR(fFile);
				if (!fFile->open(IO_ReadOnly))	// probably doesn't exist
				{
					delete fFile;
					fFile = NULL;
					fCurFile++;
					continue;	// onward
				}
				// got our file!
				fFileSize = fFile->size();
				fCurrentOffset = 0;	// from the start
				if (fCurrentRef()->FindInt64("secret:offset", (int64 *)&fCurrentOffset) == B_OK)
				{
					if (!fFile->at((int)fCurrentOffset)) // <postmaster@raasu.org> 20021026
					{
						fFile->at(0);	// this can't fail :) (I hope)
						fCurrentOffset = 0;
					}
				}
				// copy the message in our current file ref
				Message * msg = fCurrentRef();
				MessageRef headRef( GetMessageFromPool(*msg) );
				if (headRef())
				{
					headRef()->what = WDownload::TransferFileHeader;
					headRef()->AddInt64("beshare:StartOffset", fCurrentOffset);
					SendMessageToSessions(headRef);
				}

				fCurFile++;

				MessageRef mref(GetMessageFromPool(WUploadEvent::FileStarted));
				if (mref())
				{
					mref()->AddString("file", filePath.Cstr());
					mref()->AddInt64("start", fCurrentOffset);
					mref()->AddInt64("size", fFileSize);
					mref()->AddString("user", (const char *) fRemoteSessionID.utf8());
					SendReply(mref);
				}

				// nested call
				DoUpload();
			}
			else
			{
				PRINT("No more files!\n");
				fWaitingForUploadToFinish = true;
				// fActive = false;
				fFinished = true;
				fLocallyQueued = false;
				MessageRef drain(GetMessageFromPool());
				if (drain())
					RequestOutputQueuesDrainedNotification(drain);
				break;
			}
		}
	}
}

QString
WUploadThread::GetFileName(int i)
{
	if (i >= 0 && i < fNames.GetNumItems())
	{
		return fNames[i];
	}
	else
	{
		return QString::null;
	}
}

void
WUploadThread::SetRate(int rate)
{
	fTXRate = rate;
	if (rate != 0)
		SetNewOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		SetNewOutputPolicy(PolicyRef(NULL, NULL));
}

void
WUploadThread::SetRate(int rate, AbstractReflectSessionRef & ref)
{
	fTXRate = rate;
	if (rate != 0)
		ref()->SetOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		ref()->SetOutputPolicy(PolicyRef(NULL, NULL));
}

QString
WUploadThread::GetRemoteUser()
{
	if (fRemoteUser.isEmpty())
		return tr("User #%1").arg(fRemoteSessionID);
	else
		return fRemoteUser;
}

void
WUploadThread::timerEvent(QTimerEvent *e)
{
	if (IsInternalThreadRunning())
	{
		MessageRef nop(GetMessageFromPool(PR_COMMAND_NOOP));
		if ( nop() )
		{
			SendMessageToSessions(nop);
		}
	}
}

bool
WUploadThread::event(QEvent *e)
{
	int t = (int) e->type();
	switch (t)
	{
	case WUploadEvent::Type:
		{
			WUploadEvent * wue = dynamic_cast<WUploadEvent *>(e);
			if (wue)
			{
				TransferFileList(wue->Msg());
			}
			return true;
		}
	case UploadEvent:
		{
			DoUpload();
			return true;
		}
	default:
		{
			return QMessageTransceiverThread::event(e);
		}
	}
}

void
WUploadThread::TransferFileList(const MessageRef & msg)
{
	PRINT("WUploadThread::TransferFileList\n");
	
	if (fShutdownFlag && *fShutdownFlag)	// do we need to abort?
		return;
	
	if (gWin->IsScanning())
	{
		fLocallyQueued = true;
		fSavedFileList = msg;
		SendQueuedNotification();
		return;
	}
	
	if (msg())
	{
		const char * file;
		if (msg()->FindString("files", &file) == B_OK)
		{
			String sid, name;
			int32 mm;
			if (msg()->FindInt32("mm", &mm) == B_OK)
				fMungeMode = mm;
			else
				fMungeMode = WDownload::MungeModeNone;
			
			if (msg()->FindString("beshare:FromSession", sid) == B_OK)
				fRemoteSessionID = QString::fromUtf8(sid.Cstr());
			
			if (msg()->FindString("beshare:FromUserName", name) ==  B_OK)
			{
				QString user = QString::fromUtf8(name.Cstr());
				if (!user.isEmpty())
					fRemoteUser = user;
			}
			else
			{
				fRemoteUser = GetUserName(fRemoteSessionID);
			}
			
			int i;
			
			for (i = 0; (msg()->FindString("files", i, &file) == B_OK); i++)
			{
				
				MessageRef fileRef;
				
				if (fFileThread->FindFile(QString::fromUtf8(file), fileRef))
				{
					if (fileRef()) // <postmaster@raasu.org> 20021023
					{
						// remove any previous offsets
						fileRef()->RemoveName("secret:offset");
						
						// see if we need to add them
						uint64 offset = 0L;
						const uint8 * hisDigest = NULL;
						uint32 numBytes = 0L;
						if (msg()->FindInt64("offsets", i, (int64 *)&offset) == B_OK &&
							msg()->FindData("md5", B_RAW_TYPE, i, (const void **)&hisDigest, (uint32 *)&numBytes) == B_OK && 
							numBytes == MD5_DIGEST_SIZE)
						{
							uint8 myDigest[MD5_DIGEST_SIZE];
							uint64 readLen = 0;
							uint64 onSuccessOffset = offset;
							
							for (uint32 j = 0; j < ARRAYITEMS(myDigest); j++)
								myDigest[j] = 'x';
							
							if (msg()->FindInt64("numbytes", (int64 *)&readLen) == B_OK)
							{
								PRINT("\t\tULT: peer requested partial resume\n");
								uint64 temp = readLen;
								readLen = offset - readLen; // readLen is now the seekTo value
								offset = temp;				// offset is now the numBytes value
							}
							
							// figure the path to our requested file
							String path, filename;
							fileRef()->FindString("winshare:Path", path);
							fileRef()->FindString("beshare:File Name", filename);
							
							String filePath = path;
							if (!filePath.EndsWith("/"))
								filePath += "/";
							
							filePath += filename;
							
							// Notify window of our hashing
							MessageRef m(GetMessageFromPool(WUploadEvent::FileHashing));
							if (m())
							{
								m()->AddString("file", filePath);
								SendReply(m);
							}
							
							// Hash
							uint64 retBytesHashed = 0;
							
							if (HashFileMD5(QString::fromUtf8(filePath.Cstr()), offset, readLen, retBytesHashed, myDigest, 
								fShutdownFlag) == B_OK && memcmp(hisDigest, myDigest, sizeof(myDigest)) == 0)
							{
								// put this into our message ref
								fileRef()->AddInt64("secret:offset", onSuccessOffset);
							}
						}
						
						fUploads.AddTail(fileRef);
						fNames.AddTail(QString::fromUtf8(file));
					}
				}
			}
			
			fNumFiles = i;
			
			fWaitingForUploadToFinish = false;
			SendQueuedNotification();
			
			// also send a message along to our GUI telling it what the first file is
			
			if (fUploads.GetNumItems() == 0)
			{
				PRINT("WUploadThread: No Files!!!\n");
				fFinished = true;
				Reset();
			}
			else
			{
				String firstFile;
				const char * path, * filename;
				uint64 filesize;
				MessageRef fref;
				fUploads.GetItemAt(0, fref);
				if (fref()->FindString("winshare:Path", &path) == B_OK &&
					fref()->FindString("beshare:File Name", &filename) == B_OK &&
					fref()->FindInt64("beshare:File Size", (int64 *) &filesize) == B_OK)
				{
					if (filesize < gWin->fSettings->GetMinQueuedSize() || !IsLocallyQueued())
					{
						DoUpload();
					}
					else
					{
						firstFile = path;
						
						if (!firstFile.EndsWith("/"))
							firstFile += "/";
						
						firstFile += filename;
						MessageRef initmsg(GetMessageFromPool(WUploadEvent::Init));
						if (initmsg())
						{
							initmsg()->AddString("file", firstFile);
							initmsg()->AddString("user", (const char *) fRemoteSessionID.utf8());
							SendReply(initmsg);
						}
					}
				}
			}
	}
	}
}

/* 
 * Called when we have to send more data.
 *
 */

void
WUploadThread::SignalUpload()
{
	QCustomEvent *qce = new QCustomEvent(UploadEvent);
	if (qce) 
	{
		QApplication::postEvent(this, qce);
	}
}

void
WUploadThread::InitTransferRate()
{
	for (int i = 0; i < MAX_RATE_COUNT; i++)
	{
		fRate[i] = 0.0f;
	}

	fRateCount = 0;
}

void
WUploadThread::InitTransferETA()
{
	for (int i = 0; i < MAX_ETA_COUNT; i++)
	{
		fETA[i] = 0;
	}

	fETACount = 0;
}

void
WUploadThread::Reset()
{
	PRINT("WUploadThread::Reset()\n");
	SetFinished(true);
//	SetActive(false);
	SetLocallyQueued(false);
//	if ( fShutdownFlag )
//		*fShutdownFlag = true;
	QMessageTransceiverThread::Reset();
	PRINT("WUploadThread::Reset() OK\n");
}

void
WUploadThread::SetPacketSize(int s)
{
	// Clear Rate and ETA counts because changing the Packet Size between two estimate recalculations causes miscalculation
	if (fPacket != s)
	{
		fRateCount = 0;
		fETACount = 0;
		fPacket = s;
	}	
}

bool
WUploadThread::IsLocallyQueued() const
{
	return fLocallyQueued;
}

bool
WUploadThread::IsManuallyQueued() const
{
	return fManuallyQueued;
}

bool
WUploadThread::IsFinished() const
{
	return fFinished;
}

bool
WUploadThread::IsActive() const
{
	return fActive;
}

bool
WUploadThread::IsBlocked() const
{
	return fBlocked;
}

QString
WUploadThread::GetETA(uint64 cur, uint64 max, double rate)
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

double
WUploadThread::GetCalculatedRate() const
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

void 
WUploadThread::SetPacketCount(double bytes)
{
	fPackets += bytes / ((double) fPacket) ;
}

void
WUploadThread::SetMostRecentRate(double rate)
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

bool
WUploadThread::IsLastFile()
{ 
	int c = GetCurrentNum() + 1;
	int n = GetNumFiles();
	return (c >= n); 
}

void
WUploadThread::SetActive(bool b)
{
	fActive = b;
}

void
WUploadThread::SetFinished(bool b)
{
	fFinished = b;
}

int
WUploadThread::GetPacketSize()
{
	return fPacket;
}

int
WUploadThread::GetBanTime()
{
	if (fTimeLeft == 0)
		return 0;
	else if (fTimeLeft == -1)
		return -1;
	else
		return (fTimeLeft / 60000000);
}

QString
WUploadThread::GetUserName(QString sid)
{
	WUserRef uref = gWin->FindUser(sid);
	QString ret = sid;
	if (uref())
		ret = uref()->GetUserName();
	else
	{
		uref = gWin->FindUserByIPandPort(GetRemoteIP(), 0);
		if (uref())
		{
			ret = uref()->GetUserName();
		}
	}
	return ret;
}

status_t
WUploadThread::SendMessageToSessions(MessageRef msgRef, const char * optDistPath)
{
	return QMessageTransceiverThread::SendMessageToSessions(msgRef, optDistPath);
}

bool 
WUploadThread::IsInternalThreadRunning()
{
	return QMessageTransceiverThread::IsInternalThreadRunning();
}

uint32
WUploadThread::ComputeETA() const
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

// Do the same averaging for ETA's that we do for rates
void
WUploadThread::SetMostRecentETA(uint32 eta)
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

void
WUploadThread::ConnectTimer()
{
	Reset();
	MessageRef msg(GetMessageFromPool(WUploadEvent::ConnectFailed));
	if (msg())
	{
		msg()->AddString("why", QT_TR_NOOP( "Connection timed out!" ));
		SendReply(msg);
	}
}

void
WUploadThread::BlockedTimer()
{
	SetBlocked(false);
	fTimeLeft = 0;
}
