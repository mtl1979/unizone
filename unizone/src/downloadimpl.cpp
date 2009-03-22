#include <qobject.h>
#include <qdir.h>
#include <q3accel.h>
#include <QKeyEvent>
#include <QCustomEvent>
#include <QResizeEvent>
#include <Q3PopupMenu>
#include <time.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <list>
using std::list;
using std::iterator;

#include "message/Message.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"

#include "downloadimpl.h"
#include "md5.h"
#include "debugimpl.h"
#include "global.h"
#include "filethread.h"
#include "settings.h"
#include "downloadthread.h"
#include "wdownloadevent.h"
#include "wmessageevent.h"
#include "wwarningevent.h"
#include "wstring.h"
#include "transferitem.h"
#include "gotourl.h"
#include "util.h"
#include "netclient.h"

// ----------------------------------------------------------------------------------------------

WDownload::WDownload(QWidget * parent, QString localID)
: QDialog(parent, "WDownload", false, /* QWidget::WDestructiveClose |*/ Qt::WStyle_Minimize |
		  Qt::WStyle_Maximize | Qt::WStyle_Title | Qt::WStyle_SysMenu)
{
	resize(450, 265); // <postmaster@raasu.org> 20020927 Changed from 250 to 265
	fLocalSID = localID;
	
	fDownloads = new Q3ListView(this);
	Q_CHECK_PTR(fDownloads);
	fDownloads->addColumn(tr("Status"));
	fDownloads->addColumn(tr("Filename"));
	fDownloads->addColumn(tr("Received"));
	fDownloads->addColumn(tr("Total"));
	fDownloads->addColumn(tr("Rate"));
	fDownloads->addColumn(tr("ETA"));
	fDownloads->addColumn(tr("Elapsed"));
	fDownloads->addColumn(tr("User"));
	fDownloads->addColumn(tr("Index"));
	fDownloads->addColumn(tr("QR"));
	
	fDownloads->setColumnAlignment(WTransferItem::Received, Qt::AlignRight);	// <postmaster@raasu.org> 20021213
	fDownloads->setColumnAlignment(WTransferItem::Total, Qt::AlignRight);		// 
	fDownloads->setColumnAlignment(WTransferItem::Rate, Qt::AlignRight);		// 
	fDownloads->setColumnAlignment(WTransferItem::ETA, Qt::AlignRight);			// 
	fDownloads->setColumnAlignment(WTransferItem::Elapsed, Qt::AlignRight);		// 20030729
	fDownloads->setColumnAlignment(WTransferItem::QR, Qt::AlignRight);			// 20030310
	
	fDownloads->setAllColumnsShowFocus(true);
	
	
	connect(netClient(), SIGNAL(UserDisconnected(const WUserRef &)), 
			this, SLOT(UserDisconnected(const WUserRef &)));
	
	
	setCaption(tr("Downloads"));
	
	fDLPopup = new Q3PopupMenu(this, "Download Popup");
	Q_CHECK_PTR(fDLPopup);
	
	// Create the download popup menu
	fDLPopup->insertItem(tr("Queue"), ID_QUEUE);
	
	// Create throttle sub menu
	fDLThrottleMenu = new Q3PopupMenu(fDLPopup, "Throttle Popup");
	Q_CHECK_PTR(fDLThrottleMenu);
	
	fDLThrottleMenu->insertItem(tr( "No Limit" ), ID_NO_LIMIT);
	
	// Set initial throttle to none
	fDLThrottleMenu->setItemChecked(ID_NO_LIMIT, true);
	fDLThrottle = ID_NO_LIMIT;
	
	fDLThrottleMenu->insertItem(tr( "%1 B/s" ).arg(64), ID_64);
	fDLThrottleMenu->insertItem(tr( "%1 B/s" ).arg(128), ID_128);
	fDLThrottleMenu->insertItem(tr( "%1 B/s" ).arg(256), ID_256);
	fDLThrottleMenu->insertItem(tr( "%1 B/s" ).arg(512), ID_512);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(1), ID_1KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(2), ID_2KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(4), ID_4KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(6), ID_6KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(8), ID_8KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(10), ID_10KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(12), ID_12KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(14), ID_14KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(16), ID_16KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(32), ID_32KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(64), ID_64KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(128), ID_128KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(256), ID_256KB);
	fDLThrottleMenu->insertItem(tr( "%1 kB/s" ).arg(512), ID_512KB);
	fDLThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(1), ID_1MB);
	fDLThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(2), ID_2MB);
	fDLThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(4), ID_4MB);
	fDLThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(8), ID_8MB);
	fDLThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(16), ID_16MB);
	fDLThrottleMenu->insertItem(tr( "%1 MB/s" ).arg(32), ID_32MB);
	
	fDLRunMenu = new Q3PopupMenu(fDLPopup, "Run Popup");
	Q_CHECK_PTR(fDLRunMenu);
	
	fDLPopup->insertItem(tr("Throttle"), fDLThrottleMenu, ID_THROTTLE);
	fDLPopup->insertItem(tr("Run..."), fDLRunMenu, ID_RUN);
	
	fDLPopup->insertItem(tr("Move Up"), ID_MOVEUP);
	fDLPopup->insertItem(tr("Move Down"), ID_MOVEDOWN);
	
	fDLPopup->insertItem(tr("Clear Finished"), ID_CLEAR);
	fDLPopup->insertItem(tr("Cancel"), ID_CANCEL);
	
	connect(fDLPopup, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDLThrottleMenu, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDLRunMenu, SIGNAL(activated(int)), this, SLOT(DLPopupActivated(int)));
	connect(fDownloads, SIGNAL(rightButtonClicked(Q3ListViewItem *, const QPoint &, int)),
		this, SLOT(DLRightClicked(Q3ListViewItem *, const QPoint &, int)));

	fDLPopupItem = NULL;
	
}

