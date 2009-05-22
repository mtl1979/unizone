#include <qdir.h>
#include <QTimerEvent>
#include <QEvent>

#include "downloadthread.h"
#include "downloadworker.h"
#include "downloadimpl.h"
#include "events.h"
#include "netclient.h"
#include "wdownloadevent.h"
#include "wmessageevent.h"
#include "wsystemevent.h"
#include "md5.h"
#include "global.h"
#include "platform.h"
#include "settings.h"
#include "util.h"
#include "debugimpl.h"
#include "resolver.h"
#include "wtransfer.h"

#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "qtsupport/QMessageTransceiverThread.h"

#ifdef _DEBUG
#include "wstring.h"
#endif

using namespace muscle;

WDownloadThread::WDownloadThread(QObject * owner, bool * optShutdownFlag)
: QObject(owner), fLockFile() 
{
	fOwner = owner;
	fShutdownFlag = optShutdownFlag;

	setName( "WDownloadThread" );

	// Default status

	if (!fShutdownFlag)					// Force use of Shutdown Flag
	{
		fShutdown = false;
		fShutdownFlag = &fShutdown;
	}

	fFile = NULL; 
	fFileDl = NULL;
	fLocalFileDl = NULL;
	fNumFiles = -1;
	fCurFile = -2;
	timerID = 0;
	fActive = false;
	fBlocked = false;
	fFinished = false;
	fManuallyQueued = false;
	fLocallyQueued = false;
	fRemotelyQueued = false;
	fDisconnected = false;
	fNegotiating = false;
	fConnecting = false;
	fTunneled = false;
	fTXRate = 0;
	fTimeLeft = 0;
	fStartTime = 0;
	fAcceptingOn = 0;
	fPacket = 8.0f;
	fIdles = 0;
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

	connect(qmtt, SIGNAL(SessionAccepted(const String &, uint32)),
			this, SLOT(SessionAccepted(const String &, uint32)));

	connect(qmtt, SIGNAL(SessionDetached(const String &)),
			this, SLOT(SessionDetached(const String &)));

	connect(qmtt, SIGNAL(SessionConnected(const String &)),
			this, SLOT(SessionConnected(const String &)));

	connect(qmtt, SIGNAL(SessionDisconnected(const String &)),
			this, SLOT(SessionDisconnected(const String &)));

	connect(qmtt, SIGNAL(ServerExited()),
			this, SLOT(ServerExited()));

}

WDownloadThread::~WDownloadThread()
{
	PRINT("WDownloadThread dtor\n");
	if (timerID != 0) 
	{
		killTimer(timerID);
		timerID = 0;
	}

	if (fShutdownFlag && !*fShutdownFlag)
	{
		*fShutdownFlag = true;
	}

	CloseFile(fFile);
	
	if (fFileDl)
	{
		delete [] fFileDl;
		fFileDl = NULL;
	}
	
	if (fLocalFileDl)
	{
		delete [] fLocalFileDl;
		fLocalFileDl = NULL;
	}

   if (fPaths)
   {
      delete [] fPaths;
      fPaths = NULL;
   }

	if (!fTunneled)
		qmtt->ShutdownInternalThread();

	PRINT("WDownloadThread dtor OK\n");
}

void 
WDownloadThread::SetFile(QString * files, QString * lfiles, QString * lpaths, int32 numFiles, const QString & fromIP, const QString & fromSession,
						 const QString & localSession, uint32 remotePort, bool firewalled, bool partial)
{
	fFileDl = files;
	if (lfiles)
	{
		fLocalFileDl = lfiles;
	}
	else
	{
		fLocalFileDl = new QString[numFiles];
		Q_CHECK_PTR(fLocalFileDl);
		for (int l = 0; l < numFiles; l++)
		{
			fLocalFileDl[l] = QString::null;
		}
	}
	if (lpaths)
	{
		fPaths = lpaths;
	}
	else
	{
		fPaths = new QString[numFiles];
		Q_CHECK_PTR(fPaths);
		for (int l = 0; l < numFiles; l++)
		{
			fPaths[l] = QString::null;
		}
	}
	fNumFiles = numFiles;
	fCurFile = 0;
	fIP = fromIP;
	fFromSession = fromSession;
	
	QString user = GetUserName(fFromSession);
	if (user.isEmpty())
		fFromUser = fFromSession;
	else
		fFromUser = user;

	fLocalSession = localSession;
	fPort = remotePort;
	fFirewalled = firewalled;
	fPartial = partial;
	
	WDownloadEvent *wde = new WDownloadEvent(WDownloadEvent::Init);
	if (wde)
	{
		QString dlFile;
		if (fLocalFileDl[0] == QString::null)
		{
			fLocalFileDl[0] = MakePath(downloadDir(fPaths[0]), FixFileName(fFileDl[0]));
		}

		dlFile = fLocalFileDl[0];

		wde->SetFile(dlFile);
//		wde->SetSession(fromSession);
		SendReply(wde);	// send the init message to our owner
	}
}

