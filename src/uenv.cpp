#include <QString>

#include "uenv.h"

#ifdef _WIN32
#include <windows.h>
#include "wstring.h"
#else
#include <stdlib.h>
#include <QByteArray>
#endif

QString EnvironmentVariable(const QString &name)
{
#ifdef _WIN32
#  ifdef UNICODE
	WString wname(name);
	wchar_t buf[MAX_PATH];
	long hRes = GetEnvironmentVariable(wname.getBuffer(), buf, MAX_PATH);
#  else
	char buf[MAX_PATH];
	long hRes = GetEnvironmentVariable(name.local8Bit(), buf, MAX_PATH);
#  endif
	if ( hRes > 0 && hRes < MAX_PATH)
	{
#  ifdef UNICODE
		WString temp(buf);
		return temp.toQString();
#  else
		return QString::fromLocal8Bit(buf);
#  endif
	}
	return QString::null;
#else
	QByteArray qcname = name.local8Bit();
	char * out = getenv((const char *) qcname);
	return out ? QString::fromLocal8Bit(out) : QString::null;
#endif
}
