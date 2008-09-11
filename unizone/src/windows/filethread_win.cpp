#include <qstring.h>
#include <qdir.h>
#include <windows.h>
#include <shellapi.h>
#ifdef VC7
#  include <shldisp.h>	// hm... we only need this in VC7
#endif
#include <oaidl.h>
#include <shlguid.h>
#include <shlobj.h>

#include "filethread.h"
#include "wstring.h"
#include "wutil.h"
#include "winsharewindow.h"
#include "global.h"
#include "util.h"				// for endsWith()

// Unicode version of ResolveLink for Windows 2000 and newer
// IShellLinkW is only supported on Windows 2000 and later
QString
WFileThread::ResolveLink(const QString & lnk) const
{
#ifdef DEBUG2
	WString wlnk(QDir::convertSeparators(lnk));
	PRINT2("\tResolving %S\n", wlnk.getBuffer());
#endif
	
	if (endsWith(lnk, ".lnk"))
	{
		// we've got a link...
		PRINT("Is Link\n");
		
		HRESULT hres;
		// Unicode
		IShellLinkW * psl;
		wchar_t szFile[MAX_PATH];
		WIN32_FIND_DATAW wfd;
		
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
				
				WString wret(QDir::convertSeparators(ret));
				hres = ppf->Load(wret.getBuffer(), STGM_READ);
				if (SUCCEEDED(hres))
				{
					PRINT("Loaded %S\n", wret.getBuffer());
					hres = psl->Resolve(gWin->winId(), SLR_NO_UI | SLR_ANY_MATCH);
					if (SUCCEEDED(hres))
					{
						PRINT("Resolved\n");
						hres = psl->GetPath(szFile, MAX_PATH, (WIN32_FIND_DATAW *)&wfd, SLGP_UNCPRIORITY);
						if (SUCCEEDED(hres))
						{
							PRINT("GetPath() = %S\n", szFile);
							ret = wideCharToQString(szFile);
						}
						else
							MessageBoxW(gWin->winId(), L"GetPath() failed!", L"Error", MB_OK);
					}
				}
				
				ppf->Release();
			}
			psl->Release();
#ifdef _DEBUG
			WString wret(QDir::convertSeparators(ret));
			PRINT("Resolved to: %S\n", wret.getBuffer());
#endif
			return ret;
		}	
	}	
	return lnk;
}