void
WDownloadThread::SetFile(QString *files, QString *lfiles, QString *lpaths, int32 numFiles, const WUserRef &user)
{
	fTunneled = true;
	fFileDl = files;
	if (lfiles)
	{
		fLocalFileDl = lfiles;
	}
	else
	{
		fLocalFileDl = new QString[numFiles];
		Q_CHECK_PTR(fLocalFileDl);
		for (int l = 0; l < numFiles; l++)
		{
			fLocalFileDl[l] = QString::null;
		}
	}
	if (lpaths)
	{
		fPaths = lpaths;
	}
	else
	{
		fPaths = new QString[numFiles];
		Q_CHECK_PTR(fPaths);
		for (int l = 0; l < numFiles; l++)
		{
			fPaths[l] = QString::null;
		}
	}	
	fNumFiles = numFiles;
	fCurFile = 0;
	fIP = user()->GetUserHostName();
	fFromSession = user()->GetUserID();
	
	fFromUser = user()->GetUserName();

	fLocalSession = gWin->GetUserID();

	fPort = user()->GetPort();
	fFirewalled = user()->GetFirewalled();
	fPartial = user()->GetPartial();
	
	WDownloadEvent * wde = new WDownloadEvent(WDownloadEvent::Init);
	if (wde)
	{
		QString dlFile;
		if (fLocalFileDl[0] == QString::null)
		{
			fLocalFileDl[0] = MakePath(downloadDir(fPaths[0]), FixFileName(fFileDl[0]));
		}

		dlFile = fLocalFileDl[0];

		wde->SetFile(dlFile);
//		wde->SetSession(fFromSession);
		SendReply(wde);	// send the init message to our owner
	}

}

