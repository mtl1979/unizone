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

/*
 *
 *  Common functions
 *
 */

// Converts array of wchar_t to QString
QString wideCharToQString(const wchar_t *wide);

// Converts QString to pointer to array of wchar_t, pointer must be deleted when not needed anymore
wchar_t *qStringToWideChar(const QString &str);

//
// <postmaster@raasu.org> 20021021
//

// Get parameters after command, f.ex. '/nick Unizone Binky' returns 'Unizone Binky' 
QString GetParameterString(QString qCommand);

// Get command from full string, converts to lower case for using with CompareCommand()
QString GetCommandString(QString qCommand);

// Compare command strings, uses GetCommandString to split parts
bool CompareCommand(QString qCommand, const char * cCommand);

// Strip urls from string
String StripURL(const String &);
String StripURL(const char *);
QString StripURL(const QString & u);

// Is the specified string an URL?
bool IsURL(const String &);

// Format a nice size string
QString MakeSizeString(uint64 s);

// Strip only spaces of, no other non-printable characters
String Trim(String orig);

// Convert Connection string to bytes per second
int32 BandwidthToBytes(QString connection);

// Convert bytes per second to Connection string
QString BandwidthToString(int32 bps);

#endif // PLATFORM_H
