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
#include "global.h"
#include "filethread.h"
#include "accept.h"
#include "settings.h"
#include "downloadthread.h"
#include "uploadthread.h"
#include "wgenericevent.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"
#include "platform.h"	// <postmaster@raasu.org> 20021114
#include "transferitem.h"
#include "gotourl.h"

// ----------------------------------------------------------------------------------------------

WDownload::WDownload(QWidget * parent, QString localID, WFileThread * ft)
: QDialog(parent, "WDownload", false, /* QWidget::WDestructiveClose |*/ QWidget::WStyle_Minimize |
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
	fDownloads->addColumn(tr("Status"));
	fDownloads->addColumn(tr("Filename"));
	fDownloads->addColumn(tr("Received"));
	fDownloads->addColumn(tr("Total"));
	fDownloads->addColumn(tr("Rate"));
	fDownloads->addColumn(tr("ETA"));
	fDownloads->addColumn(tr("User"));
	fDownloads->addColumn(tr("Index"));
	fDownloads->addColumn(tr("QR"));
	
	fDownloads->setColumnAlignment(WTransferItem::Received, AlignRight);	// <postmaster@raasu.org> 20021213
	fDownloads->setColumnAlignment(WTransferItem::Total, AlignRight);		// 
	fDownloads->setColumnAlignment(WTransferItem::Rate, AlignRight);		// 
	fDownloads->setColumnAlignment(WTransferItem::ETA, AlignRight);			// 
	fDownloads->setColumnAlignment(WTransferItem::QR, AlignRight);			// 20030310
	
	fDownloads->setAllColumnsShowFocus(true);
	
	fBoxU = new QVBox(fMainSplit);
	CHECK_PTR(fBoxU);
	
	fUploads = new QListView(fBoxU);
	CHECK_PTR(fUploads);
	fUploads->addColumn(tr("Status"));
	fUploads->addColumn(tr("Filename"));
	fUploads->addColumn(tr("Sent"));
	fUploads->addColumn(tr("Total"));
	fUploads->addColumn(tr("Rate"));
	fUploads->addColumn(tr("ETA"));
	fUploads->addColumn(tr("User"));
	fUploads->addColumn(tr("Index"));
	fUploads->addColumn(tr("QR"));
	
	fUploads->setColumnAlignment(WTransferItem::Received, AlignRight);	// <postmaster@raasu.org> 20021213
	fUploads->setColumnAlignment(WTransferItem::Total, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::Rate, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::ETA, AlignRight);		//
	fUploads->setColumnAlignment(WTransferItem::QR, AlignRight);		// 20030310
	
	fUploads->setAllColumnsShowFocus(true);
	
	connect(gWin->fNetClient, SIGNAL(UserDisconnected(QString, QString)), this,
			SLOT(UserDisconnected(QString, QString)));

	
	setCaption(tr("File Transfers"));
//	fNumUploads = fNumDownloads = 0;

	fDLPopup = new QPopupMenu(this, "Download Popup");
	CHECK_PTR(fDLPopup);

	// Create the download popup menu
	fDLPopup->insertItem(tr("Queue"), ID_QUEUE);
	
	// Create throttle sub menu
	fDLThrottleMenu = new QPopupMenu(fDLPopup, "Throttle Popup");
	CHECK_PTR(fDLThrottleMenu);
	
	fDLThrottleMenu->insertItem(tr( "No Limit" ), ID_NO_LIMIT);
	
	// Set initial throttle to none
	fDLThrottleMenu->setItemChecked(ID_NO_LIMIT, true);
	fDLThrottle = ID_NO_LIMIT;

	fDLThrottleMenu->insertItem(tr( "64 bytes" ), ID_64);
	fDLThrottleMenu->insertItem(tr( "128 bytes" ), ID_128);
	fDLThrottleMenu->insertItem(tr( "256 bytes" ), ID_256);
	fDLThrottleMenu->insertItem(tr( "512 bytes" ), ID_512);
	fDLThrottleMenu->insertItem(tr( "1 kB" ), ID_1KB);
	fDLThrottleMenu->insertItem(tr( "2 kB" ), ID_2KB);
	fDLThrottleMenu->insertItem(tr( "4 kB" ), ID_4KB);
	fDLThrottleMenu->insertItem(tr( "6 kB" ), ID_6KB);
	fDLThrottleMenu->insertItem(tr( "8 kB" ), ID_8KB);
	fDLThrottleMenu->insertItem(tr( "10 kB" ), ID_10KB);
	fDLThrottleMenu->insertItem(tr( "12 kB" ), ID_12KB);
	fDLThrottleMenu->insertItem(tr( "14 kB" ), ID_14KB);
	fDLThrottleMenu->insertItem(tr( "16 kB" ), ID_16KB);
	fDLThrottleMenu->insertItem(tr( "32 kB" ), ID_32KB);
	fDLThrottleMenu->insertItem(tr( "64 kB" ), ID_64KB);
	fDLThrottleMenu->insertItem(tr( "128 kB" ), ID_128KB);
	fDLThrottleMenu->insertItem(tr( "256 kB" ), ID_256KB);
	fDLThrottleMenu->insertItem(tr( "512 kB" ), ID_512KB);
	fDLThrottleMenu->insertItem(tr( "1 MB" ), ID_1MB);
	fDLThrottleMenu->insertItem(tr( "2 MB" ), ID_2MB);
	fDLThrottleMenu->insertItem(tr( "4 MB" ), ID_4MB);
	fDLThrottleMenu->insertItem(tr( "8 MB" ), ID_8MB);
	fDLThrottleMenu->insertItem(tr( "16 MB" ), ID_16MB);
	fDLThrottleMenu->insertItem(tr( "32 MB" ), ID_32MB);

	fDLRunMenu = new QPopupMenu(fDLPopup, "Run Popup");
	CHECK_PTR(fDLRunMenu);

	fDLPopup->insertItem(tr("Throttle"), fDLThrottleMenu, ID_THROTTLE);
	fDLPopup->insertItem(tr("Run..."), fDLRunMenu, ID_RUN);

	fDLPopup->insertItem(tr("Move Up"), ID_MOVEUP);
	fDLPopup->insertItem(tr("Move Down"), ID_MOVEDOWN);

	fDLPopup->insertItem(tr("Clear Finished"), ID_CLEAR);
	fDLPopup->insertItem(tr("Cancel"), ID_CANCEL);

	connect(fDLPopup, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDLThrottleMenu, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDLRunMenu, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDownloads, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(DLRightClicked(QListViewItem *, const QPoint &, int)));

	fULPopup = new QPopupMenu(this, "Upload Popup");
	CHECK_PTR(fULPopup);

	// Create the upload popup menu
	fULPopup->insertItem(tr("Queue"), ID_QUEUE);
	
	// Create throttle sub menu
	fULThrottleMenu = new QPopupMenu(fULPopup, "Throttle Popup");
	CHECK_PTR(fULThrottleMenu);
	
	fULThrottleMenu->insertItem(tr( "No Limit" ), ID_NO_LIMIT);
	
	// Set initial throttle to none
	fULThrottleMenu->setItemChecked(ID_NO_LIMIT, true);
	fULThrottle = ID_NO_LIMIT;

	fULThrottleMenu->insertItem(tr( "64 bytes" ), ID_64);
	fULThrottleMenu->insertItem(tr( "128 bytes" ), ID_128);
	fULThrottleMenu->insertItem(tr( "256 bytes" ), ID_256);
	fULThrottleMenu->insertItem(tr( "512 bytes" ), ID_512);
	fULThrottleMenu->insertItem(tr( "1 kB" ), ID_1KB);
	fULThrottleMenu->insertItem(tr( "2 kB" ), ID_2KB);
	fULThrottleMenu->insertItem(tr( "4 kB" ), ID_4KB);
	fULThrottleMenu->insertItem(tr( "6 kB" ), ID_6KB);
	fULThrottleMenu->insertItem(tr( "8 kB" ), ID_8KB);
	fULThrottleMenu->insertItem(tr( "10 kB" ), ID_10KB);
	fULThrottleMenu->insertItem(tr( "12 kB" ), ID_12KB);
	fULThrottleMenu->insertItem(tr( "14 kB" ), ID_14KB);
	fULThrottleMenu->insertItem(tr( "16 kB" ), ID_16KB);
	fULThrottleMenu->insertItem(tr( "32 kB" ), ID_32KB);
	fULThrottleMenu->insertItem(tr( "64 kB" ), ID_64KB);
	fULThrottleMenu->insertItem(tr( "128 kB" ), ID_128KB);
	fULThrottleMenu->insertItem(tr( "256 kB" ), ID_256KB);
	fULThrottleMenu->insertItem(tr( "512 kB" ), ID_512KB);
	fULThrottleMenu->insertItem(tr( "1 MB" ), ID_1MB);
	fULThrottleMenu->insertItem(tr( "2 MB" ), ID_2MB);
	fULThrottleMenu->insertItem(tr( "4 MB" ), ID_4MB);
	fULThrottleMenu->insertItem(tr( "8 MB" ), ID_8MB);
	fULThrottleMenu->insertItem(tr( "16 MB" ), ID_16MB);
	fULThrottleMenu->insertItem(tr( "32 MB" ), ID_32MB);

	fULBanMenu = new QPopupMenu(fULPopup, "Ban Popup");
	CHECK_PTR(fULBanMenu);

	fULBanMenu->insertItem(tr("Unbanned"), ID_UNBAN);
	fULBanMenu->setItemChecked(ID_UNBAN, true);
	fULBan = ID_UNBAN;

	fULBanMenu->insertItem(tr("1 minute"), ID_BAN1);
	fULBanMenu->insertItem(tr("2 minutes"), ID_BAN2);
	fULBanMenu->insertItem(tr("5 minutes"), ID_BAN5);
	fULBanMenu->insertItem(tr("10 minutes"), ID_BAN10);
	fULBanMenu->insertItem(tr("15 minutes"), ID_BAN15);
	fULBanMenu->insertItem(tr("30 minutes"), ID_BAN30);
	fULBanMenu->insertItem(tr("1 hour"), ID_BAN1H);
	fULBanMenu->insertItem(tr("Infinite"), ID_BANINF);

	fULPopup->insertItem(tr("Move Up"), ID_MOVEUP);
	fULPopup->insertItem(tr("Move Down"), ID_MOVEDOWN);
	fULPopup->insertItem(tr("Ban IP"), ID_IGNORE);
	fULPopup->insertItem(tr("Clear Finished"), ID_CLEAR);
	fULPopup->insertItem(tr("Cancel"), ID_CANCEL);

	fULPopup->insertItem(tr("Throttle"), fULThrottleMenu, ID_THROTTLE);
	fULPopup->insertItem(tr("Block"), fULBanMenu, ID_BLOCK);

	connect(fULPopup, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fULThrottleMenu, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fULBanMenu, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fUploads, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(ULRightClicked(QListViewItem *, const QPoint &, int)));

}

