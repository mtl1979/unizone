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
using muscle::Queue;

#include "netclient.h"

typedef list<QString> WStrList;
typedef WStrList::iterator WStrListIter;
typedef list<MessageRef> WMsgList;
typedef WMsgList::iterator WMsgListIter;


// This class runs through a list of paths and parses each
// directory for files to search
class WFileThread : public QThread
{
public:
	WFileThread(NetClient * net, QObject * owner, bool * optShutdownFlag = NULL)
		: QThread(), fNet(net), fOwner(owner), fShutdownFlag(optShutdownFlag) {fRunning = false;}

	virtual ~WFileThread() {}

	void SetFirewalled(bool f) { fFired = f; }
	int GetNumFiles() const;

	WMsgList & GetSharedFiles() { return fFiles; }
	bool FindFile(const QString & file, MessageRef * ref);
	bool IsRunning();
	void EmptyList();

	enum { ScanDone = 'fTsD' };

	void Lock() { fLocker.lock(); }
	void Unlock() { fLocker.unlock(); }

protected:
	virtual void run();

private:
	bool fFired;
	bool fRunning;
	NetClient * fNet;
	QObject * fOwner;
	bool * fShutdownFlag;
	Queue<QString> fPaths;
	WMsgList fFiles;
	WStrList fScannedDirs;

	bool CheckFile(const QString & file);	// checks whether this file has been found or not
											// returns true if the file already exists
	struct FileInfo
	{
		uint32 fModificationTime;
		QString fMIME;
	};

	void ParseDir(const QString & d);
	QString ResolveLink(const QString & lnk);
	void ScanFiles(QString directory);

	QMutex fLocker;
};

#endif