bool
WDownloadThread::InitSession()
{
	if (fCurFile == -1) // No more files
	{
		return false;
	}
	
	fIdles = 0;

	if (fCurFile > 0) // Resuming
	{
		// Reinitialize file list
		QString * temp = new QString[fNumFiles-fCurFile];
		QString * temp2 = new QString[fNumFiles-fCurFile];
		QString * temp3 = new QString[fNumFiles-fCurFile];
		if (temp && temp2 && temp3)
		{
			int n = 0;
			for (int f = fCurFile; f < fNumFiles; f++)
			{
				temp[n] = fFileDl[f];
				temp2[n] = fLocalFileDl[f];
				temp3[n] = fPaths[f];
				n++;
			}
			fNumFiles = n;
			delete [] fFileDl;
			delete [] fLocalFileDl;
			delete [] fPaths;
			fFileDl = temp;
			fLocalFileDl = temp2;
			fPaths = temp3;
		}
		// Start from beginning
		fCurFile = 0;
	}
	
	if (fFirewalled)	
	{
		// he is firewalled?
		if (fTunneled)
		{
			// he supports tunnels ;)
			InitSessionAux();
			
			WDownloadEvent *wde = new WDownloadEvent(WDownloadEvent::ConnectInProgress);
			if (wde)
				SendReply(wde);
			
			PRINT("Requesting tunnel...\n");
			MessageRef req(GetMessageFromPool(NetClient::REQUEST_TUNNEL));
			if (req())
				SendMessageToSessions(req);
			
			return true;
		}
		else
		{
			ThreadWorkerSessionFactoryRef factoryRef;
			if (gWin->fSettings->GetDLLimit() != WSettings::LimitNone)	// throttling?
			{
				fTXRate = gWin->fSettings->ConvertToBytes( gWin->fSettings->GetDLLimit() );
				factoryRef.SetRef(new WDownloadThreadWorkerSessionFactory(fTXRate));
			}
			else if (GetRate() != 0)
			{
				factoryRef.SetRef(new WDownloadThreadWorkerSessionFactory(GetRate()));
			}
			else
			{
				fTXRate = 0;
				factoryRef.SetRef(new ThreadWorkerSessionFactory());
			}
			
			status_t ret = B_OK;
			uint32 pStart = (uint32) gWin->fSettings->GetBasePort();
			uint32 pEnd = pStart + (uint32) gWin->fSettings->GetPortRange() - 1;
			
			for (uint32 i = pStart; i <= pEnd; i++)
			{
				if ((ret = qmtt->PutAcceptFactory((uint16) i, factoryRef)) == B_OK)
				{
					if (qmtt->StartInternalThread() == B_OK)
					{
						fAcceptingOn = i;
						InitSessionAux();
						WDownloadEvent *cnt = new WDownloadEvent(WDownloadEvent::ConnectBackRequest);
						if (cnt)
						{
							cnt->SetSession(fFromSession);
							cnt->SetPort(fAcceptingOn);
							SendReply(cnt);
						}
						return true;
					}
					else
					{
						qmtt->RemoveAcceptFactory((uint16) i);
					}
				}
			}
		}
	}
	else
	{
		// the remote user is not firewalled?
		if (qmtt->StartInternalThread() == B_OK)
		{
			ThreadWorkerSessionRef connectRef(new ThreadWorkerSession());
			
			if (gWin->fSettings->GetDLLimit() != WSettings::LimitNone)
			{
				connectRef()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway()));
				SetRate(WSettings::ConvertToBytes(gWin->fSettings->GetDLLimit()), connectRef);
 			}
			else if (GetRate() != 0) // Reset transfer throttling on resume
			{
				connectRef()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway()));
				ResetRate(connectRef);
			}
			
			uint32 sIP = ResolveAddress(fIP); // <postmaster@raasu.org> 20021026
			if (qmtt->AddNewConnectSession(sIP, (uint16)fPort, connectRef) == B_OK)
			{
				InitSessionAux();
				WDownloadEvent *wde = new WDownloadEvent(WDownloadEvent::ConnectInProgress);
				if (wde)
					SendReply(wde);
				return true;
			}
			else
			{
				Reset();
				WDownloadEvent *wde = new WDownloadEvent(WDownloadEvent::ConnectFailed);
				if (wde)
				{
					wde->SetError( QT_TR_NOOP( "Could not add new connect session!" ) );
					SendReply(wde);
				}
			}
		}
		else
		{
			WDownloadEvent *wde = new WDownloadEvent(WDownloadEvent::ConnectFailed);
			if (wde)
			{
				wde->SetError( QT_TR_NOOP( "Failed to start internal thread!" ) );
				SendReply(wde);
			}
		}
	}
	return false;
}

void 
WDownloadThread::SendReply(WDownloadEvent *wde)
{
	if (wde)
	{
		if (fShutdownFlag && *fShutdownFlag)
		{
			Reset();
		}

		wde->SetSender(this);
		QApplication::postEvent(fOwner, wde);
	}
}

