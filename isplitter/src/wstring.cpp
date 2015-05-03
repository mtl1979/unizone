#include <qstring.h>
#include <stdio.h>
#include <stdarg.h>

#include "wstring.h"
#include "wutil.h"

#if defined(WIN32) || defined(_WIN32)
#include "windows\vsscanf.h"
#include <stdlib.h>
#endif

WString::WString()
{
	buffer = NULL;
	utfbuf = NULL;
	utflen = 0;
}

WString::WString(const wchar_t *str)
{
	size_t len = wcslen(str);
	buffer = new wchar_t[len+1];
	if (buffer)
#if __STDC_WANT_SECURE_LIB__
		wcopy_s(buffer, len + 1, str, len);
#else
		wcopy(buffer, str, len);
#endif
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
#if defined(WIN32) || defined(_WIN32)
	len = MultiByteToWideChar(CP_UTF8, 0, str, (unsigned int) strlen(str), NULL, 0);
	buffer = new wchar_t[len+1];
	(void) MultiByteToWideChar(CP_UTF8, 0, str, (unsigned int) strlen(str), buffer, len);
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
#ifdef _MSC_VER
		buffer = _wcsdup(str);
#else
		buffer = wcsdup(str);
#endif
	}
	return *this;
}

WString &
WString::operator=(const WString &str)
{
	free();
	if (str.getBuffer())
	{
#ifdef _MSC_VER
		buffer = _wcsdup(str.getBuffer());
#else
		buffer = wcsdup(str.getBuffer());
#endif
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
	if (buffer && length() > 0)
	{
		size_t oldlen = length();
		size_t newlen = oldlen + wcslen(str) + 1;
		wchar_t *buf2 = (wchar_t *)realloc(buffer, newlen * sizeof(wchar_t));
		if (buf2)
		{
#if __STDC_WANT_SECURE_LIB__
			wcat_s(buf2, newlen, str, oldlen);
#else
			wcat(buf2, str, oldlen);
#endif
			setBuffer(buf2);
		}
	}
	else // No string to append to
	{
#ifdef _MSC_VER
		buffer = _wcsdup(str);
#else
		buffer = wcsdup(str);
#endif
	}
	return *this;
}

WString &
WString::operator+=(const WString &str)
{
	if (buffer && length() > 0)
	{
		size_t oldlen = length();
		size_t newlen = oldlen + str.length() + 1;
		wchar_t *buf2 = (wchar_t *)realloc(buffer, newlen * sizeof(wchar_t));
		if (buf2)
		{
#if __STDC_WANT_SECURE_LIB__
			wcat_s(buf2, newlen, str.getBuffer(), oldlen);
#else
			wcat(buf2, str.getBuffer(), oldlen);
#endif
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
	if (buffer && length() > 0)
	{
		size_t oldlen = length();
		size_t newlen = oldlen + str.length() + 1;
		wchar_t *buf2 = (wchar_t *)realloc(buffer, newlen * sizeof(wchar_t));
		if (buf2)
		{
			WString s2(str);
#if __STDC_WANT_SECURE_LIB__
			wcat_s(buf2, newlen, s2.getBuffer(), oldlen);
#else
			wcat(buf2, s2.getBuffer(), oldlen);
#endif
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
	size_t len;
#if __STDC_WANT_SECURE_LIB__
	wcstombs_s(&len, NULL, 0, buffer, 0);
#else
	len = wcstombs(NULL, buffer, 0);
#endif
	if (utfbuf && (len > utflen))
	{
		char * newbuf = (char *) realloc(utfbuf, len + 1);
		if (newbuf != NULL)
		{
			utfbuf = newbuf;
			utflen = len;
		}
		else
		{
			delete utfbuf;
			utfbuf = NULL;
		}
	}
	if (!utfbuf)
	{
		utfbuf = new char[len + 1];
		utflen = len;
	}
	if (len > 0)
#if __STDC_WANT_SECURE_LIB__
		wcstombs_s(&len, utfbuf, utflen, buffer, utflen);
#else
		len = wcstombs(utfbuf, buffer, utflen);
#endif
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
#ifdef _MSC_VER
		buf2 = _wcsdup(buffer);
#else
		buf2 = wcsdup(buffer);
#endif
		if (buf2)
		{
#ifdef _MSC_VER
#  if __STDC_WANT_SECURE_LIB__
			_wcsupr_s(buf2, wcslen(buf2));
#  else
			buf2 = _wcsupr(buf2);
#  endif
#else
			buf2 = wcsupr(buf2);
#endif
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
#ifdef _MSC_VER
		buf2 = _wcsdup(buffer);
#else
		buf2 = wcsdup(buffer);
#endif
		if (buf2)
		{
#ifdef _MSC_VER
#  if __STDC_WANT_SECURE_LIB__
			_wcslwr_s(buf2, wcslen(buf2));
#  else
			buf2 = _wcslwr(buf2);
#  endif
#else
			buf2 = wcslwr(buf2);
#endif
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
		size_t len = wcslen(buffer);
		buf2 = new wchar_t[len + 1];
		wreverse(buf2, buffer, len);
		s2.setBuffer(buf2);
	}
	return s2;
}

size_t
WString::length() const
{
	if (buffer)
		return wcslen(buffer);
	else
		return 0;
}

void
WString::replace(wchar_t in, wchar_t out)
{
	wreplace(buffer, in, out);
}

int
WString::sscanf(const WString &fmt, ...)
{
	int ret = -1;
	if (buffer)
	{
		va_list a;
		va_start(a, fmt);
		ret = vswscanf(buffer, fmt.getBuffer(), a);
		va_end(a);
	}
	return ret;
}

int
WString::sscanf(const wchar_t *fmt, ...)
{
	int ret = -1;
	if (buffer)
	{
		va_list a;
		va_start(a, fmt);
		ret = vswscanf(buffer, fmt, a);
		va_end(a);
	}
	return ret;
}

// This handy helper function increases buffer size as long as the result would get truncated or we run out of memory
int
WString::_sprintf_internal(const size_t &bufsize, const wchar_t *fmt, va_list a)
{
	int ret = -1;

	if (fmt == NULL)
		return -1;

	va_list a2;		// Copy state of a to a2, so we can recurse if buffer is too small
	va_copy(a2, a);

	// Check current string length, reallocate if bufsize is not one more
	size_t len = length();

	if (buffer && (len == 0 || (len + 1) < bufsize))
	{
		wchar_t *newbuf = (wchar_t *)realloc(buffer, bufsize * sizeof(wchar_t));
		if (newbuf == NULL)
		{
			va_end(a2);
			return -1;
		}
		buffer = newbuf;
	}

	if (buffer == NULL)
		buffer = new wchar_t[bufsize];

	if (buffer == NULL)
	{
		va_end(a2);
		return -1;
	}

#if defined(WIN32) || defined(_WIN32)
#  if __STDC_WANT_SECURE_LIB__
	RtlSecureZeroMemory(buffer, bufsize * sizeof(wchar_t));
	ret = _vsnwprintf_s(buffer, bufsize - 1, _TRUNCATE, fmt, a);
#  else
	RtlZeroMemory(buffer, bufsize * sizeof(wchar_t));
	ret = _vsnwprintf(buffer, bufsize - 1, fmt, a);
#  endif
#else
	memset(buffer, 0, bufsize * sizeof(wchar_t));
	ret = vswprintf(buffer, bufsize - 1, fmt, a);
#endif
	if (ret == -1) // Recurse with bigger buffer if truncated
	{
		ret = _sprintf_internal(bufsize + 32, fmt, a2);
	}
	va_end(a2);
	return ret;
}

int
WString::sprintf(const WString &fmt, ...)
{
	int ret;
	va_list a;
	va_start(a, fmt);
	ret = _sprintf_internal(max(length(), 32), fmt.getBuffer(), a);
	va_end(a);
	return ret;
}

int
WString::sprintf(const wchar_t *fmt, ...)
{
	int ret = -1;
	va_list a;
	va_start(a, fmt);
	ret = _sprintf_internal(max(length(), 32), fmt, a);
	va_end(a);
	return ret;
}
