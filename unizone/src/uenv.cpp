#include "uenv.h"
#if defined(UNICODE)
# include "wstring.h"
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <stdlib.h>
//Added by qt3to4:
#include <Q3CString>
#endif

QString EnvironmentVariable(const QString &name)
{
#ifdef WIN32
# ifdef UNICODE
	WString wname(name);
	wchar_t buf[MAX_PATH];
	long hRes = GetEnvironmentVariable(wname.getBuffer(), buf, MAX_PATH);
	if ( hRes > 0 && hRes < MAX_PATH)
	{
		WString temp(buf);
		return temp.toQString();
	}
	return QString::null;
# else
	Q3CString qcname = name.local8Bit();
	const char * cname = (const char *) qcname;
	char buf[MAX_PATH];
	long hRes = GetEnvironmentVariableA(cname, buf, MAX_PATH);
	if (hRes > 0 && hRes < MAX_PATH)
		return QString::fromLocal8Bit(buf);
	return QString::null;
# endif
#else
	Q3CString qcname = name.local8Bit();
	char * out = getenv((const char *) qcname);
	return out ? QString::fromLocal8Bit(out) : QString::null;
#endif
}