void 
WDownloadThread::MessageReceived(const MessageRef & msg, const String & /* sessionID */)
{
	fIdles = 0;

	switch (msg()->what)
	{
		case WTransfer::TransferNotifyQueued:
		{
			WDownloadEvent *q = new WDownloadEvent(WDownloadEvent::FileQueued);
			if (q)
			{
				SendReply(q);
			}
			SetRemotelyQueued(true);
			break;
		}
					
		case WTransfer::TransferNotifyRejected:
		{
			WDownloadEvent *q = new WDownloadEvent(WDownloadEvent::FileBlocked);
			uint64 timeleft = (uint64) -1;
                        (void) msg()->FindInt64("timeleft", timeleft);
			if (q)
			{
				if (timeleft != (uint64) -1)
				{
					q->SetTime(timeleft);
				}
				SendReply(q);
			}
			SetBlocked(true, timeleft);
			break;
		}
					
		case WTransfer::TransferFileHeader:
		{
			fNegotiating = false;
			if (IsRemotelyQueued())
				SetRemotelyQueued(false);
						
			if (fStartTime == 0)
				fStartTime = GetRunTime64();
						
			if (IsBlocked())
				SetBlocked(false);
						
			if (fDownloading)	// this shouldn't happen yet... we only request a single file at a time
			{
				// in case it was not closed when it was received
				CloseFile(fFile);

				if (IsLastFile()) 
					SetFinished(true);

				WDownloadEvent *done = new WDownloadEvent(WDownloadEvent::FileDone);
				if (done)
				{
					done->SetDone(false);
					if (fCurrentOffset < fFileSize) 			// hm... cut off short?
					{					
						done->SetFailed(true);
					}
					SendReply(done);
				}
				NextFile();
			}
						
			// fields:
			//	String		beshare:File Name
			//	Int64		beshare:File Size
			//	String		beshare:FromSession
			//	String		beshare:Path
			//	Int64		beshare:StartOffset
						
			// I only care to get the size and name
			QString fname;
                        if ((msg()->FindInt64("beshare:File Size", fFileSize) == B_OK) &&
				(GetStringFromMessage(msg, "beshare:File Name", fname) == B_OK))
			{
				if (GetStringFromMessage(msg, "beshare:FromSession", fFromSession) == B_OK)
				{
					QString user = GetUserName(fFromSession);
					if (!user.isEmpty())
						fFromUser = user; 
				}

				QString fixed;
						
				if (fLocalFileDl[fCurFile] == QString::null)
				{
					// we have a "fixed" filename that eliminates characters Windows does not support
					fLocalFileDl[fCurFile] = MakePath(downloadDir(fPaths[fCurFile]), FixFileName(fname));
				}

				fixed = fLocalFileDl[fCurFile];
							
				bool append = false;
							
                                if (msg()->FindInt64("beshare:StartOffset", fCurrentOffset) == B_OK)
				{
					if (fCurrentOffset > 0)
					{
						fFile = new WFile();
						Q_CHECK_PTR(fFile);

						if (fFile->Open(fixed, QIODevice::ReadOnly))
						{
							if (fFile->Size() == fCurrentOffset)	// sizes match up?
								append = true;
							CloseFile(fFile);
						}
						else
						{
							delete fFile;
							fFile = NULL;
						}
					}
				}
							
				if (WFile::Exists(fixed) && !append)	// create a new file name
				{
					QString nf = fixed;
					int i = 1;
					while (WFile::Exists(nf))
					{
						// Check file size again, because it might have changed
						WFile* fFile2 = new WFile();
						Q_CHECK_PTR(fFile2);
						if (fFile2->Open(nf, QIODevice::ReadOnly))
						{
							if (fFile2->Size() == fCurrentOffset)	// sizes match up?
							{
								append = true;
								CloseFile(fFile2);
								break; // Get out of loop
							}
							else
							{
								CloseFile(fFile2);
							}
						}
						else
						{
							// Failed to open!
							delete fFile2;
							fFile2 = NULL;
						}
						nf = UniqueName(fixed, i++);
					}
					fixed = nf;
				}
							
				fLocalFileDl[fCurFile] = fixed;
				MessageRef status;

				fFile = new WFile();	// fixed filename again
				Q_CHECK_PTR(fFile);

				if (fFile->Open(fixed, (append ? QIODevice::Append | QIODevice::WriteOnly : QIODevice::WriteOnly)))
				{
					if (fFileSize != 0)
					{
						if (fCurrentOffset != fFileSize)
						{
							// Reset statistics
							InitTransferETA();
							InitTransferRate();

							WDownloadEvent *status = new WDownloadEvent(WDownloadEvent::FileStarted);
							if (status)
							{
								status->SetFile(fixed);
								status->SetStart(fCurrentOffset);
								status->SetSize(fFileSize);
#ifdef _DEBUG
								status->SetSession(fFromSession);
#endif
								SendReply(status);
							}

							if (gWin->fSettings->GetDownloads())
							{
								SystemEvent( gWin, tr("Downloading %1 from %2.").arg( fFileDl[fCurFile] ).arg( GetRemoteUser() ) );
							}

							fDownloading = true;
						}
						else
						{
							CloseFile(fFile);

							if (IsLastFile()) 
								SetFinished(true);

							WDownloadEvent *status = new WDownloadEvent(WDownloadEvent::FileDone);
							if (status)
							{
								status->SetDone(false);
								status->SetFile(fixed);
								SendReply(status);
							}
							NextFile();
						}
					}
				}
				else
				{
					// ERROR!
					delete fFile;
					fFile = NULL;

					WDownloadEvent *status = new WDownloadEvent(WDownloadEvent::FileError);
					if (status)
					{
						status->SetFile(fixed);
						status->SetError( QT_TR_NOOP( "Critical error: Could not create file!" ) );
						SendReply(status);
					}
					NextFile();
				}	
			}
			else
			{
				WDownloadEvent *status = new WDownloadEvent(WDownloadEvent::FileError);
				if (status)
				{
					status->SetError( QT_TR_NOOP( "Could not read file info!" ) );
					SendReply(status);
				}
			}
			break;
		}
					
		case WTransfer::TransferFileData:
		{
			PRINT("\tWDownload::TransferFileData\n");
						
			if (IsRemotelyQueued())
				SetRemotelyQueued(false);
							
			if (IsBlocked())
				SetBlocked(false);
							
			if (fStartTime == 0)
				fStartTime = GetRunTime64();
							
			if (fDownloading && fFile)
			{
				ByteBufferRef buf;
				ssize_t bufsize = 0;

				if (msg()->FindFlat("data", buf) == B_OK)
				{
					// paranoia!!!  This shouldn't be necessary, since TCP is supposed to handle this sort of thing. 
					// But I want to check anyway, just in case.
					bufsize = buf()->GetNumBytes();
					uint32 checksum;
                                        if (msg()->FindInt32("chk", checksum) == B_NO_ERROR)
					{
                  uint32 myChecksum = CalculateFileChecksum(buf);  // a little paranoia (people keep getting munged data -> download-resume failures, why?)
						if (myChecksum != checksum)
						{
							QString errStr = tr("Data Checksum mismatch in file [%1] (mine=%2, his=%3, %4 bytes)").arg(fFileDl[fCurFile]).arg(myChecksum).arg(checksum).arg(bufsize);
							
							if (gWin->fSettings->GetDownloads())
								gWin->SendErrorEvent(errStr);
							
							WDownloadEvent *error = new WDownloadEvent(WDownloadEvent::FileError);
							if (error)
							{
								error->SetError(errStr);
								SendReply(error);
							}
							
							CloseFile(fFile);
							
							Reset();
							return;
						}
					}

					// check munge-mode here... not yet
					if (fFile->WriteBlock(buf()->GetBuffer(), bufsize) == bufsize)
					{
						fFile->Flush();
						fCurrentOffset += bufsize;

						SetPacketSize((double) bufsize / 1024.0f);

						WDownloadEvent *update = new WDownloadEvent(WDownloadEvent::FileDataReceived);
						if (update)
						{
							update->SetOffset(fCurrentOffset);
							update->SetSize(fFileSize);
							update->SetReceived(buf()->GetNumBytes());
						}
										
						if ((fFileSize != 0) && (fCurrentOffset >= fFileSize))
						{
							if (update)
							{
								update->SetDone(true);	// file done!
								if (fCurFile != -1)
								{
									update->SetFile(fFileDl[fCurFile]);
								}
							}

							if (gWin->fSettings->GetDownloads())
							{
								SystemEvent( gWin, tr("Finished downloading %2 from %1.").arg( GetRemoteUser() ).arg( fFileDl[fCurFile] ) );
							}

							CloseFile(fFile);

							NextFile();
						}
						
						if (update)
						{
							SendReply(update);
						}
						
						PRINT("\tWDownload::TransferFileData OK\n");
					}
					else
					{
						// error
						PRINT("\tWDownload::TransferFileData FAIL!!!\n");

						WDownloadEvent *error = new WDownloadEvent(WDownloadEvent::FileError);
						if (error)
						{
							error->SetError( QT_TR_NOOP( "Couldn't write file data!" ) );
							SendReply(error);
						}

						CloseFile(fFile);
							
						Reset();
						NextFile();
					}
				}
			}
			break;
		}
	}
}

