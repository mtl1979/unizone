/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

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

#include "reflector/StorageReflectConstants.h"  // for PR_COMMAND_BATCH, PR_NAME_KEYS
#include "util/ByteBuffer.h"
#include "util/MiscUtilityFunctions.h"
#include "util/NetworkUtilityFunctions.h"
#include "util/StringTokenizer.h"

BEGIN_NAMESPACE(muscle);

status_t ParseArg(const String & a, Message & addTo)
{
   TCHECKPOINT;

   // Remove any initial dashes
   String argName = a.Trim();
   while(argName.StartsWith("-")) argName = argName.Substring(1);

   int equalsAt = argName.IndexOf('=');
   String argValue;
   if (equalsAt >= 0)
   {
      argValue = argName.Substring(equalsAt+1).Trim();  // this must be first!
      argName  = argName.Substring(0, equalsAt).Trim().ToLowerCase();
   }
   if (argName.HasChars())
   { 
      // Don't allow the parsing to fail just because the user specified a section name the same as a param name!
      uint32 tc;
      if ((addTo.GetInfo(argName, &tc) == B_NO_ERROR)&&(tc != B_STRING_TYPE)) (void) addTo.RemoveName(argName);
      return addTo.AddString(argName, argValue);
   }
   else return B_NO_ERROR;
}

String UnparseArgs(const Message & argsMsg)
{
   String ret, next;
   for (MessageFieldNameIterator it(argsMsg, B_STRING_TYPE); it.HasMoreFieldNames(); it++)
   {
      const String & fn = it.GetFieldName();
      const String * s;
      for (int32 i=0; argsMsg.FindString(fn, i, &s) == B_NO_ERROR; i++)
      {
         if (s->HasChars())
         {
            next = fn;
            next += '=';
            if ((s->IndexOf(' ') >= 0)||(s->IndexOf('\t') >= 0)||(s->IndexOf('\r') >= 0)||(s->IndexOf('\n') >= 0))
            {
               next += '\"';
               next += *s;
               next += '\"';
            }
            else next += *s;
         }
         else next = fn;
      }
      if (next.HasChars())
      {
         if (ret.HasChars()) ret += ' ';
         ret += next;
      }
      next.Clear();
   }
   return ret;
}

status_t ParseArgs(const String & line, Message & addTo)
{
   TCHECKPOINT;

   const String trimmed = line.Trim();
   uint32 len = trimmed.Length();

   // First, we'll pre-process the string into a StringTokenizer-friendly
   // form, by replacing all quoted spaces with gunk and removing the quotes
   String tokenizeThis; 
   if (tokenizeThis.Prealloc(len) != B_NO_ERROR) return B_ERROR;
   const char GUNK_CHAR      = (char) 0x01;
   bool lastCharWasBackslash = false;
   bool inQuotes = false;
   for (uint32 i=0; i<len; i++)
   {
      char c = trimmed[i];
      if ((lastCharWasBackslash == false)&&(c == '\"')) inQuotes = !inQuotes;
      else 
      {
         if ((inQuotes == false)&&(c == '#')) break;  // comment to EOL
         tokenizeThis += ((inQuotes)&&(c == ' ')) ? GUNK_CHAR : c;
      }
      lastCharWasBackslash = (c == '\\');
   }
   StringTokenizer tok(tokenizeThis()," \t\r\n");
   const char * t = tok();
   while(t)
   {
      String n(t);
      n.Replace(GUNK_CHAR, ' ');

      // Check to see if the next token is the equals sign...
      const char * next = tok();
      if ((next)&&(next[0] == '='))
      {
         if (next[1] != '\0')
         {
            // It's the "x =5" case (2 tokens)
            String n2(next);
            n2.Replace(GUNK_CHAR, ' ');
            if (ParseArg(n+n2, addTo) != B_NO_ERROR) return B_ERROR;
            t = tok();
         }
         else
         {
            // It's the "x = 5" case (3 tokens)
            next = tok();  // find out what's after the equal sign
            if (next)
            {
               String n3(next);
               n3.Replace(GUNK_CHAR, ' ');
               if (ParseArg(n+"="+n3, addTo) != B_NO_ERROR) return B_ERROR;
               t = tok();
            }
            else 
            {
               if (ParseArg(n, addTo) != B_NO_ERROR) return B_ERROR;  // for the "x =" case, just parse x and ignore the equals
               t = NULL;
            }
         }
      }
      else if (n.EndsWith('='))
      {
         // Try to attach the next keyword
         t = tok();
         if (t)
         {
            String n4(t);
            n4.Replace(GUNK_CHAR, ' ');
            if (ParseArg(n+n4, addTo) != B_NO_ERROR) return B_ERROR;
            t = tok();
         }
         else if (ParseArg(n, addTo) != B_NO_ERROR) return B_ERROR;
      }
      else
      {
         // Nope, it's just the normal case
         if (ParseArg(n, addTo) != B_NO_ERROR) return B_ERROR;
         t = next;
      }
   }
   return B_NO_ERROR;
}

