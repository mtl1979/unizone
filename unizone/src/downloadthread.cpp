#include "downloadthread.h"
#include "downloadworker.h"
#include "downloadimpl.h"
#include "wdownloadevent.h"
#include "md5.h"
#include "global.h"
#include "settings.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "qtsupport/QMessageTransceiverThread.h"
#include "debugimpl.h"

#include <qdir.h>

using namespace muscle;

WDownloadThread::WDownloadThread(QObject * owner, bool * optShutdownFlag)
: QMessageTransceiverThread(owner), fOwner(owner), fShutdownFlag(optShutdownFlag), fLockFile(true) 
{
	setName( "WDownloadThread" );

	// qmtt = new QMessageTransceiverThread(this);
	// CHECK_PTR(qmtt);

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
	fCurFile = -1;
	timerID = 0;
	fActive = true;
	fBlocked = false;
	fFinished = false;
	fManuallyQueued = false;
	fLocallyQueued = false;
	fRemotelyQueued = false;
	fDisconnected = false;
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
	connect(qmtt, SIGNAL(SessionAccepted(const String &, uint16)),
			this, SLOT(SessionAccepted(const String &, uint16)));
	connect(qmtt, SIGNAL(SessionConnected(const String &)),
			this, SLOT(SessionConnected(const String &)));
	connect(qmtt, SIGNAL(ServerExited()),
			this, SLOT(ServerExited()));
	connect(qmtt, SIGNAL(SessionDisconnected(const String &)),
			this, SLOT(SessionDisconnected(const String &)));
	*/
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

	if (fFile)
	{
		fFile->close();
		delete fFile;
		fFile = NULL;
	}
	
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

//	qApp->sendPostedEvents( fOwner, WDownloadEvent::Type );

	// Reset();

	PRINT("WDownloadThread dtor OK\n");
}

