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

	virtual void SetQueued(bool b);
	virtual void SetBlocked(bool b);

protected:
	virtual void SendReply(Message * m);
	virtual void SignalOwner();
	void SendQueuedNotification();

private:
	WMsgList fUploads;
	QFile * fFile;
	uint32 fRemoteIP;
	QString fStrRemoteIP;	// the accept version gets a string IP
	uint32 fPort;		// port for accept version
	int fSocket;
	uint64 fFileSize;
	uint64 fCurrentOffset;
	int fMungeMode;
	QString fRemoteSessionID;
	WFileThread * fFileThread;
	MessageRef fCurrentRef;
	bool fWaitingForUploadToFinish;
	bool fAccept;		// is this the accept version?

	void DoUpload();
};


#endif