status_t ParseArgs(int argc, char ** argv, Message & addTo)
{
   for (int i=0; i<argc; i++) if (ParseArg(argv[i], addTo) != B_NO_ERROR) return B_ERROR;
   return B_NO_ERROR;
}

static status_t ParseFileAux(FILE * fpIn, Message & addTo, char * buf, uint32 bufSize)
{
   status_t ret = B_NO_ERROR;
   while(fgets(buf, bufSize, fpIn))
   {
      String checkForSection(buf);
      checkForSection = checkForSection.Trim().ToLowerCase();
      if ((checkForSection == "begin")||(checkForSection.StartsWith("begin ")))
      {
         checkForSection = checkForSection.Substring(6).Trim();
         int32 hashIdx = checkForSection.IndexOf('#');
         if (hashIdx >= 0) checkForSection = checkForSection.Substring(0, hashIdx).Trim();
         
         // Don't allow the parsing to fail just because the user specified a section name the same as a param name!
         uint32 tc;
         if ((addTo.GetInfo(checkForSection, &tc) == B_NO_ERROR)&&(tc != B_MESSAGE_TYPE)) (void) addTo.RemoveName(checkForSection);

         MessageRef subMsg = GetMessageFromPool();
         if ((subMsg() == NULL)||(addTo.AddMessage(checkForSection, subMsg) != B_NO_ERROR)||(ParseFileAux(fpIn, *subMsg(), buf, bufSize) != B_NO_ERROR)) return B_ERROR;
      }
      else if ((checkForSection == "end")||(checkForSection.StartsWith("end "))) return B_NO_ERROR;
      else if (ParseArgs(buf, addTo) != B_NO_ERROR)
      {
         ret = B_ERROR;
         break;
      }
   }
   return ret;
}

status_t ParseFile(FILE * fpIn, Message & addTo)
{
   TCHECKPOINT;

   const int bufSize = 2048;
   char * buf = newnothrow_array(char, bufSize);
   if (buf)
   {
      status_t ret = ParseFileAux(fpIn, addTo, buf, bufSize);
      delete [] buf;
      return ret;
   }
   else 
   {
      WARN_OUT_OF_MEMORY;
      return B_ERROR;
   }
}

