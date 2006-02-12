#ifndef VSSCANF_H
#define VSSCANF_H

#ifdef __cplusplus
extern "C" {
#endif

int __cdecl vwsscanf (const wchar_t* str, const wchar_t *format, va_list arglist);
int __cdecl vsscanf (const char* str, const unsigned char *format, va_list arglist);

#ifdef __cplusplus
}
#endif

#endif
