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
	fDownloads->addColumn(tr(MSG_TX_STATUS));
	fDownloads->addColumn(tr(MSG_TX_FILENAME));
	fDownloads->addColumn(tr(MSG_TX_RECEIVED));
	fDownloads->addColumn(tr(MSG_TX_TOTAL));
	fDownloads->addColumn(tr(MSG_TX_RATE));
	fDownloads->addColumn(tr(MSG_TX_ETA));
	fDownloads->addColumn(tr(MSG_TX_USER));
	fDownloads->addColumn(tr("Index"));
	fDownloads->addColumn(tr("QR"));
	
	fDownloads->setColumnAlignment(WTransferItem::Received, AlignRight);	// <postmaster@raasu.org> 20021213
	fDownloads->setColumnAlignment(WTransferItem::Total, AlignRight);		// 
	fDownloads->setColumnAlignment(WTransferItem::Rate, AlignRight);		// 
	fDownloads->setColumnAlignment(WTransferItem::ETA, AlignRight);			// 
	fDownloads->setColumnAlignment(WTransferItem::QR, AlignRight);			// 20030310
	
	fDownloads->setAllColumnsShowFocus(true);
	
	fCancelD = new QPushButton(fBoxD);
	CHECK_PTR(fCancelD);
	fCancelD->setText(tr(MSG_CANCEL));
	connect(fCancelD, SIGNAL(clicked()), this, SLOT(CancelDL()));
	
	fBoxU = new QVBox(fMainSplit);
	CHECK_PTR(fBoxU);
	
	fUploads = new QListView(fBoxU);
	CHECK_PTR(fUploads);
	fUploads->addColumn(tr(MSG_TX_STATUS));
	fUploads->addColumn(tr(MSG_TX_FILENAME));
	fUploads->addColumn(tr(MSG_TX_SENT));
	fUploads->addColumn(tr(MSG_TX_TOTAL));
	fUploads->addColumn(tr(MSG_TX_RATE));
	fUploads->addColumn(tr(MSG_TX_ETA));
	fUploads->addColumn(tr(MSG_TX_USER));
	fUploads->addColumn(tr("Index"));
	fUploads->addColumn(tr("QR"));
	
	fUploads->setColumnAlignment(WTransferItem::Received, AlignRight);	// <postmaster@raasu.org> 20021213
	fUploads->setColumnAlignment(WTransferItem::Total, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::Rate, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::ETA, AlignRight);		//
	fUploads->setColumnAlignment(WTransferItem::QR, AlignRight);		// 20030310
	
	fUploads->setAllColumnsShowFocus(true);
	
	//fButtonsU = new QHBox(fBoxU);
	//CHECK_PTR(fButtonsU);

	fCancelU = new QPushButton(fBoxU);
	CHECK_PTR(fCancelU);
	fCancelU->setText(tr(MSG_CANCEL));
	connect(fCancelU, SIGNAL(clicked()), this, SLOT(CancelUL()));

	/*
	fUnblockU = new QPushButton(fButtonsU);
	CHECK_PTR(fUnblockU);
	fUnblockU->setText(tr(MSG_TOGGLE_BLOCK));
	connect(fUnblockU, SIGNAL(clicked()), this, SLOT(UnblockUL()));
	*/
	connect(gWin->fNetClient, SIGNAL(UserDisconnected(QString, QString)), this,
			SLOT(UserDisconnected(QString, QString)));

	
	setCaption(tr(MSG_TX_CAPTION));
	fNumUploads = fNumDownloads = 0;

	fDLPopup = new QPopupMenu(this, "Download Popup");
	CHECK_PTR(fDLPopup);

	// Create the download popup menu
	fDLQueueID = fDLPopup->insertItem(tr(MSG_TX_QUEUE), ID_QUEUE);
	
	// Create throttle sub menu
	fDLThrottleMenu = new QPopupMenu(this, "Throttle Popup");
	CHECK_PTR(fDLThrottleMenu);
	
	fDLThNone = fDLThrottleMenu->insertItem(tr( MSG_NO_LIMIT ), ID_NO_LIMIT);
	
	// Set initial throttle to none
	fDLThrottleMenu->setItemChecked(fDLThNone, true);
	fDLThrottle = fDLThNone;

	fDLTh128  = fDLThrottleMenu->insertItem(tr( "128 " MSG_BYTES ), ID_128);
	fDLTh256  = fDLThrottleMenu->insertItem(tr( "256 " MSG_BYTES ), ID_256);
	fDLTh512  = fDLThrottleMenu->insertItem(tr( "512 " MSG_BYTES ), ID_512);
	fDLTh1K   = fDLThrottleMenu->insertItem(tr( "1 kB" ), ID_1KB);
	fDLTh2K   = fDLThrottleMenu->insertItem(tr( "2 kB" ), ID_2KB);
	fDLTh4K   = fDLThrottleMenu->insertItem(tr( "4 kB" ), ID_4KB);
	fDLTh8K   = fDLThrottleMenu->insertItem(tr( "8 kB" ), ID_8KB);
	fDLTh16K  = fDLThrottleMenu->insertItem(tr( "16 kB" ), ID_16KB);
	fDLTh32K  = fDLThrottleMenu->insertItem(tr( "32 kB" ), ID_32KB);
	fDLTh64K  = fDLThrottleMenu->insertItem(tr( "64 kB" ), ID_64KB);
	fDLTh128K = fDLThrottleMenu->insertItem(tr( "128 kB" ), ID_128KB);
	fDLTh256K = fDLThrottleMenu->insertItem(tr( "256 kB" ), ID_256KB);
	fDLTh512K = fDLThrottleMenu->insertItem(tr( "512 kB" ), ID_512KB);
	fDLTh1M   = fDLThrottleMenu->insertItem(tr( "1 MB" ), ID_1MB);
	fDLTh2M   = fDLThrottleMenu->insertItem(tr( "2 MB" ), ID_2MB);
	fDLTh4M   = fDLThrottleMenu->insertItem(tr( "4 MB" ), ID_4MB);
	fDLTh8M   = fDLThrottleMenu->insertItem(tr( "8 MB" ), ID_8MB);
	fDLTh16M  = fDLThrottleMenu->insertItem(tr( "16 MB" ), ID_16MB);
	fDLTh32M  = fDLThrottleMenu->insertItem(tr( "32 MB" ), ID_32MB);

	fDLPopup->insertItem(tr(MSG_TX_THROTTLE), fDLThrottleMenu);
	fDLPopup->insertItem(tr(MSG_TX_MOVEUP), ID_MOVEUP);
	fDLPopup->insertItem(tr(MSG_TX_MOVEDOWN), ID_MOVEDOWN);

	connect(fDLPopup, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDLThrottleMenu, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDownloads, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(DLRightClicked(QListViewItem *, const QPoint &, int)));

	fULPopup = new QPopupMenu(this, "Upload Popup");
	CHECK_PTR(fULPopup);

	// Create the download popup menu
	fULQueueID = fULPopup->insertItem(tr(MSG_TX_QUEUE), ID_QUEUE);
	
	// Create throttle sub menu
	fULThrottleMenu = new QPopupMenu(this, "Throttle Popup");
	CHECK_PTR(fULThrottleMenu);
	
	fULThNone = fULThrottleMenu->insertItem(tr( MSG_NO_LIMIT ), ID_NO_LIMIT);
	
	// Set initial throttle to none
	fULThrottleMenu->setItemChecked(fULThNone, true);
	fULThrottle = fULThNone;

	fULTh128  = fULThrottleMenu->insertItem(tr( "128 " MSG_BYTES ), ID_128);
	fULTh256  = fULThrottleMenu->insertItem(tr( "256 " MSG_BYTES ), ID_256);
	fULTh512  = fULThrottleMenu->insertItem(tr( "512 " MSG_BYTES ), ID_512);
	fULTh1K   = fULThrottleMenu->insertItem(tr( "1 kB" ), ID_1KB);
	fULTh2K   = fULThrottleMenu->insertItem(tr( "2 kB" ), ID_2KB);
	fULTh4K   = fULThrottleMenu->insertItem(tr( "4 kB" ), ID_4KB);
	fULTh8K   = fULThrottleMenu->insertItem(tr( "8 kB" ), ID_8KB);
	fULTh16K  = fULThrottleMenu->insertItem(tr( "16 kB" ), ID_16KB);
	fULTh32K  = fULThrottleMenu->insertItem(tr( "32 kB" ), ID_32KB);
	fULTh64K  = fULThrottleMenu->insertItem(tr( "64 kB" ), ID_64KB);
	fULTh128K = fULThrottleMenu->insertItem(tr( "128 kB" ), ID_128KB);
	fULTh256K = fULThrottleMenu->insertItem(tr( "256 kB" ), ID_256KB);
	fULTh512K = fULThrottleMenu->insertItem(tr( "512 kB" ), ID_512KB);
	fULTh1M   = fULThrottleMenu->insertItem(tr( "1 MB" ), ID_1MB);
	fULTh2M   = fULThrottleMenu->insertItem(tr( "2 MB" ), ID_2MB);
	fULTh4M   = fULThrottleMenu->insertItem(tr( "4 MB" ), ID_4MB);
	fULTh8M   = fULThrottleMenu->insertItem(tr( "8 MB" ), ID_8MB);
	fULTh16M  = fULThrottleMenu->insertItem(tr( "16 MB" ), ID_16MB);
	fULTh32M  = fULThrottleMenu->insertItem(tr( "32 MB" ), ID_32MB);

	fULPopup->insertItem(tr(MSG_TX_THROTTLE), fULThrottleMenu);

	fULBanMenu = new QPopupMenu(this, "Ban Popup");
	CHECK_PTR(fULBanMenu);

	fULBanNone = fULBanMenu->insertItem(tr("Unbanned"), ID_UNBAN);
	fULBanMenu->setItemChecked(fULBanNone, true);
	fULBan = fULBanNone;

	fULBan1 = fULBanMenu->insertItem(tr("1 minute"), ID_BAN1);
	fULBan2 = fULBanMenu->insertItem(tr("2 minutes"), ID_BAN2);
	fULBan5 = fULBanMenu->insertItem(tr("5 minutes"), ID_BAN5);
	fULBan10 = fULBanMenu->insertItem(tr("10 minutes"), ID_BAN10);
	fULBan15 = fULBanMenu->insertItem(tr("15 minutes"), ID_BAN15);
	fULBan30 = fULBanMenu->insertItem(tr("30 minutes"), ID_BAN30);
	fULBan1H = fULBanMenu->insertItem(tr("1 hour"), ID_BAN1H);
	fULBanInf = fULBanMenu->insertItem(tr("Infinite"), ID_BANINF);

	fULPopup->insertItem(tr("Block"), fULBanMenu);
	fULPopup->insertItem(tr(MSG_TX_MOVEUP), ID_MOVEUP);
	fULPopup->insertItem(tr(MSG_TX_MOVEDOWN), ID_MOVEDOWN);
	fULIgnoredID = fULPopup->insertItem(tr("Ban IP"), ID_IGNORE);

	connect(fULPopup, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fULThrottleMenu, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fUploads, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(ULRightClicked(QListViewItem *, const QPoint &, int)));

}

