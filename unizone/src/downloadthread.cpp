#include "downloadthread.h"
#include "downloadimpl.h"
#include "wgenericevent.h"
#include "accept.h"
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
					msg()->AddString("why", "Could not add new connect session!");
					SendReply(msg);
				}
			}
		}
		else
		{
			MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectFailed));
			if (msg())
			{
				msg()->AddString("why", "Failed to start internal thread!");
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
		uint32 pEnd = pStart + LISTEN_PORT_RANGE;

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
						(void) next()->FindInt64("timeleft", (int64 *) &timeleft);
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
							MessageRef msg(GetMessageFromPool(WGenericEvent::FileDone));
							if (msg())
							{
								msg()->AddBool("done", false);
								if (fCurrentOffset < fFileSize)				// hm... cut off short?
								{					
									msg()->AddBool("error", true);
								}
								SendReply(msg);
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
						if (next()->FindInt64("beshare:File Size", (int64 *)&fFileSize) == B_OK && 
							next()->FindString("beshare:File Name", fname) == B_OK)
						{
							if (next()->FindString("beshare:FromSession", session) == B_OK)
							{
								fFromSession = QString::fromUtf8(session.Cstr());
								QString user = GetUserName(fFromSession);
								if (!user.isEmpty())
									fFromUser = user; 
							}
							// QString outFile = "downloads/";
							QString fixed;
							
							if (fLocalFileDl[fCurFile] == QString::null)
							{
								fixed = "downloads/";
								// PRINT( "WDownloadThread::SignalOwner: %s\n",fname.Cstr() );
								// we have a "fixed" filename that eliminates characters Windows does not support
								fixed += FixFileName(QString::fromUtf8(fname.Cstr()));
							}
							else
							{
								fixed = fLocalFileDl[fCurFile];
							}
							
							// outFile += QString::fromUtf8(fname.Cstr());
							bool append = false;
							
							if (next()->FindInt64("beshare:StartOffset", (int64 *)&fCurrentOffset) == B_OK)
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
//							else
//							{
//								QDir cd(".");
//								if (!cd.exists("downloads"))
//									cd.mkdir("downloads");
//							}
							
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
							MessageRef msg;
							fFile = new QFile(fixed);	// fixed filename again
							CHECK_PTR(fFile);
							if (fFile->open((append ? IO_Append | IO_WriteOnly : IO_WriteOnly)))
							{
								msg = GetMessageFromPool(WGenericEvent::FileStarted);
								if (msg())
								{
									msg()->AddString("file", (const char *) fixed.utf8());
									msg()->AddInt64("start", fCurrentOffset);
									msg()->AddInt64("size", fFileSize);
									msg()->AddString("user", (const char *) fFromSession.utf8());
									SendReply(msg);
								}
								fDownloading = true;
							}
							else
							{
								// ERROR!
								disconnected = true;	// we're done
								delete fFile;
								fFile = NULL;
								msg = GetMessageFromPool(WGenericEvent::FileError);
								if (msg())
								{
									msg()->AddString("file", (const char *) fixed.utf8());
									msg()->AddString("why", "Critical error: Could not create file!");
									SendReply(msg);
								}
								NextFile();
							}	
						}
						else
						{
							disconnected = true;
							MessageRef msg(GetMessageFromPool(WGenericEvent::FileError));
							if (msg())
							{
								msg()->AddString("why", "Could not read file info!");
								SendReply(msg);
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
							
							
							if (fDownloading)
							{
								uint8 * data;
								size_t numBytes;
								if (next()->FindDataPointer("data", B_RAW_TYPE, (void **)&data, (uint32 *)&numBytes) == B_OK)
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
										
										if (fCurrentOffset >= fFileSize)
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
										//CTimer->start(30000, true); // 30 seconds
										PRINT("\tWDownload::TransferFileData OK\n");
									}
									else
									{
										// error
										PRINT("\tWDownload::TransferFileData FAIL!!!\n");
										MessageRef error(GetMessageFromPool(WGenericEvent::FileError));
										if (error())
										{
											error()->AddString("why", "Couldn't write file data!");
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
				break;
			}
			
			case MTT_EVENT_SESSION_ACCEPTED:
				RemoveAcceptFactory(0);		// no need to accept anymore
				// fall through
			case MTT_EVENT_SESSION_CONNECTED:
				{
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
										ShutdownInternalThread();
										break;
									}
									else	// ERROR?
									{
										MessageRef e(GetMessageFromPool(WGenericEvent::FileError));
										if (e())
										{
											e()->AddString("file", (const char *) outFile.utf8());
											e()->AddString("why", "MD5 hashing failed! Can't resume.");
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
					break;
				}
				
			case MTT_EVENT_SERVER_EXITED:			// same handler for the both of these
			case MTT_EVENT_SESSION_DISCONNECTED:
				{
					if (fFinished) // Do it only once...
					{
						break;
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

