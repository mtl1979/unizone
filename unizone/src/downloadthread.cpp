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
#include <qdir.h>

WDownloadThread::WDownloadThread(QObject * owner, bool * optShutdownFlag)
		: WGenericThread(owner, optShutdownFlag) 
{
	fFile = NULL; 
}

WDownloadThread::~WDownloadThread()
{
	if (fFile)
		delete fFile;
}

void 
WDownloadThread::SetFile(QString file, QString fromIP, QString fromSession,
						QString localSession, int32 remotePort, bool firewalled, bool partial)
{
	fFileDl = file;
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
	dlFile += fFileDl;
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
				ref()->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(WSettings::ConvertToBytes(
										gWin->fSettings->GetDLLimit())), NULL));

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
			return true;
		}
	}
	return false;
}

void 
WDownloadThread::SendReply(Message * m)
{
	m->AddBool("download", true);	// the value doesn't matter
	WGenericThread::SendReply(m);
}

void 
WDownloadThread::SignalOwner()	// sent by the MTT when we have some data
{
	MessageRef next;
	uint32 code;
	String sid;
	uint16 port;
	bool disconnected = false;

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
						break;
					}

					case WDownload::TransferFileHeader:
					{
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
							QString outFile = "downloads";
							QString fixed;
							outFile += "/";
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
									nf = QObject::tr("%1 %2").arg(fixed).arg(i++);
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
							}
							SendReply(msg);
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
										fFile->close();
										delete fFile; 
										fFile = NULL;
										fDownloading = false;
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
				// check to see wether the file exists
				QString outFile("downloads");
				if (!(outFile.right(1) == "/"))
					outFile += "/";
				QString fixed = outFile;
				outFile += fFileDl;
				fixed += FixFileName(fFileDl);

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
				neg()->AddString("files", (const char *) fFileDl.utf8());
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
					dis->AddString("file", (const char *) fFileDl.utf8());
				}
				else if (fDownloading && (fCurrentOffset == fFileSize))
				{
					dis = new Message(WGenericEvent::FileDone);
					dis->AddBool("done", true);		// when we do multiple file downloads in one dialog
														// this will come in handy... it will tell the GUI to give
														// us the next file
				}
				else
				{
					dis = new Message(WGenericEvent::Disconnected);
					dis->AddBool("failed", false);
					dis->AddString("file", (const char *) fFileDl.utf8());
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