WDownload::~WDownload()
{
	EmptyLists();
	emit Closed();
}

void
WDownload::EmptyLists()
{
	EmptyDownloads();
//	EmptyUploads();
}

void
WDownload::EmptyDownloads()
{
	Lock();
	PRINT("Number of downloads: %lu\n", fDownloadList.GetNumItems());
	DLPair pair;
	while (fDownloadList.RemoveHead(pair) == B_NO_ERROR)
	{
		// Put all files in resume list
		if (pair.thread)
		{
			int n = pair.thread->GetCurrentNum();
			
			if (n > -1) 
			{
				for (int i = n; i < pair.thread->GetNumFiles(); i++)
				{
					emit FileInterrupted(pair.thread->GetFileName(i), pair.thread->GetLocalFileName(i), pair.thread->GetPath(i), pair.thread->GetRemoteUser());
				}
			}
			PRINT("Reseting download\n");
			pair.thread->Reset();
			PRINT("Deleting download\n");
			delete pair.thread;
		}
		if (pair.item)
		{
			PRINT("Deleting item\n");
			delete pair.item;
		}
	}
	Unlock();
}


void
WDownload::AddDownload(QString * files, QString * lfiles, QString * paths,
			int32 filecount, QString remoteSessionID, 
			uint32 remotePort, const QString & remoteIP, 
			uint64 /* remoteInstallID */, bool firewalled, 
			bool partial)
{
	WDownloadThread * nt = new WDownloadThread(this);
	Q_CHECK_PTR(nt);

	QString ip = remoteIP;
	if (!firewalled) // Check for valid ip and port if not firewalled
	{
		int x;
		
		// Can't use localhost to download!!!
		
		if (remoteIP == "127.0.0.1")	
		{
			ip = netClient()->GetServerIP();
			if (ip != "127.0.0.1")
			{
				for (x = 0; x < filecount; x++)
				{
					gWin->SendWarningEvent(tr("Invalid address! Download address for file %1 replaced with %2, it might fail!").arg(files[x]).arg(remoteIP));
				}
			}
		}
		
		// Detect uncommon remote port 
		
		if (!muscleInRange(remotePort, (uint32) 1, (uint32) 65535))
		{
			for (x = 0; x < filecount; x++)
			{
				gWin->SendWarningEvent(tr("Download port for file %1 might be out of range, it might fail!").arg(files[x]));
			}
		}
	}

	nt->SetFile(files, lfiles, paths, filecount, ip, remoteSessionID, fLocalSID, remotePort, firewalled, partial);
	
	DLPair p;
	p.thread = nt;
	p.item = new WTransferItem(fDownloads, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null);
	Q_CHECK_PTR(p.item);
		
	if (GetNumDownloads() < gWin->fSettings->GetMaxDownloads())
	{
		nt->InitSession();
		nt->SetLocallyQueued(false);
	}
	else
	{
		nt->SetLocallyQueued(true);
		p.item->setText(WTransferItem::Status, tr("Locally Queued."));
	}
	Lock();
	fDownloadList.AddTail(p);
	Unlock();
	SendSignal(DLRatings);
}

