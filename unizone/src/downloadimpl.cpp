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
#include "gotourl.h"

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
	
	fCancelU = new QPushButton(fBoxU);
	CHECK_PTR(fCancelU);
	fCancelU->setText(tr(MSG_CANCEL));
	connect(fCancelU, SIGNAL(clicked()), this, SLOT(CancelUL()));

	connect(gWin->fNetClient, SIGNAL(UserDisconnected(QString, QString)), this,
			SLOT(UserDisconnected(QString, QString)));

	
	setCaption(tr(MSG_TX_CAPTION));
	fNumUploads = fNumDownloads = 0;

	fDLPopup = new QPopupMenu(this, "Download Popup");
	CHECK_PTR(fDLPopup);

	// Create the download popup menu
	fDLPopup->insertItem(tr(MSG_TX_QUEUE), ID_QUEUE);
	
	// Create throttle sub menu
	fDLThrottleMenu = new QPopupMenu(fDLPopup, "Throttle Popup");
	CHECK_PTR(fDLThrottleMenu);
	
	fDLThrottleMenu->insertItem(tr( MSG_NO_LIMIT ), ID_NO_LIMIT);
	
	// Set initial throttle to none
	fDLThrottleMenu->setItemChecked(ID_NO_LIMIT, true);
	fDLThrottle = ID_NO_LIMIT;

	fDLThrottleMenu->insertItem(tr( "64 " MSG_BYTES ), ID_64);
	fDLThrottleMenu->insertItem(tr( "128 " MSG_BYTES ), ID_128);
	fDLThrottleMenu->insertItem(tr( "256 " MSG_BYTES ), ID_256);
	fDLThrottleMenu->insertItem(tr( "512 " MSG_BYTES ), ID_512);
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

	fDLPopup->insertItem(tr(MSG_TX_THROTTLE), fDLThrottleMenu, ID_THROTTLE);
	fDLPopup->insertItem(tr("Run..."), fDLRunMenu, ID_RUN);

	fDLPopup->insertItem(tr(MSG_TX_MOVEUP), ID_MOVEUP);
	fDLPopup->insertItem(tr(MSG_TX_MOVEDOWN), ID_MOVEDOWN);

	fDLPopup->insertItem(tr("Clear Finished"), ID_CLEAR);

	connect(fDLPopup, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDLThrottleMenu, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDLRunMenu, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDownloads, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(DLRightClicked(QListViewItem *, const QPoint &, int)));

	fULPopup = new QPopupMenu(this, "Upload Popup");
	CHECK_PTR(fULPopup);

	// Create the upload popup menu
	fULPopup->insertItem(tr(MSG_TX_QUEUE), ID_QUEUE);
	
	// Create throttle sub menu
	fULThrottleMenu = new QPopupMenu(fULPopup, "Throttle Popup");
	CHECK_PTR(fULThrottleMenu);
	
	fULThrottleMenu->insertItem(tr( MSG_NO_LIMIT ), ID_NO_LIMIT);
	
	// Set initial throttle to none
	fULThrottleMenu->setItemChecked(ID_NO_LIMIT, true);
	fULThrottle = ID_NO_LIMIT;

	fULThrottleMenu->insertItem(tr( "64 " MSG_BYTES ), ID_64);
	fULThrottleMenu->insertItem(tr( "128 " MSG_BYTES ), ID_128);
	fULThrottleMenu->insertItem(tr( "256 " MSG_BYTES ), ID_256);
	fULThrottleMenu->insertItem(tr( "512 " MSG_BYTES ), ID_512);
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

	fULBanMenu->insertItem(tr("1 " MSG_MINUTE), ID_BAN1);
	fULBanMenu->insertItem(tr("2 " MSG_MINUTES), ID_BAN2);
	fULBanMenu->insertItem(tr("5 " MSG_MINUTES), ID_BAN5);
	fULBanMenu->insertItem(tr("10 " MSG_MINUTES), ID_BAN10);
	fULBanMenu->insertItem(tr("15 " MSG_MINUTES), ID_BAN15);
	fULBanMenu->insertItem(tr("30 " MSG_MINUTES), ID_BAN30);
	fULBanMenu->insertItem(tr("1 " MSG_HOUR), ID_BAN1H);
	fULBanMenu->insertItem(tr("Infinite"), ID_BANINF);

	fULPopup->insertItem(tr(MSG_TX_MOVEUP), ID_MOVEUP);
	fULPopup->insertItem(tr(MSG_TX_MOVEDOWN), ID_MOVEDOWN);
	fULPopup->insertItem(tr("Ban IP"), ID_IGNORE);
	fULPopup->insertItem(tr("Clear Finished"), ID_CLEAR);

	fULPopup->insertItem(tr(MSG_TX_THROTTLE), fULThrottleMenu, ID_THROTTLE);
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

	if (GetNumDownloads() < gWin->fSettings->GetMaxDownloads())
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
	fNumUploads++;
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	PRINT("Init session\n");
	ut->InitSession();
	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "", "", "");
	CHECK_PTR(p.second);
	
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
	CHECK_PTR(ut);
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
	fNumUploads++;
	WASSERT(fNumUploads >= 0, "Upload count is negative!");
	ut->InitSession();

	WTPair p;
	p.first = ut;
	p.second = new WTransferItem(fUploads, "", "", "", "", "", "", "", "", "");
	CHECK_PTR(p.second);

	if (ut->IsLocallyQueued())
		p.second->setText(WTransferItem::Status, "Queued.");
	fLock.lock();
	fUploadList.insert(fUploadList.end(), p);
	fLock.unlock();
	UpdateLoad();
	UpdateULRatings();
}

