#ifndef DOWNLOADIMPL_H
#define DOWNLOADIMPL_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qapplication.h>
#include <qfile.h>
#include <qdialog.h>
#include <qlistview.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include <qpopupmenu.h>

#include <system/MessageTransceiverThread.h>
#include <util/Queue.h>
using namespace muscle;

#include <map>
using std::map;
using std::pair;
using std::iterator;

#include "transferlist.h"
#include "debugimpl.h"

class WDownloadEvent;
class WDownloadThread;
class WUploadEvent;
class WUploadThread;
class WFileThread;
class WUser;
class WTransferItem;
class NetClient;

typedef Ref<WUser> WUserRef;

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

	// Internal event codes
	enum
	{
		DequeueDownloads = QEvent::User + 11000,
		DequeueUploads,
		ClearDownloads,
		ClearUploads,
		DLRatings,
		ULRatings,
		UpLoad
	};

	void AddDownload(QString * files, QString * lfiles, int32 numFiles, QString remoteSessionID, uint32 remotePort,
						const QString & remoteIP, uint64 remoteInstallID, bool firewalled, bool partial);
	void AddDownloadList(Queue<QString> & fQueue, Queue<QString> & fLQueue, const WUserRef & user);

	void AddUpload(int socket, uint32 remoteIP, bool queued);
	void AddUpload(const QString & remoteIP, uint32 port);

	// Upload tunnel
	bool CreateTunnel(const QString & userID, int64 hisID, void * &myID);
	// Download tunnel
	bool CreateTunnel(QString * files, QString * lfiles, int32 numFiles, const WUserRef & remoteUser);
	void TunnelAccepted(int64 myID, int64 hisID);
	void TunnelRejected(int64 myID);

	void TunnelMessage(int64 myID, MessageRef tmsg, bool download);

	void DequeueDLSessions();
	void DequeueULSessions();
	void KillLocalQueues();

	void TransferCallBackRejected(const QString &qFrom, int64 timeLeft, uint32 port);

	void SetLocalID(const QString &sid);

	void EmptyLists();

	NetClient * netClient();

protected:
	virtual void customEvent(QCustomEvent *);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void downloadEvent(WDownloadEvent *);
	virtual void uploadEvent(WUploadEvent *);

	virtual void resizeEvent(QResizeEvent * e);

private:
		
	DLList fDownloadList;
	ULList fUploadList;

	QSplitter * fMainSplit;
	QListView * fUploads, * fDownloads;
	QVBox * fBoxU, * fBoxD;
	QPopupMenu * fDLPopup, *fULPopup;
	QPopupMenu * fDLThrottleMenu, * fULThrottleMenu;
	QPopupMenu * fDLRunMenu;
	QPopupMenu * fULBanMenu;
	QPopupMenu * fULPacketMenu;

	int fDLThrottle, fULThrottle;	// Current throttle selections
	int fULBan;						// Current ban selection
	int fULPacket;					// Current packet size selection

	QListViewItem * fDLPopupItem;	// download item that was right clicked
	QListViewItem * fULPopupItem;	//   upload item that was right clicked

	QString fLocalSID;
	WFileThread * fSharedFiles;

	QString GetUserName(WDownloadThread *gt);
	QString GetUserName(WUploadThread *gt);
	QString FormatIndex(long cur, long num);

	// Simple method that is used to decrease the download/upload count
	// when one is canceled or finished. Returns the count after everything
	// has been done.
	void UpdateLoad();

	void EmptyDownloads();
	void EmptyUploads();

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
		ID_SETPACKET,
		ID_PACKET1K,
		ID_PACKET2K,
		ID_PACKET4K,
		ID_PACKET8K,
		ID_PACKET16K,
		ID_PACKET32K,
		ID_PACKET64K,
		ID_PACKET128K,
		ID_PACKET256K,
		ID_PACKET512K,
		ID_PACKET1M,
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
	// and return the index
	bool FindDLItem(unsigned int &, QListViewItem *);
	bool FindULItem(unsigned int &, QListViewItem *);

	// Reorganize transfer queue
	void DLMoveUp(unsigned int index);
	void DLMoveDown(unsigned int index);

	void ULMoveUp(unsigned int index);
	void ULMoveDown(unsigned int index);

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

	// Get number of non-finished transfers (active or queued)
	int GetUploadQueue();

	void SendSignal(int signal);

	mutable Mutex fLock;
	void Lock(); 
	void Unlock();

private slots:

	void DLPopupActivated(int);
	void ULPopupActivated(int);

	void DLRightClicked(QListViewItem *, const QPoint &, int);
	void ULRightClicked(QListViewItem *, const QPoint &, int);

public slots:
	void UserDisconnected(const WUserRef &);

signals:
	// Parameter 1 = Remote File Name, Parameter 2 = Local File Name, Parameter 3 = User Name
	void FileFailed(const QString &, const QString &, const QString &); 
	void FileInterrupted(const QString &, const QString &, const QString &);
	// the download window has been closed
	void Closed();		

};

#endif
