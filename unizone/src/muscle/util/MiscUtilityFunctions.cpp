/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include <fcntl.h>

#ifdef __APPLE__
# include <sys/signal.h>
# include <sys/types.h>
#else
# include <signal.h>
#endif

#include <sys/stat.h>

#ifdef __linux__
# include <sched.h>
#endif

#include "util/MiscUtilityFunctions.h"
#include "util/NetworkUtilityFunctions.h"
#include "util/StringTokenizer.h"

BEGIN_NAMESPACE(muscle);

status_t ParseArg(const String & a, Message & addTo)
{
   String argName = a;

   // Remove any commenting after the hash mark
   int32 hashIdx = argName.IndexOf("#");
   if (hashIdx >= 0) argName = argName.Substring(0, hashIdx);
   argName = argName.Trim();

   // Remove any initial dashes
   while(argName.StartsWith("-")) argName = argName.Substring(1);

   int equalsAt = argName.IndexOf('=');
   String argValue;
   if (equalsAt >= 0)
   {
      argValue = argName.Substring(equalsAt+1).Trim();
      argName  = argName.Substring(0, equalsAt).Trim();
   }
   return (argName.Length() > 0) ? addTo.AddString(argName.ToLowerCase()(), argValue) : B_NO_ERROR;
}

status_t ParseArgs(int argc, char ** argv, Message & addTo)
{
   for (int i=0; i<argc; i++) if (ParseArg(argv[i], addTo) != B_NO_ERROR) return B_ERROR;
   return B_NO_ERROR;
}

status_t ParseFile(FILE * fpIn, Message & addTo)
{
   const int bufSize = 2048;
   char * buf = newnothrow char[bufSize];
   if (buf)
   {
      status_t ret = B_NO_ERROR;
      while(fgets(buf, bufSize, fpIn))
      {
         if (ParseArg(String(buf), addTo) != B_NO_ERROR)
         {
            ret = B_ERROR;
            break;
         }
      }
      delete [] buf;
      return ret;
   }
   else
   {
      WARN_OUT_OF_MEMORY;
      return B_ERROR;
   }
}

void HandleStandardDaemonArgs(const Message & args)
{
   // Do this first, so that the stuff below will affect the right process.
   if (args.HasName("daemon"))
   {
      LogTime(MUSCLE_LOG_INFO, "Spawning off a daemon-child...\n");
      if (BecomeDaemonProcess(NULL, "/dev/console") != B_NO_ERROR)
      {
         LogTime(MUSCLE_LOG_CRITICALERROR, "Couldn't spawn daemon-child process!\n");
         exit(10);
      }
   }

#ifdef WIN32
   if (args.HasName("console"))
   {
      // Open a console for debug output to appear in
      AllocConsole();
      freopen("conout$", "w", stdout);
      freopen("conout$", "w", stderr);
   }
#endif

   const char * value;
   if (args.FindString("display", &value) == B_NO_ERROR)
   {
      int ll = ParseLogLevelKeyword(value);
      if (ll >= 0) SetConsoleLogLevel(ll);
              else LogTime(MUSCLE_LOG_INFO, "Error, unknown display log level type [%s]\n", value);
   }

   if (args.FindString("log", &value) == B_NO_ERROR)
   {
      int ll = ParseLogLevelKeyword(value);
      if (ll >= 0) SetFileLogLevel(ll);
              else LogTime(MUSCLE_LOG_INFO, "Error, unknown file log level type [%s]\n", value);
   }

   if (args.FindString("localhost", &value) == B_NO_ERROR)
   {
      uint32 ip = Inet_AtoN(value);
      if (ip > 0)
      {
         char ipbuf[16]; Inet_NtoA(ip, ipbuf);
         LogTime(MUSCLE_LOG_INFO, "IP address [%s] will be used as the localhost address.\n", ipbuf);
         SetLocalHostIPOverride(ip);
      }
      else LogTime(MUSCLE_LOG_ERROR, "Error parsing localhost IP address [%s]!\n", value);
   }

#if __linux__ || __APPLE__
   {
      const char * niceStr = NULL; (void) args.FindString("nice", &niceStr);
      const char * meanStr = NULL; (void) args.FindString("mean", &meanStr);

      int32 niceLevel = niceStr ? ((strlen(niceStr) > 0) ? atoi(niceStr) : 5) : 0;
      int32 meanLevel = meanStr ? ((strlen(meanStr) > 0) ? atoi(meanStr) : 5) : 0;
      int32 effectiveLevel = niceLevel-meanLevel;

      if (effectiveLevel)
      {
         // cued isn't as time-critical as the other daemons, so he'll defer to them
         errno = 0;  // the only reliable way to check for an error here :^P
         (void) nice(effectiveLevel);
         if (errno != 0) LogTime(MUSCLE_LOG_ERROR, "Process nice(%li) failed (access denied?)\n", effectiveLevel);
                    else LogTime(MUSCLE_LOG_INFO, "Process is now %s (niceLevel=%i)\n", (effectiveLevel<0)?"mean":"nice", effectiveLevel);
      }
   }
#endif

#ifdef __linux__
   const char * priStr;
   if (args.FindString("realtime", &priStr) == B_NO_ERROR)
   {
      struct sched_param schedparam;
      int pri = (strlen(priStr) > 0) ? atoi(priStr) : 11;
      schedparam.sched_priority = pri;

      if (sched_setscheduler(0, SCHED_RR, &schedparam) == 0) LogTime(MUSCLE_LOG_INFO, "Set process to real-time priority %i\n", pri);
                                                        else LogTime(MUSCLE_LOG_ERROR, "Couldn't invoke real time scheduling priority %i (access denied?)\n", pri);
   }
#endif
}

