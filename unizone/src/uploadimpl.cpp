#include "uploadimpl.h"
#include "global.h"
#include "netclient.h"
#include "settings.h"
#include "util.h"
#include "winsharewindow.h"
#include "wstring.h"
#include "wuploadevent.h"

#include <qaccel.h>
#include <qdir.h>

WUpload::WUpload(QWidget * parent, WFileThread * ft)
: QDialog(parent, "WUpload", false, /* QWidget::WDestructiveClose |*/ QWidget::WStyle_Minimize |
		  QWidget::WStyle_Maximize | QWidget::WStyle_Title | QWidget::WStyle_SysMenu)
{
	resize(450, 265); // <postmaster@raasu.org> 20020927 Changed from 250 to 265
	fSharedFiles = ft;

	fUploads = new QListView(this);
	CHECK_PTR(fUploads);
	fUploads->addColumn(tr("Status"));
	fUploads->addColumn(tr("Filename"));
	fUploads->addColumn(tr("Sent"));
	fUploads->addColumn(tr("Total"));
	fUploads->addColumn(tr("Rate"));
	fUploads->addColumn(tr("ETA"));
	fUploads->addColumn(tr("Elapsed"));
	fUploads->addColumn(tr("User"));
	fUploads->addColumn(tr("Index"));
	fUploads->addColumn(tr("QR"));
	
	fUploads->setColumnAlignment(WTransferItem::Received, AlignRight);	// <postmaster@raasu.org> 20021213
	fUploads->setColumnAlignment(WTransferItem::Total, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::Rate, AlignRight);		// 
	fUploads->setColumnAlignment(WTransferItem::ETA, AlignRight);		//
	fUploads->setColumnAlignment(WTransferItem::Elapsed, AlignRight);	// 20030729
	fUploads->setColumnAlignment(WTransferItem::QR, AlignRight);		// 20030310
	
	fUploads->setAllColumnsShowFocus(true);

	connect(netClient(), SIGNAL(UserDisconnected(const WUserRef &)), 
			this, SLOT(UserDisconnected(const WUserRef &)));

	setCaption(tr("Uploads"));

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
	
	fULThrottleMenu->insertItem(tr( "%1 B/s" ).arg(64), ID_64);
	fULThrottleMenu->insertItem(tr( "%1 B/s" ).arg(128), ID_128);
	fULThrottleMenu->insertItem(tr( "%1 B/s" ).arg(256), ID_256);
	fULThrottleMenu->insertItem(tr( "%1 B/s" ).arg(512), ID_512);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(1), ID_1KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(2), ID_2KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(4), ID_4KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(6), ID_6KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(8), ID_8KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(10), ID_10KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(12), ID_12KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(14), ID_14KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(16), ID_16KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(32), ID_32KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(64), ID_64KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(128), ID_128KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(256), ID_256KB);
	fULThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(512), ID_512KB);
	fULThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(1), ID_1MB);
	fULThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(2), ID_2MB);
	fULThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(4), ID_4MB);
	fULThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(8), ID_8MB);
	fULThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(16), ID_16MB);
	fULThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(32), ID_32MB);
	
	fULBanMenu = new QPopupMenu(fULPopup, "Ban Popup");
	CHECK_PTR(fULBanMenu);
	
	fULBanMenu->insertItem(tr("Unbanned"), ID_UNBAN);
	fULBanMenu->setItemChecked(ID_UNBAN, true);
	fULBan = ID_UNBAN;
	
	fULBanMenu->insertItem(tr("1 minute"), ID_BAN1);
	fULBanMenu->insertItem(tr("%1 minutes").arg(2), ID_BAN2);
	fULBanMenu->insertItem(tr("%1 minutes").arg(5), ID_BAN5);
	fULBanMenu->insertItem(tr("%1 minutes").arg(10), ID_BAN10);
	fULBanMenu->insertItem(tr("%1 minutes").arg(15), ID_BAN15);
	fULBanMenu->insertItem(tr("%1 minutes").arg(30), ID_BAN30);
	fULBanMenu->insertItem(tr("1 hour"), ID_BAN1H);
	fULBanMenu->insertItem(tr("Infinite"), ID_BANINF);
	
	fULPacketMenu = new QPopupMenu(fULPopup, "Packet Size Menu");
	CHECK_PTR(fULPacketMenu);
	
	fULPacketMenu->insertItem(tr("%1 B").arg(512), ID_PACKET512);
	fULPacketMenu->insertItem(tr("%1 kB").arg(1), ID_PACKET1K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(2), ID_PACKET2K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(4), ID_PACKET4K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(8), ID_PACKET8K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(16), ID_PACKET16K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(32), ID_PACKET32K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(64), ID_PACKET64K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(128), ID_PACKET128K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(256), ID_PACKET256K);
	fULPacketMenu->insertItem(tr("%1 kB").arg(512), ID_PACKET512K);
	fULPacketMenu->insertItem(tr("%1 MB").arg(1), ID_PACKET1M);
	
	fULPacketMenu->setItemChecked(ID_PACKET8K, true);
	fULPacket = ID_PACKET8K;

	fULCompressionMenu = new QPopupMenu(fULPopup, "Compression Menu");
	CHECK_PTR(fULCompressionMenu);
	
	fULCompressionMenu->insertItem(tr("None"), ID_LEVEL0);
	fULCompressionMenu->insertItem(tr("Level %1").arg(1), ID_LEVEL1);
	fULCompressionMenu->insertItem(tr("Level %1").arg(2), ID_LEVEL2);
	fULCompressionMenu->insertItem(tr("Level %1").arg(3), ID_LEVEL3);
	fULCompressionMenu->insertItem(tr("Level %1").arg(4), ID_LEVEL4);
	fULCompressionMenu->insertItem(tr("Level %1").arg(5), ID_LEVEL5);
	fULCompressionMenu->insertItem(tr("Level %1").arg(6), ID_LEVEL6);
	fULCompressionMenu->insertItem(tr("Level %1").arg(7), ID_LEVEL7);
	fULCompressionMenu->insertItem(tr("Level %1").arg(8), ID_LEVEL8);
	fULCompressionMenu->insertItem(tr("Level %1").arg(9), ID_LEVEL9);

	fULCompressionMenu->setItemChecked(ID_LEVEL0, true);
	fULCompressionMenu->setEnabled(false); // Disabled by default
	fULCompression = ID_LEVEL0;

	fULPopup->insertItem(tr("Move Up"), ID_MOVEUP);
	fULPopup->insertItem(tr("Move Down"), ID_MOVEDOWN);
	fULPopup->insertItem(tr("Ban IP"), ID_IGNORE);
	fULPopup->insertItem(tr("Clear Finished"), ID_CLEAR);
	fULPopup->insertItem(tr("Cancel"), ID_CANCEL);
	
	fULPopup->insertItem(tr("Throttle"), fULThrottleMenu, ID_THROTTLE);
	fULPopup->insertItem(tr("Block"), fULBanMenu, ID_BLOCK);
	fULPopup->insertItem(tr("Packet Size"), fULPacketMenu, ID_SETPACKET);
	fULPopup->insertItem(tr("Compression"), fULCompressionMenu, ID_SETCOMPRESSION);
	
	connect(fULPopup, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fULThrottleMenu, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fULBanMenu, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fULPacketMenu, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fULCompressionMenu, SIGNAL(activated(int)), this, SLOT(ULPopupActivated(int)));
	connect(fUploads, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
		this, SLOT(ULRightClicked(QListViewItem *, const QPoint &, int)));
	
	fULPopupItem = NULL;

}

