#include "downloadthread.h"
#include "downloadimpl.h"
#include "wgenericevent.h"
#include "accept.h"
#include "md5.h"
#include "global.h"
#include "settings.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "platform.h"	// <postmaster@raasu.org> 20021114
#include "lang.h"		// <postmaster@raasu.org> 20030224
#include <qdir.h>

WDownloadThread::WDownloadThread(QObject * owner, bool * optShutdownFlag)
		: WGenericThread(owner, optShutdownFlag) 
{
	setName( "WDownloadThread" );
	fFile = NULL; 
	fFileDl = NULL;
	fNumFiles = -1;
	fCurFile = -1;
	CTimer = new QTimer(this, "Connect Timer");
	connect( CTimer , SIGNAL(timeout()), this, SLOT(ConnectTimer()) );
}

WDownloadThread::~WDownloadThread()
{
	if (fFile)
		delete fFile;

	if (fFileDl)
		delete [] fFileDl;
}

void 
WDownloadThread::SetFile(QString * files, int32 numFiles, QString fromIP, QString fromSession,
						QString localSession, int32 remotePort, bool firewalled, bool partial)
{
	fFileDl = files;
	fNumFiles = numFiles;
	fCurFile = 0;
	fIP = fromIP;
	fFromSession = fromSession;
	fFromUser = GetUserName(fFromSession);
	fLocalSession = localSession;
	fPort = remotePort;
	fFirewalled = firewalled;
	fPartial = partial;

	Message * msg = new Message(WGenericEvent::Init);
	QString dlFile = "downloads";
	dlFile += "/";
	dlFile += fFileDl[0];
	msg->AddString("file", (const char *) dlFile.utf8());
	msg->AddString("user", (const char *) fromSession.utf8());
	SendReply(msg);	// send the init message to our owner
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
			String sIP = (const char *) fIP.utf8(); // <postmaster@raasu.org> 20021026
			if (AddNewConnectSession(sIP, (uint16)fPort, connectRef) == B_OK)
			{
				fCurrentOffset = fFileSize = 0;
				fFile = NULL;
				fDownloading = false;
				Message * msg = new Message(WGenericEvent::ConnectInProgress);
				SendReply(msg);
				CTimer->start(60000, true);
				return true;
			}
			else
			{
				Reset();
				Message * msg = new Message(WGenericEvent::ConnectFailed);
				msg->AddString("why", "Could not add new connect session!");
				SendReply(msg);
			}
		}
		else
		{
			Message * msg = new Message(WGenericEvent::ConnectFailed);
			msg->AddString("why", "Failed to start internal thread!");
			SendReply(msg);
		}
	}
	else	// he is firewalled?
	{
		ReflectSessionFactoryRef factoryRef;
		if (gWin->fSettings->GetDLLimit() != WSettings::LimitNone)	// throttling?
		{
			ReflectSessionFactoryRef fref(new WDownloadThreadWorkerSessionFactory(gWin->fSettings->GetDLLimit()), NULL);
			factoryRef = fref;
		}
		else
		{
			ReflectSessionFactoryRef fref(new ThreadWorkerSessionFactory(), NULL);
			factoryRef = fref;
		}
		
		status_t ret = B_OK;
		for (int i = 0; i <= DEFAULT_LISTEN_PORT + LISTEN_PORT_RANGE; i++)
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
			Message * cnt = new Message(WGenericEvent::ConnectBackRequest);
			cnt->AddString("session", (const char *) fFromSession.utf8());
			cnt->AddInt32("port", fAcceptingOn);
			SendReply(cnt);
			CTimer->start(180000, true); // 180 seconds
			return true;
		}
	}
	return false;
}

void 
WDownloadThread::SendReply(Message * m)
{
	if (m)
	{
		m->AddBool("download", true);	// the value doesn't matter
		WGenericThread::SendReply(m);
	}
}

