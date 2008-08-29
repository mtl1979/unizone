/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#ifndef MuscleMiscUtilityFunctions_h
#define MuscleMiscUtilityFunctions_h

#include "message/Message.h"

BEGIN_NAMESPACE(muscle);

/** Parses the given arguments into a Message full of string fields.
  * Arguments should be of the form argname or argname=value.
  * The latter will be added as string fields; the former will
  * be treated the same, with the string equal to "".
  * Any prefixed dashes will be stripped/ignored.
  * All argument names will be forced to lower case.
  * @param argc As passed in to main()
  * @param argv As passed in to main
  * @param addTo The message to add the arguments to
  * @returns B_NO_ERROR on success, B_ERROR on failure (out of memory)
  */
status_t ParseArgs(int argc, char ** argv, Message & addTo);

/** Parses settings from the given file.  Works similarly to
 *  ParseArgs() (above), only the values are read from a file
 *  instead of from the arguments vector.  The file may contain
 *  comments that are prepended with a hash symbol (#); these
 *  will be safely ignored.
 *
 *  Also, the file may contain special lines beginning with the
 *  keyword "begin" or "end".  These lines delineate a sub-section,
 *  which will show up in (addTo) as a sub-Message.  This can
 *  be useful for expressing hierarchical data.  The "begin" line
 *  may optionally contain the name to store the sub-Message under; 
 *  if no name is present, the sub-Message will be added with name "".
 *  Multiple sub-sections with the same name are supported.
 *
 *  @param file File pointer to read from.  This file must be
 *              opened for reading, and will not be fclosed() by this function.
  * @param addTo The message to add the arguments to
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t ParseFile(FILE * file, Message & addTo);

/** Parses the single string argument (arg) and adds the results to (addTo).
 *  Formatting rules for the string are described above in the ParseArgs() documentation.
 *  @param argument string (or string=value pair) to parse.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t ParseArg(const String & arg, Message & addTo);

/** Scans a line for multiple arguments and calls ParseArg() on each one found.
 *  Quotation marks will be used to group tokens if necessary.
 *  @param arg A line of text, e.g.  arg1=blah arg2=borf arg3="quoted borf"
 *  @param addTo Message to add the argument results to.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t ParseArgs(const String & arg, Message & addTo);

/** This does the inverse operation of ParseArgs().  That is to say, it
  * takes a Message that was previously created via ParseArgs() and returns
  * a String representing the same data.  Note that the returned String isn't
  * guaranteed to be identical to the one that was previously passed in to 
  * ParseArgs(); in particular, any comments will have been stripped out,
  * and the order of the items may be different in some cases.
  * @param argMsg A Message containing String field indicating the arguments
  *               to add to the String
  * @return The resulting String.
  */
String UnparseArgs(const Message & argMsg);

/** Convenience method:  Looks for a given hostname or hostname:port string in 
 *  the given field of the args Message, and returns the appropriate parsed
 *  values if it is found.
 *  @param args the Message that was returned by ParseArg().
 *  @param fn the field name to look for in (args)
 *  @param retHost On successful return, the hostname or IP address to connect to will be written here.
 *  @param retPort On successful return, if a port number was parsed it will be written here.
 *  @returns B_NO_ERROR if an argument was parsed, or B_ERROR if it wasn't.
 */
status_t ParseConnectArg(const Message & args, const String & fn, String & retHost, uint16 & retPort);

/** Convenience method:  Looks for a port number in the given field of the Message,
 *  and sets (retPort) if it finds one.
 *  @param args the Message that was returned by ParseArg().
 *  @param fn the field name to look for in (args)
 *  @param retPort On successful return, if a port number was parsed it will be written here.
 *  @returns B_NO_ERROR if an argument was parsed, or B_ERROR if it wasn't.
 */
status_t ParsePortArg(const Message & args, const String & fn, uint16 & retPort);