WUpload::~WUpload()
{
	EmptyLists();
	emit Closed();
}

void
WUpload::EmptyLists()
{
	EmptyUploads();
}

void
WUpload::EmptyUploads()
{
	Lock();
	PRINT("Number of uploads: %lu\n", fUploadList.GetNumItems());
	ULPair pair;
	while (fUploadList.RemoveHead(pair) == B_NO_ERROR)
	{
		if (pair.thread)
		{
			PRINT("Reseting upload\n");
			pair.thread->Reset();
			PRINT("Deleting upload\n");
			delete pair.thread;
		}
		if (pair.item)
		{
			PRINT("Deleting item\n");
			delete pair.item;
		}
	}
	Unlock();
	// do this to set a load of 0
	SendSignal(UpLoad);
}

bool
WUpload::CreateTunnel(const QString & userID, int64 hisID, void * & myID)
{
	PRINT("WUpload::CreateTunnel(QString, void *, void * &)\n");
	WUploadThread * ut = new WUploadThread(this);
	CHECK_PTR(ut);

	myID = ut;
	
	PRINT("Setting upload\n");
	ut->SetPacketSize(gWin->fSettings->GetPacketSize());
	ut->SetUpload(userID, hisID, fSharedFiles);
	
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
	ULPair p;
	p.thread = ut;
	p.item = new WTransferItem(fUploads, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null);
	CHECK_PTR(p.item);
	
	if (ut->IsLocallyQueued())
	{
		PRINT("IsQueued\n");
		p.item->setText(WTransferItem::Status, tr("Queued."));
	}
	PRINT("Inserting\n");
	Lock();
	fUploadList.AddTail(p);
	Unlock();
	SendSignal(UpLoad);
	SendSignal(ULRatings);
	return true;
}

void 
WUpload::TunnelMessage(int64 myID, MessageRef tmsg)
{
	Lock();
		
	unsigned int n = fUploadList.GetNumItems();
	for (unsigned int i = 0; i < n; i++)
	{
		ULPair p;
		fUploadList.GetItemAt(i, p);
		if (ConvertPtr(p.thread) == myID)
		{
			p.thread->MessageReceived(tmsg);
			break;
		}
	}
		
	Unlock();
}

void
WUpload::AddUpload(const QString & remoteIP, uint32 port)
{
	PRINT("WUpload::AddUpload(QString, uint32)\n");
	WUploadThread * ut = new WUploadThread(this);
	CHECK_PTR(ut);
	
	PRINT("Setting upload\n");
	ut->SetPacketSize(gWin->fSettings->GetPacketSize());
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
	ULPair p;
	p.thread = ut;
	p.item = new WTransferItem(fUploads, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null);
	CHECK_PTR(p.item);
	
	if (ut->IsLocallyQueued())
	{
		PRINT("IsQueued\n");
		p.item->setText(WTransferItem::Status, tr("Queued."));
	}
	PRINT("Inserting\n");
	Lock();
	fUploadList.AddTail(p);
	Unlock();
	SendSignal(UpLoad);
	SendSignal(ULRatings);
}

