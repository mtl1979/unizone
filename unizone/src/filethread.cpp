#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#endif

#include "filethread.h"
#include "fileinfo.h"
#include "global.h"
#include "settings.h"
#include "debugimpl.h"
#include "util/String.h"
#include "wstring.h"
#include "scanprogressimpl.h"
#include "scanevent.h"
#include "netclient.h"

#include <qdir.h>
#include <qstringlist.h>
#include <string.h>
#include <qevent.h>
#include <qapplication.h>

#ifdef WIN32
#include <shellapi.h>
# ifdef VC7
#  include <shldisp.h>	// hm... we only need this in VC7
# endif
#include <shlguid.h>
#include <shlobj.h>
#endif

WFileThread::WFileThread(NetClient *net, QObject *owner, bool *optShutdownFlag)
	: QThread(), fLocker(true)
{
	fNet = net;
	fOwner = owner;
	fShutdownFlag = optShutdownFlag;

	fScanProgress = new ScanProgress(fOwner);
	CHECK_PTR(fScanProgress);
}

WFileThread::~WFileThread()
{
}

void
WFileThread::run()
{
	// reset our shutdown flag
	if (fShutdownFlag)
		*fShutdownFlag = false;

	PRINT("Clearing list\n");
	EmptyList();
	Lock(); 
	fScannedDirs.Clear();
	fPaths.Clear();
	Unlock(); 
	int iScannedDirs = 0;

	fScanProgress->show();
	SendReset(); 

#ifdef WIN32
	CoInitialize(NULL);
#endif

	fPaths.AddTail("shared");
	QString path;
	while (!fPaths.IsEmpty())
	{
		Lock();
#ifdef WIN32
		SendInt(ScanEvent::Type::DirsLeft, fPaths.GetNumItems());
#else
		SendInt(ScanEvent::DirsLeft, fPaths.GetNumItems());
#endif
		fPaths.RemoveHead(path);
		Unlock();
		if (fShutdownFlag && *fShutdownFlag)
		{
			// Make sure list is empty next time started!
			fPaths.Clear();
			break;
		}
		msleep(30);
		ParseDir(path);
		iScannedDirs++;
#ifdef WIN32
		SendInt(ScanEvent::Type::ScannedDirs, iScannedDirs);
#else
		SendInt(ScanEvent::ScannedDirs, iScannedDirs);
#endif
	} 
#ifdef WIN32
	CoUninitialize();
#endif

	fScanProgress->close();

	Lock();
	fScannedDirs.Clear();
	Unlock();

	QCustomEvent *qce = new QCustomEvent(ScanDone);
	if (qce)
		QApplication::postEvent(fOwner, qce);
}

void
WFileThread::ParseDir(const QString & d)
{
	QFileInfo * info = new QFileInfo(d);
	CHECK_PTR(info);

	WString wD = d;
	PRINT("Parsing directory %S\n", wD.getBuffer());

#ifdef WIN32
	SendString(ScanEvent::Type::ScanDirectory, d);
#else
	SendString(ScanEvent::ScanDirectory, d);
#endif

	// Directory doesn't exist?
	if (!info->exists())
	{
		delete info;
		return;
	}

	PRINT("Exists\n");

	// Read the symlink
	while (info->isSymLink())
		info->setFile(info->readLink());

	PRINT("Symlinks resolved\n");
	if (!info->isDir())
	{
		delete info;
		return;	// not a directory?
	}

	Lock();
	for ( int n = 0; n < fScannedDirs.GetNumItems(); n++ )
	{
		QString path;
		fScannedDirs.GetItemAt(n, path);
		if (path == info->absFilePath())
		{
			Unlock();
			delete info;
			return; // Already scanned!!!
		}
	}

	fScannedDirs.AddTail(info->absFilePath()); // Add to checked dirs
	Unlock();

	ScanFiles(info->absFilePath());
	delete info;
}

