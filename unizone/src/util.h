#ifndef UTIL_H
#define UTIL_H

/*
 *
 *  Common functions
 *
 */

#include <qstring.h>
#include "util/String.h"

using namespace muscle;

QString ParseChatText(const QString & str);		// this is a whole different type of
												// parse... it looks for URL's etc.
void ParseString(QString & str);
QString ParseStringStr(const QString & str);

void EscapeHTML(QString & str);					// RUN THIS BEFORE ParseString()
QString EscapeHTMLStr(const QString & str);

void FixString(QString & str);
QString FixStringStr(const QString & str);

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

// Return first parameter if non-empty, otherwise second
const QString & CheckIfEmpty(const QString & str, const QString & str2);

// Calculate Checksum for raw buffer (borrowed from BeShare)
uint32 CalculateChecksum(const uint8 * data, size_t bufSize);

#endif