void 
WDownloadThread::SignalOwner()	// sent by the MTT when we have some data
{
	MessageRef next;
	uint32 code;
	String sid;
	uint16 port;
	bool disconnected = false;

	CTimer->stop();

	while (GetNextEventFromInternalThread(code, &next, &sid, &port) >= 0)
	{
		switch (code)
		{
			case MTT_EVENT_INCOMING_MESSAGE:
			{
				switch (next()->what)
				{
					case WDownload::TransferNotifyQueued:
					{
						Message * q = new Message(WGenericEvent::FileQueued);
						SendReply(q);
						SetRemotelyQueued(true);
						CTimer->start(60000, true);
						break;
					}

					case WDownload::TransferNotifyRejected:
						{
							Message * q = new Message(WGenericEvent::FileBlocked);
							uint64 timeleft = (uint64) -1;
							(void) next()->FindInt64("timeleft", (int64 *) &timeleft);
							if (timeleft != -1)
								q->AddInt64("timeleft", timeleft);
							SendReply(q);
							SetBlocked(true, timeleft);
						}

					case WDownload::TransferFileHeader:
					{
						if (IsRemotelyQueued())
							SetRemotelyQueued(false);

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
							Message * msg = new Message(WGenericEvent::FileDone);
							msg->AddBool("done", false);
							if (fCurrentOffset < fFileSize)				// hm... cut off short?
							{					
								msg->AddBool("error", true);
							}
							SendReply(msg);
							fCurrentOffset = fFileSize = 0;
							NextFile();
						}
						// fields:
						//	String		beshare:File Name
						//	Int64		beshare:File Size
						//	String		beshare:From Session
						//	String		beshare:Path
						//	Int64		beshare:StartOffset
						// i only care to get the size and name
						String fname;
						if (next()->FindInt64("beshare:File Size", (int64 *)&fFileSize) == B_OK && 
							next()->FindString("beshare:File Name", fname) == B_OK)
						{
							QString outFile = "downloads/";
							QString fixed;
							// outFile += "/";
							fixed = outFile;
							outFile += QString::fromUtf8(fname.Cstr());
							PRINT( "WDownloadThread::SignalOwner: %s\n",fname.Cstr() );
							fixed += FixFileName(QString::fromUtf8(fname.Cstr()));	// we have a "fixed" filename that eliminates characters Windows does not support

							bool append = false;

							if (next()->FindInt64("beshare:StartOffset", (int64 *)&fCurrentOffset) == B_OK &&
								fCurrentOffset > 0)
							{
								fFile = new QFile(fixed);
								if (fFile->open(IO_ReadOnly))
								{
									if (fFile->size() == fCurrentOffset)	// sizes match up?
										append = true;
									fFile->close();
								}
								delete fFile;
								fFile = NULL;
							}
							else
							{
								QDir cd(".");
								if (!cd.exists("downloads"))
									cd.mkdir("downloads");
							}

							fFile = new QFile(fixed);	// fixed filename again
							if (fFile->exists() && !append)	// create a new file name
							{
								QString nf = fixed;
								int i = 1;
								while (QFile::exists(nf))
									// nf = QObject::tr("%1 %2").arg(fixed).arg(i++);
									nf = UniqueName(fixed, i++);
								delete fFile;
								fFile = NULL; // <postmaster@raasu.org> 20021027
								fixed = nf;
								fFile = new QFile(fixed);
							}

							Message * msg;
							if (fFile->open((append ? IO_Append | IO_WriteOnly : IO_WriteOnly)))
							{
								msg = new Message(WGenericEvent::FileStarted);
								msg->AddString("file", (const char *) fixed.utf8());
								msg->AddInt64("start", fCurrentOffset);
								msg->AddInt64("size", fFileSize);
								msg->AddString("user", (const char *) fFromSession.utf8());
								fDownloading = true;
								SendReply(msg);
							}
							else
							{
								// ERROR!
								disconnected = true;	// we're done
								msg = new Message(WGenericEvent::FileError);
								msg->AddString("file", (const char *) outFile.utf8());
								msg->AddString("why", "Critical error: Could not create file!");
								delete fFile;
								fFile = NULL;
								SendReply(msg);
								NextFile();
							}	
						}
						else
						{
							disconnected = true;
							Message * msg = new Message(WGenericEvent::FileError);
							msg->AddString("why", "Could not read file info!");
							SendReply(msg);
						}
						break;
					}

					case WDownload::TransferFileData:
					{
						if (IsRemotelyQueued())
							SetRemotelyQueued(false);

						if (IsBlocked())
							SetBlocked(false);

						if (fDownloading)
						{
							uint8 * data;
							size_t numBytes;
							if (next()->FindDataPointer("data", B_RAW_TYPE, (void **)&data, (uint32 *)&numBytes) == B_OK)
							{
								// check munge-mode here... not yet
								if (fFile->writeBlock((const char *)data, (uint)numBytes) == (int)numBytes)
								{
									Message * update = new Message(WGenericEvent::FileDataReceived);
									fCurrentOffset += numBytes;
									update->AddInt64("offset", fCurrentOffset);
									update->AddInt64("size", fFileSize);
									update->AddInt32("got", numBytes);

									if (fCurrentOffset >= fFileSize)
									{
										update->AddBool("done", true);	// file done!
										update->AddString("file", (const char *) fFileDl[fCurFile].utf8());
										fFile->close();
										delete fFile; 
										fFile = NULL;
										fDownloading = false;
										NextFile();
									}
									SendReply(update);
								}
								else
								{
									// error
									Message * error = new Message(WGenericEvent::FileError);
									error->AddString("why", "Couldn't write file data!");
									SendReply(error);
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
				break;
			}

			case MTT_EVENT_SESSION_ACCEPTED:
				RemoveAcceptFactory(0);		// no need to accept anymore
				// fall through
			case MTT_EVENT_SESSION_CONNECTED:
			{
				Message * replyMsg = new Message(WGenericEvent::Connected);
				SendReply(replyMsg);
				
				MessageRef comID(new Message(WDownload::TransferCommandPeerID), NULL);
				comID()->AddString("beshare:FromUserName", (const char *) gWin->GetUserName().utf8());
				comID()->AddString("beshare:FromSession", (const char *) gWin->GetUserID().utf8());
				SendMessageToSessions(comID);

				MessageRef neg(new Message(WDownload::TransferFileList), NULL);
				for (int c = 0; c < fNumFiles; c++)
				{
					// check to see wether the file exists
					QString outFile("downloads");
					if (!(outFile.right(1) == "/"))
						outFile += "/";
					QString fixed = outFile;
					outFile += fFileDl[c];
					fixed += FixFileName(fFileDl[fCurFile]);
					
					if (QFile::exists(fixed))
					{
						// get an MD5 hash code out of it
						uint8 digest[MD5_DIGEST_SIZE];
						uint64 fileOffset = 0;	// autodetect file size for offset
						uint64 retBytesHashed = 0;
						uint64 bytesFromBack = fPartial ? PARTIAL_RESUME_SIZE : 0;
						
						Message * hashMsg = new Message(WGenericEvent::FileHashing);
						hashMsg->AddString("file", (const char *)fixed.utf8());
						SendReply(hashMsg);
						
						if (HashFileMD5(fixed, fileOffset, bytesFromBack, retBytesHashed, digest, fShutdownFlag) == B_ERROR)
						{
							if (fShutdownFlag && *fShutdownFlag)	// were told to quit?
							{
								ShutdownInternalThread();
								break;
							}
							else	// ERROR?
							{
								Message * e = new Message(WGenericEvent::FileError);
								e->AddString("file", (const char *) outFile.utf8());
								e->AddString("why", "MD5 hashing failed! Can't resume.");
								SendReply(e);
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
				break;
			}

			case MTT_EVENT_SESSION_DISCONNECTED:
			case MTT_EVENT_SERVER_EXITED:		// same handler for the both of these
			{
				Message * dis = NULL;
				if (fDownloading && (fCurrentOffset != fFileSize))
				{
					dis = new Message(WGenericEvent::Disconnected);
					dis->AddBool("failed", true);
				}
				else if (fDownloading && (fCurrentOffset == fFileSize))
				{
					dis = new Message(WGenericEvent::FileDone);
					dis->AddBool("done", true);		
					dis->AddString("file", (const char *) fFileDl[fCurFile].utf8());
				}
				else
				{
					dis = new Message(WGenericEvent::Disconnected);
					dis->AddBool("failed", false);
				}

				fDownloading = false;
				if (fFile != NULL)
				{
					fFile->close();
					delete fFile; 
					fFile = NULL;
				}
				SendReply(dis);
				disconnected = true;
				break;
			}
		}
	}
}

QString
WDownloadThread::UniqueName(QString file, int index)
{
	QString tmp, base, ext;
	int sp = file.findRev("/", -1); // Find last /
	if (sp > -1)
	{
		tmp = file.left(sp + 1); // include slash
		base = file.mid(sp + 1); // base filename
		int d = base.find("."); // ...and find first dot
		if (d > -1)
		{
			ext = base.mid(d); // ext contains also the dot
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
	fCurFile++;
	if (fCurFile == fNumFiles)
		fCurFile = -1;
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
	if (ref && fLimit != WSettings::LimitNone)
	{
		ref->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(WSettings::ConvertToBytes(
							fLimit)), NULL));
	}
	return ref;
}

void
WDownloadThread::SetRate(int rate)
{
	WGenericThread::SetRate(rate);
	if (rate != 0)
		SetNewInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		SetNewInputPolicy(PolicyRef(NULL, NULL));
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
		Message * msg = new Message(WGenericEvent::FileBlocked);
		if (msg)
		{
			if (fTimeLeft != -1)
				msg->AddInt64("timeleft", fTimeLeft);
			SendReply(msg);
		}
	}
}