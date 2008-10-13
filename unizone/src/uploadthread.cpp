#include <qdir.h>
#include <QTimerEvent>
#include <QCustomEvent>
#include <QEvent>

#include "uploadthread.h"
#include "wmessageevent.h"
#include "wuploadevent.h"
#include "wsystemevent.h"
#include "global.h"
#include "uploadimpl.h"
#include "events.h"
#include "settings.h"
#include "netclient.h"
#include "md5.h"
#include "wstring.h"
#include "filethread.h"
#include "debugimpl.h"
#include "util.h"
#include "resolver.h"
#include "wtransfer.h"

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
	fPacket = 8.0f;
	fIdles = 0;
	fCompression = -1;

	InitTransferRate();
	InitTransferETA();

	CTimer = new QTimer(this, "Connect Timer");
	Q_CHECK_PTR(CTimer);

	connect( CTimer, SIGNAL(timeout()), this, SLOT(ConnectTimer()) );
	
	fBlockTimer = new QTimer(this, "Blocked Timer");
	Q_CHECK_PTR(fBlockTimer);

	connect( fBlockTimer, SIGNAL(timeout()), this, SLOT(BlockedTimer()) );

	// QMessageTransceiverThread

	qmtt = new QMessageTransceiverThread(this, "QMessageTransceiverThread");
	Q_CHECK_PTR(qmtt);

	connect(qmtt, SIGNAL(MessageReceived(const MessageRef &, const String &)),
			this, SLOT(MessageReceived(const MessageRef &, const String &)));

	connect(qmtt, SIGNAL(OutputQueuesDrained(const MessageRef &)),
			this, SLOT(OutputQueuesDrained(const MessageRef &)));
	
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
WUploadThread::SetUpload(const SocketRef &socket, uint32 remoteIP, WFileThread * ft)
{
	char host[16];
	fAccept = false;
	fRemoteIP = remoteIP;
	fSocket = socket;
	fFileThread = ft;
	// Set string ip too
	uint32 _ip = GetPeerIPAddress(fSocket, true);
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
WUploadThread::SetUpload(const QString & userID, int64 remoteID, WFileThread * ft)
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

	ThreadWorkerSessionRef limit(new ThreadWorkerSession());

	// First check if IP is blacklisted or ignored
	//

	if (gWin->IsIgnoredIP(fStrRemoteIP))
	{
		SetBlocked(true);
	}

	if (gWin->IsBlackListedIP(fStrRemoteIP) && (gWin->fSettings->GetBLLimit() != WSettings::LimitNone))
	{
		limit()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway()));
		SetRate(WSettings::ConvertToBytes(gWin->fSettings->GetBLLimit()), limit);
	}
	else if (gWin->fSettings->GetULLimit() != WSettings::LimitNone)
	{
		limit()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway()));
		SetRate(WSettings::ConvertToBytes(gWin->fSettings->GetULLimit()), limit);
	}

	if (fTunneled)
	{
		WUploadEvent *wue = new WUploadEvent(WUploadEvent::ConnectInProgress);
		if (wue)
		{
			SendReply(wue);
		}

		return true;
	}
	else if (fAccept)
	{
		// <postmaster@raasu.org> 20021026
		uint32 sRemoteIP = ResolveAddress(fStrRemoteIP); 
		if (qmtt->AddNewConnectSession(sRemoteIP, (uint16)fPort, limit) != B_OK)
		{
			WUploadEvent *fail = new WUploadEvent(WUploadEvent::ConnectFailed);
			if (fail)
			{
				fail->SetError( QT_TR_NOOP( "Couldn't create new connect session!" ) );
				SendReply(fail);
			}
			return false;
		}
	}
	else
	{
		if (qmtt->AddNewSession(fSocket, limit) != B_OK)
		{
			WUploadEvent *fail = new WUploadEvent(WUploadEvent::ConnectFailed);
			if (fail)
			{
				fail->SetError( QT_TR_NOOP( "Could not init session!" ) );
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
		WUploadEvent *wue = new WUploadEvent(WUploadEvent::ConnectInProgress);
		if (wue)
		{
			SendReply(wue);
		}
		CTimer->start(60000, true);
		return true;
	}

	WUploadEvent *fail = new WUploadEvent(WUploadEvent::ConnectFailed);
	if (fail)
	{
		fail->SetError( QT_TR_NOOP( "Could not start internal thread!" ) );
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
				WMessageEvent *wme = new WMessageEvent(fFileList);
				if (wme) 
					QApplication::postEvent(this, wme);
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
				WUploadEvent *con = new WUploadEvent(WUploadEvent::ConnectInProgress);
				if (con)
				{
					SendReply(con);
				}
			}
			else
			{
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
				fBlockTimer->start((int32) (timeLeft/1000), true);
			
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
WUploadThread::SendReply(WUploadEvent *wue)
{
	if (fShutdownFlag && *fShutdownFlag)
	{
		Reset();
	}

	if (wue)
	{
		wue->SetSender(this);
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

	timerID = startTimer(15000);

	WUploadEvent *con = new WUploadEvent(WUploadEvent::Connected);
	if (con)
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
		
		WUploadEvent *dis = new WUploadEvent(WUploadEvent::Disconnected);
		if (dis)
		{
			if (fCurrentOffset < fFileSize || fUploads.GetNumItems() > 0)
			{
				dis->SetFailed(true);
			}
			SendReply(dis);
		}
	}
}

void
WUploadThread::MessageReceived(const MessageRef & msg, const String & /* sessionID */)
{
	PRINT("WUploadThread::MessageReceived\n");
	switch (msg()->what)
	{
		case WTransfer::TransferCommandPeerID:
		{
			PRINT("WUpload::TransferCommandPeerID\n");
			const char *id = NULL;
			if (msg()->FindString("beshare:FromSession", &id) == B_OK)
			{
				fRemoteSessionID = QString::fromUtf8(id);
				
				{
					const char *name = NULL;
					
					if (msg()->FindString("beshare:FromUserName", &name) ==  B_OK)
					{							
						QString user = QString::fromUtf8(name);
						if ((user.isEmpty()) || (fRemoteUser == fRemoteSessionID))
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
				
				WUploadEvent *ui = new WUploadEvent(WUploadEvent::UpdateUI);
				if (ui)
				{
//					ui->SetSession(id);
					SendReply(ui);
				}
				
			}

			bool c = false;
			
			if (!fTunneled && msg()->FindBool("unishare:supports_compression", &c) == B_OK)
			{
				SetCompression(6);
			}

			double dpps = GetPacketSize() * 1024.0f;
			int32 pps = lrint(dpps);
			
			if ((msg()->FindInt32("unishare:preferred_packet_size", &pps) == B_OK) && (pps < lrint(dpps)))
				SetPacketSize((double) pps / 1024.0f);
			break;
		}
	
		case WTransfer::TransferFileList:
		{
			// TransferFileList(msg);
			WMessageEvent *wme = new WMessageEvent(msg);
			if (wme) 
			{
				QApplication::postEvent(this, wme);
			}
			break;
		}
	}
}

void
WUploadThread::OutputQueuesDrained(const MessageRef &/* msg */)
{
	PRINT2("\tMTT_EVENT_OUTPUT_QUEUES_DRAINED\n");
	OutputQueuesDrained();
}

void
WUploadThread::OutputQueuesDrained()
{
	fIdles = 0;

	if (fWaitingForUploadToFinish)
	{
		PRINT("\tfWaitingForUploadToFinish\n");
		SetFinished(true);
		fWaitingForUploadToFinish = false;

		PRINT("\t\tSending message\n");

		{
			WUploadEvent *wue = new WUploadEvent(WUploadEvent::FileDone);
			if (wue)
			{
				wue->SetDone(true);
				wue->SetFile(SimplifyPath(fFileUl));
				PRINT("\t\tSending...\n");
				SendReply(wue);
				PRINT("\t\tSent...\n"); // <postmaster@raasu.org> 20021023 -- Fixed typo
			}
			else
			{
				PRINT("\t\tNot sent!\n");
			}
		}
		
		PRINT("\t\tDisconnecting...\n");
		
		{
			WUploadEvent *dis = new WUploadEvent(WUploadEvent::Disconnected);
			if (dis)
			{
				dis->SetDone(true);
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
	MessageRef q(GetMessageFromPool(WTransfer::TransferNotifyQueued));
	if (q())
	{
		SendMessageToSessions(q);

		WUploadEvent *qf = new WUploadEvent(WUploadEvent::FileQueued);
		if (qf)
		{
			SendReply(qf);
		}
	}
}

void
WUploadThread::SendRejectedNotification(bool direct)
{
	MessageRef q(GetMessageFromPool(WTransfer::TransferNotifyRejected));
	
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
			QString node;
			if (fRemoteSessionID != QString::null)
			{
				node = "/*/";
				node += fRemoteSessionID;
			}
			else
			{
				// use /<ip>/* instead of /*/<sessionid>, because session id isn't yet known at this point
				node = "/";
				node += fStrRemoteIP;
				node += "/*";
			}
			if (
				(q()->AddString(PR_NAME_SESSION, "") == B_NO_ERROR) &&
				(AddStringToMessage(q, PR_NAME_KEYS, node) == B_NO_ERROR) &&
				(q()->AddInt32("port", (int32) fPort) == B_NO_ERROR)
				) 
			{
				gWin->SendRejectedNotification(q);
			}
		}
	}
	WUploadEvent *b = new WUploadEvent(WUploadEvent::FileBlocked);
	if (b)
	{
		if (fTimeLeft != -1)
			b->SetTime(fTimeLeft);

		SendReply(b);
	}
}

void
WUploadThread::_nobuffer()
{
   WUploadEvent *status = new WUploadEvent(WUploadEvent::FileError);
   if (status)
   {
      status->SetFile(SimplifyPath(fFileUl));
      status->SetError( QT_TR_NOOP( "Critical error: Upload buffer allocation failed!" ) );
      SendReply(status);
   }
   
   NextFile();
}

QString
WUploadThread::MakeUploadPath(MessageRef ref)
{
	QString filename;
	if (GetStringFromMessage(ref, "beshare:File Name", filename) == B_OK)
	{
		QString path;
		if (GetStringFromMessage(ref, "winshare:Path", path) == B_OK)
			return MakePath(path, filename);
		if (GetStringFromMessage(ref, "beshare:Path", path) == B_OK)
			return MakePath(path, filename);		
	}
	return QString::null;
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
			WUploadEvent *lq = new WUploadEvent(WUploadEvent::FileQueued);
			if (lq)
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
		WUploadEvent *wue = new WUploadEvent(WUploadEvent::FileBlocked);
		if (wue)
		{
			if (fTimeLeft != -1)
				wue->SetTime(fTimeLeft);
			SendReply(wue);
		}
		return;
	}

	if (fStartTime == 0)
		fStartTime = GetRunTime64();

	if (fFile)
	{
		MessageRef uref(GetMessageFromPool(WTransfer::TransferFileData));
		if (uref())
		{
			// think about doing this in a dynamic way (depending on connection)
			double dpps = GetPacketSize() * 1024.0f;
			uint32 bufferSize = lrint(dpps);	
         ByteBufferRef buf = GetByteBufferFromPool(bufferSize);

         uint8 * scratchBuffer = buf()->GetBuffer();
         if (scratchBuffer == NULL)
         {
            _nobuffer();
            return;
         }

			int32 numBytes = 0;
			numBytes = fFile->ReadBlock32((char *)scratchBuffer, bufferSize);
         if (numBytes > 0)
         {
            buf()->SetNumBytes(numBytes, true);

            // munge mode

            switch (fMungeMode)
            {
               case WTransfer::MungeModeNone:
                  {
                     uref()->AddInt32("mm", WTransfer::MungeModeNone);
                     break;
                  }

               case WTransfer::MungeModeXOR:
                  {
                     for (int32 x = 0; x < numBytes; x++)
                        scratchBuffer[x] ^= 0xFF;
                     uref()->AddInt32("mm", WTransfer::MungeModeXOR);
                     break;
                  }
                  
               default:
                  {
                     break;
                  }
            }

            if (uref()->AddFlat("data", buf) == B_OK)
            {
               // possibly do checksums here
               uref()->AddInt32("chk", CalculateFileChecksum(buf));  // a little paranoia, due to file-resumes not working.... (TCP should handle this BUT...)
               
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
               
               WUploadEvent *update = new WUploadEvent(WUploadEvent::FileDataSent);	
               if (update)
               {
                  update->SetOffset(fCurrentOffset);
                  update->SetSize(fFileSize);
                  update->SetSent(numBytes);
                  
                  if (fCurrentOffset >= fFileSize)
                  {
                     update->SetDone(true);	// file done!
                     update->SetFile(SimplifyPath(fFileUl));
                     
                     if (gWin->fSettings->GetUploads())
                     {
                        SystemEvent( gWin, tr("%1 has finished downloading %2.").arg( GetRemoteUser() ).arg( SimplifyPath(fFileUl) ) );
                     }
                  }
                  SendReply(update);
               }
               
               return;
            }
            else
            {
               _nobuffer();
               return;
            }
			}
				
			if (numBytes <= 0)
			{
				NextFile();
				SignalUpload();
				return;
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
				
				fFileUl = MakeUploadPath(fCurrentRef);

#ifdef _DEBUG
				// <postmaster@raasu.org> 20021023, 20030702 -- Add additional debug message
				WString wul(fFileUl);
				PRINT("WUploadThread::DoUpload: filePath = %S\n", wul.getBuffer()); 
#endif
				
				fFile = new WFile();
				Q_CHECK_PTR(fFile);
				if (!fFile->Open(fFileUl, QIODevice::ReadOnly))	// probably doesn't exist
				{
					delete fFile;
					fFile = NULL;
					fCurFile++;
					continue;	// onward
				}
				// got our file!
				fFileSize = fFile->Size();
				fCurrentOffset = 0;	// from the start
				if (fCurrentRef()->FindInt64("secret:offset", &fCurrentOffset) == B_OK)
				{
					if (!fFile->Seek(fCurrentOffset)) // <postmaster@raasu.org> 20021026
					{
						fFile->Seek(0);	// this can't fail :) (I hope)
						fCurrentOffset = 0;
					}
				}
				// copy the message in our current file ref
				MessageRef headRef = fCurrentRef;
				if (headRef.EnsureRefIsPrivate() == B_NO_ERROR)
				{
					headRef()->what = WTransfer::TransferFileHeader;
					headRef()->AddInt64("beshare:StartOffset", fCurrentOffset);
					SendMessageToSessions(headRef);
				}

				fCurFile++;

				// Reset statistics
				InitTransferRate();
				InitTransferETA();

				WUploadEvent *started = new WUploadEvent(WUploadEvent::FileStarted);
				if (started)
				{
					started->SetFile(SimplifyPath(fFileUl));
					started->SetStart(fCurrentOffset);
					started->SetSize(fFileSize);
#ifdef _DEBUG
					started->SetSession(fRemoteSessionID);
#endif
					SendReply(started);
				}

				if (gWin->fSettings->GetUploads())
				{
					SystemEvent( gWin, tr("%1 is downloading %2.").arg( GetRemoteUser() ).arg( SimplifyPath(fFileUl) ) );
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
					OutputQueuesDrained();
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
	if (!fTunneled)
	{
		if (rate != 0)
			qmtt->SetNewOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate)));
		else
			qmtt->SetNewOutputPolicy(PolicyRef(NULL));
	}
}

void
WUploadThread::SetRate(int rate, ThreadWorkerSessionRef & ref)
{
	fTXRate = rate;
	if (rate != 0)
		ref()->SetOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate)));
	else
		ref()->SetOutputPolicy(PolicyRef(NULL));
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
	case WMessageEvent::MessageEventType:
		{
			WMessageEvent * wme = dynamic_cast<WMessageEvent *>(e);
			if (wme)
			{
				if (wme->MessageType() == WUploadThread::TunnelData)
					MessageReceived(wme->Message(), _sessionID);
				else
					TransferFileList(wme->Message());
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
WUploadThread::TransferFileList(MessageRef msg)
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
		
		QString user;

		if (GetInt32FromMessage(msg, "mm", fMungeMode) != B_OK)
			fMungeMode = WTransfer::MungeModeNone;
		
		GetStringFromMessage(msg, "beshare:FromSession", fRemoteSessionID);
		
		if (GetStringFromMessage(msg, "beshare:FromUserName", user) ==  B_OK)
		{
			if (!user.isEmpty() && (user != fRemoteSessionID))
				fRemoteUser = user;
		}
		if (fRemoteUser == QString::null)
		{
			fRemoteUser = GetUserName(fRemoteSessionID);
		}
		
		QString file;
		
		for (int i = 0; (GetStringFromMessage(msg, "files", i, file) == B_OK); i++)
		{
			MessageRef fileRef;
			
			if (fFileThread->FindFile(file, fileRef))
			{
				if (fileRef()) // <postmaster@raasu.org> 20021023
				{		
					// see if we need to add them
					int64 offset = 0L;
					ByteBufferRef hisDigest;
					uint32 numBytes = 0L;
					if (msg()->FindInt64("offsets", i, (int64 *) &offset) == B_OK &&
						msg()->FindFlat("md5", i, hisDigest) == B_OK && 
						numBytes == MD5_DIGEST_SIZE)
					{
						uint8 myDigest[MD5_DIGEST_SIZE];
						uint64 readLen = 0;
						int64 onSuccessOffset = offset;
						
						for (uint32 j = 0; j < ARRAYITEMS(myDigest); j++)
							myDigest[j] = 'x';
						
						if ((msg()->FindInt64("numbytes", i, (int64 *)&readLen) == B_OK) && (readLen > 0))
						{
							PRINT("\t\tULT: peer requested partial resume\n");
							int64 temp = readLen;
							readLen = offset - readLen; // readLen is now the seekTo value
							offset = temp;				// offset is now the numBytes value
						}
						
						// figure the path to our requested file
						QString file = MakeUploadPath(fileRef);
						
						// Notify window of our hashing
						WUploadEvent *hash = new WUploadEvent(WUploadEvent::FileHashing);
						if (hash)
						{
							hash->SetFile(SimplifyPath(file));
							SendReply(hash);
						}
						
						// Hash
						
						if (HashFileMD5(file, offset, readLen, NULL, myDigest, fShutdownFlag) == B_OK && 
							memcmp((const uint8*) hisDigest()->GetBuffer(), myDigest, sizeof(myDigest)) == 0)
						{
							// put this into our message ref
							fileRef.EnsureRefIsPrivate();
							fileRef()->AddInt64("secret:offset", (int64) onSuccessOffset);
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
		
		if (fUploads.IsEmpty())
		{
			PRINT("WUploadThread: No Files!!!\n");
			Reset();
			return;
		}
		
		if (IsLocallyQueued())
		{
			MessageRef fref;
			fUploads.GetItemAt(0, fref);
			if (fref())
			{
				QString filename = MakeUploadPath(fref);
				
				if (!filename.isEmpty())
				{
					// Check file size if is smaller than minimum size wanted for queue
					int64 filesize;
					if (fref()->FindInt64("beshare:File Size", (int64 *) &filesize) == B_OK)
					{
						if (filesize < gWin->fSettings->GetMinQueuedSize())
						{
							SignalUpload();
							return;
						}
					}
					
					WUploadEvent *init = new WUploadEvent(WUploadEvent::Init);
					if (init)
					{
						init->SetFile(SimplifyPath(filename));
//						init->SetSession(fRemoteSessionID);
						SendReply(init);
					}
				}
			}
		}
		else
		{
			SignalUpload();
			return;
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
	fPackets = 0.0f;
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
WUploadThread::SetPacketSize(double s)
{
	if (fPacket != s)
	{
		if (fPackets > 0.0f)
			fPackets *= fPacket / s;	// Adjust Packet Count
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
WUploadThread::GetETA(int64 cur, int64 max, double rate)
{
	if (rate < 0)
		rate = GetCalculatedRate();
	// d = r * t
	// t = d / r
	int64 left = max - cur;	// amount left
	double dsecs = (double) left / rate;
	uint32 secs = lrint( dsecs );

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
	if (b && timerID)
	{
		killTimer(timerID);
		timerID = 0;
	}
}

double
WUploadThread::GetPacketSize()
{
	return fPacket;
}

int64
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
WUploadThread::SendMessageToSessions(const MessageRef & msgRef, const char * optDistPath)
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
			AddStringToMessage(msgRef, PR_NAME_KEYS, to);
			msgRef()->AddString(PR_NAME_SESSION, "");
			msgRef()->AddBool("upload", true);
			return static_cast<WUpload *>(fOwner)->netClient()->SendMessageToSessions(msgRef);
		}
		else
		{		
			MessageRef up(GetMessageFromPool(NetClient::TUNNEL_MESSAGE));
			if (up())
			{
				QString to("/*/");
				to += fRemoteSessionID;
				to += "/beshare";
				AddStringToMessage(up, PR_NAME_KEYS, to);
				up()->AddString(PR_NAME_SESSION, "");
				up()->AddMessage("message", msgRef);
				up()->AddBool("upload", true);
				up()->AddInt64("tunnel_id", hisID);
				return static_cast<WUpload *>(fOwner)->netClient()->SendMessageToSessions(up);
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
	WUploadEvent *failed = new WUploadEvent(WUploadEvent::ConnectFailed);
	if (failed)
	{
		failed->SetError( QT_TR_NOOP( "Connection timed out!" ) );
		SendReply(failed);
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

QString 
WUploadThread::GetRemoteIP() const 
{
	if (fStrRemoteIP != "127.0.0.1")
		return fStrRemoteIP;
	else
		return gWin->GetLocalIP();
}

int
WUploadThread::GetCompression() const
{
	return fCompression;
}

void
WUploadThread::SetCompression(int c)
{
	if (qmtt)
	{
		if (muscleInRange(c, 0, 9))
		{
			fCompression = c;
			qmtt->SetOutgoingMessageEncoding(MUSCLE_MESSAGE_ENCODING_DEFAULT + c);
		}
	}
}