WDownload::~WDownload()
{
	fLock.lock();
	PRINT("Number of downloads: %d\n", fDownloadList.size());
	WTIter it = fDownloadList.begin();
	while (it != fDownloadList.end())
	{
		// Put all files in resume list
		if ((*it).first)
		{
			int n = (*it).first->GetCurrentNum();

			if (n > -1) 
			{
				for (int i = n; i < (*it).first->GetNumFiles(); i++)
				{
					emit FileInterrupted((*it).first->GetFileName(i), (*it).first->GetRemoteUser());
				}
			}
			PRINT("Reseting download\n");
			(*it).first->Reset();
			PRINT("Deleting download\n");
			delete (*it).first;
		}
		PRINT("Deleting item\n");
		delete (*it).second;
		PRINT("Erasing pair\n");
		fDownloadList.erase(it);
		it = fDownloadList.begin();
	}
	fLock.unlock();

	fLock.lock();
	PRINT("Number of uploads: %d\n", fUploadList.size());
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
		PRINT("Erasing pair\n");
		fUploadList.erase(it);
		it = fUploadList.begin();
	}
	fLock.unlock();
	UpdateLoad();	// do this to set a load of 0
	emit Closed();
}

void
WDownload::AddDownload(QString * files, int32 filecount, QString remoteSessionID,
					   uint32 remotePort, QString remoteIP, uint64 remoteInstallID, bool firewalled, bool partial)
{
	WDownloadThread * nt = new WDownloadThread(this);
	CHECK_PTR(nt);
	nt->SetFile(files, filecount, remoteIP, remoteSessionID, fLocalSID, remotePort, firewalled, partial);
	
	WTPair p;
	p.first = nt;
	p.second = new WTransferItem(fDownloads, "", "", "", "", "", "", "", "", "");
	CHECK_PTR(p.second);

	

	if (!firewalled) // Check for valid ip and port if not firewalled
	{
		int x;

		// Can't use localhost to download!!!

		if (remoteIP == "127.0.0.1")	
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
	}

	if (GetNumDownloads() < gWin->fSettings->GetMaxDownloads())
	{
		nt->InitSession();
		nt->SetLocallyQueued(false);
	}
	else
	{
		nt->SetLocallyQueued(true);
		p.second->setText(WTransferItem::Status, tr("Locally Queued."));
	}
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
	CHECK_PTR(qFiles);
	QString tmp;
	int n = 0;
	while (!fQueue.IsEmpty())
	{
		fQueue.RemoveHead(tmp);
		if (
			(tmp != QString::null) && 
			(tmp != "")
			) 
		{
			qFiles[n++] = tmp;
		}
	}
	AddDownload(qFiles, n, user->GetUserID(), user->GetPort(), user->GetUserHostName(), user->GetInstallID(), user->GetFirewalled(), user->GetPartial());
}

