/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */  

#ifndef MuscleSystemInfos_h
#define MuscleSystemInfos_h

#include "util/String.h"

BEGIN_NAMESPACE(muscle);

/** Returns a human-readable name for the operating system that the code has 
  * been compiled on.  For example, "Windows", "MacOS/X", or "Linux".  If the 
  * operating system's name is unknown, returns "Unknown".
  */
const char * GetOSName();

enum {
   SYSTEM_PATH_CURRENT = 0, // our current working directory
   SYSTEM_PATH_EXECUTABLE,  // directory where our process's executable binary is 
   SYSTEM_PATH_TEMPFILES,   // scratch directory where temp files may be stored
   NUM_SYSTEM_PATHS
};

/** Given a SYSTEM_PATH_* token, returns the system's directory path
  * for that directory.  This is an easy cross-platform way to determine
  * where various various directories of interest are.
  * @param whichPath a SYSTEM_PATH_* token.
  * @param outStr on success, this string will contain the appopriate
  *               path name,  The path is guaranteed to end with a file
  *               separator character (i.e. "/" or "\\", as appropriate).
  * @returns B_NO_ERROR on success, or B_ERROR if the requested path could
  *          not be determined. 
  */
status_t GetSystemPath(uint32 whichPath, String & outStr);

/** Returns the file-path-separator character to use for this operating
  * system:  i.e. backslash for Windows, and forward-slash for every
  * other operating system.
  */
inline const char * GetFilePathSeparator()
{
#ifdef WIN32
   return "\\";
#else
   return "/";
#endif
}

END_NAMESPACE(muscle);

#endif