void
WUpload::AddUpload(const SocketRef & socket, uint32 remoteIP, bool /* queued */)
{
	PRINT("WUpload::AddUpload(int, uint32, bool)\n");
	WUploadThread * ut = new WUploadThread(this);
	CHECK_PTR(ut);
	
	PRINT("Setting upload\n");
	ut->SetPacketSize(gWin->fSettings->GetPacketSize());
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
	
	ULPair p;
	p.thread = ut;
	p.item = new WTransferItem(fUploads, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null);
	CHECK_PTR(p.item);
	
	if (ut->IsLocallyQueued())
	{
		PRINT("IsQueued\n");
		p.item->setText(WTransferItem::Status, tr("Queued."));
	}
	PRINT("Inserting\n");
	Lock();
	fUploadList.AddTail(p);
	Unlock();
	SendSignal(UpLoad);
	SendSignal(ULRatings);
}

void
WUpload::DequeueULSessions()
{
	PRINT("WUpload::DequeueULSessions\n");
	
	bool found = true;
	
	int numNotQueued = 0;
	
	
	Lock();
	for (unsigned int i = 0; i < fUploadList.GetNumItems(); i++)
	{
		ULPair pair;
		fUploadList.GetItemAt(i, pair);
		if (pair.thread)
		{
			if (
				(pair.thread->IsLocallyQueued() == false) && 
				(pair.thread->IsBlocked() == false) && 
				(pair.thread->IsActive() == true) &&
				(pair.thread->IsFinished() == false)
				)
				numNotQueued++;
		}
		
		if (pair.item)
		{
			pair.item->setText(WTransferItem::QR, QString::number(i));
		}
	}
	Unlock();
	
	while (numNotQueued < gWin->fSettings->GetMaxUploads())
	{
		found = false;
		Lock();
		for (unsigned int i = 0; i < fUploadList.GetNumItems(); i++)
		{
			ULPair pair;
			fUploadList.GetItemAt(i, pair);
			if (pair.thread)
			{
				if (
					(pair.thread->IsLocallyQueued() == true) && 
					(pair.thread->IsManuallyQueued() == false) &&
					(pair.thread->IsFinished() == false)
					)
				{
					found = true;
					pair.thread->SetLocallyQueued(false);
					numNotQueued++;
				}
			}
			if (numNotQueued == gWin->fSettings->GetMaxUploads())
				break;
		}
		Unlock();
		
		if (!found)
			break;
	}
	SendSignal(UpLoad);
	PRINT("WUpload::DequeueULSessions OK\n");
}

void
WUpload::customEvent(QCustomEvent * e)
{
	int t = (int) e->type();
	switch (t)
	{
	case WUploadEvent::Type:
		{
			WUploadEvent * u = dynamic_cast<WUploadEvent *>(e);
			if (u)
			{
				uploadEvent(u);
			}
			break;
		}
	case DequeueUploads:
		{
			DequeueULSessions();
			break;
		}
	case ClearUploads:
		{
			ClearFinishedUL();
			break;
		}
	case ULRatings:
		{
			UpdateULRatings();
			break;
		}
	case UpLoad:
		{
			UpdateLoad();
			break;
		}
	}
}