WDownload::~WDownload()
{
	fLock.lock();
	WTIter it = fDownloadList.begin();
	while (it != fDownloadList.end())
	{
		// Put all files in resume list
		if ((*it).first)
		{
			int n = (*it).first->GetCurrentNum();

			if (n == -1) // Not even started?
				n = 0;

			for (int i = n; i < (*it).first->GetNumFiles(); i++)
			{
				emit FileInterrupted((*it).first->GetFileName(i), (*it).first->GetRemoteUser());
			}
			
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
WDownload::AddDownload(QString * files, int32 filecount, QString remoteSessionID,
					   uint32 remotePort, QString remoteIP, uint64 remoteInstallID, bool firewalled, bool partial)
{
	WDownloadThread * nt = new WDownloadThread(this);
	nt->SetFile(files, filecount, remoteIP, remoteSessionID, fLocalSID, remotePort, firewalled, partial);
	
	WTPair p;
	p.first = nt;
	p.second = new WTransferItem(fDownloads, "", "", "", "", "", "", "", "", "");

	int x;

	if (remoteIP == "127.0.0.1")	// Can't use localhost to download!!!
	{
		remoteIP = gWin->fNetClient->GetServerIP();
		if (remoteIP != "127.0.0.1")
		{
			for (x = 0; x < filecount; x++)
				gWin->PrintWarning(tr("Invalid address! Download address for file %1 replaced with %2, it might fail!").arg(files[x]).arg(remoteIP), false);
		}
	}

	// Detect uncommon remote port 

	if (!muscleInRange(remotePort, (uint32) DEFAULT_LISTEN_PORT, (uint32) (DEFAULT_LISTEN_PORT + LISTEN_PORT_RANGE)))
	{
		for (x = 0; x < filecount; x++)
			gWin->PrintWarning(tr("Download port for file %1 might be out of range, it might fail!").arg(files[x]), false);
	}

	if (fNumDownloads < gWin->fSettings->GetMaxDownloads())
	{
		PRINT("DLS (%d, %d)\n", fNumDownloads, gWin->fSettings->GetMaxDownloads());
		nt->InitSession();
		nt->SetLocallyQueued(false);
	}
	else
	{
		nt->SetLocallyQueued(true);
		p.second->setText(WTransferItem::Status, "Locally Queued.");
	}
	fNumDownloads++;
	WASSERT(fNumDownloads >= 0, "Download count is negative!");
	fLock.lock();
	fDownloadList.insert(fDownloadList.end(), p);
	fLock.unlock();
	UpdateDLRatings();
}

void
WDownload::AddDownloadList(Queue<QString> & fQueue, WUser * user)
{
	int32 nFiles = fQueue.GetNumItems();
	QString * qFiles = new QString[nFiles];
	int n = 0;
	while (!fQueue.IsEmpty())
	{
		qFiles[n++] = fQueue.RemoveHead();
	}
	AddDownload(qFiles, nFiles, user->GetUserID(), user->GetPort(), user->GetUserHostName(), user->GetInstallID(), user->GetFirewalled(), user->GetPartial());
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
		ut->SetLocallyQueued(false);
	}
	else
	{
		PRINT("Queued\n");
		ut->SetLocallyQueued(true);
	}
	fNumUploads++;
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	PRINT("Init session\n");
	ut->InitSession();
	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "", "", "");
	if (ut->IsLocallyQueued())
	{
		PRINT("IsQueued\n");
		p.second->setText(WTransferItem::Status, "Queued.");
	}
	PRINT("Inserting\n");
	fLock.lock();
	fUploadList.insert(fUploadList.end(), p);
	fLock.unlock();
	UpdateLoad();
	UpdateULRatings();
}

void
WDownload::AddUpload(int socket, uint32 remoteIP, bool queued)
{
	WUploadThread * ut = new WUploadThread(this);
	ut->SetUpload(socket, remoteIP, fSharedFiles);
	
	if (fNumUploads < gWin->fSettings->GetMaxUploads())
		ut->SetLocallyQueued(false);
	else
		ut->SetLocallyQueued(true);
	fNumUploads++;
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	ut->InitSession();
	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "", "", "");
	if (ut->IsLocallyQueued())
		p.second->setText(WTransferItem::Status, "Queued.");
	fLock.lock();
	fUploadList.insert(fUploadList.end(), p);
	fLock.unlock();
	UpdateLoad();
	UpdateULRatings();
}

