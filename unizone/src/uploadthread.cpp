#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "uploadthread.h"
#include "wgenericevent.h"
#include "global.h"
#include "downloadimpl.h"
#include "settings.h"
#include "md5.h"
#include "wstring.h"
#include "debugimpl.h"

WUploadThread::WUploadThread(QObject * owner, bool * optShutdownFlag)
	: WGenericThread(owner, optShutdownFlag) 
{ 
	setName( "WUploadThread" );
	fFile = NULL; 
	fFileUl = QString::null;
	fRemoteSessionID = QString::null;
	fCurFile = -1;
	fNumFiles = -1;
	fPort = 0;
	fSocket = 0;
	fActive = false;
	fBlocked = false;
	fTimeLeft = 0;
	fForced = false;
}

WUploadThread::~WUploadThread()
{
	if (fFile)
	{
		fFile->close();
		delete fFile;
	}
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

	if (!fAccept)
	{
		if (wmt->AddNewSession(fSocket, limit) == B_OK && wmt->StartInternalThread() == B_OK)
		{
			MessageRef mref(GetMessageFromPool(WGenericEvent::ConnectInProgress));
			if (mref())
			{
				SendReply(mref);
			}
			CTimer->start(60000, true);
		}
		else
		{
			MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
			if (fail())
			{
				fail()->AddString("why", "Could not init session!");
				SendReply(fail);
			}
		}
	}
	else
	{
		const String sRemoteIP = (const char *) fStrRemoteIP.utf8(); // <postmaster@raasu.org> 20021026
		if (wmt->AddNewConnectSession(sRemoteIP, (uint16)fPort, limit) == B_OK && wmt->StartInternalThread() == B_OK)
		{
			MessageRef mref(GetMessageFromPool(WGenericEvent::ConnectInProgress));
			if (mref())
			{
				SendReply(mref);
			}
			CTimer->start(60000, true);
		}
		else
		{
			MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
			if (fail())
			{
				fail()->AddString("why", "Couldn't create new connect session!");
				SendReply(fail);
			}
		}
	}
}

void
WUploadThread::SetLocallyQueued(bool b)
{
	WGenericThread::SetLocallyQueued(b);
	if (!b)
	{
		if (wmt->IsInternalThreadRunning())  // Still connected?
		{
			// Don't need to start forced uploads ;)
			if (!fForced)
				DoUpload();		// we can start now!
		}
		else if (fActive)
		{
			Reset();
			MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
			if (fail())
			{
				fail()->AddString("why", "Connection reset by peer!");
				SendReply(fail);
			}
			fActive = false;
			fFinished = true;
		}
	}
}

void
WUploadThread::SetBlocked(bool b, int64 timeLeft)
{
	SetActive(!b);
	if (b)
	{
		if (timeLeft != -1)
			fBlockTimer->start(timeLeft/1000, true);
	}
	else if (!b && fBlockTimer->isActive())
	{
		fBlockTimer->stop();
	}

	WGenericThread::SetBlocked(b, timeLeft);
	if (wmt->IsInternalThreadRunning())
	{
		if (b)
		{
			SendRejectedNotification(true);
		}
		else
		{
			SetLocallyQueued(true); // put in queue ;)
			DoUpload();				// we can start now!
		}
	}
	else
	{
		if (b)
		{
			SendRejectedNotification(false);
		}
	}
}

void
WUploadThread::SetManuallyQueued(bool b)
{
	WGenericThread::SetManuallyQueued(b);
	if (wmt->IsInternalThreadRunning() && b)
		SendQueuedNotification();
}


void 
WUploadThread::SendReply(MessageRef &m)
{
	if (m())
	{
		m()->AddBool("upload", true);
		WGenericThread::SendReply(m);
	}
}

void
WUploadThread::SignalOwner()
{
	MessageRef next;
	uint32 code;

	CTimer->stop();

	while (wmt->GetNextEventFromInternalThread(code, &next) >= 0)
	{
		switch (code)
		{
			case MTT_EVENT_SESSION_CONNECTED:
			{
				MessageRef con(GetMessageFromPool(WGenericEvent::Connected));
				if (con())
				{
					SendReply(con);
				}
				break;
			}

			case MTT_EVENT_SERVER_EXITED:
			case MTT_EVENT_SESSION_DISCONNECTED:
			{
				if (fFinished) // Do it only once...
				{
					break;
				}

				if (fFile)
				{
					fFile->close();
					delete fFile; 
					fFile = NULL;
				}
				MessageRef dis(GetMessageFromPool(WGenericEvent::Disconnected));
				if (dis())
				{
					if (fCurrentOffset < fFileSize || fUploads.GetNumItems() > 0)
						dis()->AddBool("failed", true);
					else
						dis()->AddBool("failed", false);
					SendReply(dis);
				}
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
							fRemoteSessionID = QString::fromUtf8(id);
							
							QString user = QString::fromUtf8(name);
							if (user.isEmpty())
								fRemoteUser = GetUserName(fRemoteSessionID);
							else
								fRemoteUser = user; 

							if (gWin->IsIgnored(fRemoteSessionID, true))
							{
								SetBlocked(true);
							}

							MessageRef ui(GetMessageFromPool(WGenericEvent::UpdateUI));
							if (ui())
							{
								ui()->AddString("name", name);
								ui()->AddString("id", id);
								SendReply(ui);
							}
						}
						bool c = false;
						if (next()->FindBool("unishare:supports_compression", &c) == B_OK)
						{
							wmt->SetOutgoingMessageEncoding(MUSCLE_MESSAGE_ENCODING_ZLIB_9);
						}
						break;
					}
					case WDownload::TransferFileList:
					{
						const char * file;
						if (next()->FindString("files", &file) == B_OK)
						{
							String sid, name;
							int32 mm;
							if (next()->FindInt32("mm", &mm) == B_OK)
								fMungeMode = mm;
							else
								fMungeMode = WDownload::MungeModeNone;

							if (next()->FindString("beshare:FromSession", sid) == B_OK)
								fRemoteSessionID = QString::fromUtf8(sid.Cstr());
							
							if (next()->FindString("beshare:FromUserName", name) ==  B_OK)
							{
								QString user = QString::fromUtf8(name.Cstr());
								if (!user.isEmpty())
									fRemoteUser = user;
							}

							int i;
							
							for (i = 0; (next()->FindString("files", i, &file) == B_OK); i++)
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
										MessageRef m(GetMessageFromPool(WGenericEvent::FileHashing));
										if (m())
										{
											m()->AddString("file", filePath);
											SendReply(m);
										}

										// Hash
										uint64 retBytesHashed = 0;

										if (HashFileMD5(QString::fromUtf8(filePath.Cstr()), offset, readLen, retBytesHashed, myDigest, 
											fShutdownFlag) == B_OK && memcmp(hisDigest, myDigest, sizeof(myDigest)) == 0)
										{
											// put this into our message ref
											fileRef()->AddInt64("secret:offset", onSuccessOffset);
										}
									}
									fUploads.AddTail(fileRef);
									fNames.AddTail(QString::fromUtf8(file));
								}
							}
							fNumFiles = i;

							fWaitingForUploadToFinish = false;
							SendQueuedNotification();
							// also send a message along to our GUI telling it what the first file is
							
							if (fUploads.GetNumItems() != 0)
							{
								String firstFile;
								const char * path, * filename;
								uint64 filesize;
								MessageRef fref = fUploads[0];
								if (fref()->FindString("winshare:Path", &path) == B_OK &&
									fref()->FindString("beshare:File Name", &filename) == B_OK &&
									fref()->FindInt64("beshare:File Size", (int64 *) &filesize) == B_OK)
								{
									if (filesize < gWin->fSettings->GetMinQueuedSize() || !IsLocallyQueued())
									{
										DoUpload();
									}
									else
									{
										firstFile = path;
										if (!firstFile.EndsWith("/"))
											firstFile += "/";
										firstFile += filename;
										MessageRef initmsg(GetMessageFromPool(WGenericEvent::Init));
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
						break;
					}
				}
				break;
			}

			case MTT_EVENT_OUTPUT_QUEUES_DRAINED:
			{
				PRINT("\tMTT_EVENT_OUTPUT_QUEUES_DRAINED\n");
				if (fWaitingForUploadToFinish)
				{
					PRINT("\tfWaiting\n");
					PRINT("\t\tSending message\n");
					MessageRef msg(GetMessageFromPool(WGenericEvent::Disconnected));
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
	MessageRef q(GetMessageFromPool(WDownload::TransferNotifyQueued));
	if (q())
	{
		wmt->SendMessageToSessions(q);
	}
	MessageRef qf(GetMessageFromPool(WGenericEvent::FileQueued));
	if (qf())
	{
		SendReply(qf);
	}
}

void
WUploadThread::SendRejectedNotification(bool direct)
{
	MessageRef q(GetMessageFromPool(WDownload::TransferNotifyRejected));

	if (!q())
		return;

	if (fTimeLeft != -1)
		q()->AddInt64("timeleft", fTimeLeft);
	if (direct || (fPort == 0))
	{
		wmt->SendMessageToSessions(q);
	}
	else
	{
		String node;
		if (fRemoteSessionID != QString::null)
		{
			node = "/*/";
			node += (const char *) fRemoteSessionID.latin1();
		}
		else
		{
			// use /<ip>/* instead of /*/<sessionid>, because session id isn't yet known at this point
			node = "/";
			node += (const char *) fStrRemoteIP.latin1();
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
	MessageRef b(GetMessageFromPool(WGenericEvent::FileBlocked));
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
	if (fShutdownFlag && *fShutdownFlag)	// Do we need to interrupt?
		return;

	// Still connected?

	if (!wmt->IsInternalThreadRunning())
	{
		MessageRef fail(GetMessageFromPool(WGenericEvent::ConnectFailed));
		if (fail())
		{
			fail()->AddString("why", "Connection reset by peer!");
			SendReply(fail);
		}
		fActive = false;
		fFinished = true;
		return;
	}

	fActive = true;

	// Small files get to bypass queue
	if (IsLocallyQueued())
	{
		if (fFile && (fFileSize >= gWin->fSettings->GetMinQueuedSize()))		// not yet
		{
			fForced = false;
			MessageRef lq(GetMessageFromPool(WGenericEvent::FileQueued));
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
		MessageRef msg(GetMessageFromPool(WGenericEvent::FileBlocked));
		if (msg())
		{
			if (fTimeLeft != -1)
				msg()->AddInt64("timeleft", fTimeLeft);
			SendReply(msg);
		}
		if (gWin->IsConnected(fRemoteSessionID))
		{
			if (fTimeLeft == -1)
			{
				gWin->SendChatText(fRemoteSessionID, "Your download has been blocked!");
			}
			else
			{
				gWin->SendChatText(fRemoteSessionID, tr("Your download has been blocked for %1 minute(s)!").arg((long)(fTimeLeft/60000000)));
			}
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
			if (uref()->AddData("data", B_RAW_TYPE, NULL, bufferSize) == B_OK &&
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
					wmt->SendMessageToSessions(uref);

					// NOTE: RequestOutputQueuesDrainedNotification() can recurse, so we need to update the offset before
					//       calling it!
					fCurrentOffset += numBytes;
					MessageRef drain(GetMessageFromPool());
					if (drain())
						wmt->RequestOutputQueuesDrainedNotification(drain);

					// we'll use this event for sending as well
					MessageRef update(GetMessageFromPool(WGenericEvent::FileDataReceived));	
					if (update())
					{
						update()->AddInt64("offset", fCurrentOffset);
						update()->AddInt64("size", fFileSize);
						update()->AddInt32("sent", numBytes);
						
						if (fCurrentOffset >= fFileSize)
						{
							update()->AddBool("done", true);	// file done!
							update()->AddString("file", (const char *) fFileUl.utf8());
						}
						SendReply(update);
					}
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
	}
	else
	{
		while (!fFile)
		{
			if (fUploads.GetNumItems() != 0)
			{
				// grab the ref and remove it from the list
				fUploads.RemoveHead(fCurrentRef);
				Message * m = fCurrentRef();
				String path, filename;
				m->FindString("winshare:Path", path);
				m->FindString("beshare:File Name", filename);
				String filePath = path;

				if (!filePath.EndsWith("/"))
					filePath += "/";
				
				filePath += filename;

				fFileUl = QString::fromUtf8(filePath.Cstr());

				// <postmaster@raasu.org> 20021023, 20030702 -- Add additional debug message
				WString wFileUl = fFileUl; 
				PRINT("WUploadThread::DoUpload: filePath = %S\n", wFileUl.getBuffer()); 
				
				fFile = new QFile(fFileUl);
				CHECK_PTR(fFile);
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
				Message * msg = fCurrentRef();
				MessageRef headRef( GetMessageFromPool(*msg) );
				if (headRef())
				{
					headRef()->what = WDownload::TransferFileHeader;
					headRef()->AddInt64("beshare:StartOffset", fCurrentOffset);
					wmt->SendMessageToSessions(headRef);
				}

				fCurFile++;

				MessageRef mref(GetMessageFromPool(WGenericEvent::FileStarted));
				if (mref())
				{
					mref()->AddString("file", filePath.Cstr());
					mref()->AddInt64("start", fCurrentOffset);
					mref()->AddInt64("size", fFileSize);
					mref()->AddString("user", (const char *) fRemoteSessionID.utf8());
					SendReply(mref);
				}

				DoUpload();	// nested call
			}
			else
			{
				PRINT("No more files!\n");
				fWaitingForUploadToFinish = true;
				MessageRef drain(GetMessageFromPool());
				if (drain())
					wmt->RequestOutputQueuesDrainedNotification(drain);
				break;
			}
		}
	}
}

QString
WUploadThread::GetFileName(int i)
{
	if (i >= 0 && i < fNames.GetNumItems())
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
	WGenericThread::SetRate(rate);
	if (rate != 0)
		wmt->SetNewOutputPolicy(PolicyRef(new RateLimitSessionIOPolicy(rate), NULL));
	else
		wmt->SetNewOutputPolicy(PolicyRef(NULL, NULL));
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

QString
WUploadThread::GetRemoteUser()
{
	if (fRemoteUser.isEmpty())
		return tr("User #%1").arg(fRemoteSessionID);
	else
		return fRemoteUser;
}