void 
WDownloadThread::SessionAccepted(const String &sessionID, uint32 /* port */)
{
	// no need to accept anymore
	qmtt->RemoveAcceptFactory((int16) fAcceptingOn);
	SessionConnected(sessionID);
}

void
WDownloadThread::Accepted(int64 id)
{
	hisID = id;
	SessionConnected((const char *) fFromSession.utf8());
}

void
WDownloadThread::Rejected()
{
	MessageRef rej(GetMessageFromPool(WTransfer::TransferNotifyRejected));
	if (rej())
		MessageReceived(rej, _sessionID);
}

void
WDownloadThread::SessionConnected(const String &sessionID)
{
	fConnecting = false;
	CTimer->stop();

	timerID = startTimer(15000);

	_sessionID = sessionID;
	fNegotiating = true;

	WDownloadEvent *reply = new WDownloadEvent(WDownloadEvent::Connected);
	if (reply)
	{
		SendReply(reply);
	}
					
	MessageRef comID(GetMessageFromPool(WTransfer::TransferCommandPeerID));
	if (comID())
	{
		AddStringToMessage(comID, "beshare:FromUserName", gWin->GetUserName());
		AddStringToMessage(comID, "beshare:FromSession", gWin->GetUserID());
		comID()->AddBool("unishare:supports_compression", true);
		comID()->AddInt32("unishare:preferred_packet_size", lrint((gWin->fSettings->GetPacketSize() * 1024.0)));
		SendMessageToSessions(comID);
	}
					
	MessageRef neg(GetMessageFromPool(WTransfer::TransferFileList));
	if (neg())
	{
		for (int c = 0; c < fNumFiles; c++)
		{
			// check to see wether the file exists
			if (fLocalFileDl[c] == QString:: null)
			{
				fLocalFileDl[c] = MakePath(downloadDir(fPaths[c]), FixFileName(fFileDl[c]));
			}

			// get an MD5 hash code out of it
			uint8 digest[MD5_DIGEST_SIZE];
			int64 fileOffset = 0;	// autodetect file size for offset
			uint64 retBytesHashed = 0;
			int64 bytesFromBack = fPartial ? PARTIAL_RESUME_SIZE : 0;

			if (WFile::Exists(fLocalFileDl[c]))
			{								
				WDownloadEvent *hash = new WDownloadEvent(WDownloadEvent::FileHashing);
				if (hash)
				{
					hash->SetFile(fLocalFileDl[c]);
					SendReply(hash);
				}
								
				if (HashFileMD5(fLocalFileDl[c], fileOffset, bytesFromBack, &retBytesHashed, digest, fShutdownFlag) == B_ERROR)
				{
					if (fShutdownFlag && *fShutdownFlag)	// were told to quit?
					{
						return;
					}
					else	// ERROR?
					{
						WDownloadEvent * e = new WDownloadEvent(WDownloadEvent::FileError);
						if (e)
						{
							e->SetFile(fLocalFileDl[c]);
							e->SetError( QT_TR_NOOP( "MD5 hashing failed! Can't resume." ) );
							SendReply(e);
						}
					}
				}
			}
			neg()->AddInt64("offsets", fileOffset);
			neg()->AddInt64("numbytes", retBytesHashed);
			neg()->AddData("md5", B_RAW_TYPE, digest, (fileOffset > 0) ? sizeof(digest) : 1);
			AddStringToMessage(neg, "files", fFileDl[c]);
		}
		AddStringToMessage(neg, "beshare:FromSession", fLocalSession);
		SendMessageToSessions(neg);
	}
}

