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
#include "lang.h"
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
	Lock(); // test
	fFiles.clear();
	fScannedDirs.clear();
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
	fScannedDirs.clear();
	Unlock();

	fRunning = false;
	QCustomEvent *qce = new QCustomEvent(ScanDone);
	if (qce)
		QThread::postEvent(fOwner, qce);
}

void
WFileThread::ParseDir(const QString & d)
{
	QFileInfo * info = new QFileInfo(d);
	CHECK_PTR(info);
	PRINT("Parsing directory %S\n", qStringToWideChar(d));

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

	for (WStrListIter it = fScannedDirs.begin(); it != fScannedDirs.end(); it++)
	{
		if ((*it) == info->absFilePath())
		{
			delete info;
			return; // Already scanned!!!
		}
	}
	Lock(); // test
	fScannedDirs.insert(fScannedDirs.end(),info->absFilePath()); // Add to checked dirs
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
	if (dir->exists())	// double check
	{
		QStringList list = dir->entryList("*", (QDir::Dirs | QDir::Files) , QDir::DirsFirst);
		if (!list.isEmpty())
		{
			// not empty?
			for (QStringList::Iterator i = list.begin(); i!= list.end(); i++)
			{
				if (fShutdownFlag && *fShutdownFlag)
				{
					delete dir;
					return;
				}

				if (((*i) == ".") || ((*i) == "..") || ((*i).right(4) == ".md5"))
					continue;

				PRINT("\tChecking file %S\n", qStringToWideChar(i.node->data));
				QString filePath = dir->absFilePath(i.node->data);
				PRINT("Setting to filePath: %S\n", qStringToWideChar(filePath));

				QFileInfo * finfo = new QFileInfo(filePath);
				CHECK_PTR(finfo);

				PRINT("Set\n");
				
				if (finfo->exists())
				{
					PRINT("Exists\n");

					// resolve symlink
					QString ret = ResolveLink(finfo->filePath());
					PRINT("Resolved to: %S\n", qStringToWideChar(ret));
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
							MessageRef ref(GetMessageFromPool());
							QCString qcPath = ufi->getPath().utf8();
							ref()->AddInt32("beshare:Modification Time", ufi->getModificationTime());
							ref()->AddString("beshare:Kind", (const char *) ufi->getMIMEType().utf8());	// give BeSharer's some relief
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
							fFiles.insert(fFiles.end(), ref);
							Unlock(); // test
						}
						delete ufi;
						ufi = NULL;
					}
				}
				delete finfo;
				finfo = NULL;
			}
		}
	}
	delete dir;
	dir = NULL;
}

QString
WFileThread::ResolveLink(const QString & lnk)
{
#ifdef WIN32
	QString ret = lnk;
	PRINT("\tResolving %S\n", qStringToWideChar(lnk));
	if (lnk.contains(".lnk") > 0)
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
				wsz = qStringToWideChar(lnk);		// <postmaster@raasu.org> -- Move type definition to start of function
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
	return ret;
}

// TODO: FIX THIS METHOD, make it MUCH faster! The current implementation is
// VERY slow!
bool
WFileThread::CheckFile(const QString & file)
{
	Lock(); 
	for (WMsgListIter i = fFiles.begin(); i != fFiles.end(); i++)
	{
		String sn;
		QString name;
		(*i)()->FindString("beshare:File Name", sn);
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
	for (WMsgListIter i = fFiles.begin(); i != fFiles.end(); i++)
	{
		if (fShutdownFlag && *fShutdownFlag)
		{
			Unlock();
			return true;	// tell our caller that the file exists so that we can quit
		}
		String sn;
		QString name;
		(*i)()->FindString("beshare:File Name", sn);
		name = QString::fromUtf8(sn.Cstr());
		if (file == name)
		{
			*ref = (*i);
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
	return fFiles.size();
}

void
WFileThread::EmptyList()
{
	Lock(); // test
	fFiles.clear();
	Unlock(); // test
}

/*
bool
WFileThread::GetFileInfo(QFileInfo * file, FileInfo * info)
{

	if (info)
	{
#ifdef WIN32
		// Read the mime-type
		HKEY hkey;
		DWORD type;
		char key[MAX_PATH];
		DWORD dsize = MAX_PATH;
		QString ext = ".";
		ext += file->extension();
		wchar_t * tExt = qStringToWideChar(ext);
		if (RegOpenKey(HKEY_CLASSES_ROOT, tExt, &hkey) == ERROR_SUCCESS)
		{
			LONG ret;
			// <postmaster@raasu.org> -- Don't use Unicode (wide-char) functions for char[] and char *
			if ((ret = RegQueryValueExA(hkey, "Content Type", NULL, &type, (LPBYTE)key, &dsize)) == ERROR_SUCCESS)
			{
				PRINT("Read key: %s\n", key);
				info->fMIME = key;
			}
			else
			{
				PRINT("Error: %d [0x%08x]\n", ret, ret);
			}
			RegCloseKey(hkey);
		}
		delete [] tExt;
		tExt = NULL; // <postmaster@raasu.org> 20021027 
		// Read the modification time
		// The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
		FILETIME ftime;
		// The time functions included in the C run-time use the time_t type to represent the number of seconds elapsed since midnight, January 1, 1970.
		uint32 mtTime; // <postmaster@raasu.org> 20021230
		uint64 ftTime; // 
		HANDLE fileHandle;
		wchar_t * tFilePath = qStringToWideChar( file->filePath() );
		fileHandle = CreateFile(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		delete [] tFilePath;
		tFilePath = NULL; // <postmaster@raasu.org> 20021027
		if (fileHandle)
		{
			PRINT("File opened!\n");
			if (GetFileTime(fileHandle, NULL, NULL, &ftime))
			{
				ftTime = (uint64)
					(
					(((uint64)ftime.dwHighDateTime) << 32) + 
					((uint64)ftime.dwLowDateTime) 
					);
				ftTime -= 116444736000000000; 
				// = 11644473600 seconds
				// = 134774 days
				// = 369 years + 89 (leap) days
				mtTime = (uint32)(ftTime  / 10000000);	// converted to seconds
				info->fModificationTime = mtTime;
				PRINT("Got time: %d\n", info->fModificationTime);
				if (info->fModificationTime < 0)	// negative? wow...
				{
					PRINT("Negative?\n");
					info->fModificationTime = time(NULL);
				}
			}
			else
				info->fModificationTime = time(NULL);

			CloseHandle(fileHandle);
		}
		else
			info->fModificationTime = time(NULL);

		return true;
#else
		info->fModificationTime = time(NULL);
        info->fMIME = "";
        return true;
#endif
	}
	return false;
}
*/
bool
WFileThread::IsRunning()
{
	return fRunning;
}