void
WDownload::AddDownloadList(Queue<QString> & fQueue, Queue<QString> & fLQueue, Queue<QString> & fPaths, const WUserRef & user)
{
	int32 nFiles = fQueue.GetNumItems();
	QString * qFiles = new QString[nFiles];
	Q_CHECK_PTR(qFiles);
	
	QString * qLFiles = new QString[nFiles];
	Q_CHECK_PTR(qLFiles);
	QString tmp, tmp2, tmp3;

	QString * qPaths = new QString[nFiles];
	Q_CHECK_PTR(qPaths);

	int n = 0;
	while ((fQueue.RemoveHead(tmp) == B_NO_ERROR) && (fLQueue.RemoveHead(tmp2) == B_NO_ERROR) && (fPaths.RemoveHead(tmp3) == B_NO_ERROR))
	{
		// Remote name
		if (tmp.isEmpty()) 
		{
			qFiles[n] = QString::null;
		}
		else
		{
			qFiles[n] = tmp;
		}
		
		// Local name
		if (tmp2.isEmpty())
		{
			qLFiles[n] = QString::null;
		}
		else
		{
			qLFiles[n] = tmp2;
		}

		// Path information

		if (tmp3.isEmpty())
		{
			qPaths[n] = QString::null;
		}
		else
		{
			qPaths[n] == tmp3;
		}
		
		n++;
	}
	if (gWin->fSettings->GetFirewalled() && user()->GetFirewalled() && user()->GetTunneling())
		CreateTunnel(qFiles, qLFiles, qPaths, n, user);
	else
		AddDownload(qFiles, qLFiles, qPaths, n, user()->GetUserID(), user()->GetPort(), user()->GetUserHostName(), user()->GetInstallID(), user()->GetFirewalled(), user()->GetPartial());
}


bool
WDownload::CreateTunnel(QString *files, QString *lfiles, QString *lpaths, int32 numFiles, const WUserRef &user)
{
	WDownloadThread * nt = new WDownloadThread(this);
	Q_CHECK_PTR(nt);

	nt->SetFile(files, lfiles, lpaths, numFiles, user);
	
	DLPair p;
	p.thread = nt;
	p.item = new WTransferItem(fDownloads, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, QString::null);
	Q_CHECK_PTR(p.item);
		
	if (GetNumDownloads() < gWin->fSettings->GetMaxDownloads())
	{
		nt->InitSession();
		nt->SetLocallyQueued(false);
	}
	else
	{
		nt->SetLocallyQueued(true);
		p.item->setText(WTransferItem::Status, tr("Locally Queued."));
	}
	Lock();
	fDownloadList.AddTail(p);
	Unlock();
	SendSignal(DLRatings);
	return true;
}

void
WDownload::TunnelAccepted(int64 myID, int64 hisID)
{
	bool success = false;
	
	Lock();
	
	for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		DLPair p;
		fDownloadList.GetItemAt(i, p);
		if (ConvertPtr(p.thread) == myID)
		{
			p.thread->Accepted(hisID);
			success = true;
			break;
		}
	}
	
	Unlock();
	
	if (!success)
	{
		PRINT("WDownload::TunnelAccepted() : Item not found!\n");
	}
}

void
WDownload::TunnelRejected(int64 myID)
{
	bool success = false;
	
	Lock();
	
	for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		DLPair p;
		fDownloadList.GetItemAt(i, p);
		if (ConvertPtr(p.thread) == myID)
		{
			p.thread->Rejected();
			success = true;
			break;
		}
	}
	
	Unlock();
	
	if (!success)
	{
		PRINT("WDownload::TunnelRejected() : Item not found!\n");
	}
}

void 
WDownload::TunnelMessage(int64 myID, MessageRef tmsg)
{
	Lock();
	
	unsigned int n = fDownloadList.GetNumItems();
	for (unsigned int i = 0; i < n; i++)
	{
		DLPair p;
		fDownloadList.GetItemAt(i, p);
		if (ConvertPtr(p.thread) == myID)
		{
			WMessageEvent *wme = new WMessageEvent(tmsg);
			if (wme)
				QApplication::postEvent(p.thread, wme);
			break;
		}
	}
		
	Unlock();
}

void
WDownload::DequeueDLSessions()
{
	PRINT("WDownload::DequeueDLSessions\n");
	
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
	
	
	Lock();
	for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		DLPair pair;
		fDownloadList.GetItemAt(i, pair);
		if (pair.thread)
		{
			if (
				(pair.thread->IsLocallyQueued() == false) && 
				(pair.thread->IsActive() == true)
				)
			{
				// not queued, not finished?
				numNotQueued++;
			}
		}
		
		if (pair.item)
		{
			pair.item->setText(WTransferItem::QR, QString::number(qr++));
		}
		
	}
	Unlock();
	// check the queued amount, vs the amount allowed
	while (numNotQueued < gWin->fSettings->GetMaxDownloads())
	{
		found = false;
		Lock();
		for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
		{
			DLPair pair;
			fDownloadList.GetItemAt(i, pair);
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
					pair.thread->InitSession();
					numNotQueued++;
				}
			}
			if (numNotQueued == gWin->fSettings->GetMaxDownloads())
				break;
		}
		Unlock();
		if (!found)
			break;
	}
	PRINT("WDownload::DequeueDLSessions OK\n");
}