void
WDownloadThread::ServerExited()
{
	SessionDisconnected(_sessionID);
}

void
WDownloadThread::SessionDisconnected(const String & /* sessionID */)
{
	*fShutdownFlag = true;

	CloseFile(fFile);

	if (timerID != 0) 
	{
		killTimer(timerID);
		timerID = 0;
	}
		
	if (fActive) // Do it only once...
	{
		fActive = false;
		
		WDownloadEvent *dis;
		if (fDownloading)
		{
			if (fCurrentOffset != fFileSize)
			{
				dis = new WDownloadEvent(WDownloadEvent::Disconnected);
				if (dis)
				{
					dis->SetFailed(true);
					SendReply(dis);
				}
			}
			else if (fCurrentOffset == fFileSize)
			{
				if (IsLastFile())
				{
					dis = new WDownloadEvent(WDownloadEvent::FileDone);
					if (dis)
					{
						dis->SetDone(true);		
						dis->SetFile(fFileDl[fNumFiles - 1]);
						SendReply(dis);
					}
				}
				else
				{
					NextFile();
					
					dis = new WDownloadEvent(WDownloadEvent::FileFailed);
					if (dis)
					{
						dis->SetFailed(true);
						SendReply(dis);
					}
				}
			}
		}
		else
		{
			dis = new WDownloadEvent(WDownloadEvent::Disconnected);
			if (dis)
			{
				if (fCurFile == -1)
					dis->SetFailed(false);
				else
					dis->SetFailed(true);
				
				SendReply(dis);
			}
		}
		
		fDownloading = false;
		fDisconnected = true;
		if (!fManuallyQueued)
			SetFinished(true);
	}
}

void WDownloadThread::NextFile()
{
	fCurrentOffset = 0;
	fFileSize = 0;
	fCurFile++;
	if (fCurFile == fNumFiles)
	{
		fCurFile = -1;
		if (fTunneled)
			SessionDisconnected(_sessionID);
	}
	fDownloading = false;
}

