#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#endif

#include "filethread.h"
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
#include <qfileinfo.h>

#ifdef WIN32
#include <shellapi.h>
# ifdef VC7
#  include <shldisp.h>	// hm... we only need this in VC7
# endif
#include <shlguid.h>
#include <shlobj.h>
#endif

UFileInfo::UFileInfo(QFileInfo & info)
{
	fInfo = new QFileInfo(info);
}

UFileInfo::UFileInfo(QString file)
{
	fInfo = new QFileInfo(file);
}

UFileInfo::~UFileInfo()
{
	if (fInfo)
		delete fInfo;
}

QString
UFileInfo::getMIMEType()
{
	if (!fInfo)
		return QString::null;

#ifdef WIN32
	// Read the mime-type
	HKEY hkey;
	DWORD type;
	char key[MAX_PATH];
	DWORD dsize = MAX_PATH;
	QString mt = QString::null;

	QString ext = ".";
	ext += fInfo->extension();
	wchar_t * tExt = qStringToWideChar(ext);
	if (RegOpenKey(HKEY_CLASSES_ROOT, tExt, &hkey) == ERROR_SUCCESS)
	{
		LONG ret;
		// <postmaster@raasu.org> -- Don't use Unicode (wide-char) functions for char[] and char *
		if ((ret = RegQueryValueExA(hkey, "Content Type", NULL, &type, (LPBYTE)key, &dsize)) == ERROR_SUCCESS)
		{
			PRINT("Read key: %s\n", key);
			mt = key;
		}
		else
		{
			PRINT("Error: %d [0x%08x]\n", ret, ret);
		}
		RegCloseKey(hkey);
	}
	delete [] tExt;
	tExt = NULL; // <postmaster@raasu.org> 20021027
	return mt;
#else
	return QString::null;
#endif

}

uint32
UFileInfo::getModificationTime()
{
	if (!fInfo)
		return time(NULL);

#ifdef WIN32
			// Read the modification time
		// The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
		FILETIME ftime;
		// The time functions included in the C run-time use the time_t type to represent the number of seconds elapsed since midnight, January 1, 1970.
		uint32 mtTime; // <postmaster@raasu.org> 20021230
		uint64 ftTime; // 
		HANDLE fileHandle;
		wchar_t * tFilePath = qStringToWideChar( fInfo->filePath() );
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
				PRINT("Got time: %d\n", mtTime);
			}
			else
				mtTime = time(NULL);

			CloseHandle(fileHandle);
			return mtTime;
		}
		else
			return time(NULL);

#else
		return time(NULL);
#endif

}

QString
UFileInfo::getPath()
{
	if (!fInfo)
		return QString::null;
	return fInfo->dirPath(true);
}

QString
UFileInfo::getName()
{
	if (!fInfo)
		return QString::null;
	return fInfo->fileName();
}

uint64
UFileInfo::getSize()
{
	if (!fInfo)
		return -1;
	return fInfo->size();
}

bool
UFileInfo::isValid()
{
	if (!fInfo)				// no valid object
		return false;
	if (!fInfo->exists())	// non-existent file
		return false;
	return true;
}