void
WUpload::uploadEvent(WUploadEvent *u)
{
	MessageRef msg = u->Msg();
	WUploadThread * ut = NULL;
	WTransferItem * item = NULL;
	ULPair upload;
	
	if (!msg())
		return; // Invalid MessageRef!
	
	if (msg()->FindPointer("sender", (void **)&ut) != B_OK)
		return;	// failed! ouch!
	
	if (!ut)
		return;
	
	Lock();
	for (unsigned int i = 0; i < fUploadList.GetNumItems(); i++)
	{
		fUploadList.GetItemAt(i, upload);
		if (upload.thread == ut)
		{
			// found our thread
			item = upload.item;
			PRINT("\t\tFound ul!!!\n");
			break;
		}
	}
	Unlock();
	
	if (!item)
	{
		// <postmaster@raasu.org> 20021023 -- Add debug message
		PRINT("\t\tFailed to find file!!!\n");
		return;	// failed to find a item
	}

	switch (msg()->what)
	{
	case WUploadEvent::Init:
		{
			PRINT("\tWUploadEvent::Init\n");
			QString filename, user;
			if (
				(GetStringFromMessage(msg, "file", filename) == B_OK) && 
				(GetStringFromMessage(msg, "user", user) == B_OK)
				)
			{
				filename = QDir::convertSeparators(filename);
				item->setText(WTransferItem::Filename, filename);
				item->setText(WTransferItem::User, GetUserName(ut));
				item->setText(WTransferItem::Index, FormatIndex(ut->GetCurrentNum(), ut->GetNumFiles()));
			}
			break;
		}
		
	case WUploadEvent::FileQueued:
		{
			PRINT("\tWUploadEvent::FileQueued\n");
			item->setText(WTransferItem::Status, tr("Queued."));
			item->setText(WTransferItem::Rate, "0.0");
			item->setText(WTransferItem::ETA, QString::null);
			item->setText(WTransferItem::Elapsed, QString::null);
			break;
		}
		
	case WUploadEvent::FileBlocked:
		{
			PRINT("\tWUploadEvent::FileBlocked\n");
			uint64 timeLeft = (uint64) -1;
			(void) msg()->FindInt64("timeleft", (int64 *) &timeLeft);
			if (timeLeft == (uint64) -1)
			{
				item->setText(WTransferItem::Status, tr("Blocked."));
			}
			else
			{
				item->setText(WTransferItem::Status, tr("Blocked for %1 minute(s).").arg((int) (timeLeft/60000000)));
			}
			item->setText(WTransferItem::Rate, "0.0");
			item->setText(WTransferItem::ETA, QString::null);
			item->setText(WTransferItem::Elapsed, QString::null);

			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearUploads);
			}
			
			SendSignal(DequeueUploads);
			
			break;
		}
		
	case WUploadEvent::FileHashing:
		{
			PRINT("\tWUploadEvent::FileHashing\n");
			item->setText(WTransferItem::Status, tr("Examining for resume..."));
			break;
		}
		
	case WUploadEvent::ConnectInProgress:
		{
			PRINT("\tWUploadEvent::ConnectInProgress\n");
			item->setText(WTransferItem::Status, tr("Connecting..."));
			break;
		}
		
	case WUploadEvent::ConnectFailed:
		{
			PRINT("\tWUploadEvent::ConnectFailed\n");
			String why, mFile;
			msg()->FindString("why", why);
			item->setText(WTransferItem::Status, tr("Connect failed: %1").arg(tr(why.Cstr())));

			ut->Reset();
			
			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearUploads);
			}
			
			SendSignal(DequeueUploads);
			break;
		}
		
	case WUploadEvent::Connected:
		{
			PRINT("\tWUploadEvent::Connected\n");
			item->setText(WTransferItem::Status, tr("Negotiating..."));
			break;
		}
		
	case WUploadEvent::Disconnected:
		{
			PRINT("\tWUploadEvent::Disconnected\n");
			item->setText(WTransferItem::ETA, QString::null);
		
			bool f;
			if ((msg()->FindBool("failed", &f) == B_OK) && f)
			{
				// "failed" == true only, if the transfer has failed
				item->setText(WTransferItem::Status, tr("Disconnected."));
			}
			else
			{
				item->setText(WTransferItem::Status, tr("Finished."));
			}
			
			ut->SetFinished(true);
			
			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearUploads);
			}
			
			SendSignal(DequeueUploads);
			PRINT("\tWUploadEvent::Disconnected OK\n");
			break;
		}
		
	case WUploadEvent::FileDone:
		{
			PRINT("\tWUploadEvent::FileDone\n");
			bool d;
			if (msg()->FindBool("done", &d) == B_OK)
			{
				PRINT("\tFound done\n");
				if (ut->IsLastFile())
				{
					ut->Reset();
				}
				item->setText(WTransferItem::Status, tr("Finished."));
				item->setText(WTransferItem::ETA, QString::null);
			}
			PRINT("\tWUploadEvent::FileDone OK\n");
			break;
		}
		
	case WUploadEvent::FileStarted:
		{
			PRINT("\tWUploadEvent::FileStarted\n");
			QString file, user;
			uint64 start;
			uint64 size;
			
			if (
				(GetStringFromMessage(msg, "file", file) == B_OK) && 
				(GetStringFromMessage(msg, "user", user) == B_OK) &&
				(msg()->FindInt64("start", (int64 *)&start) == B_OK) &&
				(msg()->FindInt64("size", (int64 *)&size) == B_OK)
				)
			{
				file = QDir::convertSeparators(file);
				QString uname = GetUserName(ut);
				
#ifdef _DEBUG
				WString wuid(user);
				WString wname(uname);
				PRINT("USER ID  : %S\n", wuid.getBuffer());
				PRINT("USER NAME: %S\n", wname.getBuffer());
#endif
				
				item->setText(WTransferItem::Status, tr("Waiting for stream..."));
				item->setText(WTransferItem::Filename, file); // <postmaster@raasu.org> 20021023 -- Unicode fix
				// rec, total, rate
				item->setText(WTransferItem::Received, fromULongLong(start));
				item->setText(WTransferItem::Total, fromULongLong(size));
				item->setText(WTransferItem::Rate, "0.0");
				item->setText(WTransferItem::ETA, QString::null);
				item->setText(WTransferItem::User, uname);
				item->setText(WTransferItem::Index, FormatIndex(ut->GetCurrentNum(), ut->GetNumFiles()));
			}
			ut->fLastData.start();
			break;
		}
		
	case WUploadEvent::UpdateUI:
		{
			PRINT("\tWUploadEvent::UpdateUI\n");
			QString id; // unused
			if (GetStringFromMessage(msg, "id", id) == B_OK)
			{
				item->setText(WTransferItem::User, GetUserName(ut));
			}
			break;
		}
		
	case WUploadEvent::FileError:
		{
			PRINT("\tWUploadEvent::FileError\n");
			String why;
			QString file;
			msg()->FindString("why", why);
			if (GetStringFromMessage(msg, "file", file) == B_OK)
			{
				file = QDir::convertSeparators(file);
				item->setText(WTransferItem::Filename, file);
			}
			item->setText(WTransferItem::Status, tr("Error: %1").arg(tr(why.Cstr())));
			item->setText(WTransferItem::Index, FormatIndex(ut->GetCurrentNum(), ut->GetNumFiles()));
#ifdef _DEBUG
			// <postmaster@raasu.org> 20021023 -- Add debug message
			WString wfile(file);
			PRINT("WUploadEvent::FileError: File %S\n", wfile.getBuffer()); 
#endif
			break;
		}
		
	case WUploadEvent::FileDataSent:
		{
			int64 offset, size;
			bool done;
			String mFile;
			uint32 got;
			
			if (
				(msg()->FindInt64("offset", (int64 *)&offset) == B_OK) && 
				(msg()->FindInt64("size", (int64 *)&size) == B_OK) &&
				(msg()->FindInt32("sent", (int32 *)&got) == B_OK)
				)
			{
				PRINT2("\tWUploadEvent::FileDataSent\n");
				PRINT2("\tOffset: " UINT64_FORMAT_SPEC "\n", offset);
				PRINT2("\tSize  : " UINT64_FORMAT_SPEC "\n", size);
				PRINT2("\tSent  : %lu\n", got);
				gWin->UpdateTransmitStats(got);
				
				double secs = 0.0f;
				
				if (ut->fLastData.elapsed() > 0)
				{
					secs = (double)((double)ut->fLastData.elapsed() / 1000.0f);
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
				
				item->setText(WTransferItem::Status, tr("Uploading: [%1%]").arg(ComputePercentString(offset, size)));
				item->setText(WTransferItem::Received, fromULongLong(offset));
				// <postmaster@raasu.org> 20021104, 20030217, 20030622
				// elapsed time >= 1 s?
				if (secs >= 1.0f) 
				{
					ut->SetMostRecentRate(kps);
					ut->fLastData.restart();
				}
				else
				{
					ut->SetPacketCount(gotk);
				}
				
				// <postmaster@raasu.org> 20021026 -- Too slow transfer rate?
				double gcr = ut->GetCalculatedRate();
				
				uint64 _fileStarted = ut->GetStartTime();
				if (_fileStarted != 0)
				{
					uint64 _now = GetRunTime64();
					if (_now >= _fileStarted)
					{
						uint64 _elapsed = _now - _fileStarted;
						if (_elapsed > 0)
							_elapsed /= 1000000;	// convert microseconds to seconds
						item->setText(WTransferItem::Elapsed, fromULongLong(_elapsed));
					}
				}
				
				
				item->setText(WTransferItem::ETA, ut->GetETA(offset / 1024, size / 1024, gcr));
				
				item->setText(WTransferItem::Rate, QString::number(gcr*1024.0f));
				
				if (msg()->FindBool("done", &done) == B_OK)
				{
					item->setText(WTransferItem::Status, tr("Finished."));
					item->setText(WTransferItem::ETA, QString::null);
				}
				PRINT2("\tWUploadEvent::FileDataSent OK\n");
			}
			break;
		}
	}
}