/** Looks for some globally useful startup arguments in the (args)
 *  Message and handles them by calling the appropriate setup routines.  
 *  Recognized arguments currently include the following:
 *     daemon              -- Non-Windows only:  Run this process in the background
 *     localhost=ip        -- Treat connections from localhost as if they were coming from (ip)
 *     display=levelstr    -- Set the stdout output display filter level to (levelstr)
 *     log=levelstr        -- Set the output log filter level to (levelstr)
 *     nice[=niceLevel]    -- Linux/OSX only: makes this process nicer (i.e. lower priority)
 *     mean[=meanLevel]    -- Linux/OSX only: makes this process meaner (i.e. higher priority)
 *     realtime[=priority] -- Linux only: makes this process real-time (requires root access)
 *     console             -- Windows only:  open a DOS box to display this window's output
 *  @param args an arguments Message, as produced by ParseArgs() or ParseFile() or etc.
 */
void HandleStandardDaemonArgs(const Message & args);

/** Given an ASCII representation of a non-negative number, 
 *  returns that number as a uint64. 
 */
uint64 Atoull(const char * str);

/** Similar to Atoll(), but handles negative numbers as well */
int64 Atoll(const char * str);

/** Given a uint64 representing a time in microseconds since 1970,
  * (e.g. as returned by GetCurrentTime64()), returns the same value
  * as a set of more human-friendly units.  
  *
  * @param timeUS a time in microseconds since 1970.  Note that the interpretation of this value depends on
  *               the value passed in to the (timeType) argument.
  * @param retYear   On success, the year value (e.g. 2005) is placed here.
  * @param retMonth  On success, the month value  (which ranges between 0 and 11, inclusive) is placed here.
  * @param retDay    On success, the day value    (which ranges between 0 and 30, inclusive) is placed here.
  * @param retHour   On success, the hour value   (which ranges between 0 and 23, inclusive) is placed here.
  * @param retMinute On success, the minute value (which ranges between 0 and 59, inclusive) is placed here.
  * @param retSecond On success, the second value (which ranges between 0 and 59, inclusive) is placed here.
  * @param timeType If set to MUSCLE_TIMEZONE_UTC (the default) then (timeUS) will be interpreted as being in UTC, 
  *                 and will be converted to the local time zone as part of the conversion process.  If set to 
  *                 MUSCLE_TIMEZONE_LOCAL, on the other hand, then (timeUS) will be assumed to be already 
  *                 in the local time zone, and no time zone conversion will be done.
  *                 Note that the values returned are ALWAYS in reference to local time 
  *                 zone -- the (timeType) argument governs how (timeUS) should be interpreted.  
  *                 (timeType) does NOT control the meaning of the return values.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t GetHumanReadableTimeValues(uint64 timeUS, int & retYear, int & retMonth, int & retDay, int & retHour, int & retMinute, int & retSecond, uint32 timeType = MUSCLE_TIMEZONE_UTC);

/** Given a uint64 representing a time in microseconds since 1970,
  * (e.g. as returned by GetCurrentTime64()), returns an equivalent 
  * human-readable time/date string.  The format of the returned 
  * time string is "YYYY/MM/DD HH:MM:SS".
  * @param timeUS a time in microseconds since 1970.  Note that the interpretation of this value depends on
  *               the value passed in to the (timeType) argument.
  * @param timeType If set to MUSCLE_TIMEZONE_UTC (the default) then (timeUS) will be interpreted as being in UTC, 
  *                 and will be converted to the local time zone as part of the conversion process.  If set to 
  *                 MUSCLE_TIMEZONE_LOCAL, on the other hand, then (timeUS) will be assumed to be already 
  *                 in the local time zone, and no time zone conversion will be done.
  * @returns The equivalent ASCII string, or "" on failure.
  */
String GetHumanReadableTimeString(uint64 timeVal, uint32 timeType = MUSCLE_TIMEZONE_UTC);