void
WDownloadThread::SetRate(int rate)
{
	fTXRate = rate;
	if (rate != 0)
		qmtt->SetNewInputPolicy(AbstractSessionIOPolicyRef(new RateLimitSessionIOPolicy(rate)));
	else
		qmtt->SetNewInputPolicy(AbstractSessionIOPolicyRef(NULL));
}

void
WDownloadThread::SetRate(int rate, ThreadWorkerSessionRef & ref)
{
	fTXRate = rate;
	if (rate != 0)
		ref()->SetInputPolicy(AbstractSessionIOPolicyRef(new RateLimitSessionIOPolicy(rate)));
	else
		ref()->SetInputPolicy(AbstractSessionIOPolicyRef(NULL));
}


void
WDownloadThread::SetBlocked(bool b, int64 timeLeft)
{
	if (fBlocked != b)
	{
		fBlocked = b;
		if (b)
		{
			fTimeLeft = timeLeft;
			fStartTime = 0;
			WDownloadEvent *wde = new WDownloadEvent(WDownloadEvent::FileBlocked);
			if (wde)
			{
				if (fTimeLeft != -1)
					wde->SetTime(fTimeLeft);
				SendReply(wde);
			}
		}
		else
		{
			fTimeLeft = 0;
			fLastData.restart();
			InitTransferETA();
			InitTransferRate();
		}
	}
}

QString
WDownloadThread::GetCurrentFile() const
{
	if (fCurFile > -1 && fCurFile < fNumFiles)
		return fFileDl[fCurFile];
	else
		return QString::null;
}

QString
WDownloadThread::GetCurrentLocalFile() const
{ 
	if (fCurFile > -1 && fCurFile < fNumFiles)
		return fLocalFileDl[fCurFile]; 
	else
		return QString::null;
}

QString
WDownloadThread::GetFileName(int i) const
 { 
	if (i > -1 && i < fNumFiles)
		return fFileDl[i]; 
	else
		return QString::null;
}

QString
WDownloadThread::GetLocalFileName(int i) const
{
	if (i > -1 && i < fNumFiles)
		return fLocalFileDl[i]; 
	else
		return QString::null;
}

QString
WDownloadThread::GetPath(int i) const
{
	if (i > -1 && i < fNumFiles)
		return fPaths[i]; 
	else
		return QString::null;
}