void
WFileThread::run()
{
	// Lock();
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

	/** 20020616: Re-coded the file parsing mechanism */
	fPaths.AddTail("shared");
	QString path;
	while (!fPaths.IsEmpty())
	{
		Lock();
		fPaths.RemoveHead(path);
		Unlock();
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
		QApplication::postEvent(fOwner, qce);
	// Unlock();
	// exit();
}

void
WFileThread::ParseDir(const QString & d)
{
	QString rpath("beshare/");
	if (fFired)
		rpath += "fires/";
	else
		rpath += "files/";

	QFileInfo * info = new QFileInfo(d);
	CHECK_PTR(info);
	PRINT("Parsing directory %s\n", d.latin1());
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

	PRINT("Checking for directory existance\n");
	QDir * dir = new QDir(info->absFilePath());
	CHECK_PTR(dir);
	if (dir->exists())	// double check
	{
		QStringList list = dir->entryList("*", (QDir::Dirs | QDir::Files) , QDir::DirsFirst);
		if (!list.isEmpty())
		{
			// not empty?
			for (QStringList::Iterator i = list.begin(); i!= list.end(); i++)
			{
				if (((*i) == ".") || ((*i) == "..") || ((*i).right(4) == ".md5"))
					continue;
				PRINT("\tChecking file %s\n", i.node->data.latin1());
				QString filePath = dir->absFilePath(i.node->data);
				PRINT("Setting to filePath: %s\n", filePath.latin1());
				info->setFile(filePath);
				PRINT("Set\n");
				if (info->exists())
				{
					PRINT("Exists\n");
					// resolve symlink
					QString ret = ResolveLink(info->filePath());
					PRINT("Resolved to: %s\n", ret.latin1());
					info->setFile(ret);

					// is this a directory?
					if (info->isDir())
					{
						// ParseDir(info->absFilePath());
						Lock();
						fPaths.AddTail(info->absFilePath());
						Unlock();
					}
					else
					{
						// FileInfo * fsInfo = new FileInfo;	// filesystem info
						UFileInfo * ufi = new UFileInfo(ret);
						if (ufi && ufi->isValid())
						{
							MessageRef ref(new Message(), NULL);
							QCString qcPath = ufi->getPath().utf8();
							ref()->AddInt32("beshare:Modification Time", ufi->getModificationTime());
							ref()->AddString("beshare:Kind", (const char *) ufi->getMIMEType().utf8());	// give BeSharer's some relief
							ref()->AddString("beshare:Path", (const char *) qcPath);
							ref()->AddString("winshare:Path", (const char *) qcPath);	// secret path
							ref()->AddInt64("beshare:File Size", ufi->getSize());
							ref()->AddString("beshare:FromSession", (const char *) fNet->LocalSessionID().utf8());
							ref()->AddString("beshare:File Name", (const char *) ufi->getName().utf8());
							
							QString nodePath = rpath;
							nodePath += ufi->getName();
							
							ref()->AddString("secret:NodePath", (const char *) nodePath.utf8());	// hehe, secret :)
							Lock(); // test
							fFiles.insert(fFiles.end(), ref);
							Unlock(); // test
						}

						// delete fsInfo;
						// fsInfo = NULL;
						delete ufi;
						ufi = NULL;
					}
				}
			}
		}
	}
	delete info;
	delete dir;
}

QString
WFileThread::ResolveLink(const QString & lnk)
{
#ifdef WIN32
	QString ret = lnk;
	PRINT("\tResolving %s\n", lnk.latin1());
	if (lnk.contains(".lnk") > 0)
	{
		PRINT("Is Link\n");
		// we've got a link...
		HRESULT hres;
		IShellLink * psl;
		// wchar_t szDescription[MAX_PATH];
		wchar_t * wsz = NULL;
		wchar_t szFile[MAX_PATH];
		WIN32_FIND_DATA wfd;

		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
		if (SUCCEEDED(hres))
		{
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
						// <postmaster@raasu.org> -- Use own variable for GetPath(), don't reuse 'wsz'
						hres = psl->GetPath(szFile, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_UNCPRIORITY);
						PRINT("GetPath() = %S\n",szFile);
						if (!SUCCEEDED(hres))
							MessageBox(gWin->GetHandle(), L"GetPath() failed!", L"Error", MB_OK);
						/*
						hres = psl->GetDescription(szDescription, MAX_PATH);
						PRINT("GetDescription() = %S\n",szDescription);
						if (!SUCCEEDED(hres))
							MessageBox(gWin->GetHandle(), L"GetDescription() failed!", L"Error", MB_OK);
						*/
						ret = wideCharToQString(szFile);
					}
				}
				delete [] wsz;
				wsz = NULL; // <postmaster@raasu.org> 20021027

				ppf->Release();
			}
			psl->Release();
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

// TODO: FIX THIS METHOD, make it MUCH faster! The current implementation is
// VERY slow!
bool
WFileThread::CheckFile(const QString & file)
{
	Lock(); // test
	for (WMsgListIter i = fFiles.begin(); i != fFiles.end(); i++)
	{
		String sn;
		QString name;
		(*i)()->FindString("beshare:File Name", sn);
		name = QString::fromUtf8(sn.Cstr());
		if (file == name)
		{
			Unlock(); // test
			return true;
		}
	}
	Unlock(); // test
	return false;
}

bool
WFileThread::FindFile(const QString & file, MessageRef * ref)
{
	for (WMsgListIter i = fFiles.begin(); i != fFiles.end(); i++)
	{
		if (fShutdownFlag && *fShutdownFlag)
			return true;	// tell our caller that the file exists so that we can quit
		String sn;
		QString name;
		(*i)()->FindString("beshare:File Name", sn);
		name = QString::fromUtf8(sn.Cstr());
		if (file == name)
		{
			*ref = (*i);
			return true;
		}
	}

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