/** Gotta define this myself, since atoll() isn't standard. :^( 
  * Note that this implementation doesn't handle negative numbers!
  */
uint64 Atoull(const char * str)
{
   uint64 base = 1;
   uint64 ret  = 0;

   // Move to the last digit in the number
   const char * s = str;
   while ((*s >= '0')&&(*s <= '9')) s++;

   // Then iterate back to the beginning, tabulating as we go
   while((--s >= str)&&(*s >= '0')&&(*s <= '9')) 
   {
      ret  += base * ((uint64)(*s-'0'));
      base *= (uint64)10;
   }
   return ret;
}

int64 Atoll(const char * str)
{
   bool negative = false;
   const char * s = str;
   while((*s)&&(muscleInRange(*s, '0', '9') == false))
   {
      if (*s == '-') negative = (negative == false);
      s++;
   }
   int64 ret = (int64) Atoull(s);
   return negative ? -ret : ret;
}


String GetHumanReadableTimeString(uint64 timeUS)
{
   char buf[256] = ""; 
   if (timeUS > 0)
   {
#ifdef WIN32
      // Borland's localtime() function is buggy, so we'll use the Win32 API instead.
      uint64 winTime = timeUS*10;  // Convert to (100ns units)
      FILETIME fileTime;
      fileTime.dwHighDateTime = (DWORD) ((winTime>>32) & 0xFFFFFFFF);
      fileTime.dwLowDateTime  = (DWORD) ((winTime>> 0) & 0xFFFFFFFF);
      FILETIME localTime;
      if (FileTimeToLocalFileTime(&fileTime, &localTime))
      {
         SYSTEMTIME st;
         if (FileTimeToSystemTime(&localTime, &st)) sprintf(buf, "%02i/%02i/%02i %02i:%02i:%02i", st.wYear+(1970-1601), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
      }
#else
      time_t timeS = (time_t) (timeUS/1000000);  // timeS is seconds since 1970
      struct tm * ts = localtime(&timeS);
      if (ts) sprintf(buf, "%02i/%02i/%02i %02i:%02i:%02i", ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday, ts->tm_hour, ts->tm_min, ts->tm_sec);
#endif
   }
   return String(buf);
}
 
uint64 ParseHumanReadableTimeString(const String & s)
{
   StringTokenizer tok(s(), "/: ");
   const char * year   = tok();
   const char * month  = tok();
   const char * day    = tok();
   const char * hour   = tok();
   const char * minute = tok();
   const char * second = tok();
   if ((month)&&(day)&&(year)&&(hour)&&(minute)&&(second))
   {
      struct tm temp;
      temp.tm_sec  = atoi(second);
      temp.tm_min  = atoi(minute);
      temp.tm_hour = atoi(hour);
      temp.tm_mday = atoi(day);
      temp.tm_mon  = atoi(month)-1;
      temp.tm_year = atoi(year)-1900;
      time_t timeS = mktime(&temp);
      return ((uint64)timeS)*1000000;
   }
   else return 0;
}

/* Source code stolen from UNIX Network Programming, Volume 1
 * Comments from the Unix FAQ
 */
#ifdef WIN32
status_t SpawnDaemonProcess(bool &, const char *, const char *, bool) 
{ 
   return B_ERROR;  // Win32 can't do this trick, he's too lame  :^(
}
#else
status_t SpawnDaemonProcess(bool & returningAsParent, const char * optNewDir, const char * optOutputTo, bool createIfNecessary)
{
   // Here are the steps to become a daemon:
   // 1. fork() so the parent can exit, this returns control to the command line or shell invoking
   //    your program. This step is required so that the new process is guaranteed not to be a process
   //    group leader. The next step, setsid(), fails if you're a process group leader.
   pid_t pid = fork();
   if (pid < 0) return B_ERROR;
   if (pid > 0) 
   {
      returningAsParent = true;
      return B_NO_ERROR;
   }
   else returningAsParent = false; 

   // 2. setsid() to become a process group and session group leader. Since a controlling terminal is
   //    associated with a session, and this new session has not yet acquired a controlling terminal
   //    our process now has no controlling terminal, which is a Good Thing for daemons.
   setsid();

   // 3. fork() again so the parent, (the session group leader), can exit. This means that we, as a
   //    non-session group leader, can never regain a controlling terminal.
   signal(SIGHUP, SIG_IGN);
   pid = fork();
   if (pid < 0) return B_ERROR;
   if (pid > 0) exit(0);

   // 4. chdir("/") can ensure that our process doesn't keep any directory in use. Failure to do this
   //    could make it so that an administrator couldn't unmount a filesystem, because it was our
   //    current directory. [Equivalently, we could change to any directory containing files important
   //    to the daemon's operation.]
   if (optNewDir) chdir(optNewDir);

   // 5. umask(0) so that we have complete control over the permissions of anything we write.
   //    We don't know what umask we may have inherited. [This step is optional]
   umask(0);

   // 6. close() fds 0, 1, and 2. This releases the standard in, out, and error we inherited from our parent
   //    process. We have no way of knowing where these fds might have been redirected to. Note that many
   //    daemons use sysconf() to determine the limit _SC_OPEN_MAX. _SC_OPEN_MAX tells you the maximun open
   //    files/process. Then in a loop, the daemon can close all possible file descriptors. You have to
   //    decide if you need to do this or not. If you think that there might be file-descriptors open you should
   //    close them, since there's a limit on number of concurrent file descriptors.
   // 7. Establish new open descriptors for stdin, stdout and stderr. Even if you don't plan to use them,
   //    it is still a good idea to have them open. The precise handling of these is a matter of taste;
   //    if you have a logfile, for example, you might wish to open it as stdout or stderr, and open `/dev/null'
   //    as stdin; alternatively, you could open `/dev/console' as stderr and/or stdout, and `/dev/null' as stdin,
   //    or any other combination that makes sense for your particular daemon.
   int nullfd = open("/dev/null", O_RDWR);
   if (nullfd >= 0) dup2(nullfd, STDIN_FILENO);

   int outfd = -1;
   if (optOutputTo) 
   {
      outfd = open(optOutputTo, O_WRONLY | (createIfNecessary ? O_CREAT : 0));
      if (outfd < 0) LogTime(MUSCLE_LOG_ERROR, "BecomeDaemonProcess():  Couldn't open %s to redirect stdout, stderr\n", optOutputTo);
   }
   if (outfd >= 0) dup2(outfd, STDOUT_FILENO);
   if (outfd >= 0) dup2(outfd, STDERR_FILENO);

   return B_NO_ERROR;
}
#endif

status_t BecomeDaemonProcess(const char * optNewDir, const char * optOutputTo, bool createIfNecessary)
{
   bool isParent;
   status_t ret = SpawnDaemonProcess(isParent, optNewDir, optOutputTo, createIfNecessary);
   if ((ret == B_NO_ERROR)&&(isParent)) exit(0);
   return ret;
}


END_NAMESPACE(muscle);