void
WDownloadThread::timerEvent(QTimerEvent * /* e */)
{
	if (IsInternalThreadRunning())
	{
		if (fRemotelyQueued || fBlocked || fNegotiating)	
		{
			// Remotely queued or blocked or hashing transfer don't need idle check restricting
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
	// Manually queued files don't need any special handling
	if (fManuallyQueued)
		return;

	// fall through
	ConnectTimer();
}

bool
WDownloadThread::IsRemotelyQueued() const
{
	return fRemotelyQueued;
}

void
WDownloadThread::SetRemotelyQueued(bool b)
{
	if (fRemotelyQueued != b)
	{
		fRemotelyQueued = b;
		if (!b)
		{
			fLastData.restart();
			InitTransferETA();
			InitTransferRate();
		}
	}
}

void
WDownloadThread::InitTransferRate()
{
	for (int i = 0; i < MAX_RATE_COUNT; i++)
	{
		fRate[i] = 0.0f;
	}

	fRateCount = 0;
	fPackets = 0.0f;
}

void
WDownloadThread::InitTransferETA()
{
	for (int i = 0; i < MAX_ETA_COUNT; i++)
	{
		fETA[i] = 0;
	}

	fETACount = 0;
}

void
WDownloadThread::Reset()
{
	PRINT("WDownloadThread::Reset()\n");
	if (!fManuallyQueued)
		SetFinished(true);

	if (fTunneled)
		SessionDisconnected(_sessionID);
	else
		qmtt->Reset();
	
	// Make sure we close the file if user queues the transfer manually
	CloseFile(fFile);

	PRINT("WDownloadThread::Reset() OK\n");
}

void
WDownloadThread::SetLocallyQueued(bool b)
{
	if (fLocallyQueued != b)
	{
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
		}
	}
}

bool
WDownloadThread::IsLocallyQueued() const
{
	return fLocallyQueued;
}

bool
WDownloadThread::IsManuallyQueued() const
{
	return fManuallyQueued;
}

bool
WDownloadThread::IsFinished() const
{
	return fFinished;
}

bool
WDownloadThread::IsActive() const
{
	return fActive;
}

bool
WDownloadThread::IsConnecting() const
{
	return fConnecting;
}

QString
WDownloadThread::GetETA(int64 cur, int64 max, double rate)
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
WDownloadThread::GetCalculatedRate() const
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
WDownloadThread::SetPacketCount(double bytes)
{
	fPackets += bytes / fPacket;
}

void
WDownloadThread::SetPacketSize(double s)
{
	if (s != fPacket)
	{
		if (fPackets > 0.0f)
			fPackets *= fPacket / s;	// Adjust Packet Count
		fPacket = s;
	}
}

double
WDownloadThread::GetPacketSize()
{
	return fPacket;
}

void
WDownloadThread::SetMostRecentRate(double rate)
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
WDownloadThread::IsLastFile()
{ 
	int c = GetCurrentNum() + 1;
	int n = GetNumFiles();
	return ((c == 0) || (c >= n)); 
}

void
WDownloadThread::SetActive(bool b)
{
	fActive = b;
}

void
WDownloadThread::SetFinished(bool b)
{
	fFinished = b;

	if (b && timerID)
	{
		killTimer(timerID);
		timerID = 0;
	}	
}

void
WDownloadThread::SetManuallyQueued(bool b)
{
	fManuallyQueued = b;	
}

bool
WDownloadThread::IsBlocked() const
{
	return fBlocked;
}

QString
WDownloadThread::GetUserName(const QString & sid) const
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
WDownloadThread::SendMessageToSessions(const MessageRef & msgRef, const char * optDistPath)
{
	if (fTunneled)
	{
		if (msgRef()->what == NetClient::REQUEST_TUNNEL)
		{
			// Send directly...
			QString to("/*/");
			to += fFromSession;
			to += "/beshare";
			AddStringToMessage(msgRef, PR_NAME_KEYS, to);
			msgRef()->AddString(PR_NAME_SESSION, "");
			msgRef()->AddInt64("my_id", ConvertPtr(this));
#ifdef _DEBUG
			WString wto(to);
			PRINT("Sending message to %S...\n", wto.getBuffer());
#endif
			return static_cast<WDownload *>(fOwner)->netClient()->SendMessageToSessions(msgRef);
		}
		else
		{
			MessageRef down(GetMessageFromPool(NetClient::TUNNEL_MESSAGE));
			if (down())
			{
				QString to("/*/");
				to += fFromSession;
				to += "/beshare";
				AddStringToMessage(down, PR_NAME_KEYS, to);
				down()->AddString(PR_NAME_SESSION, "");
				down()->AddMessage("message", msgRef);
				down()->AddInt64("tunnel_id", (int64) hisID);
				return static_cast<WDownload *>(fOwner)->netClient()->SendMessageToSessions(down);
			}
			else
				return B_ERROR;
		}
	}
	else
		return qmtt->SendMessageToSessions(msgRef, optDistPath);
}

bool 
WDownloadThread::IsInternalThreadRunning()
{
	if (fTunneled)
		return gWin->IsConnected(fFromSession);
	else
		return qmtt->IsInternalThreadRunning();
}

uint32
WDownloadThread::ComputeETA() const
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
WDownloadThread::SetMostRecentETA(uint32 eta)
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
WDownloadThread::ConnectTimer()
{
	Reset();
	WDownloadEvent *wde = new WDownloadEvent(WDownloadEvent::ConnectFailed);
	if (wde)
	{
		wde->SetError( QT_TR_NOOP( "Connection timed out!" ) );
		SendReply(wde);
	}
}

void
WDownloadThread::BlockedTimer()
{
	SetBlocked(false);
	fTimeLeft = 0;
}

void
WDownloadThread::SessionDetached(const String &sessionID)
{
	SessionDisconnected(sessionID);
}

void
WDownloadThread::InitSessionAux()
{
	fCurrentOffset = fFileSize = 0;
	fFile = NULL;
	fDownloading = false;
	fConnecting = true;
	fActive = true;
	CTimer->start(30000, true); // 30 seconds
}

QString 
WDownloadThread::GetRemoteIP() const 
{
	if (fIP != "127.0.0.1")
		return fIP;
	else
		return gWin->GetLocalIP();
}

bool
WDownloadThread::event(QEvent * e)
{
	if ((int) e->type() == WMessageEvent::MessageEventType)
	{
		WMessageEvent *wme = dynamic_cast<WMessageEvent *>(e);
		if (wme)
		{
			MessageReceived(wme->Message(), _sessionID);
			return true;
		}
	}
	return QObject::event(e);
}

