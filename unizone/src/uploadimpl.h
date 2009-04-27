#ifndef UPLOADIMPL_H
#define UPLOADIMPL_H

#include <qapplication.h>
#include <qfile.h>
#include <qdialog.h>
#include <q3listview.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <q3vbox.h>
#include <q3popupmenu.h>

#include <system/MessageTransceiverThread.h>
#include <util/Queue.h>
using namespace muscle;

#include "transferlist.h"
#include "debugimpl.h"

class WUploadEvent;
class WUploadThread;
class WFileThread;
class WUser;
class WTransferItem;
class NetClient;

typedef Ref<WUser> WUserRef;

// This class needs to be able to handle downloads AND uploads
class WUpload : public QDialog
{
	Q_OBJECT
public:
	WUpload(QWidget * parent, WFileThread * sharedFiles);
	virtual ~WUpload();

	// Internal event codes
	enum
	{
		DequeueUploads = QEvent::User + 11500,
		ClearUploads,
		ULRatings,
		UpLoad
	};

	void AddUpload(const ConstSocketRef & socket, uint32 remoteIP, bool queued);
	void AddUpload(const QString & remoteIP, uint32 port);

	// Upload tunnel
	bool CreateTunnel(const QString & userID, int64 hisID, void * &myID);

	void TunnelMessage(int64 myID, MessageRef tmsg);

	void DequeueULSessions();
	void KillLocalQueues();

	void TransferCallBackRejected(const QString &qFrom, int64 timeLeft, uint32 port);

	void EmptyLists();

	NetClient * netClient();

protected:
	virtual void customEvent(QEvent *);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void uploadEvent(WUploadEvent *);

	virtual void resizeEvent(QResizeEvent * e);

private:
		
	ULList fUploadList;

	Q3ListView * fUploads;
	Q3PopupMenu * fULPopup;
	Q3PopupMenu * fULThrottleMenu;
	Q3PopupMenu * fULBanMenu;
	Q3PopupMenu * fULPacketMenu;
	Q3PopupMenu * fULCompressionMenu;

	int fULThrottle;				// Current throttle selections
	int fULBan;						// Current ban selection
	int fULPacket;					// Current packet size selection
	int fULCompression;				// Current compression selection

	Q3ListViewItem * fULPopupItem;	//   upload item that was right clicked

	WFileThread * fSharedFiles;

	QString GetUserName(WUploadThread *gt);
	QString FormatIndex(int32 cur, int32 num);

	// Simple method that is used to decrease the download/upload count
	// when one is canceled or finished. Returns the count after everything
	// has been done.
	void UpdateLoad();

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
		ID_PACKET512,
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
		ID_SETCOMPRESSION,
		ID_LEVEL0,
		ID_LEVEL1,
		ID_LEVEL2,
		ID_LEVEL3,
		ID_LEVEL4,
		ID_LEVEL5,
		ID_LEVEL6,
		ID_LEVEL7,
		ID_LEVEL8,
		ID_LEVEL9,
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
	bool FindULItem(unsigned int &, Q3ListViewItem *);

	// Reorganize transfer queue

	void ULMoveUp(unsigned int index);
	void ULMoveDown(unsigned int index);

	// Update Queue Ratings
	void UpdateULRatings();

	// Clear finished uploads from listview
	void ClearFinishedUL();

	// Get number of active transfers
	int GetNumUploads();

	// Get number of non-finished transfers (active or queued)
	int GetUploadQueue();

	void SendSignal(int signal);

	mutable Mutex fLock;
	void Lock(); 
	void Unlock();

private slots:

	void ULPopupActivated(int);

	void ULRightClicked(Q3ListViewItem *, const QPoint &, int);

public slots:
	void UserDisconnected(const WUserRef &);

signals:
	// the upload window has been closed
	void Closed();		

};

#endif
