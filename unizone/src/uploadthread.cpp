#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "uploadthread.h"
#include "wgenericevent.h"
#include "global.h"
#include "downloadimpl.h"
#include "settings.h"
#include "md5.h"
#include "wstring.h"
#include "filethread.h"
#include "debugimpl.h"

WUploadThread::WUploadThread(QObject * owner, bool * optShutdownFlag)
	: WGenericThread(owner, optShutdownFlag) 
{ 
	PRINT("WUploadThread ctor\n");
	setName( "WUploadThread" );
	fFile = NULL; 
	fFileUl = QString::null;
	fRemoteSessionID = QString::null;
	fCurFile = -1;
	fNumFiles = -1;
	fPort = 0;
	fSocket = 0;
	fActive = false;
	fInit = false;
	fBlocked = false;
	fTimeLeft = 0;
	fForced = false;

	qmtt = new QMessageTransceiverThread();
	CHECK_PTR(qmtt);

	connect(qmtt, SIGNAL(MessageReceived(MessageRef, const String &)), 
			this, SLOT(MessageReceived(MessageRef, const String &)));
	connect(qmtt, SIGNAL(SessionAttached(const String &)),
			this, SLOT(SessionAttached(const String &)));
	connect(qmtt, SIGNAL(SessionConnected(const String &)),
			this, SLOT(SessionConnected(const String &)));
	connect(qmtt, SIGNAL(ServerExited()),
			this, SLOT(ServerExited()));
	connect(qmtt, SIGNAL(SessionDisconnected(const String &)),
			this, SLOT(SessionDisconnected(const String &)));
	connect(qmtt, SIGNAL(OutputQueuesDrained(MessageRef)),
			this, SLOT(OutputQueuesDrained(MessageRef)));

	PRINT("WUploadThread ctor OK\n");
}

