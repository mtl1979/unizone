/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#ifndef MuscleMiscUtilityFunctions_h
#define MuscleMiscUtilityFunctions_h

#include "message/Message.h"
#include "util/Queue.h"

namespace muscle {

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

/** Parses the given arguments into a Queue of Strings.
  * Any prefixed dashes will be stripped/ignored.
  * All argument names will be forced to lower case.
  * @param argc As passed in to main()
  * @param argv As passed in to main
  * @param addTo The Queue to add the arguments to
  * @returns B_NO_ERROR on success, B_ERROR on failure (out of memory)
  */
status_t ParseArgs(int argc, char ** argv, Queue<String> & addTo);

/** Parses settings from the given file.  Works similarly to
 *  ParseArgs() (above), only the values are read from a file
 *  instead of from the arguments vector.  The file may contain
 *  comments that are prepended with a hash symbol (#); these
 *  will be safely ignored.
 *
 *  @param file File pointer to read from.  This file must be
 *              opened for reading, and will not be fclosed() by this function.
  * @param addTo The Queue to add the arguments to
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t ParseFile(FILE * file, Queue<String> & addTo);

/** Parses the single string argument (arg) and adds the results to (addTo).
 *  Formatting rules for the string are described above in the ParseArgs() documentation.
 *  @param argument string to parse.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t ParseArg(const String & arg, Queue<String> & addTo);

/** Scans a line for multiple arguments and calls ParseArg() on each one found.
 *  Quotation marks will be used to group tokens if necessary.
 *  @param arg A line of text, e.g.  arg1=blah arg2=borf arg3="quoted borf"
 *  @param addTo Queue to add the argument results to.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t ParseArgs(const String & arg, Queue<String> & addTo);

/** This does the inverse operation of ParseArgs().  That is to say, it
  * takes a Queue that was previously created via ParseArgs() and returns
  * a String representing the same data.  Note that the returned String isn't
  * guaranteed to be identical to the one that was previously passed in to 
  * ParseArgs(); in particular, any comments will have been stripped out.
  * @param argMsg A Queue containing String field indicating the arguments
  *               to add to the String
  * @return The resulting String.
  */
String UnparseArgs(const Queue<String> & argMsg);

/** Convenience method:  Looks for a given hostname or hostname:port string in 
 *  the given field of the args Message, and returns the appropriate parsed
 *  values if it is found.
 *  @param args the Message that was returned by ParseArg().
 *  @param fn the field name to look for in (args)
 *  @param retHost On successful return, the hostname or IP address to connect to will be written here.
 *  @param retPort On successful return, if a port number was parsed it will be written here.
 *  @param portRequired If false, this function will succeed even if no port was specified. 
 *                      If true, the function will fail if a port was not specified (e.g. "localhost:5555").
 *                      Defaults to false.
 *  @returns B_NO_ERROR if an argument was parsed, or B_ERROR if it wasn't.
 */
status_t ParseConnectArg(const Message & args, const String & fn, String & retHost, uint16 & retPort, bool portRequired = false);

/** Same as above, except that instead of looking for the specified string in a Message, in this
 *  case the string is passed in directly.
 *  @param arg The connect string (e.g. "localhost:2960")
 *  @param retHost On successful return, the hostname or IP address to connect to will be written here.
 *  @param retPort On successful return, if a port number was parsed it will be written here.
 *  @param portRequired If false, this function will succeed even if no port was specified. 
 *                      If true, the function will fail if a port was not specified (e.g. "localhost:5555").
 *                      Defaults to false.
 *  @returns B_NO_ERROR if an argument was parsed, or B_ERROR if it wasn't.
 */
status_t ParseConnectArg(const String & arg, String & retHost, uint16 & retPort, bool portRequired = false);

/** Given a hostname (or IP address) and a port number, returns the associated connect-string (e.g. "localhost:9999")
  * or ("[ff05::1]:5555")
  * @param host A hostname or IP address
  * @param port A port number.
  */
String GetConnectString(const String & host, uint16 port);

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
 *     daemon                -- Non-Windows only:  Run this process in the background
 *     localhost=ip          -- Treat connections from localhost as if they were coming from (ip)
 *     displaylevel=levelstr -- Set the stdout output display filter level to (levelstr)
 *     filelevel=levelstr    -- Set the output log file filter level to (levelstr)
 *     logfile=levelstr      -- Force the log file to have this name/location
 *     nice[=niceLevel]      -- Linux/OSX only: makes this process nicer (i.e. lower priority)
 *     mean[=meanLevel]      -- Linux/OSX only: makes this process meaner (i.e. higher priority)
 *     realtime[=priority]   -- Linux only: makes this process real-time (requires root access)
 *     debugcrashes          -- Linux only: print a stack trace when a crash occurs
 *     console               -- Windows only:  open a DOS box to display this window's output
 *  @param args an arguments Message, as produced by ParseArgs() or ParseFile() or etc.
 */
void HandleStandardDaemonArgs(const Message & args);

/** Given an ASCII representation of a non-negative number, 
 *  returns that number as a uint64. 
 */
uint64 Atoull(const char * str);

/** Similar to Atoll(), but handles negative numbers as well */
int64 Atoll(const char * str);

/** This class represents all the fields necessary to present a human with a human-readable time/date stamp.  Objects of this class are typically populated by the GetHumanReadableTimeValues() function, below. */
class HumanReadableTimeValues
{
public:
   /** Default constructor */
   HumanReadableTimeValues() : _year(0), _month(0), _dayOfMonth(0), _dayOfWeek(0), _hour(0), _minute(0), _second(0), _microsecond(0) {/* empty */}

