#ifndef FILETHREAD_H
#define FILETHREAD_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qthread.h>
#include <qstring.h>

#include <list>
using std::list;
using std::iterator;

#include "util/Queue.h"
#include "message/Message.h"
using namespace muscle;

#include "scanevent.h"
#include "debugimpl.h"

class ScanProgress;
class NetClient;

// This class runs through a list of paths and parses each
// directory for files to search
class WFileThread : public QThread
{
public:
	WFileThread(NetClient * net, QObject * owner, bool * optShutdownFlag = NULL);

	virtual ~WFileThread();

//	void SetFirewalled(bool f) { fFired = f; }
	int GetNumFiles() const;

	Queue<MessageRef> & GetSharedFiles() { return fFiles; }
	void GetSharedFile(int n, MessageRef & mref);
	bool FindFile(const QString & file, MessageRef & ref);
	void EmptyList();

	enum { ScanDone = 'fTsD' };

	void Lock() 
	{ 		
		fLocker.lock(); 
	}
	void Unlock() { fLocker.unlock(); }

protected:
	virtual void run();

private:
//	bool fFired;
	NetClient * fNet;
	QObject * fOwner;
	bool * fShutdownFlag;
	Queue<QString> fPaths;
	Queue<MessageRef> fFiles;
	Queue<QString> fScannedDirs;
	Queue<QString> files;

	bool CheckFile(const QString & file);	// checks whether this file has been found or not
											// returns true if the file already exists
	void AddFile(const QString & filePath); // Add this file or (files in this) directory to scanned files

	void ParseDir(const QString & d);
	QString ResolveLink(const QString & lnk);
	QString ResolveLinkA(const QString & lnk);	// Windows only
	void ScanFiles(const QString & directory);

	void SendReset();
	void SendString(ScanEvent::Type, QString);
	void SendInt(ScanEvent::Type, int);

	ScanProgress * fScanProgress;

	mutable QMutex fLocker;
};

#endif