void
WDownload::customEvent(QEvent * e)
{
	int t = (int) e->type();
	
	if (t > WDownloadEvent::FirstEvent && t < WDownloadEvent::LastEvent)
	{
		WDownloadEvent * d = dynamic_cast<WDownloadEvent *>(e);
		if (d)
		{
			downloadEvent(d);
			return;
		}
	}

	switch (t)
	{
	case DequeueDownloads:
		{
			DequeueDLSessions();
			break;
		}
	case ClearDownloads:
		{
			ClearFinishedDL();
			break;
		}
	case DLRatings:
		{
			UpdateDLRatings();
			break;
		}
	}
}

void
WDownload::downloadEvent(WDownloadEvent * d)
{
	WDownloadThread * dt = d->Sender();
	WTransferItem * item = NULL;
	DLPair download;
	
	if (!dt)
		return;
	
	Lock();
	for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		fDownloadList.GetItemAt(i, download);
		if (download.thread == dt)
		{
			// found our thread
			item = download.item;
			PRINT("\t\tFound dl!!!\n");
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
	
	switch ((int) d->type())
	{
	case WDownloadEvent::Init:
		{
			PRINT("\tWDownloadEvent::Init\n");
			QString filename = d->File();
			if ( !filename.isEmpty() )
			{
				item->setText(WTransferItem::Filename, QDir::convertSeparators( filename ));
				item->setText(WTransferItem::User, GetUserName(dt));
				item->setText(WTransferItem::Index, FormatIndex(dt->GetCurrentNum(), dt->GetNumFiles()));
			}
			break;
		}
		
	case WDownloadEvent::FileQueued:
		{
			PRINT("\tWDownloadEvent::FileQueued\n");
			item->setText(WTransferItem::Status, tr("Remotely Queued."));
			item->setText(WTransferItem::Rate, "0.0");
			item->setText(WTransferItem::ETA, QString::null);
			item->setText(WTransferItem::Elapsed, QString::null);
			break;
		}
		
	case WDownloadEvent::FileBlocked:
		{
			PRINT("\tWDownloadEvent::FileBlocked\n");
			uint64 timeLeft = d->Time();
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
			
			break;
		}
		
	case WDownloadEvent::ConnectBackRequest:
		{
			PRINT("\tWDownloadEvent::ConnectBackRequest\n");
			MessageRef cb(GetMessageFromPool(NetClient::CONNECT_BACK_REQUEST));
			if (cb())
			{
				QString session = d->Session();
				int32 port = d->Port();

				item->setText(WTransferItem::Status, tr("Waiting for incoming connection..."));
				QString tostr = "/*/";
				tostr += session;
				tostr += "/beshare";
				AddStringToMessage(cb, PR_NAME_KEYS, tostr);
				cb()->AddString(PR_NAME_SESSION, "");
				cb()->AddInt32("port", port);
				netClient()->SendMessageToSessions(cb);
			}
			else
				dt->Reset();	// failed...
			break;
		}
		
	case WDownloadEvent::FileHashing:
		{
			PRINT("\tWDownloadEvent::FileHashing\n");
			item->setText(WTransferItem::Status, tr("Examining for resume..."));
			break;
		}
		
	case WDownloadEvent::ConnectInProgress:
		{
			PRINT("\tWDownloadEvent::ConnectInProgress\n");
			item->setText(WTransferItem::Status, tr("Connecting..."));
			break;
		}
		
	case WDownloadEvent::ConnectFailed:
		{
			PRINT("\tWDownloadEvent::ConnectFailed\n");
			QString why = d->Error();
			item->setText(WTransferItem::Status, tr("Connect failed: %1").arg(why));
			dt->SetFinished(true);
			if (dt->GetCurrentNum() > -1)
			{
				for (int n = dt->GetCurrentNum(); n < dt->GetNumFiles(); n++)
				{
					QString qFile = dt->GetFileName(n);
					QString qLFile = dt->GetLocalFileName(n);
					QString qPath = dt->GetPath(n);
					emit FileFailed(qFile, qLFile, qPath, dt->GetRemoteUser());
				}
			}
			dt->Reset();
			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearDownloads);
			}
			
			SendSignal(DequeueDownloads);
			break;
		}
		
	case WDownloadEvent::Connected:
		{
			PRINT("\tWDownloadEvent::Connected\n");
			item->setText(WTransferItem::Status, tr("Negotiating..."));
			break;
		}
		
	case WDownloadEvent::Disconnected:
		{
			PRINT("\tWDownloadEvent::Disconnected\n");
			if (item->text(0) != tr("Finished."))
				item->setText(WTransferItem::Status, tr("Disconnected."));
			
			if (dt->IsManuallyQueued())
			{
				item->setText(WTransferItem::Status, tr("Manually Queued."));
				item->setText(WTransferItem::Rate, "0.0");
				item->setText(WTransferItem::ETA, QString::null);
				item->setText(WTransferItem::Elapsed, QString::null);
			}
			else
			{
				dt->SetFinished(true);
				// emit FileFailed signal(s), so we can record the filename and remote username for resuming later
				if (d->Failed())
				{
					// "failed" == true only, if the transfer has failed
					if (dt->GetCurrentNum() > -1)
					{
						for (int n = dt->GetCurrentNum(); n < dt->GetNumFiles(); n++)
						{
							QString qFile = dt->GetFileName(n);
							QString qLFile = dt->GetLocalFileName(n);
							QString qPath = dt->GetPath(n);
							emit FileFailed(qFile, qLFile, qPath, dt->GetRemoteUser());
						}
					}
				}
				else
				{
					item->setText(WTransferItem::Status, tr("Finished."));
					item->setText(WTransferItem::ETA, QString::null);
				}
			}
			dt->Reset();
			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearDownloads);
			}
			
			SendSignal(DequeueDownloads);
			PRINT("\tWDownloadEvent::Disconnected OK\n");
			break;
		}
		
	case WDownloadEvent::FileDone:
		{
			PRINT("\tWDownloadEvent::FileDone\n");
			if (d->Done())
			{
				PRINT("\tFound done\n");
				if (dt->IsLastFile())
				{
					dt->Reset();
					if (dt->IsTunneled())
					{
						if (gWin->fSettings->GetAutoClear())
						{
							SendSignal(ClearDownloads);
						}
					}
				}
				item->setText(WTransferItem::Status, tr("Finished."));
				item->setText(WTransferItem::ETA, QString::null);
			}
			else
			{
				item->setText(WTransferItem::Status, tr("Waiting..."));
				item->setText(WTransferItem::Filename, tr("Waiting for next file..."));
				item->setText(WTransferItem::Received, QString::null);
				item->setText(WTransferItem::Total, QString::null);
				item->setText(WTransferItem::Rate, "0.0");
				item->setText(WTransferItem::ETA, QString::null);
				// don't erase the user name
			}
			PRINT("\tWDownloadEvent::FileDone OK\n");
			break;
		}
		
	case WDownloadEvent::FileFailed:
		{
			PRINT("\tWDownloadEvent::FileFailed\n");
			dt->SetFinished(true);
			if (dt->GetCurrentNum() != -1)
			{
				for (int n = dt->GetCurrentNum(); n < dt->GetNumFiles(); n++)
				{
					QString qFile = dt->GetFileName(n);
					QString qLFile = dt->GetLocalFileName(n);
					QString qPath = dt->GetPath(n);
					emit FileFailed(qFile, qLFile, qPath, dt->GetRemoteUser());
				}
			}
			dt->Reset();
			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearDownloads);
			}
			
			SendSignal(DequeueDownloads);

			break;
		}
		
	case WDownloadEvent::FileStarted:
		{
			PRINT("\tWDownloadEvent::FileStarted\n");
			QString file = d->File();
			uint64 start = d->Start();
			uint64 size = d->Size();
			
			{
				file = QDir::convertSeparators(file);
				QString uname = GetUserName(dt);
				
#ifdef _DEBUG
				WString wuid(d->Session());
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
				item->setText(WTransferItem::Index, FormatIndex(dt->GetCurrentNum(), dt->GetNumFiles()));
			}
			dt->fLastData.start();
			break;
		}
		
	case WDownloadEvent::UpdateUI:
		{
			PRINT("\tWDownloadEvent::UpdateUI\n");
			item->setText(WTransferItem::User, GetUserName(dt));
			break;
		}
		
	case WDownloadEvent::FileError:
		{
			PRINT("\tWDownloadEvent::FileError\n");
			QString why = d->Error();
			QString file = d->File();
			file = QDir::convertSeparators(file);
			item->setText(WTransferItem::Filename, file);
			item->setText(WTransferItem::Status, tr("Error: %1").arg(why));
			item->setText(WTransferItem::Index, FormatIndex(dt->GetCurrentNum(), dt->GetNumFiles()));
#ifdef _DEBUG
			// <postmaster@raasu.org> 20021023 -- Add debug message
			WString wfile(file);
			PRINT("WDownloadEvent::FileError: File %S\n", wfile.getBuffer()); 
#endif
			break;
		}
		
	case WDownloadEvent::FileDataReceived:
		{
			int64 offset = d->Offset(), size = d->Size();
			String mFile;
			uint32 got = d->Received();
			
			{
				PRINT("\tWDownloadEvent::FileDataReceived\n");
				PRINT2("\tOffset: " UINT64_FORMAT_SPEC "\n", offset);
				PRINT2("\tSize  : " UINT64_FORMAT_SPEC "\n", size);
				PRINT2("\tGot   : %lu\n", got);
				gWin->UpdateReceiveStats(got);
				
				double secs = 0.0f;
				
				if (dt->fLastData.elapsed() > 0)
				{
					secs = (double)((double)dt->fLastData.elapsed() / 1000.0f);
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
				
				item->setText(WTransferItem::Status, tr("Downloading: [%1%]").arg(ComputePercentString(offset, size)));
				item->setText(WTransferItem::Received, fromULongLong(offset));
				// <postmaster@raasu.org> 20021104, 20030217, 20030622
				// elapsed time >= 1 s?
				if (secs >= 1.0f)
				{
					dt->SetMostRecentRate(kps);
					dt->fLastData.restart();
				}
				else
				{
					dt->SetPacketCount(gotk);
				}
				// <postmaster@raasu.org> 20021026 -- Too slow transfer rate?
				double gcr = dt->GetCalculatedRate();
				
				uint64 _fileStarted = dt->GetStartTime();
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
				
				item->setText(WTransferItem::ETA, dt->GetETA(offset / 1024, size / 1024, gcr));
				
				item->setText(WTransferItem::Rate, QString::number(gcr*1024.0f));
				
				if (d->Done())
				{
					item->setText(WTransferItem::Status, tr("File finished."));
					item->setText(WTransferItem::ETA, QString::null);
					
					if (dt->IsFinished())
					{
						dt->Reset();
					}
				}
				PRINT("\tWDownloadEvent::FileDataReceived OK\n");
			}
			break;
		}
	}
}

