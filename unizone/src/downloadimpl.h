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
#include <qpopupmenu.h>

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
	WDownload(QWidget * parent, QString localID, WFileThread * sharedFiles);
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
		TransferCommandPeerID,
		TransferNotifyRejected
	};

	void AddDownload(QString * files, int32 numFiles, QString remoteSessionID, uint32 remotePort,
						QString remoteIP, uint64 remoteInstallID, bool firewalled, bool partial);
	void AddDownloadList(Queue<QString> & fQueue, WUser * user);

	void AddUpload(int socket, uint32 remoteIP, bool queued);
	void AddUpload(QString remoteIP, uint32 port);

	void DequeueDLSessions();
	void DequeueULSessions();
	void KillLocalQueues();

	void TransferCallBackRejected(QString qFrom, int64 timeLeft, uint32 port);

	void SetLocalID(QString sid);


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
	//typedef map<WGenericThread *, WTransferItem *> WTList;
	typedef pair<WGenericThread *, WTransferItem *> WTPair;
	typedef list<WTPair> WTList;
	typedef WTList::iterator WTIter;

	WTList fDownloadList;
	WTList fUploadList;

	QSplitter * fMainSplit;
	QListView * fUploads, * fDownloads;
	QPushButton * fCancelU;
	QPushButton * fCancelD;
	QVBox * fBoxU, * fBoxD;
	QPopupMenu * fDLPopup, *fULPopup;
	QPopupMenu * fDLThrottleMenu, * fULThrottleMenu;
	QPopupMenu * fDLRunMenu;
	QPopupMenu * fULBanMenu;

	int fDLThrottle, fULThrottle;	// Current throttle selections
	int fULBan;						// current ban selection

	QListViewItem * fDLPopupItem;	// download item that was right clicked
	QListViewItem * fULPopupItem;	//   upload item that was right clicked

	QString fLocalSID;
	WFileThread * fSharedFiles;
	int fNumUploads, fNumDownloads;

	QString GetUserName(QString);
	QString FormatIndex(long cur, long num);

	// Simple method that is used to decrease the download/upload count
	// when one is canceled or finished. Returns the count after everything
	// has been done.
	int DecreaseCount(WGenericThread *, int &, bool = true);
	void UpdateLoad();

	// Popup menu id's
	enum
	{
		ID_QUEUE,
		ID_NO_LIMIT,
		ID_64,
		ID_128,
		ID_256,
		ID_512,
		ID_1KB,
		ID_2KB,
		ID_4KB,
		ID_6KB,
		ID_8KB,
		ID_10KB,
		ID_12KB,
		ID_14KB,
		ID_16KB,
		ID_32KB,
		ID_64KB,
		ID_128KB,
		ID_256KB,
		ID_512KB,
		ID_1MB,
		ID_2MB,
		ID_4MB,
		ID_8MB,
		ID_16MB,
		ID_32MB,
		ID_UNBAN,
		ID_BAN1,
		ID_BAN2,
		ID_BAN5,
		ID_BAN10,
		ID_BAN15,
		ID_BAN30,
		ID_BAN1H,
		ID_BANINF,
		ID_MOVEUP,
		ID_MOVEDOWN,
		ID_IGNORE,
		ID_CLEAR,
		ID_CANCEL,
		ID_THROTTLE,
		ID_BLOCK
	};

	enum
	{
		ID_RUN = 1024
	};

	// Find an item in the list that matches the list view item
	// and return the WTIter
	bool FindItem(WTList &, WTIter &, QListViewItem *);

	// Reorganize transfer queue
	void MoveUp(WTList & lst, WTIter & iter);
	void MoveDown(WTList & lst, WTIter & iter);

	// Update Queue Ratings
	void UpdateDLRatings();
	void UpdateULRatings();

	// Clear finished downloads from listview
	void ClearFinishedDL();

	// Clear finished uploads from listview
	void ClearFinishedUL();

	// Get number of active transfers
	int GetNumDownloads();
	int GetNumUploads();

	QMutex fLock;

private slots:
	//void CancelDL();
	//void CancelUL();
	//void UnblockUL();

	void DLPopupActivated(int);
	void ULPopupActivated(int);

	void DLRightClicked(QListViewItem *, const QPoint &, int);
	void ULRightClicked(QListViewItem *, const QPoint &, int);

public slots:
	void UserDisconnected(QString, QString);

signals:
	void FileFailed(QString, QString); // Parameter 1 = File Name, Parameter 2 = User Name
	void FileInterrupted(QString, QString);
	void Closed();		// the download window has been closed

};

#endif
