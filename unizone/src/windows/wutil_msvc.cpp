#include <windows.h>
#include <qstring.h>

#include "wutil.h"

void
wcopy (wchar_t *dest, const wchar_t *src, size_t len)
{
	for (unsigned int x = 0; x < len; x++)
		dest[x] = src[x];
	dest[len] = 0;
}

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

void
wcat(wchar_t *dest, const wchar_t *src, size_t pos)
{
	dest += pos;
	wcopy(dest, src, wcslen(src));
}

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
		wcopy(result, (const wchar_t *) str.unicode(), str.length());
		return result;
	}
	else
	{
		return NULL;
	}
}