void
WDownload::AddUpload(QString remoteIP, uint32 port)
{
	PRINT("WDownload::AddUpload(QString, uint32)\n");
	WUploadThread * ut = new WUploadThread(this);
	CHECK_PTR(ut);

	PRINT("Setting upload\n");
	ut->SetUpload(remoteIP, port, fSharedFiles);
	
	if (GetNumUploads() < gWin->fSettings->GetMaxUploads())
	{
		PRINT("Not queued\n");
		ut->SetLocallyQueued(false);
	}
	else
	{
		PRINT("Queued\n");
		ut->SetLocallyQueued(true);
	}

	PRINT("Init session\n");
	ut->InitSession();
	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "", "", "");
	CHECK_PTR(p.second);
	
	if (ut->IsLocallyQueued())
	{
		PRINT("IsQueued\n");
		p.second->setText(WTransferItem::Status, tr("Queued."));
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
	PRINT("WDownload::AddUpload(int, uint32, bool)\n");
	WUploadThread * ut = new WUploadThread(this);
	CHECK_PTR(ut);

	PRINT("Setting upload\n");
	ut->SetUpload(socket, remoteIP, fSharedFiles);
	
	if (GetNumUploads() < gWin->fSettings->GetMaxUploads())
	{
		PRINT("Not queued\n");
		ut->SetLocallyQueued(false);
	}
	else
	{
		PRINT("Queued\n");
		ut->SetLocallyQueued(true);
	}

	PRINT("Init session\n");
	ut->InitSession();

	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "", "", "");
	CHECK_PTR(p.second);

	if (ut->IsLocallyQueued())
	{
		PRINT("IsQueued\n");
		p.second->setText(WTransferItem::Status, tr("Queued."));
	}
	PRINT("Inserting\n");
	fLock.lock();
	fUploadList.insert(fUploadList.end(), p);
	fLock.unlock();
	UpdateLoad();
	UpdateULRatings();
}

void
WDownload::DequeueULSessions()
{
	if (gWin->fSettings->GetAutoClear())
	{
		ClearFinishedUL();
	}

	bool found = true;
	
	int numNotQueued = 0;
	int qr = 0;

	WTIter it;
	
	fLock.lock();
	for (it = fUploadList.begin(); it != fUploadList.end(); it++)
	{
		if ((*it).first)
		{
			if (
				((*it).first->IsLocallyQueued() == false) && 
				((*it).first->IsBlocked() == false) && 
				((*it).first->IsActive() == true) &&
				((*it).first->IsFinished() == false)
				)
				numNotQueued++;
		}

		if ((*it).second)
		{
			(*it).second->setText(WTransferItem::QR, QString::number(qr++));
		}
	}
	fLock.unlock();

	while (numNotQueued < gWin->fSettings->GetMaxUploads())
	{
		found = false;
		fLock.lock();
		for (it = fUploadList.begin(); it != fUploadList.end() && numNotQueued < gWin->fSettings->GetMaxUploads(); it++)
		{
			if ((*it).first)
			{
				if (
					((*it).first->IsLocallyQueued() == true) && 
					((*it).first->IsManuallyQueued() == false) &&
					((*it).first->IsFinished() == false)
					)
				{
					found = true;
					(*it).first->SetLocallyQueued(false);
					numNotQueued++;
				}
			}
		}
		fLock.unlock();

		if (!found)
			break;
	}
	UpdateLoad();
}