status_t ParseConnectArg(const Message & args, const String & fn, String & retHost, uint16 & retPort)
{
   TCHECKPOINT;

   if (args.FindString(fn, retHost) == B_NO_ERROR)
   {
      int32 colIdx = retHost.IndexOf(':');
      if (colIdx >= 0)
      {
         uint16 p = (uint16) atoi(retHost()+colIdx+1);
         if (p > 0) retPort = p;
         retHost = retHost.Substring(0, colIdx);
      }
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t ParsePortArg(const Message & args, const String & fn, uint16 & retPort)
{
   TCHECKPOINT;

   const char * v;
   if (args.FindString(fn, &v) == B_NO_ERROR)
   {
      uint16 r = (uint16) atoi(v);
      if (r > 0)
      {
         retPort = r;
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

#if __linux__
static void CrashSignalHandler(int sig)
{
   // Uninstall this handler, to avoid the possibility of an infinite loop
   signal(SIGSEGV, SIG_DFL);
   signal(SIGBUS,  SIG_DFL);
   signal(SIGILL,  SIG_DFL);
   signal(SIGABRT, SIG_DFL);
   signal(SIGFPE,  SIG_DFL); 

   printf("MUSCLE CrashSignalHandler called with signal %i... I'm going to print a stack trace, then kill the process.\n", sig);
   PrintStackTrace();
   printf("Crashed MUSCLE process aborting now.... bye!\n");
   fflush(stdout);
   abort();
}
#endif

void HandleStandardDaemonArgs(const Message & args)
{
   TCHECKPOINT;

   // Do this first, so that the stuff below will affect the right process.
   const char * n;
   if (args.FindString("daemon", &n) == B_NO_ERROR)
   {
      LogTime(MUSCLE_LOG_INFO, "Spawning off a daemon-child...\n");
      if (BecomeDaemonProcess(NULL, n[0] ? n : "/dev/null") != B_NO_ERROR)
      {
         LogTime(MUSCLE_LOG_CRITICALERROR, "Couldn't spawn daemon-child process!\n");
         ExitWithoutCleanup(10);
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
      ip_address ip = Inet_AtoN(value);
      if (ip != invalidIP)
      {
         char ipbuf[64]; Inet_NtoA(ip, ipbuf);
         LogTime(MUSCLE_LOG_INFO, "IP address [%s] will be used as the localhost address.\n", ipbuf);
         SetLocalHostIPOverride(ip);
      }
      else LogTime(MUSCLE_LOG_ERROR, "Error parsing localhost IP address [%s]!\n", value);
   }

#if __linux__
   if ((args.HasName("debugcrashes"))||(args.HasName("debugcrash")))
   {
      LogTime(MUSCLE_LOG_INFO, "Enabling stack-trace printing when a crash occurs.\n");

      signal(SIGSEGV, CrashSignalHandler);
      signal(SIGBUS,  CrashSignalHandler);
      signal(SIGILL,  CrashSignalHandler);
      signal(SIGABRT, CrashSignalHandler);
      signal(SIGFPE,  CrashSignalHandler); 
   }
#endif

#if __linux__ || __APPLE__
   {
      const char * niceStr = NULL; (void) args.FindString("nice", &niceStr);
      const char * meanStr = NULL; (void) args.FindString("mean", &meanStr);

      int32 niceLevel = niceStr ? ((strlen(niceStr) > 0) ? atoi(niceStr) : 5) : 0;
      int32 meanLevel = meanStr ? ((strlen(meanStr) > 0) ? atoi(meanStr) : 5) : 0;
      int32 effectiveLevel = niceLevel-meanLevel;

      if (effectiveLevel)
      {
         errno = 0;  // the only reliable way to check for an error here :^P
         (void) nice(effectiveLevel);
         if (errno != 0) LogTime(MUSCLE_LOG_WARNING, "Couldn't change process execution priority to "INT32_FORMAT_SPEC".\n", effectiveLevel);
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
   TCHECKPOINT;

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
   TCHECKPOINT;

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


status_t GetHumanReadableTimeValues(uint64 timeUS, int & retYear, int & retMonth, int & retDay, int & retHour, int & retMinute, int & retSecond, uint32 timeType)
{
   TCHECKPOINT;

   if (timeUS == MUSCLE_TIME_NEVER) return B_ERROR;

#ifdef WIN32
   // Borland's localtime() function is buggy, so we'll use the Win32 API instead.
   static const uint64 diffTime = ((uint64)116444736)*((uint64)1000000000); // add (1970-1601) to convert to Windows time base
   uint64 winTime = (timeUS*10) + diffTime;  // Convert to (100ns units)

   FILETIME fileTime;
   fileTime.dwHighDateTime = (DWORD) ((winTime>>32) & 0xFFFFFFFF);
   fileTime.dwLowDateTime  = (DWORD) ((winTime>> 0) & 0xFFFFFFFF);

   SYSTEMTIME st;
   if (FileTimeToSystemTime(&fileTime, &st)) 
   {
      if (timeType == MUSCLE_TIMEZONE_UTC)
      {
         TIME_ZONE_INFORMATION tzi;
         if ((GetTimeZoneInformation(&tzi) == TIME_ZONE_ID_INVALID)||(SystemTimeToTzSpecificLocalTime(&tzi, &st, &st) == false)) return B_ERROR;
      }

      retYear   = st.wYear;
      retMonth  = st.wMonth-1;  // convert to zero-based units
      retDay    = st.wDay-1;    // covnert to zero-based units
      retHour   = st.wHour;
      retMinute = st.wMinute;
      retSecond = st.wSecond;
      return B_NO_ERROR;
   }
#else
   time_t timeS = (time_t) (timeUS/1000000);  // timeS is seconds since 1970
   struct tm * ts = (timeType == MUSCLE_TIMEZONE_UTC) ? localtime(&timeS) : gmtime(&timeS);  // only convert if it isn't already local
   if (ts) 
   {
      retYear   = ts->tm_year+1900;
      retMonth  = ts->tm_mon;
      retDay    = ts->tm_mday-1;
      retHour   = ts->tm_hour;
      retMinute = ts->tm_min;
      retSecond = ts->tm_sec;
      return B_NO_ERROR;
   }
#endif

   return B_ERROR;
}

String GetHumanReadableTimeString(uint64 timeUS, uint32 timeType)
{
   TCHECKPOINT;

   if (timeUS == MUSCLE_TIME_NEVER) return ("(never)");
   else
   {
      int year, month, day, hour, minute, second;
      if (GetHumanReadableTimeValues(timeUS, year, month, day, hour, minute, second, timeType) == B_NO_ERROR)
      {
         char buf[256];
         sprintf(buf, "%02i/%02i/%02i %02i:%02i:%02i", year, month+1, day+1, hour, minute, second);
         return String(buf);
      }
      return "";
   }
}
 
#ifdef WIN32
extern uint64 __Win32FileTimeToMuscleTime(const FILETIME & ft);  // from SetupSystem.cpp
#endif

uint64 ParseHumanReadableTimeString(const String & s, uint32 timeType)
{
   TCHECKPOINT;

   if (s.IndexOfIgnoreCase("never") >= 0) return MUSCLE_TIME_NEVER;

   StringTokenizer tok(s(), "/: ");
   const char * year   = tok();
   const char * month  = tok();
   const char * day    = tok();
   const char * hour   = tok();
   const char * minute = tok();
   const char * second = tok();

#if defined(WIN32) && defined(WINXP)
   SYSTEMTIME st; memset(&st, 0, sizeof(st));
   st.wYear      = (WORD) (year   ? atoi(year)   : 0);
   st.wMonth     = (WORD) (month  ? atoi(month)  : 0);
   st.wDay       = (WORD) (day    ? atoi(day)    : 0);
   st.wHour      = (WORD) (hour   ? atoi(hour)   : 0);
   st.wMinute    = (WORD) (minute ? atoi(minute) : 0);
   st.wSecond    = (WORD) (second ? atoi(second) : 0);

   if (timeType == MUSCLE_TIMEZONE_UTC)
   {
      TIME_ZONE_INFORMATION tzi;
      if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID) 
      {
# if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
         // Some compilers' headers don't have this call, so we have to do it the hard way
         HMODULE lib = LoadLibrary(TEXT("kernel32.dll"));
         if (lib)
         {
#  if defined(_MSC_VER)
            typedef BOOL (*TzSpecificLocalTimeToSystemTimeProc) (IN LPTIME_ZONE_INFORMATION lpTimeZoneInformation, IN LPSYSTEMTIME lpLocalTime, OUT LPSYSTEMTIME lpUniversalTime);
#  else
            typedef WINBASEAPI BOOL WINAPI (*TzSpecificLocalTimeToSystemTimeProc) (IN LPTIME_ZONE_INFORMATION lpTimeZoneInformation, IN LPSYSTEMTIME lpLocalTime, OUT LPSYSTEMTIME lpUniversalTime);
#  endif

            TzSpecificLocalTimeToSystemTimeProc tzProc = (TzSpecificLocalTimeToSystemTimeProc) GetProcAddress(lib, "TzSpecificLocalTimeToSystemTime");
            if (tzProc) tzProc(&tzi, &st, &st);
            if (lib != NULL) FreeLibrary(lib);
         }
# else
         (void) TzSpecificLocalTimeToSystemTime(&tzi, &st, &st);
# endif
      }
   }

   FILETIME fileTime;
   return (SystemTimeToFileTime(&st, &fileTime)) ? __Win32FileTimeToMuscleTime(fileTime) : 0;
#else
   struct tm st; memset(&st, 0, sizeof(st));
   st.tm_sec  = second ? atoi(second)    : 0;
   st.tm_min  = minute ? atoi(minute)    : 0;
   st.tm_hour = hour   ? atoi(hour)      : 0;
   st.tm_mday = day    ? atoi(day)       : 0;
   st.tm_mon  = month  ? atoi(month)-1   : 0;
   st.tm_year = year   ? atoi(year)-1900 : 0;
   time_t timeS = mktime(&st);
   if (timeType == MUSCLE_TIMEZONE_LOCAL)
   {
      struct tm * t = gmtime(&timeS);
      if (t) timeS += (timeS-mktime(t));  
   }
   return ((uint64)timeS)*1000000;
#endif
}

static bool _isDaemonProcess = false;
bool IsDaemonProcess() {return _isDaemonProcess;}

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
   TCHECKPOINT;

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
   if (pid > 0) ExitWithoutCleanup(0);

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

   _isDaemonProcess = true;
   return B_NO_ERROR;
}
#endif

status_t BecomeDaemonProcess(const char * optNewDir, const char * optOutputTo, bool createIfNecessary)
{
   bool isParent;
   status_t ret = SpawnDaemonProcess(isParent, optNewDir, optOutputTo, createIfNecessary);
   if ((ret == B_NO_ERROR)&&(isParent)) ExitWithoutCleanup(0);
   return ret;
}

void RemoveANSISequences(String & s)
{
   TCHECKPOINT;

   static const char _escapeBuf[] = {0x1B, '[', '\0'};
   static String _escape; if (_escape.IsEmpty()) _escape = _escapeBuf;

   while(true)
   {
      int32 idx = s.IndexOf(_escape);  // find the next escape sequence
      if (idx >= 0)
      {
         const char * data = s()+idx+2;  // move past the ESC char and the [ char
         switch(data[0])
         {
            case 's': case 'u': case 'K':   // these are single-letter codes, so
	       data++;                      // just skip over them and we are done
	    break;

            case '=':
	       data++;
	    // fall through!
	    default:
	       // For numeric codes, keep going until we find a non-digit that isn't a semicolon
	       while((muscleInRange(*data, '0', '9'))||(*data == ';')) data++;
	       if (*data) data++;  // and skip over the trailing letter too.
	    break;
         }
	 s = s.Substring(0, idx) + s.Substring(data-s());  // remove the escape substring
      }
      else break;
   }
}

status_t NybbleizeData(const ByteBuffer & buf, String & retString)
{
   uint32 numBytes = buf.GetNumBytes();

   if (retString.Prealloc(numBytes*2) != B_NO_ERROR) return B_ERROR;

   retString.Clear();
   const uint8 * b = buf.GetBuffer();
   for (uint32 i=0; i<numBytes; i++)
   {
      uint8 c = b[i];
      retString += ((c>>0)&0x0F)+'A';
      retString += ((c>>4)&0x0F)+'A';
   }
   return B_NO_ERROR;
}

status_t DenybbleizeData(const String & nybbleizedText, ByteBuffer & retBuf)
{
   uint32 numBytes = nybbleizedText.Length();
   if (retBuf.SetNumBytes(numBytes/2, false) != B_NO_ERROR) return B_ERROR;

   uint8 * b = retBuf.GetBuffer();
   for (uint32 i=0; i<numBytes; i+=2)
   {
      char c1 = nybbleizedText[i+0];
      char c2 = nybbleizedText[i+1];
      *b++ = ((c1-'A')<<0)|((c2-'A')<<4);
   }
   return B_NO_ERROR;
}

String NybbleizeString(const String & s)
{
   String retStr;
   ByteBuffer inBuf; inBuf.AdoptBuffer(s.Length(), (uint8 *) s());

   status_t ret = NybbleizeData(inBuf, retStr);
   (void) inBuf.ReleaseBuffer();

   if (ret != B_NO_ERROR) retStr.Clear();
   return retStr;
}

String DenybbleizeString(const String & ns)
{
   ByteBuffer outBuf;
   return (DenybbleizeData(ns, outBuf) == B_NO_ERROR) ? String((const char *) outBuf.GetBuffer(), outBuf.GetNumBytes()) : String();
}

const uint8 * MemMem(const uint8 * lookIn, uint32 numLookInBytes, const uint8 * lookFor, uint32 numLookForBytes)
{
        if (numLookForBytes == 0)              return lookIn;  // hmm, existential questions here
   else if (numLookForBytes == numLookInBytes) return (memcmp(lookIn, lookFor, numLookInBytes) == 0) ? lookIn : NULL;
   else if (numLookForBytes < numLookInBytes)
   {
      const uint8 * startedAt = lookIn;
      uint32 matchCount = 0;
      for (uint32 i=0; i<numLookInBytes; i++)
      {
         if (lookIn[i] == lookFor[matchCount])
         {
            if (matchCount == 0) startedAt = &lookIn[i];
            if (++matchCount == numLookForBytes) return startedAt;
         }
         else matchCount = 0;
      }
   }
   return NULL;
}

ByteBufferRef ParseHexBytes(const char * buf)
{
   ByteBufferRef bb = GetByteBufferFromPool(strlen(buf));
   if (bb())
   {
      uint8 * b = bb()->GetBuffer();
      uint32 count = 0;
      StringTokenizer tok(buf, ", \t\r\n");
      const char * next;
      while((next = tok()) != NULL) 
      {
         if (strlen(next) > 0) 
         {
            if (next[0] == '/') b[count++] = next[1];
                           else b[count++] = (uint8) strtol(next, NULL, 16);
         }
      }
      bb()->SetNumBytes(count, true);
   }
   return bb;
}

status_t AssembleBatchMessage(MessageRef & batchMsg, const MessageRef & newMsg)
{
   if (batchMsg() == NULL)
   {
      batchMsg = newMsg;
      return B_NO_ERROR;
   }
   else if (batchMsg()->what == PR_COMMAND_BATCH) return batchMsg()->AddMessage(PR_NAME_KEYS, newMsg);
   else
   {
      MessageRef newBatchMsg = GetMessageFromPool(PR_COMMAND_BATCH);
      if ((newBatchMsg())&&(newBatchMsg()->AddMessage(PR_NAME_KEYS, batchMsg) == B_NO_ERROR)&&(newBatchMsg()->AddMessage(PR_NAME_KEYS, newMsg) == B_NO_ERROR))
      {
         batchMsg = newBatchMsg;
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

END_NAMESPACE(muscle);
