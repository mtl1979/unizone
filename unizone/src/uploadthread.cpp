#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "uploadthread.h"
#include "wgenericevent.h"
#include "global.h"
#include "downloadimpl.h"
#include "settings.h"
#include "md5.h"
#include "platform.h"		// <postmaster@raasu.org> 20021114

WUploadThread::WUploadThread(QObject * owner, bool * optShutdownFlag)
	: WGenericThread(owner, optShutdownFlag) 
{ 
	setName( "WUploadThread" );
	fFile = NULL; 
	fFileUl = QString::null;
	fCurFile = -1;
	fNumFiles = -1;
	fActive = true;
	fBlocked = false;

	CTimer = new QTimer(this, "Connect Timer");
	connect( CTimer , SIGNAL(timeout()), this, SLOT(ConnectTimer()) );
}

WUploadThread::~WUploadThread()
{
	if (fFile)
	{
		delete fFile;
	}
}

void
WUploadThread::SetUpload(int socket, uint32 remoteIP, WFileThread * ft)
{
	fAccept = false;
	fRemoteIP = remoteIP;
	fSocket = socket;
	fFileThread = ft;
}

void 
WUploadThread::SetUpload(QString remoteIP, uint32 remotePort, WFileThread * ft)
{
	fFileThread = ft;
	fAccept = true;
	fStrRemoteIP = remoteIP;
	fPort = remotePort;
}

void 
WUploadThread::InitSession()
{
	AbstractReflectSessionRef limit;

	// First check if IP is blacklisted
	//
	if (fStrRemoteIP == QString::null)
	{
		uint32 _ip = GetPeerIPAddress(fSocket);
		fStrRemoteIP = Inet_NtoA(_ip).Cstr();
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
		if (AddNewSession(fSocket, limit) == B_OK && StartInternalThread() == B_OK)
		{
			SendReply(new Message(WGenericEvent::ConnectInProgress));
			CTimer->start(60000, true);
		}
		else
		{
			Message * fail = new Message(WGenericEvent::ConnectFailed);
			fail->AddString("why", "Could not init session!");
			SendReply(fail);
		}
	}
	else
	{
		const String sRemoteIP = (const char *) fStrRemoteIP.utf8(); // <postmaster@raasu.org> 20021026
		if (AddNewConnectSession(sRemoteIP, (uint16)fPort, limit) == B_OK && StartInternalThread() == B_OK)
		{
			SendReply(new Message(WGenericEvent::ConnectInProgress));
			CTimer->start(60000, true);
		}
		else
		{
			Message * fail = new Message(WGenericEvent::ConnectFailed);
			fail->AddString("why", "Couldn't create new connect session!");
			SendReply(fail);
		}
	}
}

void
WUploadThread::SetLocallyQueued(bool b)
{
	WGenericThread::SetLocallyQueued(b);
	if (!b && IsInternalThreadRunning())
		DoUpload();		// we can start now!
}

void
WUploadThread::SetBlocked(bool b)
{
	WGenericThread::SetBlocked(b);
	if (IsInternalThreadRunning())
	{
		if (b)
			SendRejectedNotification();
		else
			DoUpload();		// we can start now!
	}
}

void
WUploadThread::SetManuallyQueued(bool b)
{
	WGenericThread::SetManuallyQueued(b);
	if (IsInternalThreadRunning() && b)
		SendQueuedNotification();
}


void 
WUploadThread::SendReply(Message * m)
{
	if (m)
	{
		m->AddBool("upload", true);
		WGenericThread::SendReply(m);
	}
}

