#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include "genericthread.h"

#include <qfile.h>
#include <qstring.h>
#include <qthread.h>
#include <qtimer.h>
#include <time.h>

class WDownloadThread : public WGenericThread
{
public:
	// This thread will throw events to this owner to udpate it's GUI
	WDownloadThread(QObject * owner, bool * optShutdownFlag = NULL);
	virtual ~WDownloadThread();

	void SetFile(QString file, QString fromIP, QString fromSession,
					QString localSession, int32 remotePort, bool firewalled, bool partial);

	QFile * GetFile() const { return fFile; }

	static QString FixFileName(const QString & fixMe);

	QString GetRemoteID() { return fFromSession; }
	QString GetRemoteUser() { return fFromUser; }

	// call this after setting the file to init the download
	// this will also send a message to show the dialog, so do not do it yourself
	bool InitSession();

public slots:
	void ConnectTimer(); // Connection timed out?

protected:
	QMutex fLockFile;
	QFile * fFile;		// file on the HD
	QString fFileDl;	// file to dl
	QString fIP;		// ip address of remote client
	QString fFromSession;	// session ID of remote client
	QString fFromUser;		// user name of remote client
	QString fLocalSession;	// our session ID
	int32 fPort;		// port of the remote client (the one it's listening on)
	int32 fAcceptingOn;	// port we're accepting on in case the user is firewalled
	uint64 fCurrentOffset;	// current offset in the file
	uint64 fFileSize;		// the file size
	time_t fCurrentFileStartTime;
	bool fDownloading;
	bool fFirewalled;
	bool fPartial;

	virtual void SendReply(Message * m);

	virtual void SignalOwner();
private:
	QString UniqueName(QString file, int index); // build up unique name using 'file' and 'index'
	QTimer * CTimer; // Connect timer


};

// subclass ThreadWorkerSessionFactory to do throttling
class WDownloadThreadWorkerSessionFactory : public ThreadWorkerSessionFactory
{
public:
	WDownloadThreadWorkerSessionFactory(int limit);
	virtual AbstractReflectSession * CreateSession(const String &);
	
private:
	int fLimit;
};

#endif
