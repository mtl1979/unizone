#ifdef WIN32
#pragma warning(disable: 4786)
#endif

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

#include <qdir.h>
#include <qstringlist.h>
#include <string.h>
#include <qevent.h>
#include <qapplication.h>

WFileThread::WFileThread(NetClient *net, QObject *owner, bool *optShutdownFlag)
	: Thread(), QObject(owner), fLocker(true)
{
	fNet = net;
	fOwner = owner;
	fShutdownFlag = optShutdownFlag;

#ifdef WIN32
	fScanProgress = new ScanProgress(dynamic_cast<QWidget *>(owner));
	CHECK_PTR(fScanProgress);
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
	fScannedDirs.Clear(true);
	fPaths.Clear();
	Unlock(); 
	int iScannedDirs = 0;

#ifdef WIN32
	SendShow();
	SendReset();

	CoInitialize(NULL);
#endif

	fPaths.AddTail("shared");
	QString path;
	while (fPaths.RemoveHead(path) == B_NO_ERROR)
	{
		Lock();
#ifdef WIN32
		SendInt(SET::DirsLeft, fPaths.GetNumItems());
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
		SendInt(SET::ScannedDirs, iScannedDirs);
#endif
	} 
#ifdef WIN32
	CoUninitialize();

	SendHide();
#endif

	Lock();
	fScannedDirs.Clear(true);
	files.Clear(true);
	Unlock();

	{
		QCustomEvent *qce = new QCustomEvent(ScanDone);
		if (qce)
			QApplication::postEvent(fOwner, qce);
	}
}

void
WFileThread::ParseDir(const QString & d)
{
#ifdef _DEBUG
	WString wD(d);
	PRINT("Parsing directory %S\n", wD.getBuffer());
#endif

#ifdef WIN32
	SendString(SET::ScanDirectory, d);
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
	
	QFileInfo *info = new QFileInfo(dir);
	
	if (info)
	{
		// Directory doesn't exist?
		if (info->exists())
		{		
			PRINT("Exists\n");
			
#ifndef WIN32
			// Read the symlink
			
			while (info->isSymLink())
			{
				QString l = info->readLink();
				delete info;
				return ParseDirAux(l);
			}
			
			PRINT("Symlinks resolved\n");
#endif
			
			if (info->isDir()) // Directory?
			{
				dir = info->absFilePath();
				String d = (const char *) dir.utf8();
				
				Lock();
				
				{
					String path;
					ret = fScannedDirs.Get(d, path);
				}
				
				Unlock();
				
				if (ret != B_NO_ERROR)
				{
					// Add to checked dirs
					Lock();
					fScannedDirs.Put(d, d); 
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
	CHECK_PTR(dir);
	if (dir->exists())	// double check
	{
#ifdef WIN32
		QString s = QObject::tr("Reading directory...", "WFileThread");
		SendString(SET::ScanFile, s);
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

				QString ndata = i.node->data;

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
					if (ndata.right(4) == ".md5")
					{
						skip = true;
					}
					else if (ndata.length() > 12)
					{
						if (ndata.left(8) == "AlbumArt" && ndata.right(4) == ".jpg")
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
			{
				WString wData(file);
				PRINT("\tChecking file %S\n", wData.getBuffer());
			}
#endif

			AddFile(file);

		}
	}
}

void
WFileThread::AddFile(const QString & filePath)
{
#ifdef DEBUG2
	WString wFilePath(filePath);
	PRINT("Setting to filePath: %S\n", wFilePath.getBuffer());
#endif
	
	UFileInfo *ufi = new UFileInfo(filePath);
				
	if (ufi)
	{
#ifdef DEBUG2
		PRINT("Set\n");
#endif
		
		if (ufi->isValid())
		{
#ifdef DEBUG2
			PRINT("Exists\n");
#endif
			
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
						String file = (const char *) name.utf8();
						
						Lock(); 
						fFiles.Put(file, filePath);
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
	Lock(); 

	bool ret = false;

	String key = (const char *) file.utf8();
	QString qFile;

	if (fFiles.Get(key, qFile) == B_NO_ERROR)
	{
		ret = true;
	}
	Unlock(); 
	return ret;
}

bool
WFileThread::FindFile(const String & file, MessageRef & ref)
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
		String key;
		QString qFile;
		fFiles.GetKeyAt(n, key);
		fFiles.Get(key, qFile);
		GetInfo(qFile, mref);
	}
	Unlock();
}

// NOTE: Doesn't seem to work with QApplication::postEvent() under Qt 2.3, so we continue using deprecated version
//       QThread::postEvent()

#ifdef WIN32
void
WFileThread::SendReset()
{
	ScanEvent *se = new ScanEvent(SET::Reset);

	if (se)
#if (QT_VERSION < 0x030000)
		QThread::postEvent(fScanProgress, se);
#else
		QApplication::postEvent(fScanProgress, se);
#endif
}

void
WFileThread::SendString(ScanEvent::Type t, const QString &str)
{
	ScanEvent *se = new ScanEvent(t, str);
	if (se)
#if (QT_VERSION < 0x030000)
		QThread::postEvent(fScanProgress, se);
#else
		QApplication::postEvent(fScanProgress, se);
#endif
}

void
WFileThread::SendInt(ScanEvent::Type t, int i)
{
	ScanEvent *se = new ScanEvent(t, i);
	if (se)
#if (QT_VERSION < 0x030000)
		QThread::postEvent(fScanProgress, se);
#else
		QApplication::postEvent(fScanProgress, se);
#endif
}

void
WFileThread::SendShow()
{
	ScanEvent *se = new ScanEvent(SET::Show);
	if (se)
#if (QT_VERSION < 0x030000)
		QThread::postEvent(fScanProgress, se);
#else
		QApplication::postEvent(fScanProgress, se);
#endif
}

void
WFileThread::SendHide()
{
	ScanEvent *se = new ScanEvent(SET::Hide);
	if (se)
#if (QT_VERSION < 0x030000)
		QThread::postEvent(fScanProgress, se);
#else
		QApplication::postEvent(fScanProgress, se);
#endif
}

#endif

bool
WFileThread::GetInfo(const QString &file, MessageRef &mref)
{
	bool ret = false;
	UFileInfo ufi(file);
	if (ufi.isValid())
	{
		String _file = (const char *) ufi.getName().utf8();
		String _path = (const char *) ufi.getPath().utf8();
		int64 size = ufi.getSize();
		
		mref = GetMessageFromPool();
		if ( mref() )
		{
			mref()->AddInt32("beshare:Modification Time", ufi.getModificationTime());
			mref()->AddString("beshare:Kind", (const char *) ufi.getMIMEType().utf8()); // give BeSharer's some relief
			mref()->AddString("beshare:Path", _path);
			mref()->AddInt64("beshare:File Size", size);
			mref()->AddString("beshare:FromSession", (const char *) fNet->LocalSessionID().utf8());
			mref()->AddString("beshare:File Name", _file);
			ret = true;
		}
	}
	return ret;
}

#ifdef WIN32
void
WFileThread::UpdateFileName(const QString &file)
{
	SendString(SET::ScanFile, file);
}

void
WFileThread::UpdateFileCount()
{
	int n = fFiles.GetNumItems();
	
	SendInt(SET::ScannedFiles, n);
}
#endif

void
WFileThread::Lock()
{
	fLocker.lock();
}

void
WFileThread::Unlock()
{
	fLocker.unlock();
}