void 
WDownloadThread::SetFile(QString * files, QString * lfiles, int32 numFiles, QString fromIP, QString fromSession,
						 QString localSession, uint32 remotePort, bool firewalled, bool partial)
{
	fFileDl = files;
	if (lfiles)
	{
		fLocalFileDl = lfiles;
	}
	else
	{
		fLocalFileDl = new QString[numFiles];
		CHECK_PTR(fLocalFileDl);
		for (int l = 0; l < numFiles; l++)
		{
			fLocalFileDl[l] = QString::null;
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
	
	MessageRef msg(GetMessageFromPool(WDownloadEvent::Init));
	if (msg())
	{
		QString dlFile;
		if (fLocalFileDl[0] == QString::null)
		{
			dlFile =  "downloads/";
			dlFile += fFileDl[0];
		}
		else
		{
			dlFile = fLocalFileDl[0];
		}
		msg()->AddString("file", (const char *) dlFile.utf8());
		msg()->AddString("user", (const char *) fromSession.utf8());
		SendReply(msg);	// send the init message to our owner
	}
}

QString 
WDownloadThread::FixFileName(const QString & fixMe)
{
	// bad characters in Windows:
	//	/, \, :, *, ?, ", <, >, |
#ifdef WIN32
	QString ret(fixMe);
	for (unsigned int i = 0; i < ret.length(); i++)
	{
		switch ((QChar) ret.at(i))
		{
		case '/':
		case '\\':
		case ':':
		case '*':
		case '?':
		case '\"':
		case '<':
		case '>':
		case '|':
			ret.replace(i, 1, "_");
			break;
		}
	}
	return ret;
#else
	return fixMe;
#endif
}

bool
WDownloadThread::InitSession()
{
	if (fCurFile == -1) // No more files
	{
		return false;
	}

	if (fCurFile > 0) // Resuming
	{

		// Reinitialize file list
		QString * temp = new QString[fNumFiles-fCurFile];
		QString * temp2 = new QString[fNumFiles-fCurFile];
		if (temp)
		{
			int n = 0;
			for (int f = fCurFile; f < fNumFiles; f++)
			{
				temp[n] = fFileDl[f];
				temp2[n] = fLocalFileDl[f];
				n++;
			}
			fNumFiles = n;
			fCurFile = 0;
			delete [] fFileDl;
			delete [] fLocalFileDl;
			fFileDl = temp;
			fLocalFileDl = temp2;
		}
		else
		{
			// Start from beginning
			fCurFile = 0;
		}
	}
	
	if (!fFirewalled)	// the remote user is not firewalled?
	{
		if (StartInternalThread() == B_OK)
		{
			AbstractReflectSessionRef connectRef;
			
			if (gWin->fSettings->GetDLLimit() != WSettings::LimitNone)
			{
				AbstractReflectSessionRef ref(new ThreadWorkerSession(), NULL);
				ref()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway(), NULL));
				SetRate(WSettings::ConvertToBytes(gWin->fSettings->GetDLLimit()), ref);
				
				connectRef = ref;
			}
			else if (GetRate() != 0) // Reset transfer throttling on resume
			{
				AbstractReflectSessionRef ref(new ThreadWorkerSession(), NULL);
				ref()->SetGateway(AbstractMessageIOGatewayRef(new MessageIOGateway(), NULL));
				ResetRate(ref);
				
				connectRef = ref;
			}
			
			String sIP = (const char *) fIP.utf8(); // <postmaster@raasu.org> 20021026
			if (AddNewConnectSession(sIP, (uint16)fPort, connectRef) == B_OK)
			{
				fCurrentOffset = fFileSize = 0;
				fFile = NULL;
				fDownloading = false;
				MessageRef msg(GetMessageFromPool(WDownloadEvent::ConnectInProgress));
				if (msg())
					SendReply(msg);
				CTimer->start(30000, true); // 30 seconds
				return true;
			}
			else
			{
				Reset();
				MessageRef msg(GetMessageFromPool(WDownloadEvent::ConnectFailed));
				if (msg())
				{
					msg()->AddString("why", QT_TR_NOOP( "Could not add new connect session!" ));
					SendReply(msg);
				}
			}
		}
		else
		{
			MessageRef msg(GetMessageFromPool(WDownloadEvent::ConnectFailed));
			if (msg())
			{
				msg()->AddString("why", QT_TR_NOOP( "Failed to start internal thread!" ));
				SendReply(msg);
			}
		}
	}
	else	// he is firewalled?
	{
		ReflectSessionFactoryRef factoryRef;
		if (gWin->fSettings->GetDLLimit() != WSettings::LimitNone)	// throttling?
		{
			fTXRate = gWin->fSettings->ConvertToBytes( gWin->fSettings->GetDLLimit() );
			factoryRef = ReflectSessionFactoryRef(new WDownloadThreadWorkerSessionFactory(fTXRate), NULL);
		}
		else if (GetRate() != 0)
		{
			factoryRef = ReflectSessionFactoryRef(new WDownloadThreadWorkerSessionFactory(GetRate()), NULL);
		}
		else
		{
			fTXRate = 0;
			factoryRef = ReflectSessionFactoryRef(new ThreadWorkerSessionFactory(), NULL);
		}
		
		status_t ret = B_OK;
		uint32 pStart = (uint32) gWin->fSettings->GetBasePort();
		uint32 pEnd = pStart + (uint32) gWin->fSettings->GetPortRange() - 1;

		for (unsigned int i = pStart; i <= pEnd; i++)
		{
			if ((ret = PutAcceptFactory(i, factoryRef)) == B_OK)
			{
				fAcceptingOn = factoryRef()->GetPort();
				ret = StartInternalThread();
				break;
			}
		}
		if (ret == B_OK)
		{
			fDownloading = false;
			fCurrentOffset = fFileSize = 0;
			MessageRef cnt(GetMessageFromPool(WDownloadEvent::ConnectBackRequest));
			if (cnt())
			{
				cnt()->AddString("session", (const char *) fFromSession.utf8());
				cnt()->AddInt32("port", fAcceptingOn);
				SendReply(cnt);
			}
			CTimer->start(30000, true); // 30 seconds
			return true;
		}
	}
	return false;
}

void 
WDownloadThread::SendReply(MessageRef &m)
{
	if (m())
	{
		if (fShutdownFlag && *fShutdownFlag)
		{
			Reset();
//			return;
		}

//		m()->AddBool("download", true);	// the value doesn't matter
		m()->AddPointer("sender", this);
		WDownloadEvent * wde = new WDownloadEvent(m);
		if (wde)
			QApplication::postEvent(fOwner, wde);
	}
}

