#ifndef UPLOADTHREAD_H
#define UPLOADTHREAD_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "genericthread.h"
#include "filethread.h"
#include "iogateway/MessageIOGateway.h"
#include "reflector/RateLimitSessionIOPolicy.h"

#include <qobject.h>
#include <qstring.h>


/** This is my wonderful file upload thread. It handles starting the
  *	connection remote peer and transfering a file to him.
  */
class WUploadThread : public WGenericThread
{
public:
	WUploadThread(QObject * owner, bool * optShutdownFlag = NULL);
	virtual ~WUploadThread();

	void SetUpload(int socket, uint32 remoteIP, WFileThread * ft);
	void SetUpload(QString remoteIP, uint32 remotePort, WFileThread * ft);
	void InitSession();

	QString GetRemoteID() {return fRemoteSessionID;}
	QString GetRemoteUser() {return fRemoteUser;}
	QString GetCurrentFile() {return fFileUl;}
	QString GetFileName(int i);

	int32 GetCurrentNum() { return fCurFile; }
	int32 GetNumFiles() { return fNumFiles; }

	virtual void SetBlocked(bool b, int64 timeLeft = -1);
	virtual void SetLocallyQueued(bool b);
	virtual void SetManuallyQueued(bool b);

	void SetRate(int rate);
	void SetRate(int rate, AbstractReflectSessionRef ref);


protected:
	virtual void SendReply(Message * m);
	virtual void SignalOwner();
	void SendQueuedNotification();
	void SendRejectedNotification();

private:
	WMsgList fUploads;
	WStrList fNames;
	QFile * fFile;
	uint32 fRemoteIP;
	QString fStrRemoteIP;				// the accept version gets a string IP
	uint32 fPort;						// port for accept version
	int fSocket;
	uint64 fFileSize;
	uint64 fCurrentOffset;
	int fMungeMode;
	QString fRemoteSessionID;
	QString fRemoteUser;
	QString fFileUl;
	WFileThread * fFileThread;
	MessageRef fCurrentRef;
	bool fWaitingForUploadToFinish;
	bool fAccept;						// is this the accept version?

	int32 fCurFile, fNumFiles;

	QTimer * CTimer;					// Connect timer
	QTimer * fBlockTimer;				// blocked timer

	void DoUpload();
};


#endif
