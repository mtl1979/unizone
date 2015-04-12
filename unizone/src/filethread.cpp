#if defined(WIN32) || defined(_WIN32)
#include <objbase.h>
#endif

#include <qdir.h>
#include <qstringlist.h>
#include <QCustomEvent>
#include <string.h>
#include <qevent.h>

#include "filethread.h"
#include "fileinfo.h"
#include "settings.h"
#include "debugimpl.h"
#include "util/String.h"
#include "wstring.h"
#ifdef WIN32
#include "scanprogressimpl.h"
#endif
#include "scanevent.h"
#include "netclient.h"
#include "messageutil.h" // for AddStringToMessage();
#include "util.h"		// for SimplifyPath()

WFileThread::WFileThread(NetClient *net, QObject *owner, bool *optShutdownFlag)
	: QObject(owner), Thread(), fLocker()
{
	fNet = net;
	fOwner = owner;
	fShutdownFlag = optShutdownFlag;

#ifdef WIN32
	fScanProgress = new ScanProgress(dynamic_cast<QWidget *>(owner));
	Q_CHECK_PTR(fScanProgress);
#endif
}

WFileThread::~WFileThread()
{
}

void
WFileThread::InternalThreadEntry()
{
	// reset our shutdown flag
	if (fShutdownFlag)
		*fShutdownFlag = false;

	PRINT("Clearing list\n");
	EmptyList();
	Lock();
	fScannedDirs.clear();
	fPaths.Clear();
	Unlock();
	int iScannedDirs = 0;

#ifdef WIN32
	SendShow();
	SendReset();

	(void) CoInitialize(NULL);
#endif

	fPaths.AddTail("shared");
	QString path;
	while (fPaths.RemoveHead(path) == B_NO_ERROR)
	{
		Lock();
#ifdef WIN32
		SendInt(ScanEvent::DirsLeft, fPaths.GetNumItems());
#endif
		Unlock();
		if (fShutdownFlag && *fShutdownFlag)
		{
			// Make sure list is empty next time started!
			fPaths.Clear();
			break;
		}
		ParseDir(path);
		iScannedDirs++;
#ifdef WIN32
		SendInt(ScanEvent::ScannedDirs, iScannedDirs);
#endif
	}
#ifdef WIN32
	CoUninitialize();

	SendHide();
#endif

	Lock();
	fScannedDirs.clear();
	files.Clear(true);
	Unlock();

	{
		QCustomEvent *qce = new QCustomEvent(ScanDone);
		if (qce)
		{
			postEvent(fOwner, qce);
		}
	}
}

void
WFileThread::postEvent( QObject *o,QEvent * e)
{
	QApplication::postEvent(o, e);
}

void
WFileThread::ParseDir(const QString & d)
{
#ifdef _DEBUG
	WString wd(QDir::convertSeparators(d));
	PRINT("Parsing directory %S\n", wd.getBuffer());
#endif

#ifdef WIN32
	SendString(ScanEvent::ScanDirectory, d);
#endif

	QString dir(d);

	if (ParseDirAux(dir) != B_NO_ERROR)
	{		
		ScanFiles(dir);		
	}
}

// Check if directory has already been scanned...
status_t
WFileThread::ParseDirAux(QString &dir)
{
	status_t ret = B_NO_ERROR;
	
	dir = ResolveLink(dir);

	PRINT("Symlinks resolved\n");

	QFileInfo *info = new QFileInfo(dir);
	
	if (info)
	{
		// Directory doesn't exist?
		if (info->exists())
		{		
			PRINT("Exists\n");
						
			if (info->isDir()) // Directory?
			{
				dir = info->absFilePath();
				
				Lock();
				
				{
					ret = fScannedDirs.findIndex(dir) == -1 ? B_ERROR : B_NO_ERROR;
				}
				
				Unlock();
				
				if (ret != B_NO_ERROR)
				{
					// Add to checked dirs
					Lock();
					fScannedDirs.append(dir);
					Unlock();
				}
			}
		}
		delete info;
	}
	return ret;
}

const char * skipList[] =
{
	".",
	"..",
#ifdef WIN32
	// Skip Windows GUI files
	//
	"Thumbs.db",
	"desktop.ini",
	"Folder.jpg",
	// Skip misc Windows files
	//
	"SThumbs.dat",			// Shareaza Thumbnail Data
	"Metadata",				// Shareaza Metadata folder
#endif
	NULL
};

void
WFileThread::ScanFiles(const QString & directory)
{
	PRINT("Checking for directory existance\n");
	QDir * dir = new QDir(directory);
	Q_CHECK_PTR(dir);
	if (dir->exists())	// double check
	{
#ifdef WIN32
		QString s = tr("Reading directory...");
		SendString(ScanEvent::ScanFile, s);
#endif

		QStringList list = dir->entryList("*", (QDir::Dirs | QDir::Files) , QDir::DirsFirst);
		if (!list.isEmpty())
		{
			// not empty?
			QStringList::ConstIterator i = list.begin();
			while (i != list.end())
			{
				if (fShutdownFlag && *fShutdownFlag)
				{
					delete dir;
					return;
				}

				QString ndata = *i;

				bool skip = false;

				for (int n = 0; skipList[n] != NULL; n++)
				{
					if (ndata == skipList[n])
					{
						skip = true;
						break;
					}
				}

				if (!skip && (ndata.length() > 4))
				{
					if (ndata.endsWith(".md5"))
					{
						skip = true;
					}
					else if (ndata.length() > 12)
					{
						if (
							ndata.startsWith("AlbumArt") &&
							ndata.endsWith(".jpg")
							)
						{
							skip = true;
						}
					}
				}

				if (!skip)
				{
					QString qfile = dir->absFilePath(ndata);
					files.AddTail(qfile);
				}

				i++;
			}
		}
	}
	delete dir;
	dir = NULL;

	if (!files.IsEmpty())
	{
		QString file;
		while (files.RemoveHead(file) == B_NO_ERROR)
		{
			if (fShutdownFlag && *fShutdownFlag)
			{
				files.Clear();
				return;
			}

#ifdef _DEBUG
			WString wfile(QDir::convertSeparators(file));
			PRINT("\tChecking file %S\n", wfile.getBuffer());
#endif

			AddFile(file);

		}
	}
}

