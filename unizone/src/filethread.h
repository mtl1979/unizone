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

class ScanProgress;


// This class runs through a list of paths and parses each
// directory for files to search
class WFileThread : public QThread
{
public:
	WFileThread(NetClient * net, QObject * owner, bool * optShutdownFlag = NULL);

	virtual ~WFileThread();

	void SetFirewalled(bool f) { fFired = f; }
	int GetNumFiles() const;

	Queue<MessageRef> & GetSharedFiles() { return fFiles; }
	MessageRef GetSharedFile(int n);
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
	Queue<MessageRef> fFiles;
	Queue<QString> fScannedDirs;

	bool CheckFile(const QString & file);	// checks whether this file has been found or not
											// returns true if the file already exists
	void AddFile(const QString & filePath); // Add this file or (files in this) directory to scanned files

	/*
	struct FileInfo
	{
		uint32 fModificationTime;
		QString fMIME;
	};
	*/

	void ParseDir(const QString & d);
	QString ResolveLink(const QString & lnk);
	QString ResolveLinkA(const QString & lnk);	// Windows only
	void ScanFiles(QString directory);

	ScanProgress * fScanProgress;

	QMutex fLocker;
};

#endif