void
WDownload::DequeueULSessions()
{
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
			(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
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
					((*it).first->IsManuallyQueued() == false)
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
			(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
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
					((*it).first->IsManuallyQueued() == false)
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
				PRINT("\tWGenericEvent::FileBlocked\n");
				uint64 timeLeft = (uint64) -1;
				(void) msg()->FindInt64("timeleft", (int64 *) &timeLeft);
				if (timeLeft == -1)
					item->setText(WTransferItem::Status, "Blocked.");
				else
					item->setText(WTransferItem::Status, tr("Blocked for %1 minutes.").arg((int) (timeLeft/1000000)));
				break;
			}
			
		case WGenericEvent::ConnectBackRequest:
			{
				PRINT("\tWGenericEvent::ConnectBackRequest\n");
				MessageRef cb(new Message(NetClient::CONNECT_BACK_REQUEST), NULL);
				if (cb())
				{
					String session;
					int32 port;
					if (
						(msg()->FindInt32("port", &port) == B_OK) && 
						(msg()->FindString("session", session) == B_OK)
						)
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
				PRINT("\tWGenericEvent::FileHashing\n");
				item->setText(WTransferItem::Status, "Examining for resume...");
				break;
			}
			
		case WGenericEvent::ConnectInProgress:
			{
				PRINT("\tWGenericEvent::ConnectInProgress\n");
				item->setText(WTransferItem::Status, "Connecting...");
				break;
			}
			
		case WGenericEvent::ConnectFailed:
			{
				PRINT("\tWGenericEvent::ConnectFailed\n");
				String why, mFile;
				//bool b;
				msg()->FindString("why", why);
				item->setText(WTransferItem::Status, tr("Connect failed: %1").arg(why.Cstr()));
				delete (*foundIt).second;
				if (upload)
				{
					DecreaseCount(gt, fNumUploads, false);
					fLock.lock();
					fUploadList.erase(foundIt);
					fLock.unlock();
					WASSERT(fNumUploads >= 0, "Upload count is negative!");
					delete gt;
					gt = NULL; // <postmaster@raasu.org> 20021027
					DequeueULSessions();

				}
				else
				{
					DecreaseCount(gt, fNumDownloads, false);
					fLock.lock();
					fDownloadList.erase(foundIt);
					fLock.unlock();
					WASSERT(fNumDownloads >= 0, "Download count is negative!");
					delete gt;
					gt = NULL; // <postmaster@raasu.org> 20021027
					DequeueDLSessions();
				}
				break;
			}
			
		case WGenericEvent::Connected:
			{
				PRINT("\tWGenericEvent::Connected\n");
				item->setText(WTransferItem::Status, "Negotiating...");
				break;
			}
			
		case WGenericEvent::Disconnected:
			{
				PRINT("\tWGenericEvent::Disconnected\n");
				if (item->text(0) != QString("Finished."))
					item->setText(WTransferItem::Status, "Disconnected.");
				if (upload)
				{
					bool f;
					if (msg()->FindBool("failed", &f) == B_OK)
					{
						// "failed" == true only, if the transfer has failed
						if (!f)
						{
							item->setText(WTransferItem::Status, "Finished.");
							item->setText(WTransferItem::ETA, "");
						}
					}
					//delete (*foundIt).second;
					gt->SetFinished(true);
					DecreaseCount(gt, fNumUploads);
					WASSERT(fNumUploads >= 0, "Upload count is negative!");
					//fLock.lock();
					//fUploadList.erase(foundIt);
					//fLock.unlock();
					//delete gt;
					//gt = NULL; // <postmaster@raasu.org> 20030317
					DequeueULSessions();
				}
				else
				{
					// delete (*foundIt).second;
					if (gt->IsManuallyQueued())
						item->setText(WTransferItem::Status, "Manually Queued.");
					else
					{
						gt->SetFinished(true);
						// emit FileFailed signal, so we can record the filename and remote username for resuming later
						//String mFile;
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
								item->setText(WTransferItem::Status, "Finished.");
								item->setText(WTransferItem::ETA, "");
							}
						}
					}
					DecreaseCount(gt, fNumDownloads);
					WASSERT(fNumDownloads >= 0, "Download count is negative!");
					/*
					fLock.lock();
					fDownloadList.erase(foundIt);
					fLock.unlock();
					delete gt;
					gt = NULL; // <postmaster@raasu.org> 20030317
					*/
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
						if (gt->IsLastFile())
							DecreaseCount(gt, fNumDownloads, false);
						WASSERT(fNumDownloads >= 0, "Download count is negative!");
						DequeueDLSessions();
					}
					else // Is this really used???
					{
						PRINT("\tIs upload\n");
						if (gt->IsLastFile())
							DecreaseCount(gt, fNumUploads);
						WASSERT(fNumUploads >= 0, "Upload count is negative!");
						DequeueULSessions();
					}
					item->setText(WTransferItem::Status, "Finished.");
					item->setText(WTransferItem::ETA, "");
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
					else // Is this really used???
					{
						item->setText(WTransferItem::Status, "Finished.");
						item->setText(WTransferItem::ETA, "");
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
					item->setText(WTransferItem::Status, "Waiting for stream...");
					item->setText(WTransferItem::Filename, QString::fromUtf8( file.Cstr() ) ); // <postmaster@raasu.org> 20021023 -- Unicode fix
					// rec, total, rate
					item->setText(WTransferItem::Received, tr("%1").arg((int) start));
					item->setText(WTransferItem::Total, tr("%1").arg((int) size));
					item->setText(WTransferItem::Rate, "0.0");
					item->setText(WTransferItem::ETA, "");
					item->setText(WTransferItem::User, uname);
					item->setText(WTransferItem::Index, FormatIndex(gt->GetCurrentNum(), gt->GetNumFiles()));
					
					if (upload)
					{
						if (gWin->fSettings->GetUploads())
						{
							gWin->PrintSystem(tr(MSG_TX_ISDOWNLOADING).arg(uname).arg(QString::fromUtf8(file.Cstr())));
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

						if (msg()->FindBool("done", &done) == B_OK)
						{
 							item->setText(WTransferItem::Status, "File finished.");
							item->setText(WTransferItem::ETA, "");

							if (msg()->FindString("file", mFile) == B_OK)
								gWin->PrintSystem( tr(MSG_TX_FINISHED).arg(gt->GetRemoteUser()).arg( QString::fromUtf8(mFile.Cstr()) ) , false);
							DecreaseCount(gt, fNumDownloads, false);
							WASSERT(fNumDownloads >= 0, "Download count is negative!");
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
						
						if (msg()->FindBool("done", &done) == B_OK)
						{
							item->setText(WTransferItem::Status, "Finished.");
							item->setText(WTransferItem::ETA, "");

							if (msg()->FindString("file", mFile) == B_OK)
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
	DequeueDLSessions();
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
	DequeueULSessions();
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
		
	case ID_CLEAR:
		{
			ClearFinishedDL();
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
	UpdateULRatings();
}


void
WDownload::ULRightClicked(QListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fULPopupItem = item;

		//fLock.lock();
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
			fULThrottleMenu->setItemChecked(ID_THROTTLE, false);
			
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
		//fLock.unlock();
	}
}

void
WDownload::DLRightClicked(QListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fDLPopupItem = item;

		//fLock.lock();
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
		//fLock.unlock();
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
			(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
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
			(*it).second->setText(WTransferItem::QR, tr("%1").arg(qr++));
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
			((*it).second->text(0) == "Finished.") ||
			((*it).second->text(0) == "Disconnected.")
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
			((*it).second->text(0) == "Finished.") ||
			((*it).second->text(0) == "Disconnected.")
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
	if ( fDownloadList.empty() )
		return n;

	fLock.lock();
	for (WTIter it = fDownloadList.begin(); it != fDownloadList.end(); it++)
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