void 
WUpload::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == QAccel::stringToKey(tr("Shift+F11")))
		hide();
	else
		QDialog::keyPressEvent(event);
}

QString
WUpload::GetUserName(WUploadThread *ut)
{
	QString sid = ut->GetRemoteID();
	QString name = ut->GetRemoteUser();
	QString ret;
	ret = tr("%1 (%2)").arg(name).arg(sid);
	return ret;
}


void
WUpload::UpdateLoad()
{
	PRINT("WUpload::UpdateLoad\n");
	if (netClient())
	{
		int mu = 0;
		if (gWin->fSettings)
		{
			mu = gWin->fSettings->GetMaxUploads();
		}
		netClient()->SetLoad(GetUploadQueue(), gWin->fSettings->GetMaxUploads());
	}
	PRINT("WUpload::UpdateLoad OK\n");
}

void
WUpload::UserDisconnected(const WUserRef & uref)
{
	QString sid = uref()->GetUserID();
	unsigned int i;
	Lock();
	for (i = 0; i < fUploadList.GetNumItems(); i++)
	{
		ULPair pair;
		fUploadList.GetItemAt(i, pair);
		if (pair.thread)
		{
			if (pair.thread->GetRemoteID() == sid)
			{
				if (gWin->fSettings->GetBlockDisconnected())
				{
					// Only block transfers that are active
					if (
						(pair.thread->IsBlocked() == false) &&
						(pair.thread->IsActive() == true)
						)
					{
						pair.thread->SetBlocked(true);
						pair.thread->Reset();
					}
				}
				if (pair.thread->IsTunneled())
					pair.thread->Reset();
			}
		}
	}
	Unlock();
}

QString
WUpload::FormatIndex(int32 cur, int32 num)
{
	return tr("%1 of %2").arg(cur+1).arg(num);
}

