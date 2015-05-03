#include <qstring.h>

#include "wutil.h"

#if defined(wcsncpy)
void wcopy(wchar_t *dest, const wchar_t *src, size_t len)
{
	wcsncpy(dest, src, len);
	dest[len] = 0;
}
#else
void wcopy(wchar_t *dest, const wchar_t *src, size_t len)
{
	for (unsigned int x = 0; x < len; x++)
		dest[x] = src[x];
	dest[len] = 0;
}
#endif // wcsncpy

void wcat(wchar_t *dest, const wchar_t *src, size_t pos)
{
	wchar_t *dpos = dest + pos;
	wcopy(dpos, src, wcslen(src));
}

void wreverse(wchar_t *dest, const wchar_t *src, size_t len)
{
	if (dest)
	{
		int dpos = 0;
		int spos = len - 1;
		while (spos >= 0)
			dest[dpos++] = src[spos--];
		dest[len] = 0;
	}
}

void wreplace(wchar_t *buffer, wchar_t in, wchar_t out)
{
	if (buffer)
	{
		wchar_t *b = buffer;
		while (*b != 0)
		{
			if (*b == in)
				*b = out;
			b++;
		};
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
    result.setUnicodeCodes((const ushort *) wide, wcslen(wide) * (sizeof(wchar_t) / 2));
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
		for (int i = 0; i < str.length(); ++i)
			result[i] = str.at(i).unicode();
		result[str.length()] = 0;
		return result;
	}
	else
	{
		return NULL;
	}
}

