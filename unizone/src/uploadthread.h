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
#include <qfile.h>


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
	QString GetRemoteUser(); 
	QString GetRemoteIP() {return fStrRemoteIP;}
	uint32 GetRemotePort() {return fPort;}
	QString GetCurrentFile() {return fFileUl;}
	QString GetFileName(int i);

	int32 GetCurrentNum() { return fCurFile; }
	int32 GetNumFiles() { return fNumFiles; }

	virtual void SetBlocked(bool b, int64 timeLeft = -1);
	virtual void SetLocallyQueued(bool b);
	virtual void SetManuallyQueued(bool b);

	void SetRate(int rate);
	void SetRate(int rate, AbstractReflectSessionRef ref);

	void SessionConnected(const String &sessionID);

public slots:

	void ServerExited();
	void SessionDisconnected(const String &sessionID);
	void MessageReceived(MessageRef msg, const String &sessionID);
	void OutputQueuesDrained(MessageRef msg);

protected:
	virtual void SendReply(MessageRef &m);
	void SendQueuedNotification();
	void SendRejectedNotification(bool);

private:
	Queue<MessageRef> fUploads;
	Queue<QString> fNames;
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
	bool fForced;						// did this transfer bypass queue?

	int32 fCurFile, fNumFiles;

	String _sessionID;

	void DoUpload();
};


#endif
