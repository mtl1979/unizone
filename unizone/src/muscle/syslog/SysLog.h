/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.TXT file for details. */

#ifndef MuscleSysLog_h
#define MuscleSysLog_h

#include <stdarg.h>
#include "util/TimeUtilityFunctions.h"

namespace muscle {

/** log level constants to use with SetLogLevel(), GetLogLevel() */
enum
{
   MUSCLE_LOG_NONE = 0,         // nothing ever gets logged at this level (default)
   MUSCLE_LOG_CRITICALERROR,    // only things that should never ever happen
   MUSCLE_LOG_ERROR,            // things that shouldn't usually happen
   MUSCLE_LOG_WARNING,          // things that are suspicious
   MUSCLE_LOG_INFO,             // things that the user might like to know
   MUSCLE_LOG_DEBUG,            // things the programmer is debugging with
   MUSCLE_LOG_TRACE,            // exhaustively detailed output
   NUM_MUSCLE_LOGLEVELS
}; 

// Define this constant in your Makefile (i.e. -DMUSCLE_MINIMALIST_LOGGING) if you don't want to have
// to link in SysLog.cpp and Hashtable.cpp and all the other stuff that is required for "real" logging.
#ifdef MUSCLE_MINIMALIST_LOGGING

// Minimalist version of Log(), just sends the output to stdout.
inline status_t Log(int, const char * fmt, ...) {va_list va; va_start(va, fmt); vprintf(fmt, va); va_end(va); return B_NO_ERROR;}

// Minimalist version of LogTime(), just sends a tiny header and the output to stdout.
inline status_t LogTime(int logLevel, const char * fmt, ...) {printf("%i: ", logLevel); va_list va; va_start(va, fmt); vprintf(fmt, va); va_end(va); return B_NO_ERROR;}

// Minimumist version of LogFlush(), just flushes stdout
inline status_t LogFlush() {fflush(stdout);}

// Minimalist version of LogStackTrace(), just prints a dummy string
inline status_t LogStackTrace() {printf("<stack trace omitted>\n");

#else

/** Returns the MUSCLE_LOG_* equivalent of the given keyword string 
 *  @param keyword a string such as "debug", "log", or "info".
 *  @return A MUSCLE_LOG_* value
 */
int ParseLogLevelKeyword(const char * keyword);

/** Returns the current log level for logging to a file.
 *  @return a MUSCLE_LOG_* value.
 */
int GetFileLogLevel();

/** Returns the current log level for logging to stdout.
 *  @return a MUSCLE_LOG_* value.
 */
int GetConsoleLogLevel();

/** Returns the max of GetFileLogLevel() and GetConsoleLogLevel()
 *  @return a MUSCLE_LOG_* value
 */
int GetMaxLogLevel();

/** Sets the log filter level for logging to a file.  
 *  Any calls to Log*() that specify a log level greater than (loglevel)
 *  will be suppressed.  Default level is MUSCLE_LOG_NONE (i.e. no file logging is done)
 *  @param loglevel The MUSCLE_LOG_* value to use in determining which log messages to save to disk.
 *  @returns B_NO_ERROR on success, or B_ERROR if the log lock couldn't be locked for some reason.
 */
status_t SetFileLogLevel(int loglevel);

/** Sets the log filter level for logging to stdout.
 *  Any calls to Log*() that specify a log level greater than (loglevel)
 *  will be suppressed.  Default level is MUSCLE_LOG_INFO.
 *  @param loglevel The MUSCLE_LOG_* value to use in determining which log messages to print to stdout.
 *  @returns B_NO_ERROR on success, or B_ERROR if the log lock couldn't be locked for some reason.
 */
status_t SetConsoleLogLevel(int loglevel);

/** Same semantics as printf, only outputs to the log file/console instead
 *  @param logLevel a MUSCLE_LOG_* value indicating the "severity" of this message.
 *  @fmt A printf-style format string (e.g. "hello %s\n").  Note that \n is NOT added for you.
 *  @returns B_NO_ERROR on success, or B_ERROR if the log lock couldn't be locked for some reason.
 */
status_t Log(int logLevel, const char * fmt, ...);

/** Formatted.  Automagically prepends a timestamp and status string to your string.
 *  e.g. LogTime(MUSCLE_LOG_INFO, "Hello %s!", "world") would generate "[I 12/18 12:11:49] Hello world!"
 *  @param logLevel a MUSCLE_LOG_* value indicating the "severity" of this message.
 *  @fmt A printf-style format string (e.g. "hello %s\n").  Note that \n is NOT added for you.
 *  @returns B_NO_ERROR on success, or B_ERROR if the log lock couldn't be locked for some reason.
 */
status_t LogTime(int logLevel, const char * fmt, ...);

/** Ensures that all previously logged output is actually sent.  That is, it simply 
 *  calls fflush() on any streams that we are logging to.
 *  @returns B_NO_ERROR on success, or B_ERROR if the log lock couldn't be locked for some reason.
 */
status_t LogFlush();

/** Logs out a stack trace, if possible.  Returns B_ERROR if not.
 *  @note Currently only works under Linux.
 *  @param logLevel a MUSCLE_LOG_* value indicating the "severity" of this message.
 *  @returns B_NO_ERROR on success, or B_ERROR if a stack trace couldn't be logged because the platform doesn't support it.
 */
status_t LogStackTrace(int logLevel);

/** Returns a human-readable string for the given log level.
 *  @param logLevel A MUSCLE_LOG_* value
 *  @return A pretty human-readable description string such as "Informational" or "Warnings and Errors Only"
 */
const char * GetLogLevelName(int logLevel);

/** Returns a brief human-readable string for the given log level.
 *  @param logLevel A MUSCLE_LOG_* value
 *  @return A brief human-readable description string such as "info" or "warn"
 */
const char * GetLogLevelKeyword(int logLevel);

/** Writes a standard text string of the format "[L mm/dd hh:mm:ss]" into (buf).
 *  @param buf Char buffer to write into.  Should be at least 20 chars long.
 *  @param logLevel the MUSCLE_LOG_* token indicating the severity of the notice.
 *  @param when time/date stamp of the message
 */
void GetStandardLogLinePreamble(char * buf, int logLevel, time_t when);

#endif

};  // end namespace muscle

#endif
