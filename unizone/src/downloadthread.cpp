#include "downloadthread.h"
#include "downloadimpl.h"
#include "wgenericevent.h"
#include "md5.h"
#include "global.h"
#include "settings.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "debugimpl.h"

#include <qdir.h>

WDownloadThread::WDownloadThread(QObject * owner, bool * optShutdownFlag)
: WGenericThread(owner, optShutdownFlag) 
{
	setName( "WDownloadThread" );
	fFile = NULL; 
	fFileDl = NULL;
	fLocalFileDl = NULL;
	fNumFiles = -1;
	fCurFile = -1;

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
}

WDownloadThread::~WDownloadThread()
{
	if (fFile)
	{
		fFile->close();
		delete fFile;
	}
	
	if (fFileDl)
		delete [] fFileDl;
	
	if (fLocalFileDl)
		delete [] fLocalFileDl;
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
	
	MessageRef msg(GetMessageFromPool(WGenericEvent::Init));
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
	else if (fCurFile > 0) // Resuming
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
		if (qmtt->StartInternalThread() == B_OK)
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
			if (qmtt->AddNewConnectSession(sIP, (uint16)fPort, connectRef) == B_OK)
			{
				fCurrentOffset = fFileSize = 0;
				fFile = NULL;
				fDownloading = false;
				MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectInProgress));
				if (msg())
					SendReply(msg);
				CTimer->start(30000, true); // 30 seconds
				return true;
			}
			else
			{
				Reset();
				MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectFailed));
				if (msg())
				{
					msg()->AddString("why", QT_TR_NOOP( "Could not add new connect session!" ));
					SendReply(msg);
				}
			}
		}
		else
		{
			MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectFailed));
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
		uint32 pEnd = pStart + 100;

		for (unsigned int i = pStart; i <= pEnd; i++)
		{
			if ((ret = qmtt->PutAcceptFactory(i, factoryRef)) == B_OK)
			{
				fAcceptingOn = factoryRef()->GetPort();
				ret = qmtt->StartInternalThread();
				break;
			}
		}
		if (ret == B_OK)
		{
			fDownloading = false;
			fCurrentOffset = fFileSize = 0;
			MessageRef cnt(GetMessageFromPool(WGenericEvent::ConnectBackRequest));
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
		m()->AddBool("download", true);	// the value doesn't matter
		WGenericThread::SendReply(m);
	}
}