   /** Explicit constructor
     * @param year The year value (e.g. 2005)
     * @param month The month value (January=0, February=1, etc)
     * @param dayOfMonth The day within the month (ranges from 0 to 30, inclusive)
     * @param dayOfWeek The day within the week (Sunday=0, Monday=1, etc)
     * @param hour The hour within the day (ranges from 0 to 23, inclusive)
     * @param minute The minute within the hour (ranges from 0 to 59, inclusive)
     * @param second The second within the minute (ranges from 0 to 59, inclusive)
     * @param microsecond The microsecond within the second (ranges from 0 to 999999, inclusive)
     */
   HumanReadableTimeValues(int year, int month, int dayOfMonth, int dayOfWeek, int hour, int minute, int second, int microsecond) : _year(year), _month(month), _dayOfMonth(dayOfMonth), _dayOfWeek(dayOfWeek), _hour(hour), _minute(minute), _second(second), _microsecond(microsecond) {/* empty */}

   /** Returns the year value (e.g. 2005) */
   int GetYear() const {return _year;}

   /** Returns the month value (January=0, February=1, March=2, ..., December=11). */
   int GetMonth() const {return _month;}

   /** Returns the day-of-month value (which ranges between 0 and 30, inclusive). */
   int GetDayOfMonth() const {return _dayOfMonth;}

   /** Returns the day-of-week value (Sunday=0, Monday=1, Tuesday=2, Wednesday=3, Thursday=4, Friday=5, Saturday=6). */
   int GetDayOfWeek() const {return _dayOfWeek;}

   /** Returns the hour value (which ranges between 0 and 23, inclusive). */
   int GetHour() const {return _hour;}

   /** Returns the minute value (which ranges between 0 and 59, inclusive). */
   int GetMinute() const {return _minute;}

   /** Returns the second value (which ranges between 0 and 59, inclusive). */
   int GetSecond() const {return _second;}

   /** Returns the microsecond value (which ranges between 0 and 999999, inclusive). */
   int GetMicrosecond() const {return _microsecond;}

   /** Sets the year value (e.g. 2005) */
   void SetYear(int year) {_year = year;}

   /** Sets the month value (January=0, February=1, March=2, ..., December=11). */
   void SetMonth(int month) {_month = month;}

   /** Sets the day-of-month value (which ranges between 0 and 30, inclusive). */
   void SetDayOfMonth(int dayOfMonth) {_dayOfMonth = dayOfMonth;}

   /** Sets the day-of-week value (Sunday=0, Monday=1, Tuesday=2, Wednesday=3, Thursday=4, Friday=5, Saturday=6). */
   void SetDayOfWeek(int dayOfWeek) {_dayOfWeek = dayOfWeek;}

   /** Sets the hour value (which ranges between 0 and 23, inclusive). */
   void SetHour(int hour) {_hour = hour;}

   /** Sets the minute value (which ranges between 0 and 59, inclusive). */
   void SetMinute(int minute) {_minute = minute;}

   /** Sets the second value (which ranges between 0 and 59, inclusive). */
   void SetSecond(int second) {_second = second;}

   /** Sets the microsecond value (which ranges between 0 and 999999, inclusive). */
   void SetMicrosecond(int microsecond) {_microsecond = microsecond;}

