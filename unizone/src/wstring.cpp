#include <qstring.h>

#include "wstring.h"

WString::WString()
{
	buffer = NULL;
}

WString::WString(const wchar_t *str)
{
	int len = wcslen(str);
	buffer = new wchar_t[len+1];
	if (buffer)
		wcscpy(buffer, str);
}

WString::WString(const QString &str)
{
	buffer = qStringToWideChar(str);
}

WString::~WString()
{
	free();
}


QString 
WString::toQString() const
{
	return wideCharToQString(buffer);
}

WString &
WString::operator=(const wchar_t *str)
{
	free();
	if (str)
	{
		int len = wcslen(str);
		buffer = new wchar_t[len+1];
		if (buffer)
			wcscpy(buffer, str);
	}
	return *this;
}

WString &
WString::operator=(const WString &str)
{
	wchar_t * buf2 = str;
	free();
	if (buf2)
	{
		int len = wcslen(buf2);
		buffer = new wchar_t[len+1];
		if (buffer)
			wcscpy(buffer, buf2);
	}
	return *this;
}

WString &
WString::operator=(const QString &str)
{
	free();
	buffer = qStringToWideChar(str);
	return *this;
}

bool
WString::operator!=(const wchar_t *str)
{
	return (wcscmp(buffer, str) != 0);
}

bool 
WString::operator!=(const WString &str)
{
	wchar_t *buf2 = str;
	return (wcscmp(buffer, buf2) != 0);
}

bool 
WString::operator!=(const QString &str)
{
	WString s2 = str;
	wchar_t *buf2 = s2;
	bool b = (wcscmp(buffer, buf2) != 0);
	return b;
}

bool 
WString::operator==(const wchar_t *str)
{
	return (wcscmp(buffer, str) == 0);
}

bool 
WString::operator==(const WString &str)
{
	wchar_t *buf2 = str;
	return (wcscmp(buffer, buf2) == 0);
}

bool 
WString::operator==(const QString &str)
{
	WString s2 = str;
	wchar_t *buf2 = s2;
	bool b = (wcscmp(buffer, buf2) == 0);
	return b;
}

void
WString::free()
{
	if (buffer)
	{
		delete [] buffer;
		buffer = NULL;
	}
}

QString 
wideCharToQString(const wchar_t *wide)
{
    QString result;
#ifdef WIN32
    result.setUnicodeCodes(wide, lstrlenW(wide));
#else
    result.setUnicodeCodes((const ushort *) wide, wcslen(wide));
#endif
    return result;
}

wchar_t *
qStringToWideChar(const QString &str)
{
   	if (str.isNull())
	{
       	return NULL;
	}

	wchar_t *result = new wchar_t[str.length() + 1];
	if (result)
	{
		for (unsigned int i = 0; i < str.length(); ++i)
			result[i] = str.at(i).unicode();
		result[str.length()] = 0;
		return result;
	}
	else
	{
		return NULL;
	}
}