void
WFileThread::ScanFiles(const QString & directory)
{
	PRINT("Checking for directory existance\n");
	QDir * dir = new QDir(directory);
	CHECK_PTR(dir);
	if (dir->exists())	// double check
	{
		QString s = QObject::tr("Reading directory...", "WFileThread");
#ifdef WIN32
		SendString(ScanEvent::Type::ScanFile, s);
#else
		SendString(ScanEvent::ScanFile, s);
#endif

		QStringList list = dir->entryList("*", (QDir::Dirs | QDir::Files) , QDir::DirsFirst);
		if (!list.isEmpty())
		{
			// not empty?
			QStringList::Iterator i = list.begin();
			while (i != list.end())
			{
				if (fShutdownFlag && *fShutdownFlag)
				{
					delete dir;
					return;
				}

				QString ndata = i.node->data;

				if ((ndata == ".") || (ndata == ".."))
				{
					i++;
					continue;
				}

				if (ndata.length() > 4)
				{
					if (ndata.right(4) == ".md5")
					{
						i++;
						continue;
					}
				}

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
		while (files.GetNumItems() > 0)
		{
			files.RemoveHead(file);
			
			{
				WString wData = file;
				PRINT("\tChecking file %S\n", wData.getBuffer());
			}

			AddFile(file);

			msleep(30);
		}
	}
}

void
WFileThread::AddFile(const QString & filePath)
{
#ifdef DEBUG2
	WString wFilePath = filePath;
	PRINT("Setting to filePath: %S\n", wFilePath.getBuffer());
#endif
	
	UFileInfo ufi(filePath);
				
#ifdef DEBUG2
	PRINT("Set\n");
#endif
				
	if (ufi.isValid())
	{
		ufi.Init();
#ifdef DEBUG2
		PRINT("Exists\n");
#endif
		
		// resolve symlink
		QString gfn = ufi.getFullName();
		QString ret = ResolveLink(gfn);
		
		if (gfn != ret) 
		{
			files.AddTail(ret);
			return;
		}
	}
	else
	{
		return;
	}

	ufi.Init();
	// is this a directory?
	if (ufi.isDir())
	{
		Lock();
		fPaths.AddTail(ufi.getAbsPath());
		Unlock();
	}
	else
	{
		MessageRef ref(GetMessageFromPool());
		if (ref())
		{
			QCString qcPath = ufi.getPath().utf8();
			int64 size = ufi.getSize();
			if (size > 0)
			{
				String file = (const char *) ufi.getName().utf8();
				ref()->AddInt32("beshare:Modification Time", ufi.getModificationTime());
				ref()->AddString("beshare:Kind", (const char *) ufi.getMIMEType().utf8()); // give BeSharer's some relief
				ref()->AddString("beshare:Path", (const char *) qcPath);
				// No actual need for winshare:Path, because the value of beshare:Path is exactly the same.
				// ref()->AddString("winshare:Path", (const char *) qcPath);	// secret path
				ref()->AddInt64("beshare:File Size", size);
				ref()->AddString("beshare:FromSession", (const char *) fNet->LocalSessionID().utf8());
				ref()->AddString("beshare:File Name", file);
				
				Lock(); 
				fFiles.Put(file, ref);
				Unlock(); 
				int n = fFiles.GetNumItems();

#ifdef WIN32
				SendString(ScanEvent::Type::ScanFile, ufi.getName());
				SendInt(ScanEvent::Type::ScannedFiles, n);
#else
				SendString(ScanEvent::ScanFile, ufi.getName());
				SendInt(ScanEvent::ScannedFiles, n);
#endif
			}
		}
	}
}

QString
WFileThread::ResolveLink(const QString & lnk)
{
#ifdef WIN32
#ifdef DEBUG2
	{
		WString wRet = lnk;
		PRINT("\tResolving %S\n", wRet.getBuffer());
	}
#endif

	if (lnk.right(4) == ".lnk")
	{
		PRINT("Is Link\n");
		// we've got a link...
		HRESULT hres;
		// Unicode
		IShellLink * psl;
		wchar_t szFile[MAX_PATH];
		WIN32_FIND_DATA wfd;
		
		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
		if (SUCCEEDED(hres))
		{
			QString ret(lnk);

			// Unicode version
			PRINT("Created instance\n");
			IPersistFile * ppf;
			
			hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
			if (SUCCEEDED(hres))
			{
				PRINT("Got persistfile\n");
				WString wsz(ret);

				hres = ppf->Load(wsz, STGM_READ);
				if (SUCCEEDED(hres))
				{
					PRINT("Loaded\n");
					hres = psl->Resolve(gWin->GetHandle(), SLR_ANY_MATCH);
					if (SUCCEEDED(hres))
					{
						PRINT("Resolved\n");
						hres = psl->GetPath(szFile, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_UNCPRIORITY);
						if (SUCCEEDED(hres))
						{
							PRINT("GetPath() = %S\n", szFile);
							ret = wideCharToQString(szFile);
						}
						else
							MessageBox(gWin->GetHandle(), L"GetPath() failed!", L"Error", MB_OK);
					}
				}
				
				ppf->Release();
			}
			psl->Release();
			return ret;
		}
		else
		{
			// Fallback to ANSI
			QString ret = ResolveLinkA(lnk);
			{
				WString wRet = ret;
				PRINT("Resolved to: %S\n", wRet.getBuffer());
			}
			return ret;
		}
	}
	return lnk;
#else
	QFileInfo inf(lnk);
	if (inf.isSymLink())
		return inf.readLink();
	else
		return lnk; 	// oops
#endif
}

// ANSI version of ResolveLink for Windows 95 & 98 and Microsoft Unicode Layer
// IShellLinkW is not supported on Windows 95 or 98
QString
WFileThread::ResolveLinkA(const QString & lnk)
{
#ifdef WIN32
	HRESULT hres;
	
	// Ansi
	IShellLinkA * psl;
	char szFile[MAX_PATH];
	WIN32_FIND_DATAA wfd;
	
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (LPVOID *)&psl);
	if (SUCCEEDED(hres))
	{
		PRINT("Created instance\n");

		IPersistFile * ppf;
		QString ret(lnk);

		// IPersistFile is all UNICODE
		hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
		if (SUCCEEDED(hres))
		{
			PRINT("Got persistfile\n");

			// Unicode
			WString wsz(lnk);
			hres = ppf->Load(wsz, STGM_READ);
			if (SUCCEEDED(hres))
			{
				PRINT("Loaded\n");
				hres = psl->Resolve(gWin->GetHandle(), SLR_ANY_MATCH);
				if (SUCCEEDED(hres))
				{
					PRINT("Resolved\n");
					hres = psl->GetPath(szFile, MAX_PATH, (WIN32_FIND_DATAA *)&wfd, SLGP_UNCPRIORITY);
					if (SUCCEEDED(hres))
					{
						PRINT("GetPath() = %s\n",szFile);
						ret = szFile;
					}
					else
					{
						MessageBoxA(gWin->GetHandle(), "GetPath() failed!", "Error", MB_OK);
					}
				}
			}						
			ppf->Release();
		}
		psl->Release();
		return ret;
	}
#endif
	return lnk;
}

// TODO: FIX THIS METHOD, make it MUCH faster! The current implementation is
// VERY slow!
bool
WFileThread::CheckFile(const QString & file)
{
	Lock(); 

	bool ret = false;

	String key = (const char *) file.utf8();
	MessageRef mref;

	if (fFiles.Get(key, mref) == B_NO_ERROR)
	{
		ret = true;
	}
	Unlock(); 
	return ret;
}

bool
WFileThread::FindFile(const QString & file, MessageRef & ref)
{
	Lock();
	bool ret = false;

	String key = (const char *) file.utf8();
	MessageRef mref;

	if (fFiles.Get(key, mref) == B_NO_ERROR)
	{
		if ( mref() )
		{
			Message *msg = mref();
			ref = GetMessageFromPool(*msg); // copy the message, don't pass a ref ;)
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
	fFiles.Clear();
	files.Clear();
	Unlock(); 
}

void
WFileThread::GetSharedFile(int n, MessageRef & mref)
{
	Lock();
	if (n >= 0 && n < fFiles.GetNumItems())
	{
		String key;
		fFiles.GetKeyAt(n, key);
		fFiles.Get(key, mref);
	}
	Unlock();
}

// NOTE: Doesn't seem to work with QApplication::postEvent() under Qt 2.3, so we continue using deprecated version
//       QThread::postEvent()

void
WFileThread::SendReset()
{
#ifdef WIN32
	ScanEvent *se = new ScanEvent(ScanEvent::Type::Reset);
#else
	ScanEvent *se = new ScanEvent(ScanEvent::Reset);
#endif
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