void
WUpload::ULPopupActivated(int id)
{
	ULPair pair;
	unsigned int index;
	WUploadThread * ut = NULL;
	bool findRet;
	
	findRet = FindULItem(index, fULPopupItem);
	
	if (findRet)
	{
		fUploadList.GetItemAt(index, pair);
	}
	else
	{
		PRINT("Upload item not found!");
		return;
	}
	
	ut = pair.thread;
	WASSERT(ut != NULL, "Upload Thread is invalid!");
	
	switch (id)
	{
	case ID_QUEUE:
		{
			if (ut->IsLocallyQueued())
			{
				ut->SetLocallyQueued(false);
				ut->SetManuallyQueued(false);
			}
			else
			{
				ut->SetLocallyQueued(true);
				ut->SetManuallyQueued(true);
			}
			break;
		}
		
	case ID_MOVEUP:
		{
			if (ut->IsLocallyQueued())
				ULMoveUp(index);
			break;
		}
		
	case ID_MOVEDOWN:
		{
			if (ut->IsLocallyQueued())
				ULMoveDown(index);
			break;
		}
		
	case ID_CLEAR:
		{
			SendSignal(ClearUploads);
			break;
		}	
		
	case ID_CANCEL:
		{
			Lock();
			// found our item, stop it
			fULPopupItem->setText(WTransferItem::Status, tr("Canceled."));
			ut->SetFinished(true);
			ut->Reset();
			Unlock();
			
			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearUploads);
			}
			
			SendSignal(DequeueUploads);
			break;
		}
		
	case ID_NO_LIMIT:
		{
			ut->SetRate(0);
			break;
		}
		
	case ID_64:
		{
			ut->SetRate(64);
			break;
		}
		
	case ID_128:
		{
			ut->SetRate(128);
			break;
		}
		
	case ID_256:
		{
			ut->SetRate(256);
			break;
		}
		
	case ID_512:
		{
			ut->SetRate(512);
			break;
		}
		
	case ID_1KB:
		{
			ut->SetRate(1024);
			break;
		}
		
	case ID_2KB:
		{
			ut->SetRate(2 * 1024);
			break;
		}
		
	case ID_4KB:
		{
			ut->SetRate(4 * 1024);
			break;
		}
		
	case ID_6KB:
		{
			ut->SetRate(6 * 1024);
			break;
		}
		
	case ID_8KB:
		{
			ut->SetRate(8 * 1024);
			break;
		}
		
	case ID_10KB:
		{
			ut->SetRate(10 * 1024);
			break;
		}
		
	case ID_12KB:
		{
			ut->SetRate(12 * 1024);
			break;
		}
		
	case ID_14KB:
		{
			ut->SetRate(14 * 1024);
			break;
		}
		
	case ID_16KB:
		{
			ut->SetRate(16 * 1024);
			break;
		}
		
	case ID_32KB:
		{
			ut->SetRate(32 * 1024);
			break;
		}
		
	case ID_64KB:
		{
			ut->SetRate(64 * 1024);
			break;
		}
		
	case ID_128KB:
		{
			ut->SetRate(128 * 1024);
			break;
		}
		
	case ID_256KB:
		{
			ut->SetRate(256 * 1024);
			break;
		}
		
	case ID_512KB:
		{
			ut->SetRate(512 * 1024);
			break;
		}
		
	case ID_1MB:
		{
			ut->SetRate(1024 * 1024);
			break;
		}
		
	case ID_2MB:
		{
			ut->SetRate(2 * 1024 * 1024);
			break;
		}
		
	case ID_4MB:
		{
			ut->SetRate(4 * 1024 * 1024);
			break;
		}
		
	case ID_8MB:
		{
			ut->SetRate(8 * 1024 * 1024);
			break;
		}
		
	case ID_16MB:
		{
			ut->SetRate(16 * 1024 * 1024);
			break;
		}
		
	case ID_32MB:
		{
			ut->SetRate(32 * 1024 * 1024);
			break;
		}
		
	case ID_LEVEL0:
		{
			ut->SetCompression(0);
			break;
		}
		
	case ID_LEVEL1:
		{
			ut->SetCompression(1);
			break;
		}
		
	case ID_LEVEL2:
		{
			ut->SetCompression(2);
			break;
		}
		
	case ID_LEVEL3:
		{
			ut->SetCompression(3);
			break;
		}
		
	case ID_LEVEL4:
		{
			ut->SetCompression(4);
			break;
		}
		
	case ID_LEVEL5:
		{
			ut->SetCompression(5);
			break;
		}
		
	case ID_LEVEL6:
		{
			ut->SetCompression(6);
			break;
		}
		
	case ID_LEVEL7:
		{
			ut->SetCompression(7);
			break;
		}
		
	case ID_LEVEL8:
		{
			ut->SetCompression(8);
			break;
		}
		
	case ID_LEVEL9:
		{
			ut->SetCompression(9);
			break;
		}
		
	case ID_UNBAN:
		{
			ut->SetBlocked(false);
			break;
		}
		
	case ID_BAN1:
		{
			ut->SetBlocked(true, 60000000);
			break;
		}
		
	case ID_BAN2:
		{
			ut->SetBlocked(true, 120000000);
			break;
		}
		
	case ID_BAN5:
		{
			ut->SetBlocked(true, 300000000);
			break;
		}
		
	case ID_BAN10:
		{
			ut->SetBlocked(true, 600000000);
			break;
		}
		
	case ID_BAN15:
		{
			ut->SetBlocked(true, 900000000);
			break;
		}
		
	case ID_BAN30:
		{
			ut->SetBlocked(true, 1800000000);
			break;
		}
		
	case ID_BAN1H:
		{
#ifdef WIN32
			ut->SetBlocked(true, 3600000000UL);
#else
			ut->SetBlocked(true, 3600000000LL);
#endif
			break;
		}
		
	case ID_BANINF:
		{
			ut->SetBlocked(true, -1);
			break;
		}
		
	case ID_PACKET512:
		{
			ut->SetPacketSize(0.5);
			break;
		}

	case ID_PACKET1K:
		{
			ut->SetPacketSize(1);
			break;
		}
		
	case ID_PACKET2K:
		{
			ut->SetPacketSize(2);
			break;
		}
		
	case ID_PACKET4K:
		{
			ut->SetPacketSize(4);
			break;
		}
		
	case ID_PACKET8K:
		{
			ut->SetPacketSize(8);
			break;
		}
		
	case ID_PACKET16K:
		{
			ut->SetPacketSize(16);
			break;
		}
		
	case ID_PACKET32K:
		{
			ut->SetPacketSize(32);
			break;
		}

	case ID_PACKET64K:
		{
			ut->SetPacketSize(64);
			break;
		}

	case ID_PACKET128K:
		{
			ut->SetPacketSize(128);
			break;
		}
		
	case ID_PACKET256K:
		{
			ut->SetPacketSize(256);
			break;
		}

	case ID_PACKET512K:
		{
			ut->SetPacketSize(512);
			break;
		}

	case ID_PACKET1M:
		{
			ut->SetPacketSize(1024);
			break;
		}
		
	case ID_IGNORE:
		{
			if ( gWin->IsIgnoredIP( ut->GetRemoteIP() ) )
			{
				gWin->RemoveIPIgnore(ut->GetRemoteIP());
				if (ut->IsBlocked())
					ut->SetBlocked(false, -1);
			}
			else
			{
				gWin->AddIPIgnore(ut->GetRemoteIP());
				if (!ut->IsBlocked())
					ut->SetBlocked(true, -1);
			}
			break;
		}
	}
	SendSignal(ULRatings);
}

void
WUpload::ULRightClicked(QListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fULPopupItem = item;
		
		unsigned int index;
		if (FindULItem(index, item))
		{
			ULPair pair;
			fUploadList.GetItemAt(index, pair);
			if ( !pair.thread )
			{
				PRINT("Generic Thread is invalid!");
				return;
			}
			
			// Disable items not applicable to current thread state
			
			// Not Locally Queued
			
			if (pair.thread->IsLocallyQueued() == false)
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
			
			if (pair.thread->IsFinished() == true)
			{
				fULPopup->setItemEnabled(ID_QUEUE, false);
				fULPopup->setItemEnabled(ID_BLOCK, false);
				fULPopup->setItemEnabled(ID_THROTTLE, false);
				fULPopup->setItemEnabled(ID_SETPACKET, false);
				fULPopup->setItemEnabled(ID_CANCEL, false);
			}
			else
			{
				fULPopup->setItemEnabled(ID_QUEUE, true);
				fULPopup->setItemEnabled(ID_BLOCK, true);
				if (pair.thread->IsTunneled() == false)
					fULPopup->setItemEnabled(ID_THROTTLE, true);
				else
					fULPopup->setItemEnabled(ID_THROTTLE, false);
				fULPopup->setItemEnabled(ID_SETPACKET, true);
				fULPopup->setItemEnabled(ID_CANCEL, true);
			}
			
			fULPopup->setItemChecked(ID_QUEUE, pair.thread->IsLocallyQueued());
			fULPopup->setItemChecked(ID_IGNORE, gWin->IsIgnoredIP( pair.thread->GetRemoteIP() ));
			int fNewRate = pair.thread->GetRate();
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
			
			int fNewBan = pair.thread->GetBanTime();
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
			
			double fNewPacket = pair.thread->GetPacketSize();
			fULPacketMenu->setItemChecked(fULPacket, false);
			
			if (fNewPacket == 0.5)
			{
				fULPacket = ID_PACKET512;
			}
			else if (fNewPacket == 1)
			{
				fULPacket = ID_PACKET1K;
			}
			else if (fNewPacket == 2)
			{
				fULPacket = ID_PACKET2K;
			}
			else if (fNewPacket == 4)
			{
				fULPacket = ID_PACKET4K;
			}
			else if (fNewPacket == 8)
			{
				fULPacket = ID_PACKET8K;
			}
			else if (fNewPacket == 16)
			{
				fULPacket = ID_PACKET16K;
			}
			else if (fNewPacket == 32)
			{
				fULPacket = ID_PACKET32K;
			}
			else if (fNewPacket == 64)
			{
				fULPacket = ID_PACKET64K;
			}
			else if (fNewPacket == 128)
			{
				fULPacket = ID_PACKET128K;
			}
			else if (fNewPacket == 256)
			{
				fULPacket = ID_PACKET256K;
			}
			else if (fNewPacket == 512)
			{
				fULPacket = ID_PACKET512K;
			}
			else if (fNewPacket == 1024)
			{
				fULPacket = ID_PACKET1M;
			}
			fULPacketMenu->setItemChecked(fULPacket, true);
			
			fULCompressionMenu->setItemChecked(fULCompression, false);
			int fNewCompression = pair.thread->GetCompression();

			if (fNewCompression == -1)
				fULCompressionMenu->setEnabled(false);
			else
				fULCompressionMenu->setEnabled(true);

			switch (fNewCompression)
			{
			case 0:
				{
					fULCompression = ID_LEVEL0;
					break;
				}
			case 1:
				{
					fULCompression = ID_LEVEL1;
					break;
				}
			case 2:
				{
					fULCompression = ID_LEVEL2;
					break;
				}
			case 3:
				{
					fULCompression = ID_LEVEL3;
					break;
				}
			case 4:
				{
					fULCompression = ID_LEVEL4;
					break;
				}
			case 5:
				{
					fULCompression = ID_LEVEL5;
					break;
				}
			case 6:
				{
					fULCompression = ID_LEVEL6;
					break;
				}
			case 7:
				{
					fULCompression = ID_LEVEL7;
					break;
				}
			case 8:
				{
					fULCompression = ID_LEVEL8;
					break;
				}
			case 9:
				{
					fULCompression = ID_LEVEL9;
					break;
				}
			}
			fULCompressionMenu->setItemChecked(fULCompression, true);

			fULPopup->popup(p);
		}
	}
}