void 
WDownload::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Q3Accel::stringToKey(tr("F11")))
		hide();
	else
		QDialog::keyPressEvent(event);
}

void
WDownload::KillLocalQueues()
{
	Lock();
	unsigned int j = 0;
	while (j < fDownloadList.GetNumItems())
	{
		DLPair pair;
		fDownloadList.GetItemAt(j, pair);
		if (pair.thread)
		{
			if (pair.thread->IsLocallyQueued())	// found queued item?
			{
				// Put all files in resume list
				int n = pair.thread->GetCurrentNum();
				
				if (n > -1) 
				{
					for (int i = n; i < pair.thread->GetNumFiles(); i++)
					{
						emit FileInterrupted(pair.thread->GetFileName(i), pair.thread->GetLocalFileName(i), pair.thread->GetPath(i), pair.thread->GetRemoteUser());
					}
				}
				
				// free it
				delete pair.item;
				pair.thread->Reset();
				delete pair.thread;
				fDownloadList.RemoveItemAt(j);
			}
			else
			{
				j++;
			}
		}
	}
	Unlock();
}

QString
WDownload::GetUserName(WDownloadThread *dt)
{
	QString sid = dt->GetRemoteID();
	QString name = dt->GetRemoteUser();
	QString ret;
	ret = tr("%1 (%2)").arg(name).arg(sid);
	return ret;
}