void 
WDownloadThread::MessageReceived(MessageRef msg, const String & sessionID)
{
	switch (msg()->what)
	{
		case WDownload::TransferNotifyQueued:
		{
			MessageRef q(GetMessageFromPool(WDownloadEvent::FileQueued));
			if (q())
			{
				SendReply(q);
			}
			SetRemotelyQueued(true);
			break;
		}
					
		case WDownload::TransferNotifyRejected:
		{
			MessageRef q(GetMessageFromPool(WDownloadEvent::FileBlocked));
			uint64 timeleft = (uint64) -1;
			(void) msg()->FindInt64("timeleft", (int64 *) &timeleft);
			if (q())
			{
				if (timeleft != -1)
				{
					q()->AddInt64("timeleft", timeleft);
				}
				SendReply(q);
			}
			SetBlocked(true, timeleft);
			break;
		}
					
		case WDownload::TransferFileHeader:
		{
			if (IsRemotelyQueued())
				SetRemotelyQueued(false);
						
			if (fStartTime == 0)
				fStartTime = GetRunTime64();
						
			if (IsBlocked())
				SetBlocked(false);
						
			if (fDownloading)	// this shouldn't happen yet... we only request a single file at a time
			{
				//fDownloading = false;
				if (fFile)	// in case it was not closed when it was received
				{
					fFile->close();
					delete fFile;
					fFile = NULL;
				}

				if (IsLastFile()) fFinished = true;

				MessageRef done(GetMessageFromPool(WDownloadEvent::FileDone));
				if (done())
				{
					
					done()->AddBool("done", false);
					if (fCurrentOffset < fFileSize) 			// hm... cut off short?
					{					
						done()->AddBool("error", true);
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
			String fname, session;
			if ((msg()->FindInt64("beshare:File Size", (int64 *)&fFileSize) == B_OK) && 
				(msg()->FindString("beshare:File Name", fname) == B_OK))
			{
				if (msg()->FindString("beshare:FromSession", session) == B_OK)
				{
					fFromSession = QString::fromUtf8(session.Cstr());
					QString user = GetUserName(fFromSession);
					if (!user.isEmpty())
						fFromUser = user; 
				}

				QString fixed;
						
				if (fLocalFileDl[fCurFile] == QString::null)
				{
					fixed = "downloads/";
					// we have a "fixed" filename that eliminates characters Windows does not support
					fixed += FixFileName(QString::fromUtf8(fname.Cstr()));
				}
				else
				{
					fixed = fLocalFileDl[fCurFile];
				}
							
				bool append = false;
							
				if (msg()->FindInt64("beshare:StartOffset", (int64 *)&fCurrentOffset) == B_OK)
				{
					if (fCurrentOffset > 0)
					{
						fFile = new QFile(fixed);
						CHECK_PTR(fFile);

						if (fFile->open(IO_ReadOnly))
						{
							if (fFile->size() == fCurrentOffset)	// sizes match up?
								append = true;
							fFile->close();
						}
						delete fFile;
						fFile = NULL;
					}
				}
							
				if (QFile::exists(fixed) && !append)	// create a new file name
				{
					QString nf = fixed;
					int i = 1;
					while (QFile::exists(nf))
					{
						// Check file size again, because it might have changed
						QFile* fFile2 = new QFile(nf);
						CHECK_PTR(fFile2);
						if (fFile2->open(IO_ReadOnly))
						{
							if (fFile2->size() == fCurrentOffset)	// sizes match up?
							{
								append = true;
								fFile2->close();
								delete fFile2;
								fFile2 = NULL;
								break; // Get out of loop
							}
							else
							{
								fFile2->close();
							}
						}

						delete fFile2;
						fFile2 = NULL;
						nf = UniqueName(fixed, i++);
					}
					fixed = nf;
				}
							
				fLocalFileDl[fCurFile] = fixed;
				MessageRef status;

				fFile = new QFile(fixed);	// fixed filename again
				CHECK_PTR(fFile);

				if (fFile->open((append ? IO_Append | IO_WriteOnly : IO_WriteOnly)))
				{
					if (fFileSize != 0)
					{
						if (fCurrentOffset != fFileSize)
						{
							status = GetMessageFromPool(WDownloadEvent::FileStarted);
							if (status())
							{
								status()->AddString("file", (const char *) fixed.utf8());
								status()->AddInt64("start", fCurrentOffset);
								status()->AddInt64("size", fFileSize);
								status()->AddString("user", (const char *) fFromSession.utf8());
								SendReply(status);
							}
							fDownloading = true;
						}
						else
						{
							delete fFile;
							fFile = NULL;

							if (IsLastFile()) fFinished = true;

							status = GetMessageFromPool(WDownloadEvent::FileDone);
							if (status())
							{
								status()->AddBool("done", false);
								status()->AddString("file", (const char *) fixed.utf8());
								SendReply(status);
							}
							NextFile();
						}
					}
				}
				else
				{
					// ERROR!
					// disconnected = true;	// we're done
					delete fFile;
					fFile = NULL;

					status = GetMessageFromPool(WDownloadEvent::FileError);
					if (status())
					{
						status()->AddString("file", (const char *) fixed.utf8());
						status()->AddString("why", QT_TR_NOOP( "Critical error: Could not create file!" ));
						SendReply(status);
					}
					NextFile();
				}	
			}
			else
			{
				//disconnected = true;
				MessageRef status(GetMessageFromPool(WDownloadEvent::FileError));
				if (status())
				{
					status()->AddString("why", QT_TR_NOOP( "Could not read file info!" ));
					SendReply(status);
				}
			}
			break;
		}
					
		case WDownload::TransferFileData:
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
				uint8 * data;
				size_t numBytes;

				if (msg()->FindDataPointer("data", B_RAW_TYPE, (void **)&data, (uint32 *)&numBytes) == B_OK)
				{
									
					// check munge-mode here... not yet
					if (fFile->writeBlock((const char *)data, (uint)numBytes) == (int)numBytes)
					{
						fCurrentOffset += numBytes;

						MessageRef update(GetMessageFromPool(WDownloadEvent::FileDataReceived));
						if (update())
						{
							update()->AddInt64("offset", fCurrentOffset);
							update()->AddInt64("size", fFileSize);
							update()->AddInt32("got", numBytes);
						}
										
						if ((fFileSize != 0) && (fCurrentOffset >= fFileSize))
						{
							if (update())
							{
								update()->AddBool("done", true);	// file done!
								if (fCurFile != -1)
								{
									update()->AddString("file", (const char *) fFileDl[fCurFile].utf8());
								}
							}
							fFile->close();
							delete fFile; 
							fFile = NULL;
							//fDownloading = false;
							NextFile();
						}
						
						if (update())
						{
							SendReply(update);
						}
						
						PRINT("\tWDownload::TransferFileData OK\n");
					}
					else
					{
						// error
						PRINT("\tWDownload::TransferFileData FAIL!!!\n");

						MessageRef error(GetMessageFromPool(WDownloadEvent::FileError));
						if (error())
						{
							error()->AddString("why", QT_TR_NOOP( "Couldn't write file data!" ));
							SendReply(error);
						}
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
WDownloadThread::SessionAccepted(const String &sessionID, uint16 port)
{
	// no need to accept anymore
	RemoveAcceptFactory(0);		
	SessionConnected(sessionID);
}

void
WDownloadThread::SessionConnected(const String &sessionID)
{
	CTimer->stop();

	timerID = startTimer(10000);

	_sessionID = sessionID;

	MessageRef replyMsg(GetMessageFromPool(WDownloadEvent::Connected));
	if (replyMsg())
	{
		SendReply(replyMsg);
	}
					
	MessageRef comID(GetMessageFromPool(WDownload::TransferCommandPeerID));
	if (comID())
	{
		comID()->AddString("beshare:FromUserName", (const char *) gWin->GetUserName().utf8());
		comID()->AddString("beshare:FromSession", (const char *) gWin->GetUserID().utf8());
		comID()->AddBool("unishare:supports_compression", true);
		SendMessageToSessions(comID);
	}
					
	MessageRef neg(GetMessageFromPool(WDownload::TransferFileList));
	if (neg())
	{
		for (int c = 0; c < fNumFiles; c++)
		{
			// check to see wether the file exists
			QString outFile("downloads");
			if (!(outFile.right(1) == "/"))
				outFile += "/";

			QString fixed;
			if (fLocalFileDl[c] == QString:: null)
			{
				fixed = outFile;							
				fixed += FixFileName(fFileDl[c]);
			}
			else
			{
				fixed = fLocalFileDl[c];
			}

			outFile += fFileDl[c];
							
			if (QFile::exists(fixed))
			{
				// get an MD5 hash code out of it
				uint8 digest[MD5_DIGEST_SIZE];
				uint64 fileOffset = 0;	// autodetect file size for offset
				uint64 retBytesHashed = 0;
				uint64 bytesFromBack = fPartial ? PARTIAL_RESUME_SIZE : 0;
								
				MessageRef hashMsg(GetMessageFromPool(WDownloadEvent::FileHashing));
				if (hashMsg())
				{
					hashMsg()->AddString("file", (const char *)fixed.utf8());
					SendReply(hashMsg);
				}
								
				if (HashFileMD5(fixed, fileOffset, bytesFromBack, retBytesHashed, digest, fShutdownFlag) == B_ERROR)
				{
					if (fShutdownFlag && *fShutdownFlag)	// were told to quit?
					{
						// qmtt->ShutdownInternalThread();
						break;
					}
					else	// ERROR?
					{
						MessageRef e(GetMessageFromPool(WDownloadEvent::FileError));
						if (e())
						{
							e()->AddString("file", (const char *) outFile.utf8());
							e()->AddString("why", QT_TR_NOOP( "MD5 hashing failed! Can't resume." ));
							SendReply(e);
						}
					}
				}
				else
				{
					// aha! succeeded!
					neg()->AddInt64("offsets", fileOffset);
								
					if (bytesFromBack > 0)
						neg()->AddInt64("numbytes", retBytesHashed);
									
					neg()->AddData("md5", B_RAW_TYPE, digest, (fileOffset > 0) ? sizeof(digest) : 1);
				}
			}
			neg()->AddString("files", (const char *) fFileDl[c].utf8());
		}
		neg()->AddString("beshare:FromSession", (const char *) fLocalSession.utf8());
		SendMessageToSessions(neg);
	}
}

void
WDownloadThread::ServerExited()
{
	SessionDisconnected(_sessionID);
}

void
WDownloadThread::SessionDisconnected(const String &sessionID)
{
	if (fActive) // Do it only once...
	{
		fActive = false;
		
		if (timerID != 0) 
		{
			killTimer(timerID);
			timerID = 0;
		}
		
		MessageRef dis;
		if (fDownloading)
		{
			if (fCurrentOffset != fFileSize)
			{
				dis = GetMessageFromPool(WDownloadEvent::Disconnected);
				if (dis())
				{
					dis()->AddBool("failed", true);
					SendReply(dis);
				}
			}
			else if (fCurrentOffset == fFileSize)
			{
				if (IsLastFile())
				{
					fFinished = true;
					dis = GetMessageFromPool(WDownloadEvent::FileDone);
					if (dis())
					{
						dis()->AddBool("done", true);		
						dis()->AddString("file", (const char *) fFileDl[fCurFile].utf8());
						SendReply(dis);
					}
				}
				else
				{
					NextFile();
					
					dis = GetMessageFromPool(WDownloadEvent::FileFailed);
					if (dis())
					{
						dis()->AddBool("failed", true);
						SendReply(dis);
					}
				}
			}
		}
		else
		{
			dis = GetMessageFromPool(WDownloadEvent::Disconnected);
			if (dis())
			{
				if (fCurFile == -1)
					dis()->AddBool("failed", false);
				else
					dis()->AddBool("failed", true);
				
				SendReply(dis);
			}
		}
		
		if (fFile != NULL)
		{
			fFile->close();
			delete fFile; 
			fFile = NULL;
		}
		fDownloading = false;
		fDisconnected = true;
	}
}

QString
WDownloadThread::UniqueName(QString file, int index)
{
	QString tmp, base, ext;
	int sp = file.findRev("/", -1); // Find last /
	if (sp > -1)
	{
		tmp = file.left(sp + 1);	// include slash
		base = file.mid(sp + 1);	// base filename
		int d = base.find(".");		// ...and find first dot
		if (d > -1)
		{
			ext = base.mid(d);		// ext contains also the dot
			base = base.left(d);
			return tr("%1%2 %3%4").arg(tmp).arg(base).arg(index).arg(ext);
		}
		else
			return tr("%1%2 %3").arg(tmp).arg(base).arg(index); // no extension
	}
	WASSERT(true, "Invalid download path!");
	return QString::null;
}

void WDownloadThread::NextFile()
{
	fCurrentOffset = 0;
	fFileSize = 0;
	fCurFile++;
	if (fCurFile == fNumFiles)
		fCurFile = -1;
	fDownloading = false;
}

void
WDownloadThread::SetRate(int rate)
{
	fTXRate = rate;
	if (rate != 0)
		SetNewInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		SetNewInputPolicy(PolicyRef(NULL, NULL));
}

void
WDownloadThread::SetRate(int rate, AbstractReflectSessionRef & ref)
{
	fTXRate = rate;
	if (rate != 0)
		ref()->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		ref()->SetInputPolicy(PolicyRef(NULL, NULL));
}


void
WDownloadThread::SetBlocked(bool b, int64 timeLeft)
{
	fBlocked = b;
	if (b)
	{
		fTimeLeft = timeLeft;
		fStartTime = 0;
		MessageRef msg(GetMessageFromPool(WDownloadEvent::FileBlocked));
		if (msg())
		{
			if (fTimeLeft != -1)
				msg()->AddInt64("timeleft", fTimeLeft);
			SendReply(msg);
		}
	}
	else
	{
		fTimeLeft = 0;
		fLastData.restart();
	}
}

QString
WDownloadThread::GetCurrentFile()
{
	if (fCurFile > -1 && fCurFile < fNumFiles)
		return fFileDl[fCurFile];
	else
		return QString::null;
}

QString
WDownloadThread::GetCurrentLocalFile()
{ 
	if (fCurFile > -1 && fCurFile < fNumFiles)
		return fLocalFileDl[fCurFile]; 
	else
		return QString::null;
}

QString
WDownloadThread::GetFileName(int i)
 { 
	if (i > -1 && i < fNumFiles)
		return fFileDl[i]; 
	else
		return QString::null;
}

QString
WDownloadThread::GetLocalFileName(int i)
 {
	if (i > -1 && i < fNumFiles)
		return fLocalFileDl[i]; 
	else
		return QString::null;
}

void
WDownloadThread::timerEvent(QTimerEvent *e)
{
	if (IsInternalThreadRunning())
	{
		MessageRef nop(GetMessageFromPool(PR_COMMAND_NOOP));
		if ( nop() )
		{
			SendMessageToSessions(nop);
		}
	}
	else
	{
		Reset();
	}
}

bool
WDownloadThread::IsRemotelyQueued() const
{
	return fRemotelyQueued;
}

void
WDownloadThread::SetRemotelyQueued(bool b)
{
	fRemotelyQueued = b;
	if (!b)
	{
		fLastData.restart();
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
	SetFinished(true);
	// SetLocallyQueued(false);
	QMessageTransceiverThread::Reset();
	PRINT("WDownloadThread::Reset() OK\n");
}

void
WDownloadThread::SetLocallyQueued(bool b)
{
	fLocallyQueued = b;
	if (b)
	{
		fStartTime = 0;
	}
	else
	{
		fLastData.restart();
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

QString
WDownloadThread::GetETA(uint64 cur, uint64 max, double rate)
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
	fPackets += bytes / ((double) fPacket) ;
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
	return (c >= n); 
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
WDownloadThread::GetUserName(QString sid)
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
WDownloadThread::SendMessageToSessions(MessageRef msgRef, const char * optDistPath)
{
	return QMessageTransceiverThread::SendMessageToSessions(msgRef, optDistPath);
}

bool 
WDownloadThread::IsInternalThreadRunning()
{
	return QMessageTransceiverThread::IsInternalThreadRunning();
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
	MessageRef msg(GetMessageFromPool(WDownloadEvent::ConnectFailed));
	if (msg())
	{
		msg()->AddString("why", QT_TR_NOOP( "Connection timed out!" ));
		SendReply(msg);
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