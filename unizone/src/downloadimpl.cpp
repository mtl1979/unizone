#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qthread.h>
#include <qobject.h>
#include <qqueue.h>
#include <qdir.h>
#include <time.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <list>
using std::list;
using std::iterator;

#include "message/Message.h"
#include "downloadimpl.h"
#include "md5.h"
#include "debugimpl.h"
#include "looper.h"
#include "global.h"
#include "filethread.h"
#include "accept.h"
#include "settings.h"
#include "downloadthread.h"
#include "uploadthread.h"
#include "wgenericevent.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "lang.h"		// <postmaster@raasu.org> 20020924
#include "platform.h"	// <postmaster@raasu.org> 20021114
#include "transferitem.h"

// ----------------------------------------------------------------------------------------------

WDownload::WDownload(QString localID, WFileThread * ft)
: QDialog(NULL, "WDownload", false, QWidget::WDestructiveClose | QWidget::WStyle_Minimize |
		  QWidget::WStyle_Maximize | QWidget::WStyle_Title | QWidget::WStyle_SysMenu)
{
	resize(450, 265); // <postmaster@raasu.org> 20020927 Changed from 250 to 265
	fSharedFiles = ft;
	fLocalSID = localID;
	
	fMainSplit = new QSplitter(this);
	CHECK_PTR(fMainSplit);
	fMainSplit->setOrientation(QSplitter::Vertical);
	
	fBoxD = new QVBox(fMainSplit);
	CHECK_PTR(fBoxD);
	fDownloads = new QListView(fBoxD);
	CHECK_PTR(fDownloads);
	fDownloads->addColumn(MSG_TX_STATUS);
	fDownloads->addColumn(MSG_TX_FILENAME);
	fDownloads->addColumn(MSG_TX_RECEIVED);
	fDownloads->addColumn(MSG_TX_TOTAL);
	fDownloads->addColumn(MSG_TX_RATE);
	fDownloads->addColumn(MSG_TX_ETA);
	fDownloads->addColumn(MSG_TX_USER);
	
	fDownloads->setColumnAlignment(2, AlignRight); // <postmaster@raasu.org> 20021213
	fDownloads->setColumnAlignment(3, AlignRight); // 
	fDownloads->setColumnAlignment(4, AlignRight); // 
	fDownloads->setColumnAlignment(5, AlignRight); // 
	
	fDownloads->setAllColumnsShowFocus(true);
	
	fCancelD = new QPushButton(fBoxD);
	CHECK_PTR(fCancelD);
	fCancelD->setText(MSG_CANCEL);
	connect(fCancelD, SIGNAL(clicked()), this, SLOT(CancelDL()));
	
	fBoxU = new QVBox(fMainSplit);
	CHECK_PTR(fBoxU);
	
	fUploads = new QListView(fBoxU);
	CHECK_PTR(fUploads);
	fUploads->addColumn(MSG_TX_STATUS);
	fUploads->addColumn(MSG_TX_FILENAME);
	fUploads->addColumn(MSG_TX_SENT);
	fUploads->addColumn(MSG_TX_TOTAL);
	fUploads->addColumn(MSG_TX_RATE);
	fUploads->addColumn(MSG_TX_ETA);
	fUploads->addColumn(MSG_TX_USER);
	
	fUploads->setColumnAlignment(WTransferItem::Received, AlignRight);	// <postmaster@raasu.org> 20021213
	fUploads->setColumnAlignment(WTransferItem::Total, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::Rate, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::ETA, AlignRight);		// 
	
	fUploads->setAllColumnsShowFocus(true);
	
	fCancelU = new QPushButton(fBoxU);
	CHECK_PTR(fCancelU);
	fCancelU->setText(MSG_CANCEL);
	connect(fCancelU, SIGNAL(clicked()), this, SLOT(CancelUL()));
	
	setCaption(MSG_TX_CAPTION);
	fNumUploads = fNumDownloads = 0;
}

WDownload::~WDownload()
{
	fLock.lock();
	WTIter it = fDownloadList.begin();
	while (it != fDownloadList.end())
	{
		if ((*it).first)
		{
			(*it).first->Reset();
			delete (*it).first;
		}
		delete (*it).second;
		fDownloadList.erase(it);
		it = fDownloadList.begin();
	}
	it = fUploadList.begin();
	while (it != fUploadList.end())
	{
		if ((*it).first)
		{
			PRINT("Reseting upload\n");
			(*it).first->Reset();
			PRINT("Deleting upload\n");
			delete (*it).first;
		}
		PRINT("Deleting item\n");
		delete (*it).second;
		PRINT("Erasing item\n");
		fUploadList.erase(it);
		it = fUploadList.begin();
	}
	fLock.unlock();
	UpdateLoad();	// do this to set a load of 0
	gWin->fDLWindow = NULL; // Don't set this to NULL if there is still threads running
}

void
WDownload::AddDownload(QString file, QString remoteSessionID,
					   uint32 remotePort, QString remoteIP, uint64 remoteInstallID, bool firewalled, bool partial)
{
	WDownloadThread * nt = new WDownloadThread(this);
	nt->SetFile(file, remoteIP, remoteSessionID, fLocalSID, remotePort, firewalled, partial);
	
	WTPair p;
	p.first = nt;
	p.second = new WTransferItem(fDownloads, "", "", "", "", "", "", "");
	
	if (fNumDownloads < gWin->fSettings->GetMaxDownloads())
	{
		PRINT("DLS (%d, %d)\n", fNumDownloads, gWin->fSettings->GetMaxDownloads());
		nt->InitSession();
		nt->SetQueued(false);
	}
	else
	{
		nt->SetQueued(true);
		p.second->setText(WTransferItem::Status, "Locally Queued.");
	}
	fNumDownloads++;
	WASSERT(fNumDownloads >= 0, "Download count is negative!");
	fLock.lock();
	fDownloadList.insert(fDownloadList.end(), p);
	fLock.unlock();
}

void
WDownload::AddUpload(QString remoteIP, uint32 port)
{
	PRINT("WDownload::AddUpload(QString, uint32)\n");
	WUploadThread * ut = new WUploadThread(this);
	PRINT("Setting upload\n");
	ut->SetUpload(remoteIP, port, fSharedFiles);
	
	if (fNumUploads < gWin->fSettings->GetMaxUploads())
	{
		PRINT("Not queued\n");
		ut->SetQueued(false);
	}
	else
	{
		PRINT("Queued\n");
		ut->SetQueued(true);
	}
	fNumUploads++;
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	PRINT("Init session\n");
	ut->InitSession();
	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "");
	if (ut->IsQueued())
	{
		PRINT("IsQueued\n");
		p.second->setText(WTransferItem::Status, "Queued.");
	}
	PRINT("Inserting\n");
	fLock.lock();
	fUploadList.insert(fUploadList.end(), p);
	fLock.unlock();
	UpdateLoad();
}

void
WDownload::AddUpload(int socket, uint32 remoteIP, bool queued)
{
	WUploadThread * ut = new WUploadThread(this);
	ut->SetUpload(socket, remoteIP, fSharedFiles);
	
	if (fNumUploads < gWin->fSettings->GetMaxUploads())
		ut->SetQueued(false);
	else
		ut->SetQueued(true);
	fNumUploads++;
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	ut->InitSession();
	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "");
	if (ut->IsQueued())
		p.second->setText(WTransferItem::Status, "Queued.");
	fLock.lock();
	fUploadList.insert(fUploadList.end(), p);
	fLock.unlock();
	UpdateLoad();
}

void
WDownload::DequeueSessions()
{
	bool found = true;
	
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	int numNotQueued = 0;
	WTIter it;
	
	fLock.lock();
	for (it = fUploadList.begin(); it != fUploadList.end(); it++)
	{
		if (((*it).first->IsQueued() == false) && ((*it).first->IsBlocked() == false))
			numNotQueued++;
	}
	fLock.unlock();

	while (numNotQueued < gWin->fSettings->GetMaxUploads())
	{
		found = false;
		fLock.lock();
		for (it = fUploadList.begin(); it != fUploadList.end() && numNotQueued < gWin->fSettings->GetMaxUploads(); it++)
		{
			if ((*it).first->IsQueued() == true)
			{
				found = true;
				(*it).first->SetQueued(false);
				numNotQueued++;
			}
		}
		fLock.unlock();

		if (!found)
			break;
	}
	WASSERT(fNumDownloads >= 0, "Download count is negative!");
	/** OK, this is how I do queuing for downloads. Since the download count
	* WILL be greater than the amount of downloads allowed, we can't
	* dequeue based on this, right? So, run through the download list
	* and count the amount of downloads NOT queued. If this amount is less
	* than the allowed downloads, find a queued download, and unqueue it :)
	*/
	numNotQueued = 0;
	fLock.lock();
	for (it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		if ((*it).first->IsQueued() == false)	// not queued?
			numNotQueued++;
	}
	fLock.unlock();
	// check the queued amount, vs the amount allowed
	while (numNotQueued < gWin->fSettings->GetMaxDownloads())
	{
		found = false;
		fLock.lock();
		for (it = fDownloadList.begin(); 
		it != fDownloadList.end() && numNotQueued < gWin->fSettings->GetMaxDownloads(); it++)
		{
			if ((*it).first->IsQueued() == true)
			{
				found = true;
				(*it).first->SetQueued(false);
				((WDownloadThread *)((*it).first))->InitSession();
				numNotQueued++;
			}
		}
		fLock.unlock();
		if (!found)
			break;
	}
	UpdateLoad();
}

void
WDownload::customEvent(QCustomEvent * e)
{
	WGenericEvent * g = NULL;
	if (e->type() == WGenericEvent::Type)
	{
		g = dynamic_cast<WGenericEvent *>(e);
	}
	if (g)
	{
		Message * msg = g->Msg();
		WGenericThread * gt = NULL;
		bool b;
		bool upload = false;
		WTransferItem * item = NULL;
		WTIter foundIt;
		
		if (msg->FindPointer("sender", (void **)&gt) != B_OK)
			return;	// failed! ouch!
		if (msg->FindBool("download", &b) == B_OK)
		{
			// a download
			fLock.lock();
			for (foundIt = fDownloadList.begin(); foundIt != fDownloadList.end(); foundIt++)
			{
				if ((*foundIt).first == gt)
				{
					// found our thread
					item = (*foundIt).second;
					PRINT("\t\tFound dl!!!\n");
					break;
				}
			}
			fLock.unlock();
		}
		else if (msg->FindBool("upload", &b) == B_OK)
		{
			fLock.lock();
			for (foundIt = fUploadList.begin(); foundIt != fUploadList.end(); foundIt++)
			{
				if ((*foundIt).first == gt)
				{
					// found our thread
					item = (*foundIt).second;
					upload = true;
					PRINT("\t\tFound ul!!!\n");
					break;
				}
			}
			fLock.unlock();
		}
		if (!item)
		{
			// <postmaster@raasu.org> 20021023 -- Add debug message
			PRINT("\t\tFailed to find file!!!\n");
			return;	// failed to find a item
		}
		switch (msg->what)
		{
		case WGenericEvent::Init:
			{
				const char * filename, * user;
				if (msg->FindString("file", &filename) == B_OK && msg->FindString("user", &user) == B_OK)
				{
					item->setText(WTransferItem::Filename, QString::fromUtf8(filename));
					item->setText(WTransferItem::User, GetUserName(user));
				}
				break;
			}
			
		case WGenericEvent::FileQueued:
			{
				if (upload)
				{
					item->setText(WTransferItem::Status, "Queued.");
				}
				else
				{
					item->setText(WTransferItem::Status, "Remotely Queued.");
				}
				break;
			}
			
		case WGenericEvent::FileBlocked:
			{
				item->setText(WTransferItem::Status, "Blocked.");
				break;
			}
			
		case WGenericEvent::ConnectBackRequest:
			{
				MessageRef cb(new Message(NetClient::CONNECT_BACK_REQUEST), NULL);
				if (cb())
				{
					String session;
					int32 port;
					if (msg->FindInt32("port", &port) == B_OK && msg->FindString("session", session) == B_OK)
					{
						item->setText(WTransferItem::Status, "Waiting for incoming connection...");
						QString tostr = "/*/";
						tostr += QString::fromUtf8(session.Cstr());
						tostr += "/beshare";
						cb()->AddString(PR_NAME_KEYS, (const char *) tostr.utf8());
						cb()->AddString("session", session);
						cb()->AddInt32("port", port);
                        gWin->fNetClient->SendMessageToSessions(cb);
						break;
					}
				}
				gt->Reset();	// failed...
				break;
			}
			
		case WGenericEvent::FileHashing:
			{
				item->setText(WTransferItem::Status, "Examining for resume...");
				break;
			}
			
		case WGenericEvent::ConnectInProgress:
			{
				item->setText(WTransferItem::Status, "Connecting...");
				break;
			}
			
		case WGenericEvent::ConnectFailed:
			{
				String why;
				msg->FindString("why", why);
				item->setText(WTransferItem::Status, tr("Connect failed: %1").arg(why.Cstr()));
				delete (*foundIt).second;
				if (upload)
				{
					DecreaseCount(gt, fNumUploads, false);
					fLock.lock();
					fUploadList.erase(foundIt);
					fLock.unlock();
					WASSERT(fNumUploads >= 0, "Upload count is negative!");
				}
				else
				{
					DecreaseCount(gt, fNumDownloads, false);
					fLock.lock();
					fDownloadList.erase(foundIt);
					fLock.unlock();
					WASSERT(fNumDownloads >= 0, "Download count is negative!");
				}
				delete gt;
				gt = NULL; // <postmaster@raasu.org> 20021027
				DequeueSessions();
				break;
			}
			
		case WGenericEvent::Connected:
			{
				item->setText(WTransferItem::Status, "Negotiating...");
				break;
			}
			
		case WGenericEvent::Disconnected:
			{
				PRINT("\tWGenericEvent::Disconnected\n");
				if (item->text(0) != QString("Finished."))
					item->setText(WTransferItem::Status, "Disconnected.");
				delete (*foundIt).second;
				if (upload)
				{
					DecreaseCount(gt, fNumUploads);
					WASSERT(fNumUploads >= 0, "Upload count is negative!");
					fLock.lock();
					fUploadList.erase(foundIt);
					fLock.unlock();
				}
				else
				{
					DecreaseCount(gt, fNumDownloads);
					WASSERT(fNumDownloads >= 0, "Download count is negative!");
					fLock.lock();
					fDownloadList.erase(foundIt);
					fLock.unlock();
				}
				delete gt;
				gt = NULL; // <postmaster@raasu.org> 20021027
				DequeueSessions();
				break;
			}
			
		case WGenericEvent::FileDone:
			{
				bool b;
				PRINT("\tWGenericEvent::FileDone\n");
				if (msg->FindBool("done", &b) == B_OK)
				{
					PRINT("\tFound done\n");
					if (upload)
					{
						PRINT("\tIs upload\n");
						DecreaseCount(gt, fNumUploads);
						WASSERT(fNumUploads >= 0, "Upload count is negative!");
					}
					else
					{
						DecreaseCount(gt, fNumDownloads, false);
						WASSERT(fNumDownloads >= 0, "Download count is negative!");
					}
					DequeueSessions();
					item->setText(WTransferItem::Status, "Finished.");
				}
				else
				{
					if (!upload)
					{
						item->setText(WTransferItem::Status, "Waiting...");
						item->setText(WTransferItem::Filename, "Waiting for next file...");
						item->setText(WTransferItem::Received, "");
						item->setText(WTransferItem::Total, "");
						item->setText(WTransferItem::Rate, "0.0");
						item->setText(WTransferItem::ETA, "");
						//item->setText(WTransferItem::User, "");	<- don't erase the user name
					}
					else
					{
						item->setText(WTransferItem::Status, "Finished.");
					}
				}
				break;
			}
			
		case WGenericEvent::FileFailed:
			{
				// not used...
				break;
			}
			
		case WGenericEvent::FileStarted:
			{
				String file;
				uint64 start;
				uint64 size;
				String user;
				
				if (msg->FindString("file", file) == B_OK && msg->FindInt64("start", (int64 *)&start) == B_OK &&
					msg->FindInt64("size", (int64 *)&size) == B_OK && msg->FindString("user", user) == B_OK)
				{
					item->setText(WTransferItem::Status, "Waiting for stream...");
					item->setText(WTransferItem::Filename, QString::fromUtf8( file.Cstr() ) ); // <postmaster@raasu.org> 20021023 -- Unicode fix
					// rec, total, rate
					item->setText(WTransferItem::Received, tr("%1").arg((int) start));
					item->setText(WTransferItem::Total, tr("%1").arg((int) size));
					item->setText(WTransferItem::Rate, "0.0");	// <postmaster@raasu.org> 20021023 -- Fix to lowercase 'k'
					item->setText(WTransferItem::ETA, "");
					item->setText(WTransferItem::User, GetUserName( QString::fromUtf8( user.Cstr() ) ));
					
					if (upload)
					{
						if (gWin->fSettings->GetUploads())
							gWin->PrintSystem(tr("%1 is downloading %2.").arg(GetUserName(QString::fromUtf8(user.Cstr()))).arg(QString::fromUtf8(file.Cstr())));
					}
				}
				gt->fLastData.start();
				break;
			}
			
		case WGenericEvent::UpdateUI:
			{
				const char * id;
				if (msg->FindString("id", &id) == B_OK)
				{
					item->setText(WTransferItem::User, GetUserName(id));
				}
				break;
			}
			
		case WGenericEvent::FileError:
			{
				String why;
				String file;
				msg->FindString("why", why);
				if (msg->FindString("file", file) == B_OK)
					item->setText(WTransferItem::Filename, QString::fromUtf8(file.Cstr()) );
				item->setText(WTransferItem::Status, tr("Error: %1").arg(why.Cstr()));
				PRINT("WGenericEvent::FileError: File %s",file.Cstr()); // <postmaster@raasu.org> 20021023 -- Add debug message
				break;
			}
			
		case WGenericEvent::FileDataReceived:
			{
				uint64 offset, size;
				bool done;
				uint32 got;
				
				if ((msg->FindInt64("offset", (int64 *)&offset) == B_OK) && 
					(msg->FindInt64("size", (int64 *)&size) == B_OK) &&
					(msg->FindInt32("got", (int32 *)&got) == B_OK))	// a download ("got")
				{
					gWin->UpdateReceiveStats(got);
					
					double secs = (double)((double)gt->fLastData.elapsed() / 1000.0f);
					double gotk = (double)((double)got / 1024.0f);
					double kps = gotk / secs;
					
					//item->setText(WTransferItem::Status, "Downloading...");
					item->setText(WTransferItem::Status, tr("Downloading: [%1%]").arg(gt->ComputePercentString(offset, size)));
					item->setText(WTransferItem::Received, tr("%1").arg((int) offset));
					// <postmaster@raasu.org> 20021104, 20030217 -- elapsed time > 50 ms?
					if (secs > 0.05f)
					{
						gt->SetMostRecentRate(kps);
						gt->fLastData.restart();
					}
					else
					{
						gt->SetPacketCount(gotk);
					}
					
					// <postmaster@raasu.org> 20021026 -- Too slow transfer rate?
					double gcr = gt->GetCalculatedRate();
					
					item->setText(WTransferItem::ETA, gt->GetETA(offset / 1024, size / 1024, gcr));
					
					item->setText(WTransferItem::Rate, tr("%1").arg(gcr*1024.0f));
					
					if (msg->FindBool("done", &done) == B_OK)
					{
						item->setText(WTransferItem::Status, "Finished.");
						DecreaseCount(gt, fNumDownloads, false);
					}
				}
				else if ((msg->FindInt64("offset", (int64 *)&offset) == B_OK) && 
					(msg->FindInt64("size", (int64 *)&size) == B_OK) &&
					(msg->FindInt32("sent", (int32 *)&got) == B_OK))	// an upload ("sent")
				{
					gWin->UpdateTransmitStats(got);
					
					double secs = (double)((double)gt->fLastData.elapsed() / 1000.0f);
					double gotk = (double)((double)got / 1024.0f);
					double kps = gotk / secs;
					
					//item->setText(WTransferItem::Status, "Uploading...");
					item->setText(WTransferItem::Status, tr("Uploading: [%1%]").arg(gt->ComputePercentString(offset, size)));
					item->setText(WTransferItem::Received, tr("%1").arg((int) offset));
					// <postmaster@raasu.org> 20021104, 20030217 -- elapsed time > 50 ms?
					if (secs > 0.05f) 
					{
						gt->SetMostRecentRate(kps);
						gt->fLastData.restart();
					}
					else
					{
						gt->SetPacketCount(gotk);
					}
					
					// <postmaster@raasu.org> 20021026 -- Too slow transfer rate?
					double gcr = gt->GetCalculatedRate();
					
					item->setText(WTransferItem::ETA, gt->GetETA(offset / 1024, size / 1024, gcr));
					
					item->setText(WTransferItem::Rate, tr("%1").arg(gcr*1024.0f));
				}
				break;
			}
		}
		return;
	}
}

void
WDownload::CancelDL()
{
	QListViewItem * s = fDownloads->selectedItem();
	if (s)
	{
		fLock.lock();
		WTIter it = fDownloadList.begin();
		while (it != fDownloadList.end())
		{
			if ((*it).second == s)
			{
				// found our item, stop it
				(*it).second->setText(WTransferItem::Status, "Canceled.");
				DecreaseCount((*it).first, fNumDownloads);
				WASSERT(fNumDownloads >= 0, "Download count is negative!");
				delete (*it).first;
				delete (*it).second;
				fDownloadList.erase(it);
				break;
			}
			it++;
		}
		fLock.unlock();
	}
	DequeueSessions();
}

void
WDownload::CancelUL()
{
	QListViewItem * s = fUploads->selectedItem();
	if (s)
	{
		fLock.lock();
		WTIter it = fUploadList.begin();
		while (it != fUploadList.end())
		{
			if ((*it).second == s)
			{
				DecreaseCount((*it).first, fNumUploads);
				WASSERT(fNumUploads >= 0, "Upload count is negative!");
				(*it).second->setText(WTransferItem::Status, "Canceled.");
				delete (*it).first;
				delete (*it).second;
				fUploadList.erase(it);
				break;
			}
			it++;
		}
		fLock.unlock();
	}
	DequeueSessions();
}

void
WDownload::KillLocalQueues()
{
	fLock.lock();
	WTIter it = fDownloadList.begin();
	while (it != fDownloadList.end())
	{
		if ((*it).first->IsQueued())	// found queued item?
		{
			// free it
			delete (*it).second;
			DecreaseCount((*it).first, fNumDownloads);
			delete (*it).first;
			WASSERT(fNumDownloads >= 0, "Download count is negative!");
			WTIter nextIt = it;
			nextIt++;
			fDownloadList.erase(it);
			it = nextIt;
		}
		else
			it++;
	}
	fLock.unlock();
}

QString
WDownload::GetUserName(QString sid)
{
	WUserRef uref = gWin->fNetClient->FindUser(sid);
	QString ret = sid;
	if (uref())
		ret = uref()->GetUserName();
	return ret;
}

// This method also resets the thread.
int
WDownload::DecreaseCount(WGenericThread * thread, int & count, bool reset)
{
	if (thread)
	{
		if (thread->IsActive())
		{
			count--;
			if (reset)
				thread->Reset();
			thread->SetActive(false);
		}
		else
		{
			if (reset)
				thread->Reset();
		}
	}
	UpdateLoad();
	return count;
}

void
WDownload::UpdateLoad()
{
	gWin->fNetClient->SetLoad(fNumUploads, gWin->fSettings->GetMaxUploads());
}
