#include <qstring.h>
#include <windows.h>
#include <shellapi.h>
# ifdef VC7
#  include <shldisp.h>	// hm... we only need this in VC7
# endif
#include <shlguid.h>
#include <shlobj.h>

#include "filethread.h"
#include "wstring.h"
#include "winsharewindow.h"
#include "global.h"

// Unicode version of ResolveLink for Windows 2000 and newer
// IShellLinkW is only supported on Windows 2000 and later
QString
WFileThread::ResolveLink(const QString & lnk) const
{
#ifdef DEBUG2
	WString wlnk(lnk);
	PRINT2("\tResolving %S\n", wlnk.getBuffer());
#endif
	
	if (lnk.right(4) == ".lnk")
	{
		// we've got a link...
		PRINT("Is Link\n");
		
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
				
				WString wret(ret);
				hres = ppf->Load(wret.getBuffer(), STGM_READ);
				if (SUCCEEDED(hres))
				{
					PRINT("Loaded\n");
					hres = psl->Resolve(gWin->winId(), SLR_NO_UI | SLR_ANY_MATCH);
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
							MessageBox(gWin->winId(), L"GetPath() failed!", L"Error", MB_OK);
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