/** Does the inverse operation of GetHumanReadableTimeString():
  * Given a time string of the format "YYYY/MM/DD HH:MM:SS",
  * returns the equivalent time value in microseconds since 1970.
  * @param an ASCII string representing a time.
  * @param timeType If set to MUSCLE_TIMEZONE_UTC (the default) then the returned value will be UTC. 
  *                 If set to MUSCLE_TIMEZONE_LOCAL, on the other hand, then the returned value will
  *                 be expressed as a time of the local time zone.
  * @returns The equivalent time value, or zero on failure.
  */
uint64 ParseHumanReadableTimeString(const String & str, uint32 timeType = MUSCLE_TIMEZONE_UTC);

/** Similar to the standard exit() call, except that no global object destructors will
  * be called.  This is sometimes useful, e.g. in fork() situations where you want the
  * parent process to just go away without any chance of a crash during cleanup.
  * @param exitCode the exit code value that should be passed back to our parent process
  *                 (i.e. the argument to pass to exit() or _exit())
  */
void ExitWithoutCleanup(int exitCode);

/** Calls fork(), setsid(), chdir(), umask(), etc, to fork an independent daemon process.
 *  Also closes all open file descriptors.
 *  Note that this function will call ExitWithoutCleanup() on the parent process if successful,
 *  and thus won't ever return in that process. 
 *  @param optNewDir If specified, the daemon will chdir() to the directory specified here.
 *  @param optOutputTo Where to redirect stderr and stdout to.  Defaults to "/dev/null".
 *                     If set to NULL, or if the output device can't be opened, output
 *                     will not be rerouted.
 *  @param createOutputFileIfNecessary if set true, and (optOutputTo) can't be opened,
 *                                     (optOutputTo) will be created.
 *  @return B_NO_ERROR on success (the child process will see this), B_ERROR on failure.
 */
status_t BecomeDaemonProcess(const char * optNewDir = NULL, const char * optOutputTo = "/dev/null", bool createOutputFileIfNecessary = true);

/** Returns true iff we are a daemon process created via BecomeDaemonProcess() or SpawnDaemonProcess() */
bool IsDaemonProcess();

/** Same as BecomeDaemonProcess(), except that the parent process returns as well as the child process.  
 *  @param returningAsParent Set to true on return of the parent process, or false on return of the child process.
 *  @param optNewDir If specified, the child will chdir() to the directory specified here.
 *  @param optOutputTo Where to redirect stderr and stdout to.  Defaults to "/dev/null".
 *                     If set to NULL, or if the output device can't be opened, output
 *                     will not be rerouted.
 *  @param createOutputFileIfNecessary if set true, and (optOutputTo) can't be opened,
 *                                     (optOutputTo) will be created.
 *  @return B_NO_ERROR (twice!) on success, B_ERROR on failure.
 */ 
status_t SpawnDaemonProcess(bool & returningAsParent, const char * optNewDir = NULL, const char * optOutputTo = "/dev/null", bool createOutputFileIfNecessary = true);

/** Convenience function:  Removes any ANSI formatting escape-sequences from (s), so 
  * that (s) can be displayed as plain text without a bunch of garbage showing up in it.
  */
void RemoveANSISequences(String & s);

/** Convenience function.  Given a buffer of arbitrary data, returns a nybble-ized String
  * that represents that same data using only the ASCII characters 'A' through 'P.  The
  * returned String will be twice the length of the passed-in buffer, and the original
  * data can be recovered from the String by calling DenybbleizeData().
  * @param buf The data to nybbleize
  * @param retString On success, the nybbleized String is written here.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t NybbleizeData(const ByteBuffer & buf, String & retString);

/** Convenience function.  Given a String that was produced by NybblizedData(),
  * returns the eqivalent ByteBuffer.
  * @param nybbleizedText The String to de-nybbleize
  * @param retBuf On success, the de-nybbleized data is written here.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t DenybbleizeData(const String & nybbleizedText, ByteBuffer & retBuf);

/** Convenience function:  Returns a string which is a nybbleized representation of
  * the passed-in string's contents (not including its NUL terminator byte)
  * @param str A string to nybbleize
  * @returns A nybbleized equivalent of (str), as described in NybbleizeData().
  */
