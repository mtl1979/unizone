#include "downloadthread.h"
#include "statusimpl.h"
#include "wgenericevent.h"
#include "md5.h"
#include "wfile.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
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
	CTimer = new QTimer(this, "Connect Timer");
	CHECK_PTR(CTimer);
	connect( CTimer , SIGNAL(timeout()), this, SLOT(ConnectTimer()) );
}

WDownloadThread::~WDownloadThread()
{
	if (fFile)
	{
		fFile->Close();
		delete fFile;
	}
	
	if (fFileDl)
		delete [] fFileDl;
	
	if (fLocalFileDl)
		delete [] fLocalFileDl;
}

void 
WDownloadThread::SetFile(QString * files, int32 numFiles, QString fromIP, uint32 remotePort, bool firewalled, bool partial)
{
	fFileDl = files;
	fLocalFileDl = new QString[numFiles];
	CHECK_PTR(fLocalFileDl);
	for (int l = 0; l < numFiles; l++)
	{
		fLocalFileDl[l] = QString::null;
	}
	fNumFiles = numFiles;
	fCurFile = 0;
	fIP = fromIP;
	fFromUser = QString::null;
	fPort = remotePort;
	fFirewalled = firewalled;
	fPartial = partial;
	
	MessageRef msg(GetMessageFromPool(WGenericEvent::Init));
	QString dlFile = "downloads";
	dlFile += "/";
	dlFile += fFileDl[0];
	msg()->AddString("file", (const char *) dlFile.utf8());
	msg()->AddString("user", "0");
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
			ThreadWorkerSessionRef connectRef;
						
			String sIP = (const char *) fIP.utf8(); // <postmaster@raasu.org> 20021026
			if (AddNewConnectSession(sIP, (uint16)fPort, connectRef) == B_OK)
			{
				fCurrentOffset = fFileSize = 0;
				fFile = NULL;
				fDownloading = false;
				MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectInProgress));
				SendReply(msg);
				CTimer->start(30000, true); // 30 seconds
				return true;
			}
			else
			{
				Reset();
				MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectFailed));
				msg()->AddString("why", "Could not add new connect session!");
				SendReply(msg);
			}
		}
		else
		{
			MessageRef msg(GetMessageFromPool(WGenericEvent::ConnectFailed));
			msg()->AddString("why", "Failed to start internal thread!");
			SendReply(msg);
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
	uint32 port;
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
				case Status::TransferNotifyQueued:
					{
						MessageRef q(GetMessageFromPool(WGenericEvent::FileQueued));
						SendReply(q);
						SetRemotelyQueued(true);
						break;
					}
					
				case Status::TransferNotifyRejected:
					{
						MessageRef q(GetMessageFromPool(WGenericEvent::FileBlocked));
						int64 timeleft = (uint64) -1;
						(void) next()->FindInt64("timeleft", (int64 *) &timeleft);
						if (timeleft != -1)
							q()->AddInt64("timeleft", timeleft);
						SendReply(q);
						SetBlocked(true, timeleft);
						break;
					}
					
				case Status::TransferFileHeader:
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
								fFile->Close();
								delete fFile;
								fFile = NULL;
							}
							MessageRef msg(GetMessageFromPool(WGenericEvent::FileDone));
							msg()->AddBool("done", false);
							if (fCurrentOffset < fFileSize)				// hm... cut off short?
							{					
								msg()->AddBool("error", true);
							}
							SendReply(msg);
							NextFile();
						}
						// fields:
						//	String		beshare:File Name
						//	Int64		beshare:File Size
						//	String		beshare:From Session
						//	String		beshare:Path
						//	Int64		beshare:StartOffset
						// I only care to get the size and name
						String fname;
						if (next()->FindInt64("beshare:File Size", (int64 *)&fFileSize) == B_OK && 
							next()->FindString("beshare:File Name", fname) == B_OK)
						{
							QString fixed("downloads/");
							fixed += FixFileName(QString::fromUtf8(fname.Cstr()));	// we have a "fixed" filename that eliminates characters Windows does not support
							
							bool append = false;
							
							if (next()->FindInt64("beshare:StartOffset", (int64 *)&fCurrentOffset) == B_OK &&
								fCurrentOffset > 0)
							{
								fFile = new WFile();
								CHECK_PTR(fFile);
								if (fFile->Open(fixed, IO_ReadOnly))
								{
									if (fFile->Size() == fCurrentOffset)	// sizes match up?
										append = true;
									fFile->Close();
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
							
							fFile = new WFile();	// fixed filename again
							CHECK_PTR(fFile);
							if (WFile::Exists(fixed) && !append)	// create a new file name
							{
								QString nf = fixed;
								int i = 1;
								while (WFile::Exists(nf))
									nf = UniqueName(fixed, i++);
								fixed = nf;
							}
							
							fLocalFileDl[GetCurrentNum()] = fixed;
							MessageRef msg;
							if (fFile->Open(fixed, (append ? IO_Append | IO_WriteOnly : IO_WriteOnly)))
							{
								msg = MessageRef(GetMessageFromPool(WGenericEvent::FileStarted));
								msg()->AddString("file", (const char *) fixed.utf8());
								msg()->AddInt64("start", fCurrentOffset);
								msg()->AddInt64("size", fFileSize);
								msg()->AddString("user", "0");
								fDownloading = true;
								SendReply(msg);
							}
							else
							{
								// ERROR!
								disconnected = true;	// we're done
								msg = MessageRef(GetMessageFromPool(WGenericEvent::FileError));
								msg()->AddString("file", (const char *) fixed.utf8());
								msg()->AddString("why", "Critical error: Could not create file!");
								delete fFile;
								fFile = NULL;
								SendReply(msg);
								NextFile();
							}	
						}
						else
						{
							disconnected = true;
							MessageRef msg(GetMessageFromPool(WGenericEvent::FileError));
							msg()->AddString("why", "Could not read file info!");
							SendReply(msg);
						}
						break;
					}
					
					case Status::TransferFileData:
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
									if (fFile->WriteBlock((const char *)data, (uint)numBytes) == (int)numBytes)
									{
										MessageRef update(GetMessageFromPool(WGenericEvent::FileDataReceived));
										fCurrentOffset += numBytes;
										update()->AddInt64("offset", fCurrentOffset);
										update()->AddInt64("size", fFileSize);
										update()->AddInt32("got", numBytes);
										
										if (fCurrentOffset >= fFileSize)
										{
											update()->AddBool("done", true);	// file done!
											if (fCurFile != -1)
												update()->AddString("file", (const char *) fFileDl[fCurFile].utf8());
											fFile->Close();
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
										MessageRef error(GetMessageFromPool(WGenericEvent::FileError));
										error()->AddString("why", "Couldn't write file data!");
										SendReply(error);
										Reset();
										fDownloading = false;
										fFile->Close();
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
					SendReply(replyMsg);
					
					MessageRef comID(GetMessageFromPool(Status::TransferCommandPeerID));
					comID()->AddString("beshare:FromUserName", "MuscleCopy" );
					comID()->AddString("beshare:FromSession", "5038");
					SendMessageToSessions(comID);
					
					MessageRef neg(GetMessageFromPool(Status::TransferFileList));
					for (int c = 0; c < fNumFiles; c++)
					{
						// check to see wether the file exists
						QString outFile("downloads");
						if (!(outFile.right(1) == "/"))
							outFile += "/";
						QString fixed = outFile;
						outFile += fFileDl[c];
						fixed += FixFileName(fFileDl[c]);
						
						uint8 digest[MD5_DIGEST_SIZE];
						uint64 fileOffset = 0;	// autodetect file size for offset
						uint64 retBytesHashed = 0;

						if (WFile::Exists(fixed))
						{
							// get an MD5 hash code out of it
							uint64 bytesFromBack = fPartial ? PARTIAL_RESUME_SIZE : 0;
							
							MessageRef hashMsg(GetMessageFromPool(WGenericEvent::FileHashing));
							hashMsg()->AddString("file", (const char *)fixed.utf8());
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
									MessageRef e(GetMessageFromPool(WGenericEvent::FileError));
									e()->AddString("file", (const char *) outFile.utf8());
									e()->AddString("why", "MD5 hashing failed! Can't resume.");
									SendReply(e);
								}
							}
						}
						neg()->AddInt64("offsets", fileOffset);
						neg()->AddInt64("numbytes", retBytesHashed);
						neg()->AddData("md5", B_RAW_TYPE, digest, (fileOffset > 0) ? sizeof(digest) : 1);
						neg()->AddString("files", (const char *) fFileDl[c].utf8());
					}
					neg()->AddString("beshare:FromSession", "5038");
					SendMessageToSessions(neg);
					break;
				}
				
			case MTT_EVENT_SERVER_EXITED:		// same handler for the both of these
				{
					// Fall through?
					break;
				}
			case MTT_EVENT_SESSION_DISCONNECTED:
				{

					MessageRef dis;
					if (fDownloading)
					{
						if (fCurrentOffset != fFileSize)
						{
							dis = MessageRef(GetMessageFromPool(WGenericEvent::Disconnected));
							dis()->AddBool("failed", true);
						}
						else if (fCurrentOffset == fFileSize)
						{
							if (IsLastFile())
							{
								dis = MessageRef(GetMessageFromPool(WGenericEvent::FileDone));
								dis()->AddBool("done", true);		
								dis()->AddString("file", (const char *) fFileDl[fCurFile].utf8());
							}
							else
							{
								NextFile();
								dis = MessageRef(GetMessageFromPool(WGenericEvent::FileFailed));
								dis()->AddBool("failed", true);
							}
						}
					}
					else
					{
						dis = MessageRef(GetMessageFromPool(WGenericEvent::Disconnected));
						if (fCurFile == -1)
							dis()->AddBool("failed", false);
						else
							dis()->AddBool("failed", true);
					}
					
					fDownloading = false;
					if (fFile != NULL)
					{
						fFile->Close();
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
		SetNewInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate)));
	else
		SetNewInputPolicy(PolicyRef(NULL));
}

void
WDownloadThread::SetRate(int rate, AbstractReflectSessionRef ref)
{
	WGenericThread::SetRate(rate, ref);
	if (rate != 0)
		ref()->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate)));
	else
		ref()->SetInputPolicy(PolicyRef(NULL));
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

ThreadWorkerSessionRef
WDownloadThreadWorkerSessionFactory::CreateThreadWorkerSession(const String & s)
{
	ThreadWorkerSessionRef ref(new ThreadWorkerSession());
	if (ref() && fLimit != 0)
	{
		ref()->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy( fLimit )));
	}
	return ref;
}

