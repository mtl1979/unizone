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


//
// <postmaster@raasu.org> 20021021
//

// Get parameters after command, f.ex. '/nick Unizone Binky' returns 'Unizone Binky' 
QString GetParameterString(const QString & qCommand);

// Get command from full string, converts to lower case for using with CompareCommand()
QString GetCommandString(const QString & qCommand);

// Compare command strings, uses GetCommandString to split parts
bool CompareCommand(const QString & qCommand, const char * cCommand);

// Strip urls from string
String StripURL(const String &);
String StripURL(const char *);
QString StripURL(const QString & u);

// Is the specified string an URL?
bool IsURL(const String &);
bool IsURL(const char *); 
bool IsURL(const QString &);

// Is the specified string an action text?
bool IsAction(const QString & text, const QString & user);

// Format a nice size string
QString MakeSizeString(uint64 s);

// Strip only spaces off, no other non-printable characters
// String Trim(String orig);

// Convert 'Connection string' to 'bytes per second'
uint32 BandwidthToBytes(const QString & connection);

// Convert 'bytes per second' to 'Connection string'
QString BandwidthToString(uint32 bps);

// Get server name from full server URL
QString GetServerName(const QString & server);

// Get server port from full server URL
uint16 GetServerPort(const QString & server);

// Convert simple wildcard pattern to regex
void ConvertToRegex(String & s);

// Localize Month Names
QString TranslateMonth(const QString & m);

// Get nice time stamp ;)
QString GetTimeStamp();

// Calculate percentage and return string representation
QString ComputePercentString(int64 cur, int64 max);

// Reverse string
void Reverse(QString &text);

// Convert file name to node path equivalent
void MakeNodePath(String &file);

// Return full filepath from 'dir' and 'file'
String MakePath(const String &dir, const String &file);

// Return filename that has no invalid characters (Windows only)
QString FixFileName(const QString & fixMe);

#endif // PLATFORM_H
