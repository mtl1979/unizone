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
#include "platform.h"

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


void
WFileThread::run()
{
	// reset our shutdown flag
	fRunning = true;
	if (fShutdownFlag)
		*fShutdownFlag = false;

	PRINT("Clearing list\n");
	EmptyList();
	Lock(); // test
	fFiles.Clear();
	fScannedDirs.Clear();
	fPaths.Clear();
	Unlock(); // test

#ifdef WIN32
	CoInitialize(NULL);
#endif

	fPaths.AddTail("shared");
	QString path;
	while (!fPaths.IsEmpty())
	{
		Lock();
		fPaths.RemoveHead(path);
		Unlock();
		if (fShutdownFlag && *fShutdownFlag)
		{
			// Make sure list is empty next time started!
			fPaths.Clear();
			break;
		}
		ParseDir(path);
	} 
#ifdef WIN32
	CoUninitialize();
#endif

	Lock();
	fScannedDirs.Clear();
	Unlock();

	fRunning = false;
	QCustomEvent *qce = new QCustomEvent(ScanDone);
	if (qce)
		postEvent(fOwner, qce);
}

void
WFileThread::ParseDir(const QString & d)
{
	QFileInfo * info = new QFileInfo(d);
	CHECK_PTR(info);

	wchar_t * wD = qStringToWideChar(d);
	PRINT("Parsing directory %S\n", wD);
	delete [] wD;

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

	for ( int n = 0; n < fScannedDirs.GetNumItems(); n++ )
	{
		if (fScannedDirs[n] == info->absFilePath())
		{
			delete info;
			return; // Already scanned!!!
		}
	}

	Lock(); // test
	fScannedDirs.AddTail(info->absFilePath()); // Add to checked dirs
	Unlock(); // test

	ScanFiles(info->absFilePath());
	delete info;
}

void
WFileThread::ScanFiles(QString directory)
{
	PRINT("Checking for directory existance\n");
	QDir * dir = new QDir(directory);
	CHECK_PTR(dir);
	QStringList list;
	if (dir->exists())	// double check
	{
		list = dir->entryList("*", (QDir::Dirs | QDir::Files) , QDir::DirsFirst);
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

				wchar_t * wData = qStringToWideChar(ndata);
				PRINT("\tChecking file %S\n", wData);
				delete [] wData;

				QString filePath = dir->absFilePath(ndata);

				AddFile(filePath);
				i++;
			}
		}
	}
	delete dir;
	dir = NULL;
}

void
WFileThread::AddFile(const QString & filePath)
{
	wchar_t * wFilePath = qStringToWideChar(filePath);
	PRINT("Setting to filePath: %S\n", wFilePath);
	delete [] wFilePath;
	
	QFileInfo * finfo = new QFileInfo(filePath);
	CHECK_PTR(finfo);
				
	PRINT("Set\n");
				
	if (finfo->exists())
	{
		PRINT("Exists\n");
					
		// resolve symlink
		QString ret = ResolveLink(finfo->filePath());

		wchar_t * wRet = qStringToWideChar(ret);
		PRINT("Resolved to: %S\n", wRet);
		delete [] wRet;

		finfo->setFile(ret);
					
		// is this a directory?
		if (finfo->isDir())
		{
			Lock();
			fPaths.AddTail(finfo->absFilePath());
			Unlock();
		}
		else
		{
			UFileInfo * ufi = new UFileInfo(ret);
			CHECK_PTR(ufi);

			if (ufi->isValid())
			{
				ufi->Init();
				MessageRef ref(GetMessageFromPool());
				if (ref())
				{
					QCString qcPath = ufi->getPath().utf8();
					ref()->AddInt32("beshare:Modification Time", ufi->getModificationTime());
					ref()->AddString("beshare:Kind", (const char *) ufi->getMIMEType().utf8()); // give BeSharer's some relief
					ref()->AddString("beshare:Path", (const char *) qcPath);
					ref()->AddString("winshare:Path", (const char *) qcPath);	// secret path
					ref()->AddInt64("beshare:File Size", ufi->getSize());
					ref()->AddString("beshare:FromSession", (const char *) fNet->LocalSessionID().utf8());
					ref()->AddString("beshare:File Name", (const char *) ufi->getName().utf8());
							
					QString nodePath;
					if (fFired)
						nodePath = "beshare/fires/";
					else
						nodePath = "beshare/files/";
							
					nodePath += ufi->getName();
							
					ref()->AddString("secret:NodePath", (const char *) nodePath.utf8());	// hehe, secret :)
					Lock(); // test
					fFiles.AddTail(ref);
					Unlock(); // test
				}
			}
			delete ufi;
			ufi = NULL;
		}
	}
	delete finfo;
	finfo = NULL;
}