/*
*
* Block users that disconnect if user has chosen that feature in settings
*
*/
void
WDownload::UserDisconnected(const WUserRef & uref)
{
	QString sid = uref()->GetUserID();
	unsigned int i;

	Lock();
	for (i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		DLPair pair;
		fDownloadList.GetItemAt(i, pair);
		if (pair.thread)
		{
			if (pair.thread->GetRemoteID() == sid)
			{
				if (pair.thread->IsTunneled())
					pair.thread->Reset();
			}
		}
	}
	Unlock();
}

QString
WDownload::FormatIndex(int32 cur, int32 num)
{
	return tr("%1 of %2").arg(cur+1).arg(num);
}

void
WDownload::DLPopupActivated(int id)
{
	DLPair pair;
	unsigned int index;
	WDownloadThread * dt = NULL;
	bool findRet;
	
	findRet = FindDLItem(index, fDLPopupItem);
	
	if (findRet)
	{
		fDownloadList.GetItemAt(index, pair);
	}
	else
	{
		PRINT("Download item not found!");
		return;
	}
	
	dt = pair.thread;
	WASSERT(dt != NULL, "Download Thread is invalid!");
	
	switch (id)
	{
	case ID_QUEUE:
		{
			if (dt->IsLocallyQueued())
			{
				dt->SetLocallyQueued(false);
				dt->SetManuallyQueued(false);
				dt->InitSession();
			}
			else
			{
				dt->SetLocallyQueued(true);
				dt->SetManuallyQueued(true);
				dt->Reset();
			}
			break;
		}
		
	case ID_MOVEUP:
		{
			if (dt->IsLocallyQueued() || dt->IsRemotelyQueued())
				DLMoveUp(index);
			break;
		}
		
	case ID_MOVEDOWN:
		{
			if (dt->IsLocallyQueued() || dt->IsRemotelyQueued())
				DLMoveDown(index);
			break;
		}
		
	case ID_CLEAR:
		{
			SendSignal(ClearDownloads);
			break;
		}
		
	case ID_CANCEL:
		{
			Lock();
			// found our item, stop it
			fDLPopupItem->setText(WTransferItem::Status, tr("Canceled."));
			dt->SetFinished(true);
			dt->Reset();
			Unlock();
			
			if (gWin->fSettings->GetAutoClear())
			{
				SendSignal(ClearDownloads);
			}
			
			SendSignal(DequeueDownloads);
			break;
		}
		
	case ID_NO_LIMIT:
		{
			dt->SetRate(0);
			break;
		}
		
	case ID_64:
		{
			dt->SetRate(64);
			break;
		}
		
	case ID_128:
		{
			dt->SetRate(128);
			break;
		}
		
	case ID_256:
		{
			dt->SetRate(256);
			break;
		}
		
	case ID_512:
		{
			dt->SetRate(512);
			break;
		}
		
	case ID_1KB:
		{
			dt->SetRate(1024);
			break;
		}
		
	case ID_2KB:
		{
			dt->SetRate(2 * 1024);
			break;
		}
		
	case ID_4KB:
		{
			dt->SetRate(4 * 1024);
			break;
		}
		
	case ID_6KB:
		{
			dt->SetRate(6 * 1024);
			break;
		}
		
	case ID_8KB:
		{
			dt->SetRate(8 * 1024);
			break;
		}
		
	case ID_10KB:
		{
			dt->SetRate(10 * 1024);
			break;
		}
		
	case ID_12KB:
		{
			dt->SetRate(12 * 1024);
			break;
		}
		
	case ID_14KB:
		{
			dt->SetRate(14 * 1024);
			break;
		}
		
	case ID_16KB:
		{
			dt->SetRate(16 * 1024);
			break;
		}
		
	case ID_32KB:
		{
			dt->SetRate(32 * 1024);
			break;
		}
		
	case ID_64KB:
		{
			dt->SetRate(64 * 1024);
			break;
		}
		
	case ID_128KB:
		{
			dt->SetRate(128 * 1024);
			break;
		}
		
	case ID_256KB:
		{
			dt->SetRate(256 * 1024);
			break;
		}
		
	case ID_512KB:
		{
			dt->SetRate(512 * 1024);
			break;
		}
		
	case ID_1MB:
		{
			dt->SetRate(1024 * 1024);
			break;
		}
		
	case ID_2MB:
		{
			dt->SetRate(2 * 1024 * 1024);
			break;
		}
		
	case ID_4MB:
		{
			dt->SetRate(4 * 1024 * 1024);
			break;
		}
		
	case ID_8MB:
		{
			dt->SetRate(8 * 1024 * 1024);
			break;
		}
		
	case ID_16MB:
		{
			dt->SetRate(16 * 1024 * 1024);
			break;
		}
		
	case ID_32MB:
		{
			dt->SetRate(32 * 1024 * 1024);
			break;
		}
	default:
		{
			// Handle "Run..." items here...
			
			if (id & ID_RUN)
			{
				int n = id - ID_RUN - 1;
				RunCommand(dt->GetLocalFileName(n));
			}
		}
		
	}
	SendSignal(DLRatings);
}


