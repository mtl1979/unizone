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

#include "fileinfo.h"
#include "filethread.h"
#include "wstring.h"
#include "winsharewindow.h"
#include "global.h"
#include "util.h"				// for endsWith()

// ANSI version of ResolveLink for Windows 95, 98 & ME and Microsoft Unicode Layer
// IShellLinkW is not supported on Windows 95, 98 or ME
QString
WFileThread::ResolveLink(const QString & lnk) const
{
	WString wlnk(QDir::convertSeparators(lnk));
#ifdef DEBUG2
	PRINT2("\tResolving %S\n", wlnk.getBuffer());
#endif
	
	if (endsWith(lnk, ".lnk"))
	{
		// we've got a link...
		PRINT("Is Link\n");
		
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
				hres = ppf->Load(wlnk.getBuffer(), STGM_READ);
				if (SUCCEEDED(hres))
				{
					PRINT("Loaded %S\n", wlnk.getBuffer());
					hres = psl->Resolve(gWin->winId(), SLR_NO_UI | SLR_ANY_MATCH);
					if (SUCCEEDED(hres))
					{
						PRINT("Resolved\n");
						hres = psl->GetPath(szFile, MAX_PATH, (WIN32_FIND_DATAA *)&wfd, SLGP_UNCPRIORITY);
						if (SUCCEEDED(hres))
						{
							PRINT("GetPath() = %s\n",szFile);
							ret = QString::fromLocal8Bit(szFile);
						}
						else
						{
							MessageBoxA(gWin->winId(), "GetPath() failed!", "Error", MB_OK);
						}
					}
				}						
				ppf->Release();
			}
			psl->Release();
#ifdef _DEBUG
			WString wret(ret);
			PRINT("Resolved to: %S\n", wret.getBuffer());
#endif
			return ret;
		}		
	}
	return lnk;
}

