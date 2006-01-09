#include <qstring.h>

#include "wstring.h"
#include "wutil.h"

WString::WString()
{
	buffer = NULL;
	utfbuf = NULL;
	utflen = 0;
}

WString::WString(const wchar_t *str)
{
	int len = wcslen(str);
	buffer = new wchar_t[len+1];
	if (buffer)
		wcopy(buffer, str, len);
	utfbuf = NULL;
	utflen = 0;
}

WString::WString(const QString &str)
{
	buffer = qStringToWideChar(str);
	utfbuf = NULL;
	utflen = 0;
}

WString::WString(const char * str)
{
	int len;
#if defined(WIN32)
	len = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), NULL, 0);
	buffer = new wchar_t[len+1];
	(void) MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), buffer, len);
#else
	len = mbstowcs(NULL, str, MB_CUR_MAX);
	buffer = new wchar_t[len+1];
	(void) mbstowcs(buffer, str, len);
#endif
	buffer[len] = 0;
	utfbuf = NULL;
	utflen = 0;
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
			wcopy(buffer, str, len);
	}
	return *this;
}

WString &
WString::operator=(const WString &str)
{
	free();
	if (str.getBuffer())
	{
		int len = str.length();
		buffer = new wchar_t[len+1];
		if (buffer)
			wcopy(buffer, str.getBuffer(), len);
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

WString &
WString::operator+=(const wchar_t *str)
{
	if (length() != -1)
	{
		int oldlen = length();
		int newlen = oldlen + wcslen(str) + 1;
		wchar_t *buf2 = new wchar_t[newlen];
		if (buf2)
		{
			wcopy(buf2, buffer, oldlen);
			wcat(buf2, str, oldlen);
			setBuffer(buf2);
		}
	}
	else // No string to append to
	{
		int newlen = wcslen(str);
		buffer = new wchar_t[newlen + 1];
		if (buffer)
			wcopy(buffer, str, newlen);
	}
	return *this;
}

WString &
WString::operator+=(const WString &str)
{
	if (length() != -1)
	{
		int oldlen = length();
		int newlen = oldlen + str.length() + 1;
		wchar_t *buf2 = new wchar_t[newlen];
		if (buf2)
		{
			wcopy(buf2, buffer, oldlen);
			wcat(buf2, str.getBuffer(), oldlen);
			setBuffer(buf2);
		}
	}
	else // No string to append to
	{
		operator=(str);
	}
	return *this;
}

WString &
WString::operator+=(const QString &str)
{
	if (length() != -1)
	{
		int oldlen = length();
		int newlen = oldlen + str.length() + 1;
		wchar_t *buf2 = new wchar_t[newlen];
		if (buf2)
		{
			WString s2(str);
			wcopy(buf2, buffer, oldlen);
			wcat(buf2, s2.getBuffer(), oldlen);
			setBuffer(buf2);
		}
	}
	else // No string to append to
	{
		operator=(str);
	}
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
	return (wcscmp(buffer, str.getBuffer()) != 0);
}

bool 
WString::operator!=(const QString &str)
{
	WString s2(str);
	bool b = (wcscmp(buffer, s2.getBuffer()) != 0);
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
	return (wcscmp(buffer, str.getBuffer()) == 0);
}

bool 
WString::operator==(const QString &str)
{
	WString s2(str);
	bool b = (wcscmp(buffer, s2.getBuffer()) == 0);
	return b;
}

WString::operator const char *() const
{
	int len = wcstombs(NULL, buffer, 0);
	if (utfbuf && (len > utflen))
	{
		delete utfbuf;
		utfbuf = NULL;
	}
	if (!utfbuf)
	{
		utfbuf = new char[len + 1];
		utflen = len;
	}
	len = wcstombs(utfbuf, buffer, utflen);
	utfbuf[len] = 0;
	return utfbuf;
}

void
WString::free()
{
	if (buffer)
	{
		delete [] buffer;
		buffer = NULL;
	}
	if (utfbuf)
	{
		delete [] utfbuf;
		utfbuf = NULL;
		utflen = 0;
	}
}

void
WString::setBuffer(wchar_t *buf)
{
	free();
	buffer = buf;
}

WString
WString::upper() const 
{
	WString s2;
	if (buffer)
	{
		wchar_t *buf2;
		buf2 = wcsdup(buffer);
		if (buf2)
		{
			buf2 = wcsupr(buf2);
			s2.setBuffer(buf2);
		}
	}
	return s2;
}

WString
WString::lower() const
{
	WString s2;
	if (buffer)
	{
		wchar_t *buf2;
		buf2 = wcsdup(buffer);
		if (buf2)
		{
			buf2 = wcslwr(buf2);
			s2.setBuffer(buf2);
		}
	}
	return s2;
}

WString
WString::reverse() const
{
	WString s2;
	if (buffer)
	{
		wchar_t *buf2;
		int len = wcslen(buffer);
		buf2 = new wchar_t[len + 1];
		wreverse(buf2, buffer, len);
		s2.setBuffer(buf2);
	}
	return s2;
}

int
WString::length() const
{ 
	if (buffer) 
		return wcslen(buffer);
	else
		return -1;
}

void
WString::replace(wchar_t in, wchar_t out)
{
	wreplace(buffer, in, out);
}