bool
WUpload::FindULItem(unsigned int & index, QListViewItem * s)
{
	PRINT("WUpload::FindULItem()\n");
	bool success = false;
	
	Lock();
	
	for (unsigned int i = 0; i < fUploadList.GetNumItems(); i++)
	{
		ULPair p;
		fUploadList.GetItemAt(i, p);
		if (p.item == s)
		{
			index = i;
			success = true;
			break;
		}
	}
	
	Unlock();
	
	if (!success)
	{
		PRINT("WUpload::FindULItem() : Item not found!\n");
	}
	return success;
}

void
WUpload::ULMoveUp(unsigned int index)
{
	if (index == 0)
		return;
	
	ULPair wp;
	
	Lock();
	
	unsigned int index2 = index;
	fUploadList.RemoveItemAt(index, wp);
	Unlock();
	
	Lock();
	while (index2 > 0) 
	{
		index2--;
		ULPair p;
		fUploadList.GetItemAt(index2, p);
		if (p.thread->IsLocallyQueued())
			break;
	}
	fUploadList.InsertItemAt(index2, wp);
	
	Unlock();
}

void
WUpload::ULMoveDown(unsigned int index)
{
	if (index == fUploadList.GetNumItems() - 1)
		return;
	
	ULPair wp;
	
	Lock();
	
	unsigned int index2 = index;
	fUploadList.RemoveItemAt(index, wp);
	Unlock();
	
	Lock();
	while (index2 < (fUploadList.GetNumItems() - 2))
	{
		index2++;
		ULPair p;
		fUploadList.GetItemAt(index2, p);
		if (p.thread->IsLocallyQueued())
			break;
	}
	
	index2++;
	
	fUploadList.InsertItemAt(index2, wp);
	
	Unlock();
}

void
WUpload::UpdateULRatings()
{
	PRINT("\tWUpload::UpdateULRatings\n");
	if ( fUploadList.IsEmpty() )
	{
		PRINT("\tUpload list is empty!\n");
		return;
	}
	
	Lock();
	
	for (unsigned int i = 0; i < fUploadList.GetNumItems(); i++)
	{
		ULPair pair;
		fUploadList.GetItemAt(i, pair);
		if (pair.item)
		{
#ifdef _DEBUG
			WString wFile(pair.thread->GetCurrentFile());
			if (wFile.length() > 0)
				PRINT("Item %d: %S\n", i, wFile.getBuffer() );
			else
				PRINT("Item %d\n", i);
#endif
			
			pair.item->setText(WTransferItem::QR, QString::number(i));
		}
	}
	Unlock();
	
	PRINT("\tWUpload::UpdateULRatings OK\n");
}

// Clear all finished uploads from listview
void
WUpload::ClearFinishedUL()
{
	PRINT("\tWUpload::ClearFinishedUL\n");
	
	if (fUploadList.GetNumItems() > 0)
	{
		Lock();
		unsigned int i = 0;
		while (i < fUploadList.GetNumItems())
		{
			ULPair pair;
			fUploadList.GetItemAt(i, pair);
			if (pair.thread->IsFinished() == true)
			{
				// found finished item, erase it
				fUploadList.RemoveItemAt(i);
				// free resources
				pair.thread->Reset();
				delete pair.thread;
				delete pair.item;
			}
			else
			{
				i++;
			}
		}
		if ((fUploadList.IsEmpty()) && gWin->fSettings->GetAutoClose())
		{
			// Automatically hide if there is no more uploads
			hide();
		}
		Unlock();
	}
	
	SendSignal(ULRatings);
	
	PRINT("\tWUpload::ClearFinishedUL OK\n");
}

int
WUpload::GetNumUploads()
{
	PRINT("\tWUpload::GetNumUploads\n");
	int n = 0;
	if ( fUploadList.GetNumItems() > 0)
	{
		Lock();
		for (unsigned int i = 0; i < fUploadList.GetNumItems(); i++)
		{
			ULPair pair;
			fUploadList.GetItemAt(i, pair);
			if (pair.thread)
			{
				if (
					(pair.thread->IsLocallyQueued() == false) &&
					(pair.thread->IsBlocked() == false) &&
					(pair.thread->IsFinished() == false)
					)
				{
					n++;
				}
			}
		}
		Unlock();
	}
	return n;
}

int
WUpload::GetUploadQueue()
{
	PRINT("\tWUpload::GetUploadQueue\n");
	int n = 0;
	if ( fUploadList.GetNumItems() > 0)
	{
		Lock();
		for (unsigned int i = 0; i < fUploadList.GetNumItems(); i++)
		{
			ULPair pair;
			fUploadList.GetItemAt(i, pair);
			if (pair.thread)
			{
				if ((pair.thread->IsFinished() == false) && (pair.thread->IsBlocked() == false))
				{
					n++;
				}
			}
		}
		Unlock();
	}
	return n;
}

/*void
WUpload::SetLocalID(const QString &sid)
{
	fLocalSID = sid;
}*/

void
WUpload::SendSignal(int signal)
{
	QCustomEvent *qce = new QCustomEvent(signal);
	if (qce) 
		QApplication::postEvent(this, qce);
}


NetClient *
WUpload::netClient()
{
	return gWin->fNetClient;
}

void 
WUpload::resizeEvent(QResizeEvent * e)
{
	fUploads->resize(e->size());
	QDialog::resizeEvent(e);
}

void 
WUpload::Lock() 
{ 
	fLock.Lock(); 
}

void 
WUpload::Unlock() 
{ 
	fLock.Unlock(); 
}

