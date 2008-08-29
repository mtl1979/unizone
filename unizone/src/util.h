#ifndef UTIL_H
#define UTIL_H

/*
 *
 *  Common functions
 *
 */

class QString;
class WFile;

#include "util/String.h"
#include "util/ByteBuffer.h"
#include "message/Message.h"

using namespace muscle;

#include <qregexp.h>

// this is a whole different type of parse... it looks for URL's etc.
QString ParseChatText(const QString & str);		

QString ParseString(const QString & str);

// RUN THIS BEFORE ParseString()
QString EscapeHTML(const QString & str);

QString FixString(const QString & str);

//
// <postmaster@raasu.org> 20021021
//

// Get parameters after command, f.ex. '/nick Unizone Binky' returns 'Unizone Binky' 
QString GetParameterString(const QString & qCommand);

// Get command from full string, converts to lower case for using with CompareCommand()
QString GetCommandString(const QString & qCommand);

// Compare command strings, uses GetCommandString to split parts
bool CompareCommand(const QString & qCommand, const QString & cCommand);

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
void ConvertToRegex(QString & s, bool simple = false);
bool HasRegexTokens(const QString & s);

// Localize Month Names
QString TranslateMonth(const QString & m);

// Get nice time stamp ;)
QString GetTimeStamp();
QString GetTimeStamp2();
QString GetTimeStampAux(const QString & stamp);
QString GetDateStampAux(const QString & stamp);

// Calculate percentage and return string representation
QString ComputePercentString(int64 cur, int64 max);

// Reverse string
void Reverse(QString &text);

// Convert file name to node path equivalent
void MakeNodePath(QString &file);

// Return full filepath from 'dir' and 'file'
String MakePath(const String &dir, const String &file);
QString MakePath(const QString &dir, const QString &file);

// Return filename that has no invalid characters (Windows only)
QString FixFileName(const QString & fixMe);

// Return first parameter if non-empty, otherwise second
const QString & CheckIfEmpty(const QString & str, const QString & str2);

// Calculate Checksum for raw buffer (borrowed from BeShare)
uint32 CalculateFileChecksum(const ByteBufferRef &buf);

// Return (first) IPv4 address from hostname
uint32 GetHostByName(const QString &name);


QString UniqueName(const QString & file, int index); // build up unique name using 'file' and 'index'

// Save picture to file, makes sure file doesn't exist before writing...
void SavePicture(QString & file, const ByteBufferRef & buf);

// Close file if necessary and delete the object
void CloseFile(WFile * & file);

uint64 toULongLong(const QString &, bool * = NULL);
QString fromULongLong(const uint64 &);
QString hexFromULongLong(const uint64 &, int);

int64 toLongLong(const QString &, bool * = NULL);
QString fromLongLong(const int64 &);
QString hexFromLongLong(const int64 &, int);

void HEXClean(QString &);
void BINClean(QString &);
void OCTClean(QString &);

QString BINEncode(const QString &);
QString BINDecode(const QString &);

QString OCTEncode(const QString &);
QString OCTDecode(const QString &);

void AddToList(QString &slist, const QString &entry);
void AddToList(String &slist, const String &entry);

void RemoveFromList(QString &slist, const QString &entry);
void RemoveFromList(String &slist, const String &entry);

bool Contains(const QString &slist, const QString &entry); 

int Match(const QString &string, const QRegExp &exp);

int64 ConvertPtr(void *);

bool startsWith(const QString &str1, const QString &str2, bool cs = true);
bool endsWith(const QString &str1, const QString &str2, bool cs = true);

bool startsWith(const QString &str1, const QChar &str2, bool cs = true);
bool endsWith(const QString &str1, const QChar &str2, bool cs = true);

bool BinkyCheck(const QString &user);

QString downloadDir(const QString & = QString::null);
QString imageFormats();

QString WikiEscape(const QString &page);
QString URLEscape(const QString &page);

status_t GetStringFromMessage(const MessageRef &msg, const String key, QString &value);
status_t GetStringFromMessage(const MessageRef &msg, const String key, uint32 index, QString &value);
status_t GetInt32FromMessage(const MessageRef &msg, const String key, int32 &value);
status_t GetUInt32FromMessage(const MessageRef &msg, const String key, uint32 &value);

status_t AddStringToMessage(const MessageRef &msg, const String key, const QString &value);

status_t ReplaceStringInMessage(const MessageRef &msg, bool okayToAdd, const String key, const QString &value);
status_t ReplaceStringInMessage(const MessageRef &msg, bool okayToAdd, const String key, uint32 index, const QString &value);

QString SimplifyPath(const QString &path);

#endif
