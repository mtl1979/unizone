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

/** Given an ASCII representation of a non-negative number, 
 *  returns that number as a uint64. 
 */
uint64 Atoull(const char * str);

};  // end namespace muscle

#endif
