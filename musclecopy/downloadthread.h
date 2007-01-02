#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include "genericthread.h"
#include "wfile.h"

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

	void SetFile(QString * files, int32 numFiles, QString fromIP,  uint32 remotePort, bool firewalled, bool partial);
	void NextFile();
	int32 GetCurrentNum() { return fCurFile; }
	int32 GetNumFiles() { return fNumFiles; }

	WFile * GetFile() const { return fFile; }
	QString GetCurrentFile() { return fFileDl[fCurFile]; }
	QString GetFileName(int i) { return fFileDl[i]; }
	QString GetLocalFileName(int i) { return fLocalFileDl[i]; }


	static QString FixFileName(const QString & fixMe);

	QString GetRemoteUser() { return fFromUser; }
	QString GetRemoteIP() { return fIP; }
	uint32 GetRemotePort() { return fPort; }

	// call this after setting the file to init the download
	// this will also send a message to show the dialog, so do not do it yourself
	bool InitSession();

	void SetRate(int rate);
	void SetRate(int rate, AbstractReflectSessionRef ref);

	void SetBlocked(bool b, int64 timeLeft = -1);

protected:
	QMutex fLockFile;
	WFile * fFile;			// file on the HD
	QString * fFileDl;		// file to dl
	QString * fLocalFileDl; // local filenames for downloaded files
	QString fIP;			// ip address of remote client
	QString fFromUser;		// user name of remote client
	uint32 fPort;			// port of the remote client (the one it's listening on)
	int32 fAcceptingOn;		// port we're accepting on in case the user is firewalled
	int64 fCurrentOffset;		// current offset in the file
	int64 fFileSize;		// the file size
	time_t fCurrentFileStartTime;
	bool fDownloading;
	bool fFirewalled;
	bool fPartial;
	int32 fNumFiles, fCurFile;

	virtual void SendReply(MessageRef &m);

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
