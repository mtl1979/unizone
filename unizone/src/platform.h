#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qstring.h>
#include "string.h"
#include "util/String.h"

using muscle::String;

#ifdef WIN32

#if defined(BUILD_WIN98)

#if !defined(FLASHWINFO) // Microsoft Visual C++ 6.0 doesn't define this!!!

#undef BUILD_WIN98
#pragma message ("platform.h: BUILD_WIN98 is not supported on your compiler, undefining BUILD_WIN98!")

#endif // !defined(FLASHWINFO)  

#endif // defined(BUILD_WIN98)

// Get a registry key's value
long GetRegKey( HKEY key, wchar_t *subkey, wchar_t *retdata, wchar_t value = NULL);

// Flash window specified by fWinHandle
void WFlashWindow(HWND fWinHandle);
#endif // WIN32

#endif // PLATFORM_H