String NybbleizeString(const String & str);

/** Convenience function:  Returns a string which is the denybbleized 
  * representation of the passed-in nybbleized string.  
  * @param nybStr A string to denybbleize.  Note that not all nybbleized
  *               strings can be de-nybblized correctly back into a 
  *               String object:  in particular, if the de-nybbleized
  *               data contains any NUL bytes, then the String
  *               returned by this function will be truncated at the first NUL.
  *               If you need to be able to decode any legal nybbleized string,
  *               call NybbleizeData() instead of this function.
  */
String DenybbleizeString(const String & nybStr);

/** This function is like strstr(), except that instead of searching for a substring
  * within a string, it looks for a given binary pattern inside a binary buffer.
  * @param lookIn The buffer to look for the sub-region inside
  * @param numLookInBytes The number of bytes pointed to by (lookIn)
  * @param lookFor The byte pattern to look for inside of (lookin)
  * @param numLookForBytes The number of bytes pointed to by (lookFor) 
  * @returns A pointer to the first instance of (lookFor) found inside of (lookIn), or NULL
  *          if no such pattern was found inside of (lookIn).
  */
const uint8 * MemMem(const uint8 * lookIn, uint32 numLookInBytes, const uint8 * lookFor, uint32 numLookForBytes);

/** This is a convenience function for debugging.  It will print to stdout the
  * specified array of bytes in human-readable hexadecimal format, along with
  * an ASCII sidebar when possible.
  * @param bytes The bytes to print out
  * @param numBytes How many bytes (bytes) points to
  * @param optDesc if non-NULL, this will be used as a prefix/title string.
  * @param numColumns If specified non zero, then the bytes will be printed
  *                   out with this many bytes per row.  Defaults to 16.
  *                   If set to zero, then all the output will be placed
  *                   on a single line, using a simpler hex-only format.
  * @param optFile Optional file to print the output to.  If left NULL, printing will go to stdout.
  */
void PrintHexBytes(const void * bytes, uint32 numBytes, const char * optDesc = NULL, uint32 numColumns = 16, FILE * optFile = NULL);

/** Given a string with an ASCII representation of hexadecimal bytes,
  * returns the corresponding binary data.
  * @param buf A hexadecimal string.  Each hex byte should be expressed as
  *            two ASCII characters (e.g. "f0 1f 7e f7"), or alternatively
  *            you can enter chars in ASCII if you prepend each one with
  *            a slash (e.g. "/h /e /l /l /o").
  * @returns a ByteBufferRef containing the corresponding binary data,
  *          or a NULL ByteBufferRef on failure (out of memory?)
  */
ByteBufferRef ParseHexBytes(const char * buf);

/** A convenience method, useful to optimally create a single PR_COMMAND_BATCH
  * Message out of a set of zero or more other Messages.  Here's how it works:
  * If (batchMsg) is a NULL ref, then (batchMsg) is set to reference the same
  * Message as (newMsg).  If (batchMsg) is a PR_COMMAND_BATCH Message, then
  * (newMsg) is appended to its PR_NAME_KEYS field.  Otherwise, a new 
  * PR_COMMAND_BATCH Message is created, and both (batchMsg) and (newMsg) are
  * added to it.
  * @param batchMsg Reference to the Message that you will want to eventually
  *                 send to the server.  This Reference will be modified.
  *                 May be a NULL ref.
  * @param newMsg Reference to the Message to add to (batchMsg).  May not
  *                 be a NULL ref.
  * @returns B_NO_ERROR on success, or B_ERROR on error (out of memory?)
  */
status_t AssembleBatchMessage(MessageRef & batchMsg, const MessageRef & newMsg);

END_NAMESPACE(muscle);

#endif