WUploadThread::~WUploadThread()
{
	PRINT("WUploadThread dtor\n");

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
WUploadThread::InitSessionAux()
{
	PRINT("WUploadThread::InitSession\n");
	if (gWin->IsScanning())
	{
		fInit = true;
		return false;
	}

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
		if (qmtt->AddNewSession(fSocket, limit) == B_OK && qmtt->StartInternalThread() == B_OK)
		{
			MessageRef mref(GetMessageFromPool(WGenericEvent::ConnectInProgress));
			if (mref())
			{
				SendReply(mref);
			}
			CTimer->start(60000, true);
		}
		else
		{
			MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
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
		if (qmtt->AddNewConnectSession(sRemoteIP, (uint16)fPort, limit) == B_OK && qmtt->StartInternalThread() == B_OK)
		{
			MessageRef mref(GetMessageFromPool(WGenericEvent::ConnectInProgress));
			if (mref())
			{
				SendReply(mref);
			}
			CTimer->start(60000, true);
		}
		else
		{
			MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
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
	WGenericThread::SetLocallyQueued(b);
	if (!b)
	{
		if (qmtt->IsInternalThreadRunning())  // Still connected?
		{
			// Don't need to start forced uploads ;)
			if (!fForced)
				DoUpload();		// we can start now!
		}
		else if (fInit)
		{
			fInit = false;
			InitSession();
		}
		else if (fActive)
		{
			qmtt->Reset();
			MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
			if (fail())
			{
				fail()->AddString("why", QT_TR_NOOP( "Connection reset by peer!" ));
				SendReply(fail);
			}
			fActive = false;
			fFinished = true;
		}
	}
}

void
WUploadThread::SetBlocked(bool b, int64 timeLeft)
{
	SetActive(!b);
	if (b)
	{
		if (timeLeft != -1)
			fBlockTimer->start(timeLeft/1000, true);
	}
	else if (!b && fBlockTimer->isActive())
	{
		fBlockTimer->stop();
	}

	WGenericThread::SetBlocked(b, timeLeft);
	if (qmtt->IsInternalThreadRunning())
	{
		if (b)
		{
			SendRejectedNotification(true);
		}
		else
		{
			SetLocallyQueued(true); // put in queue ;)
			DoUpload();				// we can start now!
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
	WGenericThread::SetManuallyQueued(b);
	if (qmtt->IsInternalThreadRunning() && b)
		SendQueuedNotification();
}


void 
WUploadThread::SendReply(MessageRef &m)
{
	if (m())
	{
		m()->AddBool("upload", true);
		WGenericThread::SendReply(m);
	}
}

void
WUploadThread::SessionAttached(const String &sessionID)
{
	// If you aren't firewalled
	_sessionID = sessionID;
	CTimer->stop();

	MessageRef con(GetMessageFromPool(WGenericEvent::Connected));
	if (con())
	{
		SendReply(con);
	}
}

void
WUploadThread::SessionConnected(const String &sessionID)
{
	// If you are firewalled
	_sessionID = sessionID;
	CTimer->stop();

	MessageRef con(GetMessageFromPool(WGenericEvent::Connected));
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

	*fShutdownFlag = true;
	fDisconnected = true;

	if (fFinished) // Do it only once...
	{
		return;
	}
	else
	{
		fFinished = true;
	}

	if (fFile)
	{
		fFile->close();
		delete fFile; 
		fFile = NULL;
	}

	MessageRef dis(GetMessageFromPool(WGenericEvent::Disconnected));
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

void
WUploadThread::MessageReceived(MessageRef msg, const String &sessionID)
{
	PRINT("WUploadThread::MessageReceived\n");
	switch (msg()->what)
	{
		case WDownload::TransferCommandPeerID:
		{
			PRINT("WDownload::TransferCommandPeerID\n");
			const char * name, * id;
			if (msg()->FindString("beshare:FromUserName", &name) ==  B_OK &&
				msg()->FindString("beshare:FromSession", &id) == B_OK)
			{
				fRemoteSessionID = QString::fromUtf8(id);
							
				QString user = QString::fromUtf8(name);
				if (user.isEmpty())
					fRemoteUser = GetUserName(fRemoteSessionID);
				else
					fRemoteUser = user; 

				if (gWin->IsIgnored(fRemoteSessionID, true))
				{
					SetBlocked(true);
				}

				MessageRef ui(GetMessageFromPool(WGenericEvent::UpdateUI));
				if (ui())
				{
					ui()->AddString("name", name);
					ui()->AddString("id", id);
					SendReply(ui);
				}
			}

			bool c = false;
			
			if (msg()->FindBool("unishare:supports_compression", &c) == B_OK)
			{
				qmtt->SetOutgoingMessageEncoding(MUSCLE_MESSAGE_ENCODING_ZLIB_9);
			}
			break;
		}
	
		case WDownload::TransferFileList:
		{
			PRINT("WDownload::TransferFileList\n");
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
									readLen = offset - readLen;	// readLen is now the seekTo value
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
								MessageRef m(GetMessageFromPool(WGenericEvent::FileHashing));
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
							
				if (fUploads.GetNumItems() != 0)
				{
					String firstFile;
					const char * path, * filename;
					uint64 filesize;
					MessageRef fref = fUploads[0];
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
								MessageRef initmsg(GetMessageFromPool(WGenericEvent::Init));
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
		qmtt->Reset();
		PRINT("\t\tSending message\n");

		MessageRef msg(GetMessageFromPool(WGenericEvent::FileDone));
		if (msg())
		{
			msg()->AddBool("done", true);
			msg()->AddString("file", (const char *) fFileUl.utf8());
			PRINT("\t\tSending...\n");
			SendReply(msg);
			PRINT("\t\tSent...\n"); // <postmaster@raasu.org> 20021023 -- Fixed typo
			fFinished = true;
		}
		else
		{
			PRINT("\t\tNot sent!\n");
		}
		fWaitingForUploadToFinish = false;
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
		qmtt->SendMessageToSessions(q);
	}
	MessageRef qf(GetMessageFromPool(WGenericEvent::FileQueued));
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
		qmtt->SendMessageToSessions(q);
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
	MessageRef b(GetMessageFromPool(WGenericEvent::FileBlocked));
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
		return;

	// Still connected?

	if (!qmtt->IsInternalThreadRunning())
	{
		MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
		if (fail())
		{
			fail()->AddString("why", QT_TR_NOOP( "Connection reset by peer!" ));
			SendReply(fail);
		}
		fActive = false;
		fFinished = true;
		return;
	}

	fActive = true;

	// Small files get to bypass queue
	if (IsLocallyQueued())
	{
		if (fFile && (fFileSize >= gWin->fSettings->GetMinQueuedSize()))		// not yet
		{
			fForced = false;
			MessageRef lq(GetMessageFromPool(WGenericEvent::FileQueued));
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
		MessageRef msg(GetMessageFromPool(WGenericEvent::FileBlocked));
		if (msg())
		{
			if (fTimeLeft != -1)
				msg()->AddInt64("timeleft", fTimeLeft);
			SendReply(msg);
		}
		if (gWin->IsConnected(fRemoteSessionID))
		{
			if (fTimeLeft == -1)
			{
				gWin->SendChatText(fRemoteSessionID, "Your download has been blocked!");
			}
			else
			{
				gWin->SendChatText(fRemoteSessionID, tr("Your download has been blocked for %1 minute(s)!").arg((long)(fTimeLeft/60000000)));
			}
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
					qmtt->SendMessageToSessions(uref);

					// NOTE: RequestOutputQueuesDrainedNotification() can recurse, so we need to update the offset before
					//       calling it!
					fCurrentOffset += numBytes;
					MessageRef drain(GetMessageFromPool());
					if (drain())
						qmtt->RequestOutputQueuesDrainedNotification(drain);

					// we'll use this event for sending as well
					MessageRef update(GetMessageFromPool(WGenericEvent::FileDataReceived));	
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
					qmtt->SendMessageToSessions(headRef);
				}

				fCurFile++;

				MessageRef mref(GetMessageFromPool(WGenericEvent::FileStarted));
				if (mref())
				{
					mref()->AddString("file", filePath.Cstr());
					mref()->AddInt64("start", fCurrentOffset);
					mref()->AddInt64("size", fFileSize);
					mref()->AddString("user", (const char *) fRemoteSessionID.utf8());
					SendReply(mref);
				}

				DoUpload();	// nested call
			}
			else
			{
				PRINT("No more files!\n");
				fWaitingForUploadToFinish = true;
				MessageRef drain(GetMessageFromPool());
				if (drain())
					qmtt->RequestOutputQueuesDrainedNotification(drain);
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
	WGenericThread::SetRate(rate);
	if (rate != 0)
		qmtt->SetNewOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		qmtt->SetNewOutputPolicy(PolicyRef(NULL, NULL));
}

void
WUploadThread::SetRate(int rate, AbstractReflectSessionRef ref)
{
	WGenericThread::SetRate(rate);
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
