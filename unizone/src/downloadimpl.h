#ifndef DOWNLOADIMPL_H
#define DOWNLOADIMPL_H

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
	WDownload(QWidget * parent, QString localID);
	virtual ~WDownload();

	// Internal event codes
	enum
	{
		DequeueDownloads = QEvent::User + 11000,
		ClearDownloads,
		DLRatings
	};

	void AddDownload(QString * files, QString * lfiles, QString * paths, QString * tfiles, int32 numFiles, QString remoteSessionID, uint32 remotePort,
						const QString & remoteIP, uint64 remoteInstallID, bool firewalled, bool partial);
	void AddDownloadList(Queue<QString> & fQueue, Queue<QString> & fLQueue, Queue<QString> & fLPaths, Queue<QString> & fTQueue, const WUserRef & user);


	// Download tunnel
	bool CreateTunnel(QString * files, QString * lfiles, QString * lpaths, QString * tfiles, int32 numFiles, const WUserRef & remoteUser);
	void TunnelAccepted(int64 myID, int64 hisID);
	void TunnelRejected(int64 myID);

	void TunnelMessage(int64 myID, MessageRef tmsg);

	void DequeueDLSessions();
	void KillLocalQueues();

	void TransferCallBackRejected(const QString &qFrom, int64 timeLeft, uint32 port);

	void SetLocalID(const QString &sid);

	void EmptyLists();

	NetClient * netClient();

protected:
	virtual void customEvent(QEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void downloadEvent(WDownloadEvent *);

	virtual void resizeEvent(QResizeEvent *);

private:

	DLList fDownloadList;

	QSplitter * fMainSplit;
	Q3ListView * fDownloads;
	Q3PopupMenu * fDLPopup;
	Q3PopupMenu * fDLThrottleMenu;
	Q3PopupMenu * fDLRunMenu;

	int fDLThrottle;	// Current throttle selections

	Q3ListViewItem * fDLPopupItem;	// download item that was right clicked

	QString fLocalSID;

	QString GetUserName(WDownloadThread *gt);
	QString FormatIndex(int32 cur, int32 num);

	void EmptyDownloads();

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
		ID_MOVEUP,
		ID_MOVEDOWN,
		ID_CLEAR,
		ID_CANCEL,
		ID_THROTTLE
	};

	enum
	{
		ID_RUN = 1024
	};

	// Find an item in the list that matches the list view item
	// and return the index
	bool FindDLItem(unsigned int &, Q3ListViewItem *);

	// Reorganize transfer queue
	void DLMoveUp(unsigned int index);
	void DLMoveDown(unsigned int index);

	// Update Queue Ratings
	void UpdateDLRatings();

	// Clear finished downloads from listview
	void ClearFinishedDL();

	// Get number of active transfers
	int GetNumDownloads();

	void SendSignal(int signal);

	mutable Mutex fLock;
	void Lock();
	void Unlock();

private slots:

	void DLPopupActivated(int);

	void DLRightClicked(Q3ListViewItem *, const QPoint &, int);

public slots:
	void UserDisconnected(const WUserRef &);

signals:
	// Parameter 1 = Remote File Name, Parameter 2 = Local File Name, Parameter 3 = Remote Path, Parameter 4 = Temporary name, Parameter 5 = User Name
	void FileFailed(const QString &, const QString &, const QString &, const QString &, const QString &);
	void FileInterrupted(const QString &, const QString &, const QString &, const QString &, const QString &);
	// the download window has been closed
	void Closed();

};

#endif
