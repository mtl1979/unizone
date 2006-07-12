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
#if defined(WIN32) && defined(UNICODE)
   wchar_t wbuf[2048];
#endif

   switch(whichPath)
   {
      case SYSTEM_PATH_CURRENT: // current working directory
      {
#ifdef WIN32
# if defined(UNICODE)
         DWORD dirLen = GetCurrentDirectoryW(ARRAYITEMS(wbuf), wbuf);
         found = muscleInRange((int)dirLen, 1, (int)ARRAYITEMS(wbuf)-1);
         if (found) found = muscleInRange((int)WideCharToMultiByte(CP_UTF8, 0, wbuf, dirLen, buf, sizeof(buf), NULL, NULL), 1, (int)ARRAYITEMS(buf)-1);
# else
         found = muscleInRange((int)GetCurrentDirectoryA(sizeof(buf), buf), 1, (int)sizeof(buf)-1);
# endif
#else
         found = (getcwd(buf, sizeof(buf)) != NULL);
#endif
         if (found) outStr = buf;
      }
      break;

      case SYSTEM_PATH_EXECUTABLE: // executable's directory
      {
#ifdef WIN32
# if defined(UNICODE)
         DWORD moduleNameLen = GetModuleFileNameW(NULL, wbuf, ARRAYITEMS(wbuf));
         found = muscleInRange((int)moduleNameLen, (int)1, (int)ARRAYITEMS(wbuf)-1);
         if (found)
         {
            // My own quick implementation of PathRemoveFileSpecW(wbuf)
            // Since calling PathRemoveFileSpecW() causes link errors in Borland
            int lastSlashIdx=-1;
            for (int i=0; (wbuf[i] != 0); i++) {if (wbuf[i] == '\\') lastSlashIdx=i;}
            if (lastSlashIdx >= 0) wbuf[lastSlashIdx] = 0;

            found = muscleInRange((int)WideCharToMultiByte(CP_UTF8, 0, wbuf, moduleNameLen, buf, sizeof(buf), NULL, NULL), 1, (int)ARRAYITEMS(buf)-1);
            if (found) outStr = buf;
         }
# else
         found = muscleInRange((int)GetModuleFileNameA(NULL, buf, sizeof(buf)), (int)1, (int)sizeof(buf)-1);
         if (found) 
         {
            outStr = buf;
            int32 lastSlash = outStr.LastIndexOf(GetFilePathSeparator());
            if (lastSlash >= 0) outStr = outStr.Substring(0, lastSlash+1);
         }
# endif
#else
# ifdef __APPLE__
         CFURLRef bundleURL = CFBundleCopyExecutableURL(CFBundleGetMainBundle());
         CFStringRef cfPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
         char bsdPath[2048];
         if (CFStringGetCString((CFStringRef)cfPath, bsdPath, sizeof(bsdPath), kCFStringEncodingUTF8))
         {
            found = true;
            outStr = bsdPath;

            int32 lastSlash = outStr.LastIndexOf(GetFilePathSeparator());
            if (lastSlash >= 0) outStr = outStr.Substring(0, lastSlash+1);
         }
         CFRelease(cfPath);
         CFRelease(bundleURL);
# else
         // For Linux, anyway, we can try to find out our pid's executable via the /proc filesystem
         // And it can't hurt to at least try it under other OS's, anyway...
         char linkName[64]; sprintf(linkName, "/proc/%i/exe", getpid());
         char linkVal[1024];
         int rl = readlink(linkName, linkVal, sizeof(linkVal));
         if ((rl >= 0)&&(rl < (int)sizeof(linkVal)))
         {
            linkVal[rl] = '\0';  // gotta terminate the string!
            char * lastSlash = strrchr(linkVal, '/');  // cut out the executable name, we only want the dir
            if (lastSlash) *lastSlash = '\0';
            found = true;
            outStr = linkVal;
         }
# endif
#endif
      }
      break;

      case SYSTEM_PATH_TEMPFILES:   // scratch directory
      {
#ifdef WIN32
# if defined(UNICODE)
         DWORD dirLen = GetTempPathW(ARRAYITEMS(wbuf), wbuf);
         found = muscleInRange((int)dirLen, 1, (int)ARRAYITEMS(wbuf)-1);
         if (found) found = muscleInRange((int)WideCharToMultiByte(CP_UTF8, 0, wbuf, dirLen, buf, sizeof(buf), NULL, NULL), 1, (int)ARRAYITEMS(buf)-1);
# else
         found = muscleInRange((int)GetTempPathA(sizeof(buf), buf), 1, (int)sizeof(buf)-1);
# endif
         if (found) outStr = buf;
#else
         found = true;
         outStr = "/tmp";
#endif
      }
      break;

      case SYSTEM_PATH_USERHOME:  // user's home directory
      {
         const char * homeDir = getenv("HOME");
#ifdef WIN32
         if (homeDir == NULL) homeDir = getenv("USERPROFILE");
#endif
         if (homeDir)
         {
            found = true;
            outStr = homeDir;
         }
      }
      break;

      case SYSTEM_PATH_DESKTOP:  // user's desktop directory
         if (GetSystemPath(SYSTEM_PATH_USERHOME, outStr) == B_NO_ERROR)
         {
            found = true;
            outStr += "Desktop";  // it's the same under WinXP, Linux, and OS/X, yay!
         }
      break;

      case SYSTEM_PATH_DOCUMENTS:  // user's documents directory
         if (GetSystemPath(SYSTEM_PATH_USERHOME, outStr) == B_NO_ERROR)
         {
            found = true;
#ifndef WIN32
            outStr += "Documents";  // For WinXP, it's the same as the home dir; for others, add Documents to the end
#endif
         }
      break;

      case SYSTEM_PATH_ROOT:  // the highest possible directory
      {
#ifdef WIN32
         const char * homeDrive = getenv("HOMEDRIVE");
         if (homeDrive)
         {
            outStr = homeDrive;
            found = true;
         }
#else
         outStr = "/";
         found = true;
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
