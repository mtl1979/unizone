#include <qstring.h>

#include "wutil.h"

#if defined(QT_QSTRING_UCS_4)
# if defined(wcsncpy)
void wcopy(wchar_t *dest, const wchar_t *src, size_t len)
{
	wcsncpy(dest, src, len);
	dest[len] = 0;
}
# else
void wcopy(wchar_t *dest, const wchar_t *src, size_t len)
{
	for (unsigned int x = 0; x < len; x++)
		dest[x] = src[x];
	dest[len] = 0;
}
# endif
#else
void wcopy(wchar_t *dest, const wchar_t *src, size_t len)
{
	dest[len] = 0;
	if (len == 0) return;
	len--;
	if ((len % 2) == 0)
	{
		dest[len] = src[len];
		if (len == 0) return;
		len--;
	}
	len--;
	const uint32 *s = (const uint32 *) src;
	uint32 *d = (uint32 *) dest;
	len /= 2;
	while (1)
	{
		d[len] = s[len];
		if (len == 0) return;
		len--;
	}
}
#endif // wcsncpy
#endif // QT_QSTRING_UCS_4

void wcat(wchar_t *dest, wchar_t *src, size_t pos)
{
	dest += pos;
	wcopy(dest, src, wcslen(src));
}

void wreverse(wchar_t *dest, const wchar_t *src, ssize_t len)
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
wchar_t *wcsupr(wchar_t *s)
{
	wchar_t *b = s;
	while (*b != 0)
	{
		*b = towupper( *b );
		b++;
	}
	return s;
}

wchar_t *wcslwr(wchar_t *s)
{
	wchar_t *b = s;
	while (*b != 0)
	{
		*b = towlower( *b );
		b++;
	}
	return s;
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
    result.setUnicodeCodes((const ushort *) wide, wcslen(wide));
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

