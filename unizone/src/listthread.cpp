#include <qapplication.h>

#include "events.h"
#include "filethread.h"
#include "listthread.h"
#include "netclient.h"
#include "settings.h"
#include "util.h"
#include "winsharewindow.h"

#include "zlib/ZLibUtilityFunctions.h"


WListThread::WListThread(NetClient *net, WFileThread * ft, QObject *owner, bool *optShutdownFlag)
	: QObject(owner), Thread(), fLocker()
{
	fNet = net;
	fFileScanThread = ft;
	fOwner = owner;
	fShutdownFlag = optShutdownFlag;

}

WListThread::~WListThread()
{
}

void
WListThread::InternalThreadEntry()
{
	Lock();
	WinShareWindow * win = dynamic_cast<WinShareWindow *>(fOwner);
	
	// reset our shutdown flag
	if (fShutdownFlag)
		*fShutdownFlag = false;
	
	fFileScanThread->Lock();
	int numShares = fFileScanThread->GetNumFiles();
	if (win->Settings()->GetInfo())
		SystemEvent(fOwner, tr("Sharing %1 file(s).").arg(numShares));
	fNet->SetFileCount(numShares);
	PRINT("Doing a scan of the returned files for uploading.\n");
	int m = 0;
	MessageRef refScan(GetMessageFromPool(PR_COMMAND_SETDATA));
	
	if (refScan())
	{
		MessageRef mref;
		HashtableIterator<String, QString> filesIter = fFileScanThread->GetSharedFiles().GetIterator();
		while (filesIter.HasMoreKeys())
		{
			// stop iterating if we are waiting for file scan thread to finish
			if (fShutdownFlag && *fShutdownFlag)
				break;
			
			String s;
			MessageRef mref;
			filesIter.GetNextKey(s);
			
			if (fFileScanThread->FindFile(s, mref))
			{
				MakeNodePath(s);
				uint32 enc = win->Settings()->GetEncoding(GetServerName(win->CurrentServer()), GetServerPort(win->CurrentServer()));
				// Use encoded file attributes?
				if (enc != 0)
				{
					MessageRef packed = DeflateMessage(mref, enc, true);
					if (packed())
					{
						refScan()->AddMessage(s, packed);
						m++;
					}
					else	
					{
						// Failed to pack the message?
						refScan()->AddMessage(s, mref);
						m++;
					}
				}
				else
				{
					refScan()->AddMessage(s, mref);
					m++;
				}
				if (m == 20)
				{
					m = 0;
					fNet->SendMessageToSessions(refScan, 0);
					refScan = GetMessageFromPool(PR_COMMAND_SETDATA);
				}
			}
		}
		if (refScan())
		{
			if (!refScan()->IsEmpty())
			{
				fNet->SendMessageToSessions(refScan, 0);
				refScan.Reset();
			}
		}
	}
	
	fFileScanThread->Unlock();
	
	QCustomEvent *qce = new QCustomEvent(ListDone);
	if (qce)
	{
		QApplication::postEvent(fOwner, qce);
	}
	Unlock();
}