void
WDownload::DequeueSessions()
{
	bool found = true;
	
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	int numNotQueued = 0;
	WTIter it;
	
	int qr = 0;
	fLock.lock();
	for (it = fUploadList.begin(); it != fUploadList.end(); it++)
	{
		if (((*it).first->IsLocallyQueued() == false) && ((*it).first->IsBlocked() == false))
			numNotQueued++;
		(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
	}
	fLock.unlock();

	while (numNotQueued < gWin->fSettings->GetMaxUploads())
	{
		found = false;
		fLock.lock();
		for (it = fUploadList.begin(); it != fUploadList.end() && numNotQueued < gWin->fSettings->GetMaxUploads(); it++)
		{
			if (((*it).first->IsLocallyQueued() == true) && ((*it).first->IsManuallyQueued() == false))
			{
				found = true;
				(*it).first->SetLocallyQueued(false);
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
	qr = 0;
	fLock.lock();
	for (it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		if ((*it).first->IsLocallyQueued() == false)	// not queued?
			numNotQueued++;
		(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
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
			if (((*it).first->IsLocallyQueued() == true) && ((*it).first->IsManuallyQueued() == false))
			{
				found = true;
				(*it).first->SetLocallyQueued(false);
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
					item->setText(WTransferItem::Index, FormatIndex(gt->GetCurrentNum(), gt->GetNumFiles()));
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
				uint64 timeLeft = (uint64) -1;
				(void) msg->FindInt64("timeleft", (int64 *) &timeLeft);
				if (timeLeft == -1)
					item->setText(WTransferItem::Status, "Blocked.");
				else
					item->setText(WTransferItem::Status, tr("Blocked for %1 minutes.").arg((int) (timeLeft/1000000)));
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
				String why, mFile;
				bool b;
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
					if (msg->FindBool("retry", &b) == B_OK)
					{
						if (b)
						{
							int nFiles = gt->GetNumFiles();
							int n = gt->GetCurrentNum();
							if (n == -1) // Test!!!
								n = 0;
							while (n < nFiles)
							{
								QString qFile = gt->GetFileName(n++);
								emit FileFailed(qFile, gt->GetRemoteUser());
							}
						}
					}
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
					// emit FileFailed signal, so we can record the filename and remote username for resuming later
					String mFile;
					if (msg->FindBool("failed", &b) == B_OK)
					{
						// "failed" == true only, if the transfer has failed
						if (b)
						{
							for (int n = gt->GetCurrentNum(); n < gt->GetNumFiles(); n++)
							{
								QString qFile = gt->GetFileName(n);
								emit FileFailed(qFile, gt->GetRemoteUser());
							}
						}
					}
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
						if (gt->IsLastFile())
							DecreaseCount(gt, fNumUploads);
						WASSERT(fNumUploads >= 0, "Upload count is negative!");
					}
					else
					{
						PRINT("\tIs download\n");
						if (gt->IsLastFile())
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
					item->setText(WTransferItem::Rate, "0.0");
					item->setText(WTransferItem::ETA, "");
					item->setText(WTransferItem::User, GetUserName( QString::fromUtf8( user.Cstr() ) ));
					item->setText(WTransferItem::Index, FormatIndex(gt->GetCurrentNum(), gt->GetNumFiles()));

					if (upload)
					{
						if (gWin->fSettings->GetUploads())
							gWin->PrintSystem(tr(MSG_TX_ISDOWNLOADING).arg(GetUserName(QString::fromUtf8(user.Cstr()))).arg(QString::fromUtf8(file.Cstr())));
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
				item->setText(WTransferItem::Index, FormatIndex(gt->GetCurrentNum(), gt->GetNumFiles()));
				PRINT("WGenericEvent::FileError: File %s",file.Cstr()); // <postmaster@raasu.org> 20021023 -- Add debug message
				break;
			}
			
		case WGenericEvent::FileDataReceived:
			{
				uint64 offset, size;
				bool done;
				String mFile;
				uint32 got;
				
				if ((msg->FindInt64("offset", (int64 *)&offset) == B_OK) && 
					(msg->FindInt64("size", (int64 *)&size) == B_OK))
				{
					if (msg->FindInt32("got", (int32 *)&got) == B_OK)	// a download ("got")
					{
						gWin->UpdateReceiveStats(got);
						
						double secs = (double)((double)gt->fLastData.elapsed() / 1000.0f);
						double gotk = (double)((double)got / 1024.0f);
						double kps = gotk / secs;
						
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
							if (msg->FindString("file", mFile) == B_OK)
								gWin->PrintSystem( tr(MSG_TX_FINISHED).arg(gt->GetRemoteUser()).arg( QString::fromUtf8(mFile.Cstr()) ) , false);
							DecreaseCount(gt, fNumDownloads, false);
						}
					}
					else if (msg->FindInt32("sent", (int32 *)&got) == B_OK)	// an upload ("sent")
					{
						gWin->UpdateTransmitStats(got);
						
						double secs = (double)((double)gt->fLastData.elapsed() / 1000.0f);
						double gotk = (double)((double)got / 1024.0f);
						double kps = gotk / secs;
						
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
						
						if (msg->FindBool("done", &done) == B_OK)
						{
							item->setText(WTransferItem::Status, "Finished.");
							if (msg->FindString("file", mFile) == B_OK)
								gWin->PrintSystem( tr(MSG_TX_HASFINISHED).arg(gt->GetRemoteUser()).arg( QString::fromUtf8(mFile.Cstr()) ) , false);
						}
						
					}
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
WDownload::UnblockUL()
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
				if ((*it).first->IsBlocked() == true)
				{
					(*it).first->SetBlocked(false);
				}
				else
				{
					(*it).first->SetBlocked(true);
				}
				break;
			}
			it++;
		}
		fLock.unlock();
	}
}

void
WDownload::KillLocalQueues()
{
	fLock.lock();
	WTIter it = fDownloadList.begin();
	while (it != fDownloadList.end())
	{
		if ((*it).first->IsLocallyQueued())	// found queued item?
		{
			// Put all files in resume list
			int n = (*it).first->GetCurrentNum();

			if (n == -1) // Not even started?
				n = 0;

			for (int i = n; i < (*it).first->GetNumFiles(); i++)
			{
				emit FileInterrupted((*it).first->GetFileName(i), (*it).first->GetRemoteUser());
			}

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

/*
 *
 * Block users that disconnect if user has chosen that feature in settings
 *
 */
void
WDownload::UserDisconnected(QString sid, QString name)
{
	if (gWin->fSettings->GetBlockDisconnected())
	{
		fLock.lock();
		WTIter it = fUploadList.begin();
		while (it != fUploadList.end())
		{
			if ((*it).first->GetRemoteID() == sid)
			{
				if ((*it).first->IsBlocked() == false)
				{
					(*it).first->SetBlocked(true);
				}
			}
			it++;
		}
		fLock.unlock();
	}
}

QString
WDownload::FormatIndex(int32 cur, int32 num)
{
	return QObject::tr("%1 of %2").arg(cur+1).arg(num);
}

void
WDownload::DLPopupActivated(int id)
{
	fLock.lock();
	WTIter i;
	WGenericThread * gt = NULL;
	bool findRet;

	findRet = FindItem(fDownloadList, i, fDLPopupItem);

	if (!findRet)
	{
		fLock.unlock();
		return;
	}

	gt = (*i).first;
	WASSERT(gt != NULL, "Generic Thread is invalid!");

	switch (id)
	{
		case ID_QUEUE:
		{
				if (!gt->IsLocallyQueued())
				{
					gt->SetLocallyQueued(true);
					gt->SetManuallyQueued(true);
					gt->Reset();
					// download count will be decremented
					// when the disconnect message is received
				}
				else
				{
					gt->SetLocallyQueued(false);
					gt->SetManuallyQueued(false);
					((WDownloadThread *)gt)->InitSession();
					// and we need to increase the dl count
					fNumDownloads++;
				}
			break;
		}

		case ID_MOVEUP:
			{
				if (gt->IsLocallyQueued() || gt->IsRemotelyQueued())
					MoveUp(fDownloadList, i);
				break;
			}

		case ID_MOVEDOWN:
			{
				if (gt->IsLocallyQueued() || gt->IsRemotelyQueued())
					MoveDown(fDownloadList, i);
				break;
			}

		case ID_NO_LIMIT:
			gt->SetRate(0);
			break;

		case ID_128:
			gt->SetRate(128);
			break;

		case ID_256:
			gt->SetRate(256);
			break;

		case ID_512:
			gt->SetRate(512);
			break;
			
		case ID_1KB:
			gt->SetRate(1024);
			break;

		case ID_2KB:
			gt->SetRate(2 * 1024);
			break;

		case ID_4KB:
			gt->SetRate(4 * 1024);
			break;

		case ID_8KB:
			gt->SetRate(8 * 1024);
			break;

		case ID_16KB:
			gt->SetRate(16 * 1024);
			break;

		case ID_32KB:
			gt->SetRate(32 * 1024);
			break;

		case ID_64KB:
			gt->SetRate(64 * 1024);
			break;

		case ID_128KB:
			gt->SetRate(128 * 1024);
			break;

		case ID_256KB:
			gt->SetRate(256 * 1024);
			break;

		case ID_512KB:
			gt->SetRate(512 * 1024);
			break;

		case ID_1MB:
			gt->SetRate(1024 * 1024);
			break;

		case ID_2MB:
			gt->SetRate(2 * 1024 * 1024);
			break;

		case ID_4MB:
			gt->SetRate(4 * 1024 * 1024);
			break;

		case ID_8MB:
			gt->SetRate(8 * 1024 * 1024);
			break;

		case ID_16MB:
			gt->SetRate(16 * 1024 * 1024);
			break;

		case ID_32MB:
			gt->SetRate(32 * 1024 * 1024);
			break;
	}
	fLock.unlock();
	UpdateDLRatings();
}

void
WDownload::ULPopupActivated(int id)
{
	fLock.lock();
	WTIter i;
	WGenericThread * gt = NULL;
	bool findRet;
	
	findRet = FindItem(fUploadList, i, fULPopupItem);
	
	if (!findRet)
	{
		fLock.lock();
		return;
	}
	
	gt = (*i).first;
	WASSERT(gt != NULL, "Generic Thread is invalid!");
	
	switch (id)
	{
	case ID_QUEUE:
		{
			if (!gt->IsLocallyQueued())
			{
				gt->SetLocallyQueued(true);
				gt->SetManuallyQueued(true);
			}
			else
			{
				gt->SetLocallyQueued(false);
				gt->SetManuallyQueued(false);
			}
			break;
		}
		
	case ID_MOVEUP:
		{
			if (gt->IsLocallyQueued())
				MoveUp(fUploadList, i);
			break;
		}
		
	case ID_MOVEDOWN:
		{
			if (gt->IsLocallyQueued())
				MoveDown(fUploadList, i);
			break;
		}
		
		
	case ID_NO_LIMIT:
		{
			gt->SetRate(0);
			break;
		}
		
	case ID_128:
		{
			gt->SetRate(128);
			break;
		}
		
	case ID_256:
		{
			gt->SetRate(256);
			break;
		}
		
	case ID_512:
		{
			gt->SetRate(512);
			break;
		}
		
	case ID_1KB:
		{
			gt->SetRate(1024);
			break;
		}
		
	case ID_2KB:
		{
			gt->SetRate(2 * 1024);
			break;
		}
		
	case ID_4KB:
		{
			gt->SetRate(4 * 1024);
			break;
		}
		
	case ID_8KB:
		{
			gt->SetRate(8 * 1024);
			break;
		}
		
	case ID_16KB:
		{
			gt->SetRate(16 * 1024);
			break;
		}
		
	case ID_32KB:
		{
			gt->SetRate(32 * 1024);
			break;
		}
		
	case ID_64KB:
		{
			gt->SetRate(64 * 1024);
			break;
		}
		
	case ID_128KB:
		{
			gt->SetRate(128 * 1024);
			break;
		}
		
	case ID_256KB:
		{
			gt->SetRate(256 * 1024);
			break;
		}
		
	case ID_512KB:
		{
			gt->SetRate(512 * 1024);
			break;
		}
		
	case ID_1MB:
		{
			gt->SetRate(1024 * 1024);
			break;
		}
		
	case ID_2MB:
		{
			gt->SetRate(2 * 1024 * 1024);
			break;
		}
		
	case ID_4MB:
		{
			gt->SetRate(4 * 1024 * 1024);
			break;
		}
		
	case ID_8MB:
		{
			gt->SetRate(8 * 1024 * 1024);
			break;
		}
		
	case ID_16MB:
		{
			gt->SetRate(16 * 1024 * 1024);
			break;
		}
		
	case ID_32MB:
		{
			gt->SetRate(32 * 1024 * 1024);
			break;
		}
		
	case ID_UNBAN:
		{
			gt->SetBlocked(false);
			break;
		}
		
	case ID_BAN1:
		{
			gt->SetBlocked(true, 1000000);
			break;
		}
		
	case ID_BAN2:
		{
			gt->SetBlocked(true, 2000000);
			break;
		}
		
	case ID_BAN5:
		{
			gt->SetBlocked(true, 5000000);
			break;
		}
		
	case ID_BAN10:
		{
			gt->SetBlocked(true, 10000000);
			break;
		}
		
	case ID_BAN15:
		{
			gt->SetBlocked(true, 15000000);
			break;
		}
		
	case ID_BAN30:
		{
			gt->SetBlocked(true, 30000000);
			break;
		}
		
	case ID_BAN1H:
		{
			gt->SetBlocked(true, 60000000);
			break;
		}

	case ID_BANINF:
		{
			gt->SetBlocked(true, -1);
			break;
		}

	case ID_IGNORE:
		{
			if ( gWin->IsIgnoredIP( gt->GetRemoteIP() ) )
			{
				gWin->RemoveIPIgnore(gt->GetRemoteIP());
				if (gt->IsBlocked())
					gt->SetBlocked(false, -1);
			}
			else
			{
				gWin->AddIPIgnore(gt->GetRemoteIP());
				if (!gt->IsBlocked())
					gt->SetBlocked(true, -1);
			}
			break;
		}
	}
	fLock.unlock();
	UpdateULRatings();
}


void
WDownload::ULRightClicked(QListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fULPopupItem = item;

		fLock.lock();
		WTIter iter;
		if (FindItem(fUploadList, iter, item))
		{
			fULPopup->setItemChecked(fULQueueID, (*iter).first->IsLocallyQueued());
			fULPopup->setItemChecked(fULIgnoredID, gWin->IsIgnoredIP( (*iter).first->GetRemoteIP() ));
			int fNewRate = (*iter).first->GetRate();
			fULThrottleMenu->setItemChecked(fULThrottle, false);
			
			switch (fNewRate)
			{
			case 0: 	
				{
					fULThrottle = fULThNone;	
					break;
				}
			case 128:	
				{
					fULThrottle = fULTh128;		
					break;
				}
			case 256:	
				{
					fULThrottle = fULTh256;		
					break;
				}
			case 512:	
				{
					fULThrottle = fULTh512;		
					break;
				}
			case 1024:	
				{
					fULThrottle = fULTh1K;		
					break;
				}
			case 2 * 1024:	
				{
					fULThrottle = fULTh2K;	
					break;
				}
			case 4 * 1024:	
				{
					fULThrottle = fULTh4K;	
					break;
				}
			case 8 * 1024:	
				{
					fULThrottle = fULTh8K;	
					break;
				}
			case 16 * 1024:	
				{
					fULThrottle = fULTh16K; 
					break;
				}
			case 32 * 1024: 
				{
					fULThrottle = fULTh32K;	
					break;
				}
			case 64 * 1024:	
				{
					fULThrottle = fULTh64K;	
					break;
				}
			case 128 * 1024:	
				{
					fULThrottle = fULTh128K; 
					break;
				}
			case 256 * 1024:	
				{
					fULThrottle = fULTh256K; 
					break;
				}
			case 512 * 1024:	
				{
					fULThrottle = fULTh512K;	
					break;
				}
			case 1048576:		
				{
					fULThrottle = fULTh1M;	
					break;
				}
			case 2 * 1048576:	
				{
					fULThrottle = fULTh2M;	
					break;
				}
			case 4 * 1048576:	
				{
					fULThrottle = fULTh4M;	
					break;
				}
			case 8 * 1048576:	
				{
					fULThrottle = fULTh8M;	
					break;
				}
			case 16 * 1048576:	
				{
					fULThrottle = fULTh16M;	
					break;
				}
			case 32 * 1048576:	
				{
					fULThrottle = fULTh32M; 
					break;
				}
			}
			
			fULThrottleMenu->setItemChecked(fULThrottle, true);
			
			int fNewBan = (*iter).first->GetBanTime();
			fULThrottleMenu->setItemChecked(fULBan, false);
			
			switch (fNewBan)
			{
			case -1:
				{
					fULBan = fULBanInf;
					break;
				}
			case 0:
				{
					fULBan = fULBanNone;
					break;
				}
			case 1:
				{
					fULBan = fULBan1;
					break;
				}
			case 2:
				{
					fULBan = fULBan2;
					break;
				}
			case 5:
				{
					fULBan = fULBan5;
					break;
				}
			case 10:
				{
					fULBan = fULBan10;
					break;
				}
			case 15:
				{
					fULBan = fULBan15;
					break;
				}
			case 30:
				{
					fULBan = fULBan30;
					break;
				}
			case 60:
				{
					fULBan = fULBan1H;
					break;
				}
			}
			fULThrottleMenu->setItemChecked(fULThrottle, true);
			fULPopup->popup(p);
		}
		fLock.unlock();
	}
}

void
WDownload::DLRightClicked(QListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fDLPopupItem = item;

		fLock.lock();
		WTIter iter;
		if (FindItem(fDownloadList, iter, item))
		{
			fDLPopup->setItemChecked(fDLQueueID, (*iter).first->IsLocallyQueued());
			int fNewRate = (*iter).first->GetRate();
			fDLThrottleMenu->setItemChecked(fDLThrottle, false);
			switch (fNewRate)
			{
			case 0: 	
				{
					fDLThrottle = fDLThNone;	
					break;
				}
			case 128:	
				{
					fDLThrottle = fDLTh128;		
					break;
				}
			case 256:	
				{
					fDLThrottle = fDLTh256;		
					break;
				}
			case 512:	
				{
					fDLThrottle = fDLTh512;		
					break;
				}
			case 1024:	
				{
					fDLThrottle = fDLTh1K;		
					break;
				}
			case 2 * 1024:	
				{
					fDLThrottle = fDLTh2K;	
					break;
				}
			case 4 * 1024:	
				{
					fDLThrottle = fDLTh4K;	
					break;
				}
			case 8 * 1024:	
				{
					fDLThrottle = fDLTh8K;	
					break;
				}
			case 16 * 1024:	
				{
					fDLThrottle = fDLTh16K; 
					break;
				}
			case 32 * 1024: 
				{
					fDLThrottle = fDLTh32K;	
					break;
				}
			case 64 * 1024:	
				{
					fDLThrottle = fDLTh64K;	
					break;
				}
			case 128 * 1024:	
				{
					fDLThrottle = fDLTh128K; 
					break;
				}
			case 256 * 1024:	
				{
					fDLThrottle = fDLTh256K; 
					break;
				}
			case 512 * 1024:	
				{
					fDLThrottle = fDLTh512K;	
					break;
				}
			case 1048576:		
				{
					fDLThrottle = fDLTh1M;	
					break;
				}
			case 2 * 1048576:	
				{
					fDLThrottle = fDLTh2M;	
					break;
				}
			case 4 * 1048576:	
				{
					fDLThrottle = fDLTh4M;	
					break;
				}
			case 8 * 1048576:	
				{
					fDLThrottle = fDLTh8M;	
					break;
				}
			case 16 * 1048576:	
				{
					fDLThrottle = fDLTh16M;	
					break;
				}
			case 32 * 1048576:	
				{
					fDLThrottle = fDLTh32M; 
					break;
				}
			}
			fDLThrottleMenu->setItemChecked(fDLThrottle, true);
			fDLPopup->popup(p);
		}
		fLock.unlock();
	}
}

bool
WDownload::FindItem(WTList & lst, WTIter & ret, QListViewItem * s)
{
	bool success = false;
	WTIter it = lst.begin();

	while (it != lst.end())
	{
		if ((*it).second == s)
		{
			ret = it;
			success = true;
			break;
		}
		it++;
	}
	return success;
}

void
WDownload::MoveUp(WTList & lst, WTIter iter)
{
	if (iter == lst.begin())
		return;
	WTIter iter2 = iter;
	while (1) 
	{
		iter2--;
		if ((*iter2).first->IsLocallyQueued())
			break;
		if (iter2 == lst.begin())
			break;
	}
	WTPair wp = (*iter);
	lst.erase(iter);
	lst.insert(iter2, wp);
}

void
WDownload::MoveDown(WTList & lst, WTIter iter)
{
	if (iter == lst.end())
		return;
	WTIter iter2 = iter;
	while (1)
	{
		iter2++;
		if ((*iter2).first->IsLocallyQueued())
			break;
		if (iter2 == lst.end())
			break;
	}
	WTPair wp = (*iter);
	lst.erase(iter);
	lst.insert(iter2, wp);
}

void
WDownload::UpdateULRatings()
{
	int qr = 0;
	fLock.lock();
	for (WTIter it = fUploadList.begin(); it != fUploadList.end(); it++)
	{
		(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
	}
	fLock.unlock();
}

void
WDownload::UpdateDLRatings()
{
	int qr = 0;
	fLock.lock();
	for (WTIter it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
	}
	fLock.unlock();
}

void
WDownload::TransferCallBackRejected(QString qFrom, int64 timeLeft, uint32 port)
{
	fLock.lock();
	for (WTIter it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		if (((*it).first->GetRemoteUser() == qFrom) && 
			((*it).first->IsActive() == false) &&
			((*it).first->GetRemotePort() == port)
			)
		{
			(*it).first->SetBlocked(true, timeLeft);
			break;
		}
	}
	fLock.unlock();
}
