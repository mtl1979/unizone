#include "statusimpl.h"
#include "wgenericevent.h"
#include "downloadthread.h"
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qdir.h>

Status::Status(QWidget* parent, const char* name, bool modal, WFlags fl) 
{
	gt = NULL;

	QDir dir("./");
	dir.mkdir("downloads");

	connect(buttonStart, SIGNAL(clicked()), this, SLOT(StartTransfer()));
}

Status::~Status()
{
}

void
Status::StartTransfer()
{
	if (gt)
		return;
	QString * files = new QString[1];
	files[0] = File->text();
	AddDownload(files, 1, Port->text().toUInt(), Host->text());
}

void
Status::customEvent(QCustomEvent *e)
{
	WGenericEvent * g = NULL;
	if (e->type() == WGenericEvent::Type)
	{
		g = dynamic_cast<WGenericEvent *>(e);
	}
	
	if (g)
	{
		MessageRef msg = g->Msg();
		
		if (!msg())
			return; // Invalid MessageRef!
		
		switch (msg()->what)
		{
		case WGenericEvent::Init:
			{
				const char * filename, * user;
				if (
					(msg()->FindString("file", &filename) == B_OK) && 
					(msg()->FindString("user", &user) == B_OK)
					)
				{
				}
				break;
			}
			
		case WGenericEvent::FileQueued:
			{
				setCaption(tr("Remotely Queued."));
				
				break;
			}
			
		case WGenericEvent::FileBlocked:
			{
				uint64 timeLeft = (uint64) -1;
				(void) msg()->FindInt64("timeleft", (int64 *) &timeLeft);
				if (timeLeft == -1)
					setCaption(tr("Blocked."));
				else
					setCaption(tr("Blocked for %1 minute(s).").arg((int) (timeLeft/60000000)));
				break;
			}
			
		case WGenericEvent::ConnectBackRequest:
			{
				gt->Reset();	// failed...
				break;
			}
			
		case WGenericEvent::FileHashing:
			{
				setCaption(tr("Examining for resume..."));
				break;
			}
			
		case WGenericEvent::ConnectInProgress:
			{
				setCaption(tr("Connecting..."));
				break;
			}
			
		case WGenericEvent::ConnectFailed:
			{
				String why, mFile;
				msg()->FindString("why", why);
				setCaption(tr("Connect failed: %1").arg(why.Cstr()));
				gt->Reset();
				delete gt;
				gt = NULL;
				break;
			}
			
		case WGenericEvent::Connected:
			{
				setCaption(tr("Negotiating..."));
				break;
			}
			
		case WGenericEvent::Disconnected:
			{
				if (caption() != tr("Finished."))
				{
					setCaption(tr("Disconnected."));
				}
				
				if (gt->IsManuallyQueued())
				{
					setCaption(tr("Manually Queued."));
				}
				else
				{
					gt->SetFinished(true);
					bool f;
					if (msg()->FindBool("failed", &f) == B_OK)
					{
						// "failed" == true only, if the transfer has failed
						if (f)
						{
							setCaption(tr("Failed."));
						}
						else
						{
							setCaption(tr("Finished."));
						}
					}
				}
				gt->Reset();
				delete gt;
				gt = NULL;
				break;
			}
			
		case WGenericEvent::FileDone:
			{
				bool d;
				if (msg()->FindBool("done", &d) == B_OK)
				{
					setCaption(tr("Finished."));
				}
				else
				{
					setCaption(tr("Waiting..."));
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
				
				if (
					(msg()->FindString("file", file) == B_OK) && 
					(msg()->FindInt64("start", (int64 *)&start) == B_OK) &&
					(msg()->FindInt64("size", (int64 *)&size) == B_OK) && 
					(msg()->FindString("user", user) == B_OK)
					)
				{
					QString uname;
					uname = gt->GetRemoteIP();
					setCaption(tr("Waiting for stream..."));
					// rec, total, rate
					ProgressBar1->setTotalSteps(size);
					ProgressBar1->setProgress(start);
					
				}
				gt->fLastData.start();
				break;
			}
			
		case WGenericEvent::UpdateUI:
			{
				const char * id;
				if (msg()->FindString("id", &id) == B_OK)
				{
				}
				break;
			}
			
		case WGenericEvent::FileError:
			{
				String why;
				String file;
				msg()->FindString("why", why);
				if (msg()->FindString("file", file) == B_OK)
				{
				}
				setCaption(tr("Error: %1").arg(why.Cstr()));
				break;
			}
			
		case WGenericEvent::FileDataReceived:
			{
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
						
						ProgressBar1->setProgress(offset);
						
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
						
						setCaption(tr("Downloading: [%1%]").arg(gt->ComputePercentString(offset, size)));
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
						
						
						if (msg()->FindBool("done", &done) == B_OK)
						{
							setCaption(tr("File finished."));
							
							if (msg()->FindString("file", mFile) == B_OK)
							{
							}
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
Status::AddDownload(QString * files, int32 filecount, uint32 remotePort, QString remoteIP)
{
	gt = new WDownloadThread(this);
	CHECK_PTR(gt);
	gt->SetFile(files, filecount, remoteIP, remotePort, false, true);
	
	gt->InitSession();
	gt->SetLocallyQueued(false);
}
