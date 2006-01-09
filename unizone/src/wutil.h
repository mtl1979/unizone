#ifndef WUTIL_H
#define WUTIL_H

#ifdef _cplusplus
class QString;
#endif

#include "platform.h"

#ifdef WIN32
#  include <windows.h>
#else
// For Unicode support on Linux or FreeBSD???
#  include <wchar.h>
#  include <wctype.h>
#  include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef __APPLE__
wchar_t *wcsdup(const wchar_t *);
wchar_t *wcslwr(wchar_t *);
wchar_t *wcsupr(wchar_t *);
#endif

// Replace all instances of 'in' with 'out'
void wreplace(wchar_t *buffer, wchar_t in, wchar_t out);
// Concatenate 'src' and 'dest' at position 'pos'
void wcat(wchar_t *dest, const wchar_t *src, size_t pos);
// Copy 'len' characters of 'src' to 'dest'
void wcopy(wchar_t *dest, const wchar_t *src, size_t len);
// Reverse 'len' characters of 'src' to 'dest'
void wreverse(wchar_t *dest, const wchar_t *src, ssize_t len);

#ifdef __cplusplus
}

// Converts array of wchar_t to QString
QString wideCharToQString(const wchar_t *wide);

// Converts QString to pointer to array of wchar_t, pointer must be deleted when not needed anymore
wchar_t *qStringToWideChar(const QString &str);

#endif 

#endif