#include "uenv.h"
#if (QT_VERSION < 0x030000) && defined(UNICODE)
# include "wstring.h"
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

QString EnvironmentVariable(const QString &name)
{
#ifdef WIN32
# ifdef UNICODE
#  if (QT_VERSION < 0x030000)
	WString wname(name);
#  endif
	wchar_t buf[MAX_PATH];
#  if (QT_VERSION < 0x030000)
	long hRes = GetEnvironmentVariable(wname.getBuffer(), buf, MAX_PATH);
#  else
	long hRes = GetEnvironmentVariable(name.ucs2(), buf, MAX_PATH);
#  endif
	if ( hRes > 0 && hRes < MAX_PATH)
	{
#if (QT_VERSION < 0x030000)
		WString temp(buf);
		return temp.toQString();
#else
		return QString::fromUcs2(buf);
#endif
	}
	return QString::null;
# else
	QCString qcname = name.local8Bit();
	const char * cname = (const char *) qcname;
	char buf[MAX_PATH];
	long hRes = GetEnvironmentVariableA(cname, buf, MAX_PATH);
	if (hRes > 0 && hRes < MAX_PATH)
		return QString::fromLocal8Bit(buf);
	return QString::null;
# endif
#else
	QCString qcname = name.local8Bit();
	char * out = getenv((const char *) qcname);
	return out ? QString::fromLocal8Bit(out) : QString::null;
#endif
}