void
WUploadThread::SignalOwner()
{
	MessageRef next;
	uint32 code;

	CTimer->stop();

	while (GetNextEventFromInternalThread(code, &next) >= 0)
	{
		switch (code)
		{
			case MTT_EVENT_SESSION_CONNECTED:
			{
				SendReply(new Message(WGenericEvent::Connected));
				// if queued, send a queued message here, otherwise, nothing
				break;
			}

			case MTT_EVENT_SESSION_DISCONNECTED:
			{
				if (fFile)
				{
					fFile->close();
					delete fFile; 
					fFile = NULL;
				}
				Message * dis = new Message(WGenericEvent::Disconnected);
				if (fCurrentOffset < fFileSize || fUploads.size() > 0)
					dis->AddBool("failed", true);
				else
					dis->AddBool("failed", false);
				SendReply(dis);
				break;
			}

			case MTT_EVENT_INCOMING_MESSAGE:
			{
				switch (next()->what)
				{
					case WDownload::TransferCommandPeerID:
					{
						const char * name, * id;
						if (next()->FindString("beshare:FromUserName", &name) ==  B_OK &&
							next()->FindString("beshare:FromSession", &id) == B_OK)
						{
							Message * ui = new Message(WGenericEvent::UpdateUI);
							ui->AddString("name", name);
							ui->AddString("id", id);
							fRemoteSessionID = QString::fromUtf8(id);
							fRemoteUser = QString::fromUtf8(name);
							if (gWin->IsIgnored(fRemoteSessionID, true))
							{
								SetBlocked(true);
							}
							SendReply(ui);
						}
						break;
					}
					case WDownload::TransferFileList:
					{
						const char * file;
						if (next()->FindString("files", &file) == B_OK)
						{
							String sid;
							int32 mm;
							if (next()->FindInt32("mm", &mm) == B_OK)
								fMungeMode = mm;
							else
								fMungeMode = WDownload::MungeModeNone;

							if (next()->FindString("beshare:FromSession", sid) == B_OK)
								fRemoteSessionID = QString::fromUtf8(sid.Cstr());
							else
								fRemoteSessionID = "";

							fFileThread->Lock();

							for (int i = 0; (next()->FindString("files", i, &file) == B_OK); i++)
							{
								MessageRef fileRef;

								if (fFileThread->FindFile(QString::fromUtf8(file), &fileRef) && (fileRef())) // <postmaster@raasu.org> 20021023
								{
									// remove any previous offsets
									fileRef()->RemoveName("secret:offset");
									// see if we need to add them
									uint64 offset = 0L;
									const uint8 * hisDigest = NULL;
									uint32 numBytes = 0L;
									if (next()->FindInt64("offsets", i, (int64 *)&offset) == B_OK &&
										next()->FindData("md5", B_RAW_TYPE, i, (const void **)&hisDigest,
										(uint32 *)&numBytes) == B_OK && numBytes == MD5_DIGEST_SIZE)
									{
										uint8 myDigest[MD5_DIGEST_SIZE];
										uint64 readLen = 0;
										uint64 onSuccessOffset = offset;

										for (uint32 j = 0; j < ARRAYITEMS(myDigest); j++)
											myDigest[j] = 'x';

										if (next()->FindInt64("numbytes", (int64 *)&readLen) == B_OK)
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
										Message * m = new Message(WGenericEvent::FileHashing);
										m->AddString("file", filePath);
										SendReply(m);

										// Hash
										uint64 retBytesHashed = 0;

										if (HashFileMD5(QString::fromUtf8(filePath.Cstr()), offset, readLen, retBytesHashed, myDigest, 
											fShutdownFlag) == B_OK && memcmp(hisDigest, myDigest, sizeof(myDigest)) == 0)
										{
											// put this into our message ref
											fileRef()->AddInt64("secret:offset", onSuccessOffset);
										}
									}
									fUploads.insert(fUploads.end(), fileRef);
									fNames.insert(fNames.end(), QString::fromUtf8(file));
								}
							}
							fNumFiles = i;
							fFileThread->Unlock();
							fWaitingForUploadToFinish = false;
							if (!IsLocallyQueued())	// we're not queued?
							{
								DoUpload();
							}
							else
							{
								SendQueuedNotification();
								// also send a message along to our GUI telling it what the first file is
								WMsgListIter it = fUploads.begin();
								if (it != fUploads.end())
								{
									String firstFile;
									const char * path, * filename;
									MessageRef fref = (*it);
									if (fref()->FindString("winshare:Path", &path) == B_OK &&
										fref()->FindString("beshare:File Name", &filename) == B_OK)
									{
										firstFile = path;
										if (!firstFile.EndsWith("/"))
											firstFile += "/";
										firstFile += filename;
										Message * initmsg = new Message(WGenericEvent::Init);
										initmsg->AddString("file", firstFile);
										initmsg->AddString("user", (const char *) fRemoteSessionID.utf8());
										SendReply(initmsg);
									}
								}
							}
						}
						break;
					}
				}
				break;
			}

			case MTT_EVENT_OUTPUT_QUEUES_DRAINED:
			{
#ifndef WIN32
				PRINT("\tMTT_EVENT_OUTPUT_QUEUES_DRAINED\n");
#endif
				if (fWaitingForUploadToFinish)
				{
#ifndef WIN32
					PRINT("\tfWaiting\n");
#endif
					PRINT("\t\tSending message\n");
					Message * msg = new Message(WGenericEvent::Disconnected);
					msg->AddBool("done", true);
					msg->AddString("file", (const char *) fFileUl.utf8());
					PRINT("\t\tSending...\n");
					SendReply(msg);
					PRINT("\t\tSent...\n"); // <postmaster@raasu.org> 20021023 -- Fixed typo
					return;
				}
				else
					DoUpload();
				break;
			}
		}
	}
}

void 
WUploadThread::SendQueuedNotification()
{
	MessageRef q(new Message(WDownload::TransferNotifyQueued), NULL);
	SendMessageToSessions(q);
	SendReply(new Message(WGenericEvent::FileQueued));
}

void
WUploadThread::SendRejectedNotification()
{
	MessageRef q(new Message(WDownload::TransferNotifyRejected), NULL);
	SendMessageToSessions(q);
	SendReply(new Message(WGenericEvent::FileBlocked));
}

void 
WUploadThread::DoUpload()
{
	if (IsLocallyQueued())		// not yet
	{
		SendReply(new Message(WGenericEvent::FileQueued));
		return;
	}

	if (IsBlocked())
	{
		SendReply(new Message(WGenericEvent::FileBlocked));
		if (gWin->IsConnected(fRemoteSessionID))
			gWin->SendChatText(fRemoteSessionID, "Your download has been blocked!");
		return;
	}

	if (fFile)
	{
		MessageRef uref(new Message(WDownload::TransferFileData), NULL);
		const uint32 bufferSize = 8 * 1024;	// think about doing this in a dynamic way (depending on connection)
		uint8 * scratchBuffer;
		if (uref() && uref()->AddData("data", B_RAW_TYPE, NULL, bufferSize) == B_OK &&
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
				SendMessageToSessions(uref);
				RequestOutputQueuesDrainedNotification(MessageRef());
				fCurrentOffset += numBytes;
				Message * update = new Message(WGenericEvent::FileDataReceived);	// we'll use this event for sending as well
				update->AddInt64("offset", fCurrentOffset);
				update->AddInt64("size", fFileSize);
				update->AddInt32("sent", numBytes);
				
				if (fCurrentOffset >= fFileSize)
				{
					update->AddBool("done", true);	// file done!
					update->AddString("file", (const char *) fFileUl.utf8());
				}
				SendReply(update);
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
	else
	{
		while (!fFile)
		{
			WMsgListIter first = fUploads.begin();
			if (first != fUploads.end())
			{
				// grab the ref and remove it from the list
				fCurrentRef = (*first);
				fUploads.erase(first);
				Message * m = fCurrentRef.GetItemPointer();
				String path, filename;
				m->FindString("winshare:Path", path);
				m->FindString("beshare:File Name", filename);
				String filePath = path;
				if (!filePath.EndsWith("/"))
					filePath += "/";
				filePath += filename;
				PRINT("WUploadThread::DoUpload: filePath = %s\n", filePath.Cstr()); // <postmaster@raasu.org> 20021023 -- Add additional debug message
				fFileUl = QString::fromUtf8(filePath.Cstr());
				fFile = new QFile(fFileUl);
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
				MessageRef headRef(new Message(*(fCurrentRef.GetItemPointer())), NULL);
				headRef()->what = WDownload::TransferFileHeader;
				headRef()->AddInt64("beshare:StartOffset", fCurrentOffset);
				SendMessageToSessions(headRef);

				fCurFile++;

				Message * msg = new Message(WGenericEvent::FileStarted);
				msg->AddString("file", filePath.Cstr());
				msg->AddInt64("start", fCurrentOffset);
				msg->AddInt64("size", fFileSize);
				msg->AddString("user", (const char *) fRemoteSessionID.utf8());
				SendReply(msg);

				DoUpload();	// nested call
			}
			else
			{
#ifndef WIN32
				PRINT("No more files!\n");
#endif
				fWaitingForUploadToFinish = true;
				RequestOutputQueuesDrainedNotification(MessageRef());
				break;
			}
		}
	}
}

QString
WUploadThread::GetFileName(int i)
{
	int n = 0;
	WStrListIter iter = fNames.begin();
	while (n < i)
	{
		iter++;
		n++;
	}
	return (*iter);
}

void
WUploadThread::SetRate(int rate)
{
	WGenericThread::SetRate(rate);
	if (rate != 0)
		SetNewOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		SetNewOutputPolicy(PolicyRef(NULL, NULL));
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
