#include <qstring.h>

#include "wstring.h"

char * utfbuf = NULL;
int utflen = 0;

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

WString::WString(const char * str)
{
#if defined(WIN32)
	int len = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), NULL, 0);
	buffer = new wchar_t[len+1];
	(void) MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), buffer, len);
#else
	int len = mbstowcs(NULL, str, MB_CUR_MAX);
	buffer = new wchar_t[len+1];
	(void) mbstowcs(buffer, str, len);
#endif
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

WString &
WString::operator+=(const wchar_t *str)
{
	if (length() != -1)
	{
		int newlen = length() + wcslen(str) + 1;
		wchar_t *buf2 = new wchar_t[newlen];
		if (buf2)
		{
			wcscpy(buf2, buffer);
			wcscat(buf2, str);
			setBuffer(buf2);
		}
	}
	else // No string to append to
	{
		buffer = new wchar_t[wcslen(str) + 1];
		if (buffer)
			wcscpy(buffer, str);
	}
	return *this;
}

WString &
WString::operator+=(const WString &str)
{
	if (length() != -1)
	{
		int newlen = length() + str.length() + 1;
		wchar_t *buf2 = new wchar_t[newlen];
		if (buf2)
		{
			wcscpy(buf2, buffer);
			wcscat(buf2, str.getBuffer());
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
		int newlen = length() + str.length() + 1;
		wchar_t *buf2 = new wchar_t[newlen];
		if (buf2)
		{
			WString s2(str);
			wcscpy(buf2, buffer);
			wcscat(buf2, s2.getBuffer());
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
	wchar_t *buf2 = str;
	return (wcscmp(buffer, buf2) != 0);
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
	wchar_t *buf2 = str;
	return (wcscmp(buffer, buf2) == 0);
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

#ifdef WIN32
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
		buf2 = wcsdup(buffer);
		if (buf2)
		{
			buf2 = wcsrev(buf2);
			s2.setBuffer(buf2);
		}
	}
	return s2;
}
#endif

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
	if (buffer)
	{
		wchar_t *b = buffer;
		do
		{
			if (*b == in)
				*b = out;
			b++;
		} while (*b != 0);
	}
}

/*
 *
 *  Conversion functions
 *
 */

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
#ifdef QT_QSTRING_UCS_4
		for (unsigned int i = 0; i < str.length(); ++i)
			result[i] = str.at(i).unicode();
#else
		memcpy(result, str.unicode(), str.length() * 2);
#endif
		result[str.length()] = 0;
		return result;
	}
	else
	{
		return NULL;
	}
}