void
WDownload::DLRightClicked(Q3ListViewItem * item, const QPoint & p, int)
{
	if (item)
	{
		fDLPopupItem = item;
		
		unsigned int index;
		if (FindDLItem(index, item))
		{
			DLPair pair;
			fDownloadList.GetItemAt(index, pair);
			if ( !pair.thread )
			{
				PRINT("Generic Thread is invalid!");
				return;
			}
			
			// Disable items not applicable to current thread state
			
			// Not Locally Queued
			
			if (pair.thread->IsLocallyQueued() == false)
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
			
			if (pair.thread->IsFinished() == true)
			{
				fDLPopup->setItemEnabled(ID_THROTTLE, false);
				fDLPopup->setItemEnabled(ID_QUEUE, false);
				fDLPopup->setItemEnabled(ID_RUN, true);
				fDLPopup->setItemEnabled(ID_CANCEL, false);
			}
			else
			{
				if (pair.thread->IsTunneled() == false)
					fDLPopup->setItemEnabled(ID_THROTTLE, true);
				else
					fDLPopup->setItemEnabled(ID_THROTTLE, false);
				fDLPopup->setItemEnabled(ID_RUN, false);
				fDLPopup->setItemEnabled(ID_CANCEL, true);

				// Still connecting?

				if (pair.thread->IsConnecting() == true)
				{
					fDLPopup->setItemEnabled(ID_QUEUE, false);
				}
				else
				{
					fDLPopup->setItemEnabled(ID_QUEUE, true);
				}
			}
			
			fDLPopup->setItemChecked(ID_QUEUE, pair.thread->IsLocallyQueued());
			int fNewRate = pair.thread->GetRate();
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
			int n = pair.thread->GetNumFiles();
			for (int r = 0; r < n; r++)
			{
				if (!pair.thread->GetLocalFileName(r).isEmpty())
					fDLRunMenu->insertItem(pair.thread->GetLocalFileName(r), ID_RUN+1+r);
			}
			fDLPopup->popup(p);
		}
	}
}

