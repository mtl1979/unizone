#include <windows.h>
#include <qstring.h>

#include "wutil.h"

#if __STDC_WANT_SECURE_LIB__
errno_t
wcopy_s(wchar_t *dest, size_t destlen, const wchar_t *src, size_t len)
{
	return wcsncpy_s(dest, destlen, src, len);
}
#else
void
wcopy(wchar_t *dest, const wchar_t *src, size_t len)
{
#if _MSC_VER >= 1800
	wcsncpy(dest, src, len);
#else
	for (unsigned int x = 0; x < len; x++)
		dest[x] = src[x];
#endif
	dest[len] = 0;
}
#endif

void
wreplace(wchar_t *buffer, wchar_t win, wchar_t wout)
{
	if (buffer)
	{
		wchar_t *b = buffer;
		while (*b != 0)
		{
			if (*b == win)
				*b = wout;
			b++;
		};
	}
}

#if __STDC_WANT_SECURE_LIB__
errno_t
wcat_s(wchar_t *dest, size_t destlen, const wchar_t *src, size_t pos)
{
	wchar_t *dpos = dest + pos;
	size_t dlen = destlen - pos;
	return wcopy_s(dpos, dlen, src, wcslen(src));
}
#else
void
wcat(wchar_t *dest, const wchar_t *src, size_t pos)
{
	wchar_t *dpos = dest + pos;
	wcopy(dpos, src, wcslen(src));
}
#endif

void
wreverse(wchar_t *dest, const wchar_t *src, size_t len)
{
	if (dest)
	{
		int dpos = 0;
		size_t spos = len - 1;
		while (spos >= 0)
			dest[dpos++] = src[spos--];
		dest[len] = 0;
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
	result.setUnicodeCodes((const ushort *)wide, lstrlenW(wide));
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
#if __STDC_WANT_SECURE_LIB__
		wcopy_s(result, str.length() + 1, (const wchar_t *)str.unicode(), str.length());
#else
		wcopy(result, (const wchar_t *) str.unicode(), str.length());
#endif
		return result;
	}
	else
	{
		return NULL;
	}
}

