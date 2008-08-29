#ifndef FILETHREAD_H
#define FILETHREAD_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qapplication.h>
#include <qstring.h>
#include <qobject.h>

#include <list>
using std::list;
using std::iterator;

#include "util/Queue.h"
#include "message/Message.h"
#include "system/Thread.h"
#include "qtsupport/QMuscleSupport.h"

using namespace muscle;

#include "scanevent.h"
#include "debugimpl.h"

#ifdef WIN32
class ScanProgress;
#endif
class NetClient;

// This class runs through a list of paths and parses each
// directory for files to search
class WFileThread : public QObject, public Thread
{
	Q_OBJECT
public:
	WFileThread(NetClient * net, QObject * owner, bool * optShutdownFlag = NULL);

	virtual ~WFileThread();

	int GetNumFiles() const;

	Hashtable<QString, QString> & GetSharedFiles() { return fFiles; }
	HashtableIterator<QString, QString> GetSharedFilesIterator(uint32 flags) { return fFiles.GetIterator(flags); }
	void GetSharedFile(unsigned int n, MessageRef & mref);
	bool FindFile(const QString & file, MessageRef & ref);
	void EmptyList();

	enum { 
		ScanDone = QEvent::User + 28000 
	};

	void Lock(); 
	void Unlock(); 

protected:
	virtual void InternalThreadEntry();

private:
	NetClient * fNet;
	QObject * fOwner;
	bool * fShutdownFlag;
	Queue<QString> fPaths;
	Hashtable<QString, QString> fFiles;
	QStringList fScannedDirs;
	Queue<QString> files;

	bool CheckFile(const QString & file);	// checks whether this file has been found or not
											// returns true if the file already exists
	void AddFile(const QString & filePath); // Add this file or (files in this) directory to scanned files

	void ParseDir(const QString & d);
	status_t ParseDirAux(QString &);

	friend class WinShareWindow;
	QString ResolveLink(const QString & lnk) const;

	void ScanFiles(const QString & directory);
	bool GetInfo(const QString & file, MessageRef & mref) const;

#ifdef WIN32
	void SendReset();
	void SendString(ScanEvent::Type, const QString &);
	void SendInt(ScanEvent::Type, int);
	void SendShow();
	void SendHide();

	void UpdateFileCount();
	void UpdateFileName(const QString &);

	ScanProgress * fScanProgress;
#endif

	void postEvent( QObject *,QEvent * );

	mutable Mutex fLocker;
};

#endif