bool
WDownload::FindDLItem(unsigned int & index, Q3ListViewItem * s)
{
	PRINT("WDownload::FindDLItem()\n");
	bool success = false;
	
	Lock();
	
	for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		DLPair p;
		fDownloadList.GetItemAt(i, p);
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
		PRINT("WDownload::FindDLItem() : Item not found!\n");
	}
	return success;
}


void
WDownload::DLMoveUp(unsigned int index)
{
	if (index == 0)
		return;
	
	DLPair wp;
	
	Lock();
	
	unsigned int index2 = index;
	fDownloadList.RemoveItemAt(index, wp);
	Unlock();
	
	Lock();
	while (index2 > 0) 
	{
		index2--;
		DLPair p;
		fDownloadList.GetItemAt(index2, p);
		if (p.thread->IsLocallyQueued())
			break;
	}
	fDownloadList.InsertItemAt(index2, wp);
	
	Unlock();
}

void
WDownload::DLMoveDown(unsigned int index)
{
	if (index == fDownloadList.GetNumItems() - 1)
		return;
	
	DLPair wp;
	
	Lock();
	
	unsigned int index2 = index;
	fDownloadList.RemoveItemAt(index, wp);
	Unlock();
	
	Lock();
	while (index2 < (fDownloadList.GetNumItems() - 2))
	{
		index2++;
		DLPair p;
		fDownloadList.GetItemAt(index2, p);
		if (p.thread->IsLocallyQueued())
			break;
	}
	
	index2++;
	
	fDownloadList.InsertItemAt(index2, wp);
	
	Unlock();
}

void
WDownload::UpdateDLRatings()
{
	PRINT("\tWDownload::UpdateDLRatings\n");
	if ( fDownloadList.IsEmpty() )
	{
		PRINT("\tDownload list is empty!\n");
		return;
	}
	
	Lock();
	for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		DLPair pair;
		fDownloadList.GetItemAt(i, pair);
		if (pair.item)
		{
			pair.item->setText(WTransferItem::QR, QString::number(i));
		}
	}
	Unlock();
	
	PRINT("\tWDownload::UpdateDLRatings OK\n");
}

void
WDownload::TransferCallBackRejected(const QString &qFrom, int64 timeLeft, uint32 port)
{
	Lock();
	for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
	{
		DLPair pair;
		fDownloadList.GetItemAt(i, pair);
		if (pair.thread)
		{
			if (
				(pair.thread->IsActive() == false) &&
				(pair.thread->GetRemoteUser() == qFrom) && 
				(pair.thread->GetRemotePort() == port)
				)
			{
				pair.thread->SetBlocked(true, timeLeft);
				break;
			}
		}
	}
	Unlock();
}

// Clear all finished downloads from listview
void
WDownload::ClearFinishedDL()
{
	PRINT("\tWDownload::ClearFinishedDL\n");
	
	if (fDownloadList.GetNumItems() > 0)
	{
		Lock();
		unsigned int i = 0;
		while (i < fDownloadList.GetNumItems())
		{
			DLPair pair;
			fDownloadList.GetItemAt(i, pair);
			if ((pair.thread->IsFinished() == true) && (pair.thread->IsManuallyQueued() == false))
			{
				// found finished item, erase it
				fDownloadList.RemoveItemAt(i);
				
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
		Unlock();
	}
	
	SendSignal(DLRatings);
	
	PRINT("\tWDownload::ClearFinishedDL OK\n");
}

int
WDownload::GetNumDownloads()
{
	PRINT("\tWDownload::GetNumDownloads\n");
	int n = 0;
	if ( fDownloadList.GetNumItems() > 0)
	{
		Lock();
		for (unsigned int i = 0; i < fDownloadList.GetNumItems(); i++)
		{
			DLPair pair;
			fDownloadList.GetItemAt(i, pair);
			if (pair.thread)
			{
				if (
					(pair.thread->IsLocallyQueued() == false) &&
					(pair.thread->IsRemotelyQueued() == false) &&
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

void
WDownload::SetLocalID(const QString &sid)
{
	fLocalSID = sid;
}

void
WDownload::SendSignal(int signal)
{
	QCustomEvent *qce = new QCustomEvent(signal);
	if (qce) 
		QApplication::postEvent(this, qce);
}


NetClient *
WDownload::netClient()
{
	return gWin->fNetClient;
}

void 
WDownload::resizeEvent(QResizeEvent * e)
{
	fDownloads->resize(e->size());
	QDialog::resizeEvent(e);
}

void 
WDownload::Lock() 
{ 
	fLock.Lock(); 
}

void 
WDownload::Unlock() 
{ 
	fLock.Unlock(); 
}

