/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#ifndef MuscleMiscUtilityFunctions_h
#define MuscleMiscUtilityFunctions_h

#include "message/Message.h"

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
  * (e.g. as returned by GetCurrentTime64()), returns an equivalent 
  * human-readable time/date string.  The format of the returned 
  * time string is "YYYY/MM/DD HH:MM:SS".
  * @param timeVal a time in microseconds since 1970.
  * @returns The equivalent ASCII string, or "" on failure.
  */
String GetHumanReadableTimeString(uint64 timeVal);

/** Does the inverse operation of GetHumanReadableTimeString():
  * Given a time string of the format "YYYY/MM/DD HH:MM:SS",
  * returns the equivalent time value in microseconds since 1970.
  * @param an ASCII string representing a time.
  * @returns The equivalent time value, or zero on failure.
  */
uint64 ParseHumanReadableTimeString(const String & str);

/** Calls fork(), setsid(), chdir(), umask(), etc, to fork an independent daemon process.
 *  Also closes all open file descriptors.
 *  Note that this function will call exit() on the parent process if successful,
 *  and thus won't ever return in that process. 
 *  @param optNewDir If specified, the daemon will chdir() to the directory specified here.
 *  @param optOutputTo Where to redirect stderr and stdout to.  Defaults to "/dev/null".
 *                     If set to NULL, or if the output device can't be opened, output
 *                     will not be rerouted.
 *  @param createOutputFileIfNecessary if set true, and (optOutputTo) can't be opened,
 *                                     (optOutputTo) will be created.
 *  @return B_NO_ERROR on success (the child process will see this), B_ERROR on failure.
 */
status_t BecomeDaemonProcess(const char * optNewDir = NULL, const char * optOutputTo = "/dev/null", bool createOutputFileIfNecessary = false);

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
status_t SpawnDaemonProcess(bool & returningAsParent, const char * optNewDir = NULL, const char * optOutputTo = "/dev/null", bool createOutputFileIfNecessary = false);

};  // end namespace muscle

#endif