void 
WDownloadThread::MessageReceived(MessageRef msg, const String & sessionID)
{
	switch (msg()->what)
	{
		case WDownload::TransferNotifyQueued:
		{
			MessageRef q(GetMessageFromPool(WGenericEvent::FileQueued));
			if (q())
			{
				SendReply(q);
			}
			SetRemotelyQueued(true);
			break;
		}
					
		case WDownload::TransferNotifyRejected:
		{
			MessageRef q(GetMessageFromPool(WGenericEvent::FileBlocked));
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
				fDownloading = false;
				if (fFile)	// in case it was not closed when it was received
				{
					fFile->close();
					delete fFile;
					fFile = NULL;
				}
				MessageRef done(GetMessageFromPool(WGenericEvent::FileDone));
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
					status = GetMessageFromPool(WGenericEvent::FileStarted);
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
					// ERROR!
					// disconnected = true;	// we're done
					delete fFile;
					fFile = NULL;

					status = GetMessageFromPool(WGenericEvent::FileError);
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
				MessageRef status(GetMessageFromPool(WGenericEvent::FileError));
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
							
			if (fDownloading)
			{
				uint8 * data;
				size_t numBytes;

				if (msg()->FindDataPointer("data", B_RAW_TYPE, (void **)&data, (uint32 *)&numBytes) == B_OK)
				{
									
					// check munge-mode here... not yet
					if (fFile->writeBlock((const char *)data, (uint)numBytes) == (int)numBytes)
					{
						fCurrentOffset += numBytes;

						MessageRef update(GetMessageFromPool(WGenericEvent::FileDataReceived));
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
							fDownloading = false;
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

						MessageRef error(GetMessageFromPool(WGenericEvent::FileError));
						if (error())
						{
							error()->AddString("why", QT_TR_NOOP( "Couldn't write file data!" ));
							SendReply(error);
						}
						Reset();
						fDownloading = false;
						fFile->close();
						delete fFile; 
						fFile = NULL;
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
	qmtt->RemoveAcceptFactory(0);		// no need to accept anymore
	SessionConnected(sessionID);
}

void
WDownloadThread::SessionConnected(const String &sessionID)
{
	CTimer->stop();

	_sessionID = sessionID;

	MessageRef replyMsg(GetMessageFromPool(WGenericEvent::Connected));
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
		qmtt->SendMessageToSessions(comID);
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
								
				MessageRef hashMsg(GetMessageFromPool(WGenericEvent::FileHashing));
				if (hashMsg())
				{
					hashMsg()->AddString("file", (const char *)fixed.utf8());
					SendReply(hashMsg);
				}
								
				if (HashFileMD5(fixed, fileOffset, bytesFromBack, retBytesHashed, digest, fShutdownFlag) == B_ERROR)
				{
					if (fShutdownFlag && *fShutdownFlag)	// were told to quit?
					{
						qmtt->ShutdownInternalThread();
						break;
					}
					else	// ERROR?
					{
						MessageRef e(GetMessageFromPool(WGenericEvent::FileError));
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
		qmtt->SendMessageToSessions(neg);
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
	if (fFinished) // Do it only once...
	{
		return;
	}

	MessageRef dis;
	if (fDownloading)
	{
		if (fCurrentOffset != fFileSize)
		{
			dis = GetMessageFromPool(WGenericEvent::Disconnected);
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
				dis = GetMessageFromPool(WGenericEvent::FileDone);
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

				dis = GetMessageFromPool(WGenericEvent::FileFailed);
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
		dis = GetMessageFromPool(WGenericEvent::Disconnected);
		if (dis())
		{
			if (fCurFile == -1)
				dis()->AddBool("failed", false);
			else
				dis()->AddBool("failed", true);

			SendReply(dis);
		}
	}
					
	fDownloading = false;
	if (fFile != NULL)
	{
		fFile->close();
		delete fFile; 
		fFile = NULL;
	}
	//disconnected = true;
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
}

void
WDownloadThread::SetRate(int rate)
{
	WGenericThread::SetRate(rate);
	if (rate != 0)
		qmtt->SetNewInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		qmtt->SetNewInputPolicy(PolicyRef(NULL, NULL));
}

void
WDownloadThread::SetRate(int rate, AbstractReflectSessionRef ref)
{
	WGenericThread::SetRate(rate, ref);
	if (rate != 0)
		ref()->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		ref()->SetInputPolicy(PolicyRef(NULL, NULL));
}


void
WDownloadThread::SetBlocked(bool b, int64 timeLeft)
{
	WGenericThread::SetBlocked(b, timeLeft);
	if (b)
	{
		MessageRef msg(GetMessageFromPool(WGenericEvent::FileBlocked));
		if (msg())
		{
			if (fTimeLeft != -1)
				msg()->AddInt64("timeleft", fTimeLeft);
			SendReply(msg);
		}
	}
}

// -----------------------------------------------------------------------------
WDownloadThreadWorkerSessionFactory::WDownloadThreadWorkerSessionFactory(int limit)
{
	fLimit = limit;
}

AbstractReflectSession *
WDownloadThreadWorkerSessionFactory::CreateSession(const String & s)
{
	AbstractReflectSession * ref = ThreadWorkerSessionFactory::CreateSession(s);
	if (ref && fLimit != 0)
	{
		ref->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(fLimit), NULL));
	}
	return ref;
}