void
WDownload::DequeueDLSessions()
{
	if (gWin->fSettings->GetAutoClear())
	{
		ClearFinishedDL();
	}

	bool found = true;
	
	/*
	** OK, this is how I do queuing for downloads. Since the download count
	** WILL be greater than the amount of downloads allowed, we can't
	** dequeue based on this, right? So, run through the download list
	** and count the amount of downloads NOT queued. If this amount is less
	** than the allowed downloads, find a queued download, and unqueue it :)
	*/
	int numNotQueued = 0;
	int qr = 0;

	WTIter it;
	
	fLock.lock();
	for (it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		if ((*it).first)
		{
			if (
				((*it).first->IsLocallyQueued() == false) && 
				((*it).first->IsActive() == true)
				)
			{
				// not queued, not finished?
				numNotQueued++;
			}
		}

		if ((*it).second)
		{
			(*it).second->setText(WTransferItem::QR, QString::number(qr++));
		}
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
			if ((*it).first)
			{
				if (
					((*it).first->IsLocallyQueued() == true) && 
					((*it).first->IsManuallyQueued() == false) &&
					((*it).first->IsFinished() == false)
					)
				{
					found = true;
					(*it).first->SetLocallyQueued(false);
					((WDownloadThread *)((*it).first))->InitSession();
					numNotQueued++;
				}
			}
		}
		fLock.unlock();
		if (!found)
			break;
	}
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
		MessageRef msg = g->Msg();
		WGenericThread * gt = NULL;
		bool b;
		bool upload = false;
		WTransferItem * item = NULL;
		WTIter foundIt;
		
		if (!msg())
			return; // Invalid MessageRef!

		if (msg()->FindPointer("sender", (void **)&gt) != B_OK)
			return;	// failed! ouch!

		if (!gt)
			return;

		if (msg()->FindBool("download", &b) == B_OK)
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
		else if (msg()->FindBool("upload", &b) == B_OK)
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
		switch (msg()->what)
		{
		case WGenericEvent::Init:
			{
				PRINT("\tWGenericEvent::Init\n");
				const char * filename, * user;
				if (
					(msg()->FindString("file", &filename) == B_OK) && 
					(msg()->FindString("user", &user) == B_OK)
					)
				{
					item->setText(WTransferItem::Filename, QString::fromUtf8(filename));
					item->setText(WTransferItem::User, GetUserName(user));
					item->setText(WTransferItem::Index, FormatIndex(gt->GetCurrentNum(), gt->GetNumFiles()));
				}
				break;
			}
			
		case WGenericEvent::FileQueued:
			{
				PRINT("\tWGenericEvent::FileQueued\n");
				if (upload)
				{
					item->setText(WTransferItem::Status, tr("Queued."));
					item->setText(WTransferItem::Rate, "0.0");
					item->setText(WTransferItem::ETA, "");

				}
				else
				{
					item->setText(WTransferItem::Status, tr("Remotely Queued."));
					item->setText(WTransferItem::Rate, "0.0");
					item->setText(WTransferItem::ETA, "");
				}
				break;
			}
			
		case WGenericEvent::FileBlocked:
			{
				PRINT("\tWGenericEvent::FileBlocked\n");
				uint64 timeLeft = (uint64) -1;
				(void) msg()->FindInt64("timeleft", (int64 *) &timeLeft);
				if (timeLeft == -1)
				{
					item->setText(WTransferItem::Status, tr("Blocked."));
				}
				else
				{
					item->setText(WTransferItem::Status, tr("Blocked for %1 minute(s).").arg((int) (timeLeft/60000000)));
				}
				item->setText(WTransferItem::Rate, "0.0");
				item->setText(WTransferItem::ETA, "");
				break;
			}
			
		case WGenericEvent::ConnectBackRequest:
			{
				PRINT("\tWGenericEvent::ConnectBackRequest\n");
				MessageRef cb(GetMessageFromPool(NetClient::CONNECT_BACK_REQUEST));
				if (cb())
				{
					String session;
					int32 port;
					if (
						(msg()->FindInt32("port", &port) == B_OK) && 
						(msg()->FindString("session", session) == B_OK)
						)
					{
						item->setText(WTransferItem::Status, tr("Waiting for incoming connection..."));
						String tostr = "/*/";
						tostr += session.Cstr();
						tostr += "/beshare";
						cb()->AddString(PR_NAME_KEYS, tostr);
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
				PRINT("\tWGenericEvent::FileHashing\n");
				item->setText(WTransferItem::Status, tr("Examining for resume..."));
				break;
			}
			
		case WGenericEvent::ConnectInProgress:
			{
				PRINT("\tWGenericEvent::ConnectInProgress\n");
				item->setText(WTransferItem::Status, tr("Connecting..."));
				break;
			}
			
		case WGenericEvent::ConnectFailed:
			{
				PRINT("\tWGenericEvent::ConnectFailed\n");
				String why, mFile;
				msg()->FindString("why", why);
				item->setText(WTransferItem::Status, tr("Connect failed: %1").arg(why.Cstr()));
				gt->SetFinished(true);
				gt->SetActive(false);
				if (upload)
				{
					DequeueULSessions();

				}
				else
				{
					for (int n = gt->GetCurrentNum(); n < gt->GetNumFiles(); n++)
					{
						QString qFile = gt->GetFileName(n);
						emit FileFailed(qFile, gt->GetRemoteUser());
					}

					DequeueDLSessions();
				}
				break;
			}
			
		case WGenericEvent::Connected:
			{
				PRINT("\tWGenericEvent::Connected\n");
				item->setText(WTransferItem::Status, tr("Negotiating..."));
				break;
			}
			
		case WGenericEvent::Disconnected:
			{
				PRINT("\tWGenericEvent::Disconnected\n");
				if (item->text(0) != tr("Finished."))
					item->setText(WTransferItem::Status, tr("Disconnected."));
				if (upload)
				{
					bool f;
					if (msg()->FindBool("failed", &f) == B_OK)
					{
						// "failed" == true only, if the transfer has failed
						if (!f)
						{
							item->setText(WTransferItem::Status, tr("Finished."));
							item->setText(WTransferItem::ETA, "");
						}
					}
					gt->SetFinished(true);
					gt->SetActive(false);
					gt->Reset();
					DequeueULSessions();
				}
				else
				{
					if (gt->IsManuallyQueued())
					{
						item->setText(WTransferItem::Status, tr("Manually Queued."));
						item->setText(WTransferItem::Rate, "0.0");
						item->setText(WTransferItem::ETA, "");
					}
					else
					{
						gt->SetFinished(true);
						gt->SetActive(false);
						// emit FileFailed signal, so we can record the filename and remote username for resuming later
						bool f;
						if (msg()->FindBool("failed", &f) == B_OK)
						{
							// "failed" == true only, if the transfer has failed
							if (f)
							{
								for (int n = gt->GetCurrentNum(); n < gt->GetNumFiles(); n++)
								{
									QString qFile = gt->GetFileName(n);
									emit FileFailed(qFile, gt->GetRemoteUser());
								}
							}
							else
							{
								item->setText(WTransferItem::Status, tr("Finished."));
								item->setText(WTransferItem::ETA, "");
							}
						}
					}
					gt->Reset();
					DequeueDLSessions();
				}
				break;
			}
			
		case WGenericEvent::FileDone:
			{
				PRINT("\tWGenericEvent::FileDone\n");
				bool d;
				if (msg()->FindBool("done", &d) == B_OK)
				{
					PRINT("\tFound done\n");
					if (!upload)
					{
						PRINT("\tIs download\n");
						DequeueDLSessions();
					}
					item->setText(WTransferItem::Status, tr("Finished."));
					item->setText(WTransferItem::ETA, "");
				}
				else
				{
					if (!upload)
					{
						item->setText(WTransferItem::Status, tr("Waiting..."));
						item->setText(WTransferItem::Filename, tr("Waiting for next file..."));
						item->setText(WTransferItem::Received, "");
						item->setText(WTransferItem::Total, "");
						item->setText(WTransferItem::Rate, "0.0");
						item->setText(WTransferItem::ETA, "");
						//item->setText(WTransferItem::User, "");	<- don't erase the user name
					}
				}
				break;
			}
			
		case WGenericEvent::FileFailed:
			{
				PRINT("\tWGenericEvent::FileFailed\n");
				// not used...
				break;
			}
			
		case WGenericEvent::FileStarted:
			{
				PRINT("\tWGenericEvent::FileStarted\n");
				String file;
				uint64 start;
				uint64 size;
				String user;
				
				if (
					(msg()->FindString("file", file) == B_OK) && 
					(msg()->FindInt64("start", (int64 *)&start) == B_OK) &&
					(msg()->FindInt64("size", (int64 *)&size) == B_OK) && 
					(msg()->FindString("user", user) == B_OK)
					)
				{
					QString uname = GetUserName(QString::fromUtf8(user.Cstr()));
					PRINT("USER ID  : %s\n", user.Cstr());
					PRINT("USER NAME: %S\n", qStringToWideChar(uname));
					if (strcmp(uname.latin1(), user.Cstr()) == 0) // No user name?
					{
						uname = gt->GetRemoteIP();
					}
					item->setText(WTransferItem::Status, tr("Waiting for stream..."));
					item->setText(WTransferItem::Filename, QString::fromUtf8( file.Cstr() ) ); // <postmaster@raasu.org> 20021023 -- Unicode fix
					// rec, total, rate
					item->setText(WTransferItem::Received, QString::number((int) start));
					item->setText(WTransferItem::Total, QString::number((int) size));
					item->setText(WTransferItem::Rate, "0.0");
					item->setText(WTransferItem::ETA, "");
					item->setText(WTransferItem::User, uname);
					item->setText(WTransferItem::Index, FormatIndex(gt->GetCurrentNum(), gt->GetNumFiles()));
					
					if (upload)
					{
						if (gWin->fSettings->GetUploads())
						{
							gWin->PrintSystem(tr("%1 is downloading %2.").arg(uname).arg(QString::fromUtf8(file.Cstr())));
						}
					}
				}
				gt->fLastData.start();
				break;
			}
			
		case WGenericEvent::UpdateUI:
			{
				PRINT("\tWGenericEvent::UpdateUI\n");
				const char * id;
				if (msg()->FindString("id", &id) == B_OK)
				{
					item->setText(WTransferItem::User, GetUserName(id));
				}
				break;
			}
			
		case WGenericEvent::FileError:
			{
				PRINT("\tWGenericEvent::FileError\n");
				String why;
				String file;
				msg()->FindString("why", why);
				if (msg()->FindString("file", file) == B_OK)
					item->setText(WTransferItem::Filename, QString::fromUtf8(file.Cstr()) );
				item->setText(WTransferItem::Status, tr("Error: %1").arg(why.Cstr()));
				item->setText(WTransferItem::Index, FormatIndex(gt->GetCurrentNum(), gt->GetNumFiles()));
				PRINT("WGenericEvent::FileError: File %s",file.Cstr()); // <postmaster@raasu.org> 20021023 -- Add debug message
				break;
			}
			
		case WGenericEvent::FileDataReceived:
			{
				PRINT("\tWGenericEvent::FileDataReceived\n");
				uint64 offset, size;
				bool done;
				String mFile;
				uint32 got;
				
				if (
					(msg()->FindInt64("offset", (int64 *)&offset) == B_OK) && 
					(msg()->FindInt64("size", (int64 *)&size) == B_OK)
					)
				{
					if (msg()->FindInt32("got", (int32 *)&got) == B_OK)	// a download ("got")
					{
						PRINT("\tOffset: %d\n", offset);
						PRINT("\tSize  : %d\n", size);
						PRINT("\tGot   : %d\n", got);
						gWin->UpdateReceiveStats(got);
						
						double secs = 0.0f;
						
						if (gt->fLastData.elapsed() > 0)
						{
							secs = (double)((double)gt->fLastData.elapsed() / 1000.0f);
						}

						double gotk = 0.0f;
						
						if (got > 0)
						{
							gotk = (double)((double)got / 1024.0f);
						}

						double kps = 0.0f;
						
						if ( (gotk > 0) && (secs > 0) )
						{
							kps = gotk / secs;
						}
						
						item->setText(WTransferItem::Status, tr("Downloading: [%1%]").arg(gt->ComputePercentString(offset, size)));
						item->setText(WTransferItem::Received, QString::number((int) offset));
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

						if (msg()->FindBool("done", &done) == B_OK)
						{
 							item->setText(WTransferItem::Status, tr("File finished."));
							item->setText(WTransferItem::ETA, "");

							if (msg()->FindString("file", mFile) == B_OK)
								gWin->PrintSystem( tr("Finished downloading %2 from %1.").arg(gt->GetRemoteUser()).arg( QString::fromUtf8(mFile.Cstr()) ) , false);
						}
						PRINT("\tWGenericEvent::FileDataReceived OK\n");
					}
					else if (msg()->FindInt32("sent", (int32 *)&got) == B_OK)	// an upload ("sent")
					{
						PRINT("\tOffset: %d\n", offset);
						PRINT("\tSize  : %d\n", size);
						PRINT("\tSent  : %d\n", got);
						gWin->UpdateTransmitStats(got);
						
						double secs = 0.0f;
						
						if (gt->fLastData.elapsed() > 0)
						{
							secs = (double)((double)gt->fLastData.elapsed() / 1000.0f);
						}

						double gotk = 0.0f;
						
						if (got > 0)
						{
							gotk = (double)((double)got / 1024.0f);
						}

						double kps = 0.0f;
						
						if ( (gotk > 0) && (secs > 0) )
						{
							kps = gotk / secs;
						}
												
						item->setText(WTransferItem::Status, tr("Uploading: [%1%]").arg(gt->ComputePercentString(offset, size)));
						item->setText(WTransferItem::Received, QString::number((int) offset));
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
						
						item->setText(WTransferItem::Rate, QString::number(gcr*1024.0f));
						
						if (msg()->FindBool("done", &done) == B_OK)
						{
							item->setText(WTransferItem::Status, tr("Finished."));
							item->setText(WTransferItem::ETA, "");

							if (msg()->FindString("file", mFile) == B_OK)
								gWin->PrintSystem( tr("%1 has finished downloading %2.").arg(gt->GetRemoteUser()).arg( QString::fromUtf8(mFile.Cstr()) ) , false);
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
WDownload::KillLocalQueues()
{
	fLock.lock();
	WTIter it = fDownloadList.begin();
	while (it != fDownloadList.end())
	{
		if ((*it).first)
		{
			if ((*it).first->IsLocallyQueued())	// found queued item?
			{
				// Put all files in resume list
				int n = (*it).first->GetCurrentNum();
				
				if (n > -1) 
				{
					for (int i = n; i < (*it).first->GetNumFiles(); i++)
					{
						emit FileInterrupted((*it).first->GetFileName(i), (*it).first->GetRemoteUser());
					}
				}
				
				// free it
				delete (*it).second;
				(*it).first->Reset();
				delete (*it).first;
				WTIter nextIt = it;
				nextIt++;
				fDownloadList.erase(it);
				it = nextIt;
			}
			else
				it++;
		}
	}
	fLock.unlock();
}

QString
WDownload::GetUserName(QString sid)
{
	WUserRef uref = gWin->fNetClient->FindUser(sid);
	QString ret = sid;
	if (uref())
		ret = tr("%1 (%2)").arg(uref()->GetUserName()).arg(sid);
	return ret;
}

void
WDownload::UpdateLoad()
{
	gWin->fNetClient->SetLoad(GetUploadQueue(), gWin->fSettings->GetMaxUploads());
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
			if ((*it).first)
			{
				if ((*it).first->GetRemoteID() == sid)
				{
					// Only block transfers that are active
					if (
						((*it).first->IsBlocked() == false) &&
						((*it).first->IsActive() == true)
						)
					{
						(*it).first->SetBlocked(true);
					}
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
	WTIter i;
	WGenericThread * gt = NULL;
	bool findRet;
	
	findRet = FindItem(fDownloadList, i, fDLPopupItem);
	
	if (!findRet)
	{
		PRINT("Download item not found!");
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
//				fNumDownloads++;
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
		
	case ID_CLEAR:
		{
			ClearFinishedDL();
			break;
		}
		
	case ID_CANCEL:
		{
			fLock.lock();
			// found our item, stop it
			(*i).second->setText(WTransferItem::Status, tr("Canceled."));
			gt->SetFinished(true);
			gt->SetActive(false);
			gt->Reset();
			fLock.unlock();
			DequeueDLSessions();
			break;
		}
		
	case ID_NO_LIMIT:
		{
			gt->SetRate(0);
			break;
		}
		
	case ID_64:
		{
			gt->SetRate(64);
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
		
	case ID_6KB:
		{
			gt->SetRate(6 * 1024);
			break;
		}

	case ID_8KB:
		{
			gt->SetRate(8 * 1024);
			break;
		}

	case ID_10KB:
		{
			gt->SetRate(10 * 1024);
			break;
		}

	case ID_12KB:
		{
			gt->SetRate(12 * 1024);
			break;
		}

	case ID_14KB:
		{
			gt->SetRate(14 * 1024);
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
	default:
		{
			// Handle "Run..." items here...

			if (id & ID_RUN)
			{
				int n = id - ID_RUN - 1;
				RunCommand(gt->GetLocalFileName(n));
			}
		}
			
	}
	UpdateDLRatings();
}

void
WDownload::ULPopupActivated(int id)
{
	WTIter i;
	WGenericThread * gt = NULL;
	bool findRet;
	
	findRet = FindItem(fUploadList, i, fULPopupItem);
	
	if (!findRet)
	{
		PRINT("Upload item not found!");
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
		
	case ID_CLEAR:
		{
			ClearFinishedUL();
			break;
		}	

	case ID_CANCEL:
		{
			fLock.lock();
			// found our item, stop it
			(*i).second->setText(WTransferItem::Status, tr("Canceled."));
			gt->SetFinished(true);
			gt->SetActive(false);
			gt->Reset();
			fLock.unlock();
			DequeueULSessions();
			break;
		}
				
	case ID_NO_LIMIT:
		{
			gt->SetRate(0);
			break;
		}
		
	case ID_64:
		{
			gt->SetRate(64);
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
		
	case ID_6KB:
		{
			gt->SetRate(6 * 1024);
			break;
		}

	case ID_8KB:
		{
			gt->SetRate(8 * 1024);
			break;
		}
		
	case ID_10KB:
		{
			gt->SetRate(10 * 1024);
			break;
		}

	case ID_12KB:
		{
			gt->SetRate(12 * 1024);
			break;
		}

	case ID_14KB:
		{
			gt->SetRate(14 * 1024);
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
			gt->SetBlocked(true, 60000000);
			break;
		}
		
	case ID_BAN2:
		{
			gt->SetBlocked(true, 120000000);
			break;
		}
		
	case ID_BAN5:
		{
			gt->SetBlocked(true, 300000000);
			break;
		}
		
	case ID_BAN10:
		{
			gt->SetBlocked(true, 600000000);
			break;
		}
		
	case ID_BAN15:
		{
			gt->SetBlocked(true, 900000000);
			break;
		}
		
	case ID_BAN30:
		{
			gt->SetBlocked(true, 1800000000);
			break;
		}
		
	case ID_BAN1H:
		{
#ifdef __LINUX__
			gt->SetBlocked(true, 3600000000LL);
#else
			gt->SetBlocked(true, 3600000000UL);
#endif
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
	UpdateULRatings();
}

void
WDownload::ULRightClicked(QListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fULPopupItem = item;

		WTIter iter;
		if (FindItem(fUploadList, iter, item))
		{
			if ( !(*iter).first )
			{
				PRINT("Generic Thread is invalid!");
				return;
			}

			// Disable items not applicable to current thread state

			// Not Locally Queued

			if ((*iter).first->IsLocallyQueued() == false)
			{
				fULPopup->setItemEnabled(ID_MOVEUP, false);
				fULPopup->setItemEnabled(ID_MOVEDOWN, false);
			}
			else
			{
				fULPopup->setItemEnabled(ID_MOVEUP, true);
				fULPopup->setItemEnabled(ID_MOVEDOWN, true);
			}

			// Already finished

			if ((*iter).first->IsFinished() == true)
			{
				fULPopup->setItemEnabled(ID_QUEUE, false);
				fULPopup->setItemEnabled(ID_BLOCK, false);
				fULPopup->setItemEnabled(ID_THROTTLE, false);
			}
			else
			{
				fULPopup->setItemEnabled(ID_QUEUE, true);
				fULPopup->setItemEnabled(ID_BLOCK, true);
				fULPopup->setItemEnabled(ID_THROTTLE, true);
			}

			fULPopup->setItemChecked(ID_QUEUE, (*iter).first->IsLocallyQueued());
			fULPopup->setItemChecked(ID_IGNORE, gWin->IsIgnoredIP( (*iter).first->GetRemoteIP() ));
			int fNewRate = (*iter).first->GetRate();
			fULThrottleMenu->setItemChecked(fULThrottle, false);
			
			switch (fNewRate)
			{
			case 0: 	
				{
					fULThrottle = ID_NO_LIMIT;	
					break;
				}

			case 64:
				{
					fULThrottle = ID_64;
					break;
				}

			case 128:	
				{
					fULThrottle = ID_128;		
					break;
				}

			case 256:	
				{
					fULThrottle = ID_256;		
					break;
				}

			case 512:	
				{
					fULThrottle = ID_512;		
					break;
				}

			case 1024:	
				{
					fULThrottle = ID_1KB;		
					break;
				}

			case 2 * 1024:	
				{
					fULThrottle = ID_2KB;	
					break;
				}

			case 4 * 1024:	
				{
					fULThrottle = ID_4KB;	
					break;
				}

			case 6 * 1024:
				{
					fULThrottle = ID_6KB;
					break;
				}

			case 8 * 1024:	
				{
					fULThrottle = ID_8KB;	
					break;
				}

			case 10 * 1024:
				{
					fULThrottle = ID_10KB;
					break;
				}

			case 12 * 1024:
				{
					fULThrottle = ID_12KB;
					break;
				}

			case 14 * 1024:
				{
					fULThrottle = ID_14KB;
					break;
				}

			case 16 * 1024:	
				{
					fULThrottle = ID_16KB; 
					break;
				}

			case 32 * 1024: 
				{
					fULThrottle = ID_32KB;	
					break;
				}

			case 64 * 1024:	
				{
					fULThrottle = ID_64KB;	
					break;
				}

			case 128 * 1024:	
				{
					fULThrottle = ID_128KB; 
					break;
				}

			case 256 * 1024:	
				{
					fULThrottle = ID_256KB; 
					break;
				}

			case 512 * 1024:	
				{
					fULThrottle = ID_512KB;	
					break;
				}

			case 1048576:		
				{
					fULThrottle = ID_1MB;	
					break;
				}

			case 2 * 1048576:	
				{
					fULThrottle = ID_2MB;	
					break;
				}

			case 4 * 1048576:	
				{
					fULThrottle = ID_4MB;	
					break;
				}

			case 8 * 1048576:	
				{
					fULThrottle = ID_8MB;	
					break;
				}

			case 16 * 1048576:	
				{
					fULThrottle = ID_16MB;	
					break;
				}

			case 32 * 1048576:	
				{
					fULThrottle = ID_32MB; 
					break;
				}
			}
			
			fULThrottleMenu->setItemChecked(fULThrottle, true);
			
			int fNewBan = (*iter).first->GetBanTime();
			fULBanMenu->setItemChecked(fULBan, false);
			
			switch (fNewBan)
			{
			case -1:
				{
					fULBan = ID_BANINF;
					break;
				}
			case 0:
				{
					fULBan = ID_UNBAN;
					break;
				}
			case 1:
				{
					fULBan = ID_BAN1;
					break;
				}
			case 2:
				{
					fULBan = ID_BAN2;
					break;
				}
			case 5:
				{
					fULBan = ID_BAN5;
					break;
				}
			case 10:
				{
					fULBan = ID_BAN10;
					break;
				}
			case 15:
				{
					fULBan = ID_BAN15;
					break;
				}
			case 30:
				{
					fULBan = ID_BAN30;
					break;
				}
			case 60:
				{
					fULBan = ID_BAN1H;
					break;
				}
			}
			fULBanMenu->setItemChecked(fULBan, true);
			fULPopup->popup(p);
		}
	}
}

void
WDownload::DLRightClicked(QListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fDLPopupItem = item;

		WTIter iter;
		if (FindItem(fDownloadList, iter, item))
		{
			if ( !(*iter).first )
			{
				PRINT("Generic Thread is invalid!");
				return;
			}

			// Disable items not applicable to current thread state

			// Not Locally Queued

			if ((*iter).first->IsLocallyQueued() == false)
			{
				fDLPopup->setItemEnabled(ID_MOVEUP, false);
				fDLPopup->setItemEnabled(ID_MOVEDOWN, false);
			}
			else
			{
				fDLPopup->setItemEnabled(ID_MOVEUP, true);
				fDLPopup->setItemEnabled(ID_MOVEDOWN, true);
			}

			// Already finished

			if ((*iter).first->IsFinished() == true)
			{
				fDLPopup->setItemEnabled(ID_THROTTLE, false);
				fDLPopup->setItemEnabled(ID_QUEUE, false);
				fDLPopup->setItemEnabled(ID_RUN, true);
			}
			else
			{
				fDLPopup->setItemEnabled(ID_THROTTLE, true);
				fDLPopup->setItemEnabled(ID_QUEUE, true);
				fDLPopup->setItemEnabled(ID_RUN, false);
			}

			fDLPopup->setItemChecked(ID_QUEUE, (*iter).first->IsLocallyQueued());
			int fNewRate = (*iter).first->GetRate();
			fDLThrottleMenu->setItemChecked(fDLThrottle, false);
			switch (fNewRate)
			{
			case 0: 	
				{
					fDLThrottle = ID_NO_LIMIT;	
					break;
				}

			case 64:
				{
					fDLThrottle = ID_64;
					break;
				}

			case 128:	
				{
					fDLThrottle = ID_128;		
					break;
				}

			case 256:	
				{
					fDLThrottle = ID_256;		
					break;
				}

			case 512:	
				{
					fDLThrottle = ID_512;		
					break;
				}

			case 1024:	
				{
					fDLThrottle = ID_1KB;		
					break;
				}

			case 2 * 1024:	
				{
					fDLThrottle = ID_2KB;	
					break;
				}

			case 4 * 1024:	
				{
					fDLThrottle = ID_4KB;	
					break;
				}

			case 6 * 1024:
				{
					fDLThrottle = ID_6KB;
					break;
				}

			case 8 * 1024:	
				{
					fDLThrottle = ID_8KB;	
					break;
				}

			case 10 * 1024:
				{
					fDLThrottle = ID_10KB;
					break;
				}

			case 12 * 1024:
				{
					fDLThrottle = ID_12KB;
					break;
				}

			case 14 * 1024:
				{
					fDLThrottle = ID_14KB;
					break;
				}

			case 16 * 1024:	
				{
					fDLThrottle = ID_16KB; 
					break;
				}

			case 32 * 1024: 
				{
					fDLThrottle = ID_32KB;	
					break;
				}

			case 64 * 1024:	
				{
					fDLThrottle = ID_64KB;	
					break;
				}

			case 128 * 1024:	
				{
					fDLThrottle = ID_128KB; 
					break;
				}

			case 256 * 1024:	
				{
					fDLThrottle = ID_256KB; 
					break;
				}

			case 512 * 1024:	
				{
					fDLThrottle = ID_512KB;	
					break;
				}

			case 1048576:		
				{
					fDLThrottle = ID_1MB;	
					break;
				}

			case 2 * 1048576:	
				{
					fDLThrottle = ID_2MB;	
					break;
				}

			case 4 * 1048576:	
				{
					fDLThrottle = ID_4MB;	
					break;
				}

			case 8 * 1048576:	
				{
					fDLThrottle = ID_8MB;	
					break;
				}

			case 16 * 1048576:	
				{
					fDLThrottle = ID_16MB;	
					break;
				}

			case 32 * 1048576:	
				{
					fDLThrottle = ID_32MB; 
					break;
				}
			}
			fDLThrottleMenu->setItemChecked(fDLThrottle, true);

			// Add "Run..." items...

			fDLRunMenu->clear();
			int n = (*iter).first->GetNumFiles();
			for (int r = 0; r < n; r++)
			{
				if ((*iter).first->GetLocalFileName(r) != QString::null)
					fDLRunMenu->insertItem((*iter).first->GetLocalFileName(r), ID_RUN+1+r);
			}
			fDLPopup->popup(p);
		}
	}
}

bool
WDownload::FindItem(WTList & lst, WTIter & ret, QListViewItem * s)
{
	bool success = false;
	fLock.lock();
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
	fLock.unlock();
	if (!success)
	{
		PRINT("WDownload::FindItem() : Item not found!");
	}
	return success;
}

void
WDownload::MoveUp(WTList & lst, WTIter & iter)
{
	fLock.lock();
	WTIter iter2 = iter;
	WTPair wp = (*iter);
	fLock.unlock();

	if (iter == lst.begin())
		return;

	fLock.lock();
	while (1) 
	{
		iter2--;
		if ((*iter2).first->IsLocallyQueued())
			break;
		if (iter2 == lst.begin())
			break;
	}
	WTPair wp2 = (*iter2);
	lst.erase(iter);
	iter = lst.insert(iter2, wp);
	fLock.unlock();
}

void
WDownload::MoveDown(WTList & lst, WTIter & iter)
{
	fLock.lock();
	WTIter iter2 = iter;
	WTPair wp = (*iter);
	fLock.unlock();

	if (iter == lst.end())
		return;

	fLock.lock();
	while (1)
	{
		iter2++;
		if (iter2 == lst.end())
			break;
		if ((*iter2).first->IsLocallyQueued())
			break;
	}

	if (iter2 != lst.end())
		iter2++;

	WTPair wp2 = (*iter2);
	lst.erase(iter);
	iter = lst.insert(iter2, wp);
	fLock.unlock();
}

void
WDownload::UpdateULRatings()
{
	PRINT("\tWDownload::UpdateULRatings\n");
	if ( fUploadList.empty() )
		return;

	int qr = 0;
	fLock.lock();
	for (WTIter it = fUploadList.begin(); it != fUploadList.end(); it++)
	{
		if ((*it).second)
		{
			PRINT("Item %d: %S\n", qr, qStringToWideChar( (*it).second->text(WTransferItem::Filename) ) );
			(*it).second->setText(WTransferItem::QR, QString::number(qr++));
		}
	}
	fUploads->triggerUpdate();
	fLock.unlock();
}

void
WDownload::UpdateDLRatings()
{
	PRINT("\tWDownload::UpdateDLRatings\n");
	if ( fDownloadList.empty() )
		return;

	int qr = 0;
	fLock.lock();
	for (WTIter it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		if ((*it).second)
		{
			(*it).second->setText(WTransferItem::QR, QString::number(qr++));
		}
	}
	fDownloads->triggerUpdate();
	fLock.unlock();
}

void
WDownload::TransferCallBackRejected(QString qFrom, int64 timeLeft, uint32 port)
{
	fLock.lock();
	for (WTIter it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		if ((*it).first)
		{
			if (((*it).first->IsActive() == false) &&
				((*it).first->GetRemoteUser() == qFrom) && 
				((*it).first->GetRemotePort() == port)
				)
			{
				(*it).first->SetBlocked(true, timeLeft);
				break;
			}
		}
	}
	fLock.unlock();
}

// Clear all finished downloads from listview
void
WDownload::ClearFinishedDL()
{
	fLock.lock();
	WTIter it = fDownloadList.begin();
	while (it != fDownloadList.end())
	{
		if (
			((*it).first->IsFinished() == true)
			)
		{
			// found finished item, erase it
			if ((*it).first)
				(*it).first->Reset();
			delete (*it).first;
			delete (*it).second;
			fDownloadList.erase(it);
			// start from beginning
			it = fDownloadList.begin();
		}
		else
		{
			it++;
		}
	}
	fLock.unlock();
}

// Clear all finished uploads from listview
void
WDownload::ClearFinishedUL()
{
	fLock.lock();
	WTIter it = fUploadList.begin();
	while (it != fUploadList.end())
	{
		if (
			((*it).first->IsFinished() == true)
			)
		{
			// found finished item, erase it
			if ((*it).first)
				(*it).first->Reset();
			delete (*it).first;
			delete (*it).second;
			fUploadList.erase(it);
			// start from beginning
			it = fUploadList.begin();
		}
		else
		{
			it++;
		}
	}
	fLock.unlock();
}

int
WDownload::GetNumDownloads()
{
	PRINT("\tWDownload::GetNumDownloads\n");
	int n = 0;
	if ( fDownloadList.empty() )
		return n;

	fLock.lock();
	for (WTIter it = fDownloadList.begin(); it != fDownloadList.end(); it++)
	{
		if ((*it).first)
		{
			if (
				((*it).first->IsLocallyQueued() == false) &&
				((*it).first->IsRemotelyQueued() == false) &&
				((*it).first->IsBlocked() == false) &&
				((*it).first->IsFinished() == false)
				)
			{
				n++;
			}
		}
	}
	fLock.unlock();
	return n;
}

int
WDownload::GetNumUploads()
{
	PRINT("\tWDownload::GetNumUploads\n");
	int n = 0;
	if ( fUploadList.empty() )
		return n;

	fLock.lock();
	for (WTIter it = fUploadList.begin(); it != fUploadList.end(); it++)
	{
		if ((*it).first)
		{
			if (
				((*it).first->IsLocallyQueued() == false) &&
				((*it).first->IsBlocked() == false) &&
				((*it).first->IsFinished() == false)
				)
			{
				n++;
			}
		}
	}
	fLock.unlock();
	return n;
}

int
WDownload::GetUploadQueue()
{
	PRINT("\tWDownload::GetUploadQueue\n");
	int n = 0;
	if ( fUploadList.empty() )
		return n;

	fLock.lock();
	for (WTIter it = fUploadList.begin(); it != fUploadList.end(); it++)
	{
		if ((*it).first)
		{
			if (
				((*it).first->IsFinished() == false)
				)
			{
				n++;
			}
		}
	}
	fLock.unlock();
	return n;
}

void
WDownload::SetLocalID(QString sid)
{
	fLocalSID = sid;
}