   /** Equality operator. */
   bool operator == (const HumanReadableTimeValues & rhs) const
   {
      return ((_year       == rhs._year)&&
              (_month      == rhs._month)&&
              (_dayOfMonth == rhs._dayOfMonth)&&
              (_dayOfWeek  == rhs._dayOfWeek)&&
              (_hour       == rhs._hour)&&
              (_minute     == rhs._minute)&&
              (_second     == rhs._second)&&
              (_microsecond == rhs._microsecond));
   }

   /** Inequality operator */
   bool operator != (const HumanReadableTimeValues & rhs) const {return !(*this==rhs);}

   /** This method will expand the following tokens in the specified String out to the following values:
     * %Y -> Current year (e.g. "2005")
     * %M -> Current month (e.g. "01" for January, up to "12" for December)
     * %Q -> Current month as a string (e.g. "January", "February", "March", etc)
     * %D -> Current day of the month (e.g. "01" through "31")
     * %d -> Current day of the month (e.g. "01" through "31") (synonym for %D)
     * %W -> Current day of the week (e.g. "1" through "7")
     * %w -> Current day of the week (e.g. "1" through "7") (synonym for %W)
     * %q -> Current day of the week as a string (e.g. "Sunday", "Monday", "Tuesday", etc)
     * %h -> Current hour (military style:  e.g. "00" through "23")
     * %m -> Current minute (e.g. "00" through "59")
     * %s -> Current second (e.g. "00" through "59")
     * %x -> Current microsecond (e.g. "000000" through "999999", inclusive)
     * %r -> A random number between 0 and (2^64-1) (for spicing up the uniqueness of a filename)
     * %T -> A human-readable time/date stamp, for convenience (e.g. "January 01 2005 23:59:59")
     * %t -> A numeric time/date stamp, for convenience (e.g. "2005/01/01 15:23:59")
     * %f -> A filename-friendly numeric time/date stamp, for convenience (e.g. "2005-01-01_15h23m59")
     * %% -> A single percent sign.
     * @param s The string to expand the tokens of
     * @returns The same string, except with any and all of the above tokens expanded as described.
     */
   String ExpandTokens(const String & s) const;

private:
   int _year;
   int _month;
   int _dayOfMonth;
   int _dayOfWeek;
   int _hour;
   int _minute;
   int _second;
   int _microsecond;
};

/** Given a uint64 representing a time in microseconds since 1970,
  * (e.g. as returned by GetCurrentTime64()), returns the same value
  * as a set of more human-friendly units.  
  *
  * @param timeUS a time in microseconds since 1970.  Note that the interpretation of this value depends on
  *               the value passed in to the (timeType) argument.
  * @param retValues On success, this object will be filled out with the various human-readable time/date value fields
  *                  that human beings like to read.  See the HumanReadableTimeValues class documentation for details.
  * @param timeType If set to MUSCLE_TIMEZONE_UTC (the default) then (timeUS) will be interpreted as being in UTC, 
  *                 and will be converted to the local time zone as part of the conversion process.  If set to 
  *                 MUSCLE_TIMEZONE_LOCAL, on the other hand, then (timeUS) will be assumed to be already 
  *                 in the local time zone, and no time zone conversion will be done.
  *                 Note that the values returned are ALWAYS in reference to local time 
  *                 zone -- the (timeType) argument governs how (timeUS) should be interpreted.  
  *                 (timeType) does NOT control the meaning of the return values.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t GetHumanReadableTimeValues(uint64 timeUS, HumanReadableTimeValues & retValues, uint32 timeType = MUSCLE_TIMEZONE_UTC);

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

/** Given a string that represents a time interval, returns the equivalent value in microsends.
  * A time interval should be expressed as a non-negative integer, optionally followed by
  * any of the following suffixes:
  *   us = microseconds
  *   ms = milliseconds
  *   s  = seconds
  *   m  = minutes
  *   h  = hours
  *   d  = days
  *   w  = weeks
  * As a special case, the string "forever" will parse to MUSCLE_TIME_NEVER.
  * If no suffix is supplied, the units are presumed to be in seconds.
  * @param str The string to parse 
  * @returns a time interval value, in microseconds.
  */
uint64 ParseHumanReadableTimeIntervalString(const String & str);

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

/** Given a string, returns that same string except with any symbols that are not illegal
  * in a DNS label removed.  (According to DNS rules, only letters, digits, and the '-'
  * character are legal in a DNS label, and the label must be less than 64 characters long).
  * Note that this string cleans up just a single part of a DNS hostname path.
  * If you want to clean up a path string (e.g. "www.foo.com"), call CleanupDNSPath() instead.
  * @param s A string that is presented as a candidate for being a DNS label.
  * @returns the DNS label that most closely resembles (s).
  */
String CleanupDNSLabel(const String & s);

/** Given a DNS path string (e.g. "www.foo.com") runs each dot-separated portion of the
  * path through CleanupDNSLabel() and returns the cleaned up result.
  * @param s A string that is presented as a candidate for being a DNS path.
  * @returns the DNS path that most closely resembles (s).
  */
String CleanupDNSPath(const String & s);

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

/** This is a convenience function for debugging.  It will print to stdout the
  * specified array of bytes in human-readable hexadecimal format, along with
  * an ASCII sidebar when possible.
  * @param bytes A Queue of uint8s representing the bytes to print out.
  * @param optDesc if non-NULL, this will be used as a prefix/title string.
  * @param numColumns If specified non zero, then the bytes will be printed
  *                   out with this many bytes per row.  Defaults to 16.
  *                   If set to zero, then all the output will be placed
  *                   on a single line, using a simpler hex-only format.
  * @param optFile Optional file to print the output to.  If left NULL, printing will go to stdout.
  */
void PrintHexBytes(const Queue<uint8> & bytes, const char * optDesc = NULL, uint32 numColumns = 16, FILE * optFile = NULL);

/** This function is the same as PrintHexBytes(), but the output is sent to Log() instead of fprintf().
  * @param bytes The bytes to print out
  * @param numBytes How many bytes (bytes) points to
  * @param optDesc if non-NULL, this will be used as a prefix/title string.
  * @param numColumns If specified non zero, then the bytes will be printed
  *                   out with this many bytes per row.  Defaults to 16.
  *                   If set to zero, then all the output will be placed
  *                   on a single line, using a simpler hex-only format.
  * @param optFile Optional file to print the output to.  If left NULL, printing will go to stdout.
  */
void LogHexBytes(int logLevel, const void * bytes, uint32 numBytes, const char * optDesc = NULL, uint32 numColumns = 16);

/** This function is the same as PrintHexBytes(), but the output is sent to Log() instead of fprintf().
  * @param bytes A Queue of uint8s representing the bytes to print out.
  * @param optDesc if non-NULL, this will be used as a prefix/title string.
  * @param numColumns If specified non zero, then the bytes will be printed
  *                   out with this many bytes per row.  Defaults to 16.
  *                   If set to zero, then all the output will be placed
  *                   on a single line, using a simpler hex-only format.
  * @param optFile Optional file to print the output to.  If left NULL, printing will go to stdout.
  */
void LogHexBytes(int logLevel, const Queue<uint8> & bytes, const char * optDesc = NULL, uint32 numColumns = 16);

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

/** Given a byte buffer, returns an ASCII representation of it.
  * @param buf A buffer of bytes
  * @param numBytes The number of bytes that (buf) points to.
  * @returns a String with human-readable contents:  e.g. "5F A3 A2"...
  */
String HexBytesToString(const uint8 * buf, uint32 numBytes);

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

/** Returns true iff the file with the specified path exists. 
  * @param filePath Path of the file to check for.
  * @returns true if the file exists (and is readable), false otherwise.
  */
bool FileExists(const char * filePath);

/** Attempts to rename from (oldPath) to (newPath).
  * @param oldPath the path of an existing file or directory.
  * @param newPath the new name that the file should have.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t RenameFile(const char * oldPath, const char * newPath);

/** Attempts to copy from (oldPath) to (newPath).
  * @param oldPath the path of an existing file or directory.
  * @param newPath the name that the new file should have.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t CopyFile(const char * oldPath, const char * newPath);

/** Attempts to delete the file with the specified file path.
  * @param filePath Path of the file to delete.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t DeleteFile(const char * filePath);

/** Given argv[0], returns a human-readable program title based on the file name.
  * For example, "c:\Program Files\Blah.exe" is returned as "Blah", or
  * "/Users/jaf/MyProg/MyProg.app/Contents/MacOS/MyProg" is returned as "MyProg".
  * @param argv0 argv[0], as passed to main().
  * @returns a human-readable title string.
  */
String GetHumanReadableProgramNameFromArgv0(const char * argv0);

#ifdef WIN32
/** This function is only available on Win32, and does the 
  * standard AllocConsole() and freopen() trick to cause a
  * Console window to appear and be available for stdin/stdout/stderr
  * to operate on.
  */
void Win32AllocateStdioConsole();
#endif

}; // end namespace muscle

#endif