QString
WFileThread::ResolveLink(const QString & lnk)
{
#ifdef WIN32
	QString ret = lnk;
	wchar_t * wRet = qStringToWideChar(ret);
	PRINT("\tResolving %S\n", wRet);
	delete [] wRet;

	if (ret.contains(".lnk") > 0)
	{
		PRINT("Is Link\n");
		// we've got a link...
		HRESULT hres;
		// Unicode
		IShellLink * psl;
		wchar_t * wsz = NULL;
		wchar_t szFile[MAX_PATH];
		WIN32_FIND_DATA wfd;

		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
		if (SUCCEEDED(hres))
		{
			// Unicode version
			PRINT("Created instance\n");
			IPersistFile * ppf;

			hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
			if (SUCCEEDED(hres))
			{
				PRINT("Got persistfile\n");
				wsz = qStringToWideChar(ret);		// <postmaster@raasu.org> -- Move type definition to start of function
				hres = ppf->Load(wsz, STGM_READ);
				if (SUCCEEDED(hres))
				{
					PRINT("Loaded\n");
					hres = psl->Resolve(gWin->GetHandle(), SLR_ANY_MATCH);
					if (SUCCEEDED(hres))
					{
						PRINT("Resolved\n");
						hres = psl->GetPath(szFile, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_UNCPRIORITY);
						PRINT("GetPath() = %S\n",szFile);
						if (!SUCCEEDED(hres))
							MessageBoxA(gWin->GetHandle(), "GetPath() failed!", "Error", MB_OK);
						ret = wideCharToQString(szFile);
					}
				}
				delete [] wsz;
				wsz = NULL; // <postmaster@raasu.org> 20021027

				ppf->Release();
			}
			psl->Release();
		}
		else
		{
			// Fallback to ANSI
			ret = ResolveLinkA(lnk);
		}
	}

	return ret;

#else
	QFileInfo inf(lnk);
	if (inf.isSymLink())
		return inf.readLink();
	return lnk; 	// oops
#endif
}

// ANSI version of ResolveLink for Windows 95 & 98 and Microsoft Unicode Layer
// IShellLinkW is not supported on Windows 95 or 98
QString
WFileThread::ResolveLinkA(const QString & lnk)
{
	QString ret = lnk;
#ifdef WIN32
	HRESULT hres;
	
	// Unicode
	wchar_t * wsz = NULL;
	
	// Ansi
	IShellLinkA * psl;
	char szFile[MAX_PATH];
	WIN32_FIND_DATAA wfd;
	
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (LPVOID *)&psl);
	if (SUCCEEDED(hres))
	{
		PRINT("Created instance\n");
		IPersistFile * ppf;
					
		// IPersistFile is all UNICODE
		hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
		if (SUCCEEDED(hres))
		{
			PRINT("Got persistfile\n");
			// <postmaster@raasu.org> -- Move type definition to start of function
			wsz = qStringToWideChar(lnk);		
			hres = ppf->Load(wsz, STGM_READ);
			if (SUCCEEDED(hres))
			{
				PRINT("Loaded\n");
				hres = psl->Resolve(gWin->GetHandle(), SLR_ANY_MATCH);
				if (SUCCEEDED(hres))
				{
					PRINT("Resolved\n");
					hres = psl->GetPath(szFile, MAX_PATH, (WIN32_FIND_DATAA *)&wfd, SLGP_UNCPRIORITY);
					PRINT("GetPath() = %s\n",szFile);
					if (!SUCCEEDED(hres))
						MessageBoxA(gWin->GetHandle(), "GetPath() failed!", "Error", MB_OK);
					ret = szFile;
				}
			}
			delete [] wsz;
			wsz = NULL; // <postmaster@raasu.org> 20021027
						
			ppf->Release();
		}
		psl->Release();
	}
#endif
	return ret;
}

// TODO: FIX THIS METHOD, make it MUCH faster! The current implementation is
// VERY slow!
bool
WFileThread::CheckFile(const QString & file)
{
	Lock(); 
	for (int i = 0; i < fFiles.GetNumItems(); i++)
	{
		String sn;
		QString name;
		fFiles[i]()->FindString("beshare:File Name", sn);
		name = QString::fromUtf8(sn.Cstr());
		if (file == name)
		{
			Unlock(); 
			return true;
		}
	}
	Unlock(); 
	return false;
}

bool
WFileThread::FindFile(const QString & file, MessageRef * ref)
{
	Lock();
	for (int i = 0; i < fFiles.GetNumItems(); i++)
	{
		if (fShutdownFlag && *fShutdownFlag)
		{
			Unlock();
			return true;	// tell our caller that the file exists so that we can quit
		}
		String sn;
		QString name;
		fFiles[i]()->FindString("beshare:File Name", sn);
		name = QString::fromUtf8(sn.Cstr());
		if (file == name)
		{
			*ref = fFiles[i];
			Unlock();
			return true;
		}
	}

	Unlock();
	return false;
}

int
WFileThread::GetNumFiles() const
{
	return fFiles.GetNumItems();
}

void
WFileThread::EmptyList()
{
	Lock(); // test
	while (fFiles.GetNumItems() != 0)
	{
		MessageRef mref;
		fFiles.RemoveHead(mref);
		mref.Reset();
	}
	Unlock(); // test
}

bool
WFileThread::IsRunning()
{
	return fRunning;
}

MessageRef
WFileThread::GetSharedFile(int n)
{
	MessageRef mref;
	if (n >= 0 && n < fFiles.GetNumItems())
		mref = fFiles[n];
	return mref;
}