#ifndef DOWNLOADIMPL_H
#define DOWNLOADIMPL_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <system/MessageTransceiverThread.h>
#include <util/Queue.h>
using namespace muscle;

#include <map>
using std::map;
using std::pair;
using std::iterator;
#include <qfile.h>
#include <qdialog.h>
#include <qlistview.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <qvbox.h>

#include "filethread.h"
#include "transferitem.h"

class MD5Thread;
class MD5Looper;
class WGenericThread;
class WDownloadThread;
class WUploadThread;

// This class needs to be able to handle downloads AND uploads
class WDownload : public QDialog
{
	Q_OBJECT
public:
	WDownload(QString localID, WFileThread * sharedFiles);
	virtual ~WDownload();

	// ways to disguise data
	enum 
	{
		MungeModeNone = 0,
		MungeModeXOR,
		MungeModeNum	// amount of modes
	};

	// transfer commands
	enum
	{
		TransferConnectedToPeer = 'tshr',	// um... I don't use this one
		TransferDisconnectedFromPeer,		// nor this one... I have my own events
		TransferFileList,
		TransferFileHeader,
		TransferFileData,
		TransferDeprecated,					// OLD beshare message
		TransferNotifyQueued,
		TransferMD5SendReadDone,			// these are sent by the MD5 looper...
		TransferMD5RecvReadDone,
		TransferCommandPeerID
	};

	void AddDownload(QString file, QString remoteSessionID, uint32 remotePort,
						QString remoteIP, uint64 remoteInstallID, bool firewalled, bool partial);
	void AddUpload(int socket, uint32 remoteIP, bool queued);
	void AddUpload(QString remoteIP, uint32 port);

	void DequeueSessions();
	void KillLocalQueues();
	
protected:
	virtual void customEvent(QCustomEvent *);
	virtual void resizeEvent(QResizeEvent * e)
	{
		fMainSplit->resize(e->size());
	}

private:
	typedef map<MD5Thread *, bool> MD5List;
	typedef pair<MD5Thread *, bool> MD5Pair;
	typedef MD5List::iterator MD5Iter;
	typedef map<WGenericThread *, WTransferItem *> WTList;
	typedef pair<WGenericThread *, WTransferItem *> WTPair;
	typedef WTList::iterator WTIter;

	WTList fDownloadList;
	WTList fUploadList;

	QSplitter * fMainSplit;
	QListView * fUploads, * fDownloads;
	QPushButton * fCancelU;
	QPushButton * fUnblockU;
	QPushButton * fCancelD;
	QVBox * fBoxU, * fBoxD;
	QHBox * fButtonsU;

	QString fLocalSID;
	WFileThread * fSharedFiles;
	int fNumUploads, fNumDownloads;

	QString GetUserName(QString);
	// Simple method that is used to decrease the download/upload count
	// when one is canceled or finished. Returns the count after everything
	// has been done.
	int DecreaseCount(WGenericThread *, int &, bool = true);
	void UpdateLoad();

	QMutex fLock;

private slots:
	void CancelDL();
	void CancelUL();
	void UnblockUL();

};

#endif
