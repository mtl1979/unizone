/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "system/SystemInfo.h"

#if defined(__APPLE__)
# include <CoreFoundation/CoreFoundation.h>
#endif

BEGIN_NAMESPACE(muscle);

const char * GetOSName()
{
   const char * ret = "Unknown";
   (void) ret;  // just to shut the Borland compiler up

#ifdef WIN32
   ret = "Windows";
#endif

#ifdef __CYGWIN__
   ret = "Windows (CygWin)";
#endif

#ifdef __APPLE__
   ret = "MacOS/X";
#endif

#ifdef __linux__
   ret = "Linux";
#endif

#ifdef __BEOS__
   ret = "BeOS";
#endif

#ifdef __ATHEOS__
   ret = "AtheOS";
#endif

#if defined(__QNX__) || defined(__QNXTO__)
   ret = "QNX";
#endif

#ifdef __FreeBSD__
   ret = "FreeBSD";
#endif

#ifdef __OpenBSD__
   ret = "OpenBSD";
#endif

#ifdef __NetBSD__
   ret = "NetBSD";
#endif

#ifdef __osf__
   ret = "Tru64";
#endif

#if defined(IRIX) || defined(__sgi)
   ret = "Irix";
#endif

#ifdef __OS400__
   ret = "AS400";
#endif

#ifdef __OS2__
   ret = "OS/2";
#endif

#ifdef _AIX
   ret = "AIX";
#endif

#ifdef _SEQUENT_
   ret = "Sequent";
#endif

#ifdef _SCO_DS
   ret = "OpenServer";
#endif

#if defined(_HP_UX) || defined(__hpux) || defined(_HPUX_SOURCE)
   ret = "HPUX";
#endif

#if defined(SOLARIS) || defined(__SVR4)
   ret = "Solaris";
#endif

#if defined(__UNIXWARE__) || defined(__USLC__)
   ret = "UnixWare";
#endif

   return ret;
}

status_t GetSystemPath(uint32 whichPath, String & outStr)
{
   bool found = false;
   char buf[2048];  // scratch space
   switch(whichPath)
   {
      case SYSTEM_PATH_CURRENT: // current working directory
      {
#ifdef WIN32
         found = muscleInRange((int)GetCurrentDirectoryA(sizeof(buf), buf), 1, (int)sizeof(buf)-1);
#else
         found = (getcwd(buf, sizeof(buf)) != NULL);
#endif
         if (found) outStr = buf;
      }
      break;

      case SYSTEM_PATH_EXECUTABLE: // executable's directory
      {
#ifdef WIN32
         found = muscleInRange((int)GetModuleFileNameA(NULL, buf, sizeof(buf)), (int)1, (int)sizeof(buf)-1);
         if (found) 
         {
            outStr = buf;
            int32 lastSlash = outStr.LastIndexOf(GetFilePathSeparator());
            if (lastSlash >= 0) outStr= outStr.Substring(0, lastSlash+1);
         }
#else
# ifdef __APPLE__
         CFURLRef bundleURL = CFBundleCopyExecutableURL(CFBundleGetMainBundle());
         CFStringRef cfPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
         char bsdPath[2048];
         if (CFStringGetCString((CFStringRef)cfPath, bsdPath, sizeof(bsdPath), kCFStringEncodingASCII))
         {
            found = true;
            outStr = bsdPath;

            int32 lastSlash = outStr.LastIndexOf(GetFilePathSeparator());
            if (lastSlash >= 0) outStr= outStr.Substring(0, lastSlash+1);
         }
         CFRelease(cfPath);
         CFRelease(bundleURL);
# else
         // NOT IMPLEMENTED YET!  (TODO: figure out how to find executabe folder under POSIX!)
# endif
#endif
      }
      break;

      case SYSTEM_PATH_TEMPFILES:   // scratch directory
      {
#ifdef WIN32
         found = muscleInRange((int)GetTempPathA(sizeof(buf), buf), 1, (int)sizeof(buf)-1);
         if (found) outStr = buf;
#else
         found = true;
         outStr = "/tmp";
#endif
      }
      break;
   }

   // Make sure the path name ends in a slash
   if (found)
   {
      const char * c = GetFilePathSeparator();
      if (outStr.EndsWith(c) == false) outStr += c;
   }

   return found ? B_NO_ERROR : B_ERROR;
};

END_NAMESPACE(muscle);
