#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "uploadthread.h"
#include "wuploadevent.h"
#include "wsystemevent.h"
#include "global.h"
#include "downloadimpl.h"
#include "settings.h"
#include "netclient.h"
#include "md5.h"
#include "wstring.h"
#include "filethread.h"
#include "debugimpl.h"
#include "util.h"

#include <qapplication.h>

WUploadThread::WUploadThread(QObject * owner, bool * optShutdownFlag)
	: QObject(owner), fOwner(owner), fShutdownFlag(optShutdownFlag) 
{ 
	PRINT("WUploadThread ctor\n");
	setName( "WUploadThread" );
	fFile = NULL; 
	fFileUl = QString::null;
	fRemoteSessionID = QString::null;

	// Default status

	if (!fShutdownFlag)					// Force use of Shutdown Flag
	{
		fShutdown = false;
		fShutdownFlag = &fShutdown;
	}

	fCurFile = -1;
	fNumFiles = 0;
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
	fTunneled = false;
	fTXRate = 0;
	fTimeLeft = 0;
	fStartTime = 0;
	fPacket = 8;
	fIdles = 0;

	InitTransferRate();
	InitTransferETA();

	CTimer = new QTimer(this, "Connect Timer");
	CHECK_PTR(CTimer);

	connect( CTimer, SIGNAL(timeout()), this, SLOT(ConnectTimer()) );
	
	fBlockTimer = new QTimer(this, "Blocked Timer");
	CHECK_PTR(fBlockTimer);

	connect( fBlockTimer, SIGNAL(timeout()), this, SLOT(BlockedTimer()) );

	// QMessageTransceiverThread

	qmtt = new QMessageTransceiverThread(this);
	CHECK_PTR(qmtt);

	connect(qmtt, SIGNAL(MessageReceived(MessageRef, const String &)),
			this, SLOT(MessageReceived(MessageRef, const String &)));

	connect(qmtt, SIGNAL(OutputQueuesDrained(MessageRef)),
			this, SLOT(OutputQueuesDrained(MessageRef)));
	
	connect(qmtt, SIGNAL(SessionConnected(const String &)),
			this, SLOT(SessionConnected(const String &)));

	connect(qmtt, SIGNAL(SessionDisconnected(const String &)),
			this, SLOT(SessionDisconnected(const String &)));

	connect(qmtt, SIGNAL(SessionAttached(const String &)),
			this, SLOT(SessionAttached(const String &)));
    
	connect(qmtt, SIGNAL(SessionDetached(const String &)),
			this, SLOT(SessionDetached(const String &)));

	connect(qmtt, SIGNAL(ServerExited()),
			this, SLOT(ServerExited()));

	// End of QMessageTransceiverThread

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

	PRINT("\tCleaning up files...\n");
	CloseFile(fFile);

	if (!fTunneled)
		qmtt->ShutdownInternalThread();

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
WUploadThread::SetUpload(const QString & remoteIP, uint32 remotePort, WFileThread * ft)
{
	fFileThread = ft;
	fAccept = true;
	fStrRemoteIP = remoteIP;
	fPort = remotePort;
}

void
WUploadThread::SetUpload(const QString & userID, int32 remoteID, WFileThread * ft)
{
	fTunneled = true;
	fFileThread = ft;
	fAccept = false;
	fRemoteSessionID = userID;
	hisID = remoteID;
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

	if (fTunneled)
	{
		MessageRef mref(GetMessageFromPool(WUploadEvent::ConnectInProgress));
		if (mref())
		{
			SendReply(mref);
		}

		return true;
	}
	else if (fAccept)
	{
		// <postmaster@raasu.org> 20021026
		const String sRemoteIP = (const char *) fStrRemoteIP.utf8(); 
		if (qmtt->AddNewConnectSession(sRemoteIP, (uint16)fPort, limit) != B_OK)
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
	else
	{
		if (qmtt->AddNewSession(fSocket, limit) != B_OK)
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

	//
	// We should have a session created now
	//

	if (qmtt->StartInternalThread() == B_OK)
	{
		MessageRef mref(GetMessageFromPool(WUploadEvent::ConnectInProgress));
		if (mref())
		{
			SendReply(mref);
		}
		CTimer->start(60000, true);
		return true;
	}

	MessageRef fail(GetMessageFromPool(WUploadEvent::ConnectFailed));
	if (fail())
	{
		fail()->AddString("why", QT_TR_NOOP( "Could not start internal thread!" ));
		SendReply(fail);
	}
	return false;
}

void
WUploadThread::SetLocallyQueued(bool b)
{
	if (fLocallyQueued != b)
	{
		if (!fConnecting && !IsInternalThreadRunning())
		{
			Reset();
		}
		
		if (fFinished)
			return;
		
		// -----------------------------------------------
		
		fLocallyQueued = b;
		
		if (b)
		{
			fStartTime = 0;
		}
		else
		{
			fLastData.restart();
			InitTransferETA();
			InitTransferRate();
			
			if (fSavedFileList())
			{
				MessageRef fFileList = fSavedFileList;
				fSavedFileList.Reset();
				WUploadEvent *wue = new WUploadEvent(fFileList);
				if (wue) 
					QApplication::postEvent(this, wue);
				// TransferFileList(fFileList);
				return;
			}
			else if (IsInternalThreadRunning())  // Still connected?
			{
				// Don't need to start forced or finished uploads ;)
				if (!fForced)
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
			else
			{
				// fActive   == true
				// fFinished == false
				ConnectTimer();
			}
		}
	}
}

void
WUploadThread::SetBlocked(bool b, int64 timeLeft)
{
	if (fBlocked != b)
	{
		SetActive(!b);
		fBlocked = b;
		if (b)
		{
			fTimeLeft = timeLeft;
			fStartTime = 0;
			if (timeLeft != -1)
				fBlockTimer->start(timeLeft/1000, true);
			
			// Send notification
			
			if (IsInternalThreadRunning())
				SendRejectedNotification(true);
			else
				SendRejectedNotification(false);
		}
		else // !b
		{
			fTimeLeft = 0;
			fLastData.restart();
			InitTransferETA();
			InitTransferRate();
			if (fBlockTimer->isActive())
			{
				fBlockTimer->stop();
			}
			
			if (IsInternalThreadRunning())
			{
				SetLocallyQueued(true); // put in queue ;)
				SendQueuedNotification();
			}
		}
	}
}

void
WUploadThread::SetManuallyQueued(bool b)
{
	if (fManuallyQueued != b)
	{
		fManuallyQueued = b;	
		if (IsInternalThreadRunning() && b)
			SendQueuedNotification();
	}
}


void 
WUploadThread::SendReply(MessageRef &m)
{
	if (fShutdownFlag && *fShutdownFlag)
	{
		Reset();
	}

	if (m())
	{
		m()->AddPointer("sender", this);
		WUploadEvent * wue = new WUploadEvent(m);
		if (wue)
			QApplication::postEvent(fOwner, wue);
	}
}

void
WUploadThread::SessionAttached(const String &sessionID)
{
	// If you aren't firewalled

	SessionConnected(sessionID);
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
	fActive = true;

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
WUploadThread::SessionDisconnected(const String & /* sessionID */)
{
	PRINT("WUploadThread::SessionDisconnected\n");

	if (timerID != 0) 
	{
		killTimer(timerID);
		timerID = 0;
	}

	*fShutdownFlag = true;

	CloseFile(fFile);

	fDisconnected = true;
	fFinished = true;
	fLocallyQueued = false;

	if (fActive || fConnecting || fBlocked) // Do it only once...
	{
		fActive = false;
		fConnecting = false;
		fBlocked = false;
		
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
WUploadThread::MessageReceived(MessageRef msg, const String & /* sessionID */)
{
	PRINT("WUploadThread::MessageReceived\n");
	switch (msg()->what)
	{
		case WDownload::TransferCommandPeerID:
		{
			PRINT("WDownload::TransferCommandPeerID\n");
			const char *id = NULL;
			if (msg()->FindString("beshare:FromSession", &id) == B_OK)
			{
				fRemoteSessionID = QString::fromUtf8(id);
				
				{
					const char *name = NULL;
					
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
				}
				
				if (gWin->IsIgnored(fRemoteSessionID, true))
				{
					SetBlocked(true);
				}
				
				MessageRef ui(GetMessageFromPool(WUploadEvent::UpdateUI));
				if (ui())
				{
					ui()->AddString("id", id);
					SendReply(ui);
				}
				
			}

			bool c = false;
			
			if (!fTunneled && msg()->FindBool("unishare:supports_compression", &c) == B_OK)
			{
				qmtt->SetOutgoingMessageEncoding(MUSCLE_MESSAGE_ENCODING_ZLIB_9);
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
WUploadThread::OutputQueuesDrained(MessageRef /* msg */)
{
#ifdef DEBUG2
	PRINT("\tMTT_EVENT_OUTPUT_QUEUES_DRAINED\n");
#endif

	fIdles = 0;

	if (fWaitingForUploadToFinish)
	{
		PRINT("\tfWaitingForUploadToFinish\n");
		SetFinished(true);
		fWaitingForUploadToFinish = false;

		PRINT("\t\tSending message\n");

		{
			MessageRef msg(GetMessageFromPool(WUploadEvent::FileDone));
			if (msg())
			{
				msg()->AddBool("done", true);
				msg()->AddString("file", (const char *) fFileUl.utf8());
				PRINT("\t\tSending...\n");
				SendReply(msg);
				PRINT("\t\tSent...\n"); // <postmaster@raasu.org> 20021023 -- Fixed typo
			}
			else
			{
				PRINT("\t\tNot sent!\n");
			}
		}
		
		PRINT("\t\tDisconnecting...\n");
		
		{
			MessageRef dis(GetMessageFromPool(WUploadEvent::Disconnected));
			if (dis())
			{
				dis()->AddBool("done", true);
				SendReply(dis);
				PRINT("\t\tDisconnected...\n");
			}
			else
			{
				PRINT("\t\tCould not disconnect!\n");
			}
		}

		PRINT("\tfWaitingForUploadToFinish OK\n");
	}
	else if (!fFinished)
	{
		SignalUpload();
	}
}

void 
WUploadThread::SendQueuedNotification()
{
	MessageRef q(GetMessageFromPool(WDownload::TransferNotifyQueued));
	if (q())
	{
		SendMessageToSessions(q);

		MessageRef qf(GetMessageFromPool(WUploadEvent::FileQueued));
		if (qf())
		{
			SendReply(qf);
		}
	}
}

void
WUploadThread::SendRejectedNotification(bool direct)
{
	MessageRef q(GetMessageFromPool(WDownload::TransferNotifyRejected));
	
	if (q())
	{
		
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
				node += (const char *) fRemoteSessionID.utf8();
			}
			else
			{
				// use /<ip>/* instead of /*/<sessionid>, because session id isn't yet known at this point
				node = "/";
				node += (const char *) fStrRemoteIP.utf8();
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
		ConnectTimer();
		return;
	}

	// Still connected?

	if (!IsInternalThreadRunning())
	{
		ConnectTimer();
		return;
	}

	// fActive = true;

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
			int32 numBytes;
			if (uref()->AddData("data", B_RAW_TYPE, NULL, bufferSize) == B_OK &&
				uref()->FindDataPointer("data", B_RAW_TYPE, (void **)&scratchBuffer, NULL) == B_OK)
			{
				numBytes = fFile->ReadBlock((char *)scratchBuffer, bufferSize);
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
					uref()->AddInt32("chk", CalculateChecksum(scratchBuffer, numBytes));  // a little paranoioa, due to file-resumes not working.... (TCP should handle this BUT...)

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
					if (fTunneled)
					{
						SignalUpload();
					}
					else
					{
						MessageRef drain(GetMessageFromPool());
						if (drain())
							qmtt->RequestOutputQueuesDrainedNotification(drain);
					}

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

							if (gWin->fSettings->GetUploads())
							{
								gWin->SendSystemEvent( tr("%1 has finished downloading %2.").arg( GetRemoteUser() ).arg( fFileUl ) );
							}
						}
						SendReply(update);
					}
										

					return;
				}
				
				if (numBytes <= 0)
				{
					NextFile();
					SignalUpload();
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
				String file;
				
				{
					String path, filename;
					fCurrentRef()->FindString("beshare:Path", path);
					fCurrentRef()->FindString("beshare:File Name", filename);

					file = MakePath(path, filename);
				}

				fFileUl = QString::fromUtf8(file.Cstr());

#ifdef _DEBUG
				// <postmaster@raasu.org> 20021023, 20030702 -- Add additional debug message
				WString wFileUl(fFileUl); 
				PRINT("WUploadThread::DoUpload: filePath = %S\n", wFileUl.getBuffer()); 
#endif
				
				fFile = new WFile();
				CHECK_PTR(fFile);
				if (!fFile->Open(fFileUl, IO_ReadOnly))	// probably doesn't exist
				{
					delete fFile;
					fFile = NULL;
					fCurFile++;
					continue;	// onward
				}
				// got our file!
				fFileSize = fFile->Size();
				fCurrentOffset = 0;	// from the start
				if (fCurrentRef()->FindInt64("secret:offset", (int64 *)&fCurrentOffset) == B_OK)
				{
					if (!fFile->Seek(fCurrentOffset)) // <postmaster@raasu.org> 20021026
					{
						fFile->Seek(0);	// this can't fail :) (I hope)
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

				// Reset statistics
				InitTransferRate();
				InitTransferETA();

				MessageRef mref(GetMessageFromPool(WUploadEvent::FileStarted));
				if (mref())
				{
					mref()->AddString("file", file.Cstr());
					mref()->AddInt64("start", fCurrentOffset);
					mref()->AddInt64("size", fFileSize);
					mref()->AddString("user", (const char *) fRemoteSessionID.utf8());
					SendReply(mref);
				}

				if (gWin->fSettings->GetUploads())
				{
					gWin->SendSystemEvent( tr("%1 is downloading %2.").arg( GetRemoteUser() ).arg( fFileUl ) );
				}

				// nested call
				SignalUpload();
				return;
			}
			else
			{
				PRINT("No more files!\n");
				fWaitingForUploadToFinish = true;
				SetFinished(true);
				if (fTunneled)
				{
					MessageRef drain(GetMessageFromPool());
					if (drain())
						OutputQueuesDrained(drain);
				}
				else
				{
					MessageRef drain(GetMessageFromPool());
					if (drain())
						qmtt->RequestOutputQueuesDrainedNotification(drain);
				}
				break;
			}
		}
	}
}

QString
WUploadThread::GetFileName(unsigned int i) const
{
	if (i < fNames.GetNumItems())
	{
		QString file = QString::fromUtf8(fNames[i].Cstr());
		return file;
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
	if (!fTunneled)
	{
		if (rate != 0)
			qmtt->SetNewOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
		else
			qmtt->SetNewOutputPolicy(PolicyRef(NULL, NULL));
	}
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
WUploadThread::GetRemoteUser() const
{
	if (fRemoteUser.isEmpty())
		return tr("User #%1").arg(fRemoteSessionID);
	else
		return fRemoteUser;
}

void
WUploadThread::timerEvent(QTimerEvent * /* e */)
{
	if (IsInternalThreadRunning())
	{
		if ((fLocallyQueued && !fForced) || fBlocked)	
		{
			// Locally queued or blocked transfer don't need idle check restricting
			MessageRef nop(GetMessageFromPool(PR_COMMAND_NOOP));
			if ( nop() )
			{
				SendMessageToSessions(nop);
			}
			return;
		}
		else if (fIdles < 4) 		
		{
			// 1 minute maximum
			fIdles++;
			if (fIdles < 2)
			{
				// 30 seconds to avoid postponing OutputQueuesDrained() message too far
				MessageRef nop(GetMessageFromPool(PR_COMMAND_NOOP));
				if ( nop() )
				{
					SendMessageToSessions(nop);
				}
			}
			return;
		}
	}
	// fall through
	ConnectTimer();
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
			return QObject::event(e);
		}
	}
}

void
WUploadThread::TransferFileList(const MessageRef & msg)
{
	PRINT("WUploadThread::TransferFileList\n");
	
	if (msg())
	{
		
		if (fShutdownFlag && *fShutdownFlag)	// do we need to abort?
		{
			Reset();
			return;
		}
		
		if (gWin->IsScanning())
		{
			fSavedFileList = msg;
			if (!fBlocked)
			{
				SetLocallyQueued(true);
				SendQueuedNotification();
			}
			return;
		}
		
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
		
		const char * file;
		
		for (int i = 0; (msg()->FindString("files", i, &file) == B_OK); i++)
		{
			MessageRef fileRef;
			
			if (fFileThread->FindFile(file, fileRef))
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
						String file;

						{
							String path, filename;
							fileRef()->FindString("beshare:Path", path);
							fileRef()->FindString("beshare:File Name", filename);
						
							file = MakePath(path, filename);
						}
						
						// Notify window of our hashing
						MessageRef m(GetMessageFromPool(WUploadEvent::FileHashing));
						if (m())
						{
							m()->AddString("file", file);
							SendReply(m);
						}
						
						// Hash
						uint64 retBytesHashed = 0;
						
						QString qFile = QString::fromUtf8(file.Cstr());
						
						if (HashFileMD5(qFile, offset, readLen, retBytesHashed, myDigest, fShutdownFlag) == B_OK && 
							memcmp(hisDigest, myDigest, sizeof(myDigest)) == 0)
						{
							// put this into our message ref
							fileRef()->AddInt64("secret:offset", onSuccessOffset);
						}
					}
					
					fUploads.AddTail(fileRef);
					fNames.AddTail(file);
				}
			}
		}
		
		msg()->GetInfo("files", NULL, &fNumFiles);
		
		fWaitingForUploadToFinish = false;
		SendQueuedNotification();
		
		// also send a message along to our GUI telling it what the first file is
		
		if (fUploads.GetNumItems() == 0)
		{
			PRINT("WUploadThread: No Files!!!\n");
			Reset();
			return;
		}
		
		MessageRef fref;
		fUploads.GetItemAt(0, fref);
		if (fref())
		{
			String path, filename;
			
			if (fref()->FindString("beshare:Path", path) == B_OK &&
				fref()->FindString("beshare:File Name", filename) == B_OK)
			{
				if (!IsLocallyQueued())
				{
					SignalUpload();
					return;
				}
				else
				{
					uint64 filesize;
					if (fref()->FindInt64("beshare:File Size", (int64 *) &filesize) == B_OK)
					{
						if (filesize < gWin->fSettings->GetMinQueuedSize())
						{
							SignalUpload();
							return;
						}
					}
				}
				
				String firstFile = MakePath(path, filename);
				
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
	fPackets = 0;
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
	if (fTunneled)
		SessionDisconnected(_sessionID);
	else
		qmtt->Reset();
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
WUploadThread::GetUserName(const QString & sid) const
{
	WUserRef uref = gWin->FindUser(sid);
	QString ret;
	if (uref())
		ret = uref()->GetUserName();
	else
	{
		uref = gWin->FindUserByIPandPort(GetRemoteIP(), 0);
		if (uref())
		{
			ret = uref()->GetUserName();
		}
		else
		{
			ret = sid;
		}
	}
	return ret;
}

status_t
WUploadThread::SendMessageToSessions(MessageRef msgRef, const char * optDistPath)
{
	if (fTunneled)
	{
		if (
			(msgRef()->what == NetClient::ACCEPT_TUNNEL) ||
			(msgRef()->what == NetClient::REJECT_TUNNEL)
			)
		{
			// Send directly...
			QString to("/*/");
			to += fRemoteSessionID;
			to += "/beshare";
			msgRef()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			msgRef()->AddString(PR_NAME_SESSION, "");
			// msgRef()->AddInt32("my_id", (int32) this);
			msgRef()->AddBool("upload", true);
			return static_cast<WDownload *>(fOwner)->netClient()->SendMessageToSessions(msgRef);
		}
		else
		{		
			MessageRef up(GetMessageFromPool(NetClient::TUNNEL_MESSAGE));
			if (up())
			{
				QString to("/*/");
				to += fRemoteSessionID;
				to += "/beshare";
				up()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
				up()->AddString(PR_NAME_SESSION, "");
				// up()->AddInt32("my_id", (int32) this);
				up()->AddMessage("message", msgRef);
				up()->AddBool("upload", true);
				up()->AddInt32("tunnel_id", hisID);
				return static_cast<WDownload *>(fOwner)->netClient()->SendMessageToSessions(up);
			}
			else
				return B_ERROR;
		}
	}
	else
		return qmtt->SendMessageToSessions(msgRef, optDistPath);
}

bool 
WUploadThread::IsInternalThreadRunning()
{
	if (fTunneled)
		return gWin->IsConnected(fRemoteSessionID);
	else
		return qmtt->IsInternalThreadRunning();
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

void
WUploadThread::NextFile()
{
	// next file
	CloseFile(fFile);
	fCurrentOffset = fFileSize = 0;
}
