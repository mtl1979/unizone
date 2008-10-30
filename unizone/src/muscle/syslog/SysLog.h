/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.TXT file for details. */

#ifndef MuscleSysLog_h
#define MuscleSysLog_h

#include <stdarg.h>
#include "util/TimeUtilityFunctions.h"

BEGIN_NAMESPACE(muscle);

class String;

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

/** This is similar to LogStackTrace(), except that the stack trace is printed directly
  * to stdout (or another file you specify) instead of via calls to Log() and LogTime().  
  * This call is handy when you need to print a stack trace in situations where the log
  * isn't available.
  * @param optFile If non-NULL, the text will be printed to this file.  If left as NULL, stdout will be used as a default.
  * @param maxDepth The maximum number of levels of stack trace that we should print out.  Defaults to
  *                 64.  The absolute maximum is 256; if you specify a value higher than that, you will still get 256.
  * @note This function is currently only implemented under Linux and MacOS/X Leopard; for other OS's, this function is a no-op.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t PrintStackTrace(FILE * optFile = NULL, uint32 maxDepth = 64);

/** Similar to LogStackTrace(), except that the current stack trace is returned as a String
  * instead of being printed out anywhere.
  * @param retStr On success, the stack trace is written to this String object.
  * @param maxDepth The maximum number of levels of stack trace that we should print out.  Defaults to
  *                 64.  The absolute maximum is 256; if you specify a value higher than that, you will still get 256.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  * @note This function is currently only implemented under Linux and MacOS/X Leopard; for other OS's, this function is a no-op.
  */
status_t GetStackTrace(String & retStr, uint32 maxDepth = 64);

// Define this constant in your Makefile (i.e. -DMUSCLE_DISABLE_LOGGING) to turn all the
// Log commands into no-ops.
#ifdef MUSCLE_DISABLE_LOGGING
# define MUSCLE_INLINE_LOGGING

// No-op implementation of Log()
inline status_t Log(int, const char * , ...) {return B_NO_ERROR;}

// No-op implementation of LogTime()
inline status_t LogTime(int, const char *, ...) {return B_NO_ERROR;}

// No-op implementation of LogFlush()
inline status_t LogFlush() {return B_NO_ERROR;}

// No-op implementation of LogStackTrace()
inline status_t LogStackTrace(int level = MUSCLE_LOG_INFO, uint32 maxLevel=64) {(void) level; (void) maxLevel; return B_NO_ERROR;}

#else

// Define this constant in your Makefile (i.e. -DMUSCLE_MINIMALIST_LOGGING) if you don't want to have
// to link in SysLog.cpp and Hashtable.cpp and all the other stuff that is required for "real" logging.
# ifdef MUSCLE_MINIMALIST_LOGGING
#  define MUSCLE_INLINE_LOGGING

// Minimalist version of Log(), just sends the output to stdout.
inline status_t Log(int, const char * fmt, ...) {va_list va; va_start(va, fmt); vprintf(fmt, va); va_end(va); return B_NO_ERROR;}

// Minimalist version of LogTime(), just sends a tiny header and the output to stdout.
inline status_t LogTime(int logLevel, const char * fmt, ...) {printf("%i: ", logLevel); va_list va; va_start(va, fmt); vprintf(fmt, va); va_end(va); return B_NO_ERROR;}

// Minimumist version of LogFlush(), just flushes stdout
inline status_t LogFlush() {fflush(stdout); return B_NO_ERROR;}

// Minimalist version of LogStackTrace(), just prints a dummy string
inline status_t LogStackTrace(int level = MUSCLE_LOG_INFO, uint32 maxDepth = 64) {(void) level; (void) maxDepth; printf("<stack trace omitted>\n"); return B_NO_ERROR;}

# endif
#endif

#ifdef MUSCLE_INLINE_LOGGING
inline int ParseLogLevelKeyword(const char *) {return MUSCLE_LOG_NONE;}
inline int GetFileLogLevel() {return MUSCLE_LOG_NONE;}
inline int GetConsoleLogLevel() {return MUSCLE_LOG_NONE;}
inline int GetMaxLogLevel() {return MUSCLE_LOG_NONE;}
inline status_t SetFileLogLevel(int) {return B_NO_ERROR;}
inline status_t SetConsoleLogLevel(int) {return B_NO_ERROR;}
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

/** Attempts to lock the Mutex that is used to serialize LogCallback calls.
  * Typically you won't need to call this function, as it is called for you
  * before any LogCallback calls are made.
  * @returns B_NO_ERROR on success or B_ERROR on failure.
  * @note Be sure to call UnlockLog() when you are done!
  */
status_t LockLog();

/** Unlocks the Mutex that is used to serialize LogCallback calls.
  * Typically you won't need to call this function, as it is called for you
  * after any LogCallback calls are made.  The only time you need to call it
  * is after you've made a call to LockLog() and are now done with your critical
  * section.
  * @returns B_NO_ERROR on success or B_ERROR on failure.
  */
status_t UnlockLog();

/** Logs out a stack trace, if possible.  Returns B_ERROR if not.
 *  @note Currently only works under Linux and MacOS/X Leopard, and then only if -rdynamic is specified as a compile flag.
 *  @param logLevel a MUSCLE_LOG_* value indicating the "severity" of this message.
 *  @param maxDepth The maximum number of levels of stack trace that we should print out.  Defaults to
 *                  64.  The absolute maximum is 256; if you specify a value higher than that, you will still get 256.
 *  @returns B_NO_ERROR on success, or B_ERROR if a stack trace couldn't be logged because the platform doesn't support it.
 */
status_t LogStackTrace(int logLevel = MUSCLE_LOG_INFO, uint32 maxDepth = 64);

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

END_NAMESPACE(muscle);

#endif
