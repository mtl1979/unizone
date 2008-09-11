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
	WString wname(name);
	wchar_t buf[MAX_PATH];
	long hRes = GetEnvironmentVariable(wname.getBuffer(), buf, MAX_PATH);
	if ( hRes > 0 && hRes < MAX_PATH)
	{
		WString temp(buf);
		return temp.toQString();
	}
	return QString::null;
#else
	QByteArray qcname = name.local8Bit();
	char * out = getenv((const char *) qcname);
	return out ? QString::fromLocal8Bit(out) : QString::null;
#endif
}
