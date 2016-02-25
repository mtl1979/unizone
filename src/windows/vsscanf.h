#ifndef VSSCANF_H
#define VSSCANF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#if _MSC_VER < 1800
int __cdecl vswscanf (const wchar_t* str, const wchar_t *format, va_list arglist);
int __cdecl vsscanf (const char* str, const unsigned char *format, va_list arglist);
#endif

#ifdef __cplusplus
}
#endif

#endif