void
WFileThread::AddFile(const QString & filePath)
{
#ifdef DEBUG2
	WString wpath(filePath);
	PRINT2("Setting to filePath: %S\n", wpath.getBuffer());
#endif
	
	UFileInfo *ufi = new UFileInfo(filePath);
				
	if (ufi)
	{
		PRINT2("Set\n");
		
		if (ufi->isValid())
		{
			PRINT2("Exists\n");
			
			// resolve symlink
			QString gfn = ufi->getFullName();
			QString ret = ResolveLink(gfn);
			
			if (gfn == ret)
			{
				// is this a directory?
				if (ufi->isDir())
				{
					Lock();
					fPaths.AddTail(ufi->getAbsPath());
					Unlock();
				}
				else
				{
					int64 size = ufi->getSize();
					if (size > 0)
					{
						QString name = ufi->getName();
						
						Lock();
						fFiles.Put(name, filePath);
						Unlock();

#ifdef WIN32
						UpdateFileCount();
						UpdateFileName(name);
#endif
					}
				}
			}
			else
			{
				files.AddTail(ret);
			}
			delete ufi;
		}
	}
}

bool
WFileThread::CheckFile(const QString & file)
{
	bool ret;

	Lock();

	ret = fFiles.ContainsKey(file);

	Unlock();
	return ret;
}

bool
WFileThread::FindFile(const QString & file, MessageRef & ref)
{
	Lock();
	bool ret = false;

	QString qFile;

	if (fFiles.Get(file, qFile) == B_NO_ERROR)
	{
		if (GetInfo(qFile, ref))
		{
			ret = true;
		}
	}

	Unlock();
	return ret;
}

int
WFileThread::GetNumFiles() const
{
	return fFiles.GetNumItems();
}

void
WFileThread::EmptyList()
{
	Lock();
	fFiles.Clear(true);
	files.Clear(true);
	Unlock();
}

void
WFileThread::GetSharedFile(unsigned int n, MessageRef & mref)
{
	Lock();
	if (n < fFiles.GetNumItems())
	{
		QString key;
		QString qFile;
		fFiles.GetKeyAt(n, key);
		fFiles.Get(key, qFile);
		GetInfo(qFile, mref);
	}
	Unlock();
}

#ifdef WIN32
void
WFileThread::SendReset()
{
	ScanEvent *se = new ScanEvent(ScanEvent::Reset);

	if (se)
	{
		postEvent(fScanProgress, se);
	}
}

void
WFileThread::SendString(ScanEvent::Type t, const QString &str)
{
	ScanEvent *se = new ScanEvent(t, str);
	if (se)
	{
		postEvent(fScanProgress, se);
	}
}

void
WFileThread::SendInt(ScanEvent::Type t, int i)
{
	ScanEvent *se = new ScanEvent(t, i);
	if (se)
	{
		postEvent(fScanProgress, se);
	}
}

void
WFileThread::SendShow()
{
	ScanEvent *se = new ScanEvent(ScanEvent::Show);
	if (se)
	{
		postEvent(fScanProgress, se);
	}
}

void
WFileThread::SendHide()
{
	ScanEvent *se = new ScanEvent(ScanEvent::Hide);
	if (se)
	{
		postEvent(fScanProgress, se);
	}
}

#endif

bool
WFileThread::GetInfo(const QString &file, MessageRef &mref) const
{
	bool ret = false;
	UFileInfo ufi(file);
	if (ufi.isValid())
	{
		QString upath = ufi.getPath();
      QString qPath = SimplifyPath(upath);
      if (qPath.isEmpty())
         return false; // Don't return files from Unizone main directory
      // strip "shared" from path
      if (qPath == "shared")
         qPath = "";
      if (qPath.startsWith(QString("shared") + QDir::separator()))
         qPath = qPath.mid(7);
      //
		int64 size = ufi.getSize();
		
		mref = GetMessageFromPool();
		if ( mref() )
		{
			mref()->AddInt32("beshare:Modification Time", ufi.getModificationTime());
			AddStringToMessage(mref, "beshare:Kind", ufi.getMIMEType()); // give BeSharer's some relief
			AddStringToMessage(mref, "beshare:Path", qPath);
			AddStringToMessage(mref, "winshare:Path", upath);
			mref()->AddInt64("beshare:File Size", size);
			AddStringToMessage(mref, "beshare:FromSession", fNet->LocalSessionID());
			AddStringToMessage(mref, "beshare:File Name", ufi.getName());
			ret = true;
		}
	}
	return ret;
}

#ifdef WIN32
void
WFileThread::UpdateFileName(const QString &file)
{
	SendString(ScanEvent::ScanFile, file);
}

void
WFileThread::UpdateFileCount()
{
	int n = fFiles.GetNumItems();
	
	SendInt(ScanEvent::ScannedFiles, n);
}
#endif

void
WFileThread::Lock()
{
	fLocker.Lock();
}

void
WFileThread::Unlock()
{
	fLocker.Unlock();
}
