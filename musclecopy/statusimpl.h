#ifndef STATUS_H
#define STATUS_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "status.h"
#include "downloadthread.h"
#include "system/SetupSystem.h"
#include <qevent.h>
#include <qlayout.h>


class Status : public StatusBase
{ 
    Q_OBJECT

public:
    Status( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~Status();

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

protected:
	virtual void customEvent(QCustomEvent *);
	virtual void resizeEvent(QResizeEvent * e)
	{
		Layout18->mainWidget()->resize(e->size());
	}


private slots:
	void StartTransfer();
private:
	void AddDownload(QString * files, int32 filecount, uint32 remotePort, QString remoteIP);

	WDownloadThread *gt;
	muscle::CompleteSetupSystem fMuscle;
};

#endif // STATUS_H
