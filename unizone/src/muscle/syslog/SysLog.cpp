/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include <stdio.h>
#include <stdarg.h>
#include "syslog/LogCallback.h"
#include "system/SetupSystem.h"
#include "util/Hashtable.h"
#include "util/NestCount.h"
#include "util/String.h"

#if defined(__APPLE__)
# include "AvailabilityMacros.h"  // so we can find out if this version of MacOS/X is new enough to include backtrace() and friends
#endif

#if defined(__linux__) || (defined(MAC_OS_X_VERSION_10_5) && defined(MAC_OS_X_VERSION_MAX_ALLOWED) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5))
# include <execinfo.h>
# define MUSCLE_USE_BACKTRACE 1
#endif

namespace muscle {

#define MAX_STACK_TRACE_DEPTH ((uint32)(256))

status_t PrintStackTrace(FILE * optFile, uint32 maxDepth)
{
   TCHECKPOINT;

   if (optFile == NULL) optFile = stdout;

#if defined(MUSCLE_USE_BACKTRACE)
   void *array[MAX_STACK_TRACE_DEPTH];
   size_t size = backtrace(array, muscleMin(maxDepth, MAX_STACK_TRACE_DEPTH));
   char ** strings = backtrace_symbols(array, size);
   if (strings)
   {
      fprintf(optFile, "--Stack trace follows (%zd frames):\n", size);
      for (size_t i = 0; i < size; i++) fprintf(optFile, "  %s\n", strings[i]);
      fprintf(optFile, "--End Stack trace\n");
      free(strings);
      return B_NO_ERROR;
   }
   else fprintf(optFile, "PrintStackTrace:  Error, couldn't generate stack trace!\n");
#else
   (void) maxDepth;  // shut the compiler up
   fprintf(optFile, "PrintStackTrace:  Error, stack trace printing not available on this platform!\n");
#endif

   return B_ERROR;  // I don't know how to do this for other systems!
}

status_t GetStackTrace(String & retStr, uint32 maxDepth)
{
   TCHECKPOINT;

#if defined(MUSCLE_USE_BACKTRACE)
   void *array[MAX_STACK_TRACE_DEPTH];
   size_t size = backtrace(array, muscleMin(maxDepth, MAX_STACK_TRACE_DEPTH));
   char ** strings = backtrace_symbols(array, size);
   if (strings)
   {
      char buf[128];
      sprintf(buf, "--Stack trace follows (%zd frames):", size); retStr += buf;
      for (size_t i = 0; i < size; i++) 
      {
         retStr += "\n  ";
         retStr += strings[i];
      }
      retStr += "\n--End Stack trace\n";
      free(strings);
      return B_NO_ERROR;
   }
#else
   (void) retStr;   // shut the compiler up
   (void) maxDepth;
#endif

   return B_ERROR;
}

#ifndef MUSCLE_INLINE_LOGGING

// VC++6 can't handle partial template specialization, so we'll do it explicitly here
#ifdef MUSCLE_USING_OLD_MICROSOFT_COMPILER
DECLARE_HASHTABLE_KEY_CLASS(Ref<LogCallback>);
#endif

static const char * const _logLevelNames[] = {
   "None",
   "Critical Errors Only",
   "Errors Only",
   "Warnings and Errors Only",
   "Informational",
   "Debug",
   "Trace"
};

static const char * const _logLevelKeywords[] = {
   "none",
   "critical",
   "errors",
   "warnings",
   "info",
   "debug",
   "trace"
};

class DefaultConsoleLogger : public LogCallback
{
public:
   DefaultConsoleLogger() : _consoleLogLevel(MUSCLE_LOG_INFO)
   {
       // empty
   }

   virtual void Log(const LogCallbackArgs & a)
   {
      if (a.GetLogLevel() <= _consoleLogLevel) 
      {
         vprintf(a.GetText(), *a.GetArgList());                  
         fflush(stdout);
      }
   }

   virtual void Flush()
   {
      fflush(stdout);
   }

   int _consoleLogLevel;
};

// Win32 doesn't have localtime_r, so we have to roll our own
#if defined(WIN32)
static inline struct tm * muscle_localtime_r(time_t * clock, struct tm * result)
{
   // Note that in Win32, (ret) points to thread-local storage, so this really
   // is thread-safe despite the fact that it looks like it isn't!
   struct tm * ret = localtime(clock);
   if (ret) *result = *ret;
   return ret;
}
#else
static inline struct tm * muscle_localtime_r(time_t * clock, struct tm * result) 
{
   return localtime_r(clock, result);
}
#endif

class DefaultFileLogger : public LogCallback
{
public:
   DefaultFileLogger() : _fileLogLevel(MUSCLE_LOG_NONE), _logFile(NULL)
   {
       // empty
   }

   virtual void Log(const LogCallbackArgs & a)
   {
      if ((a.GetLogLevel() <= _fileLogLevel)&&(_logFile == NULL))
      {
         struct tm wtm;
         time_t when = a.GetWhen();
         struct tm * now = muscle_localtime_r(&when, &wtm);
         char tb[100]; sprintf(tb, "%02i%02i%02i%02i.log", now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min); 
         const char * fn = _fileLogName.HasChars() ? _fileLogName() : tb;
         _logFile = fopen(fn, "w");              
         if (_logFile) LogTime(MUSCLE_LOG_INFO, "Created log file %s\n", fn);
      }                                          
      if ((a.GetLogLevel() <= _fileLogLevel)&&(_logFile))
      {                                         
         vfprintf(_logFile, a.GetText(), *a.GetArgList());
         Flush();
      }
   }

   virtual void Flush()
   {
      if (_logFile) fflush(_logFile);
   }

   int _fileLogLevel;
   String _fileLogName;
   FILE * _logFile;
};

LogLineCallback :: LogLineCallback() : _writeTo(_buf)
{
   _buf[0] = '\0';
   _buf[sizeof(_buf)-1] = '\0';  // just in case vsnsprintf() has to truncate
}

LogLineCallback :: ~LogLineCallback() 
{
   // empty
}

void LogLineCallback :: Log(const LogCallbackArgs & a)
{
   TCHECKPOINT;

   // Generate the new text
#ifdef __MWERKS__
   int bytesAttempted = vsprintf(_writeTo, a.GetText(), *a.GetArgList());  // BeOS/PPC doesn't know vsnprintf :^P
#elif WIN32
   int bytesAttempted = _vsnprintf(_writeTo, (sizeof(_buf)-1)-(_writeTo-_buf), a.GetText(), *a.GetArgList());  // the -1 is for the guaranteed NUL terminator
#else
   int bytesAttempted = vsnprintf(_writeTo, (sizeof(_buf)-1)-(_writeTo-_buf), a.GetText(), *a.GetArgList());  // the -1 is for the guaranteed NUL terminator
#endif
   bool wasTruncated = (bytesAttempted != (int)strlen(_writeTo));  // do not combine with above line!

   // Log any newly completed lines
   char * logFrom  = _buf;
   char * searchAt = _writeTo;
   LogCallbackArgs tmp(a);
   while(true)
   {
      char * nextReturn = strchr(searchAt, '\n');
      if (nextReturn)
      {
         *nextReturn = '\0';  // terminate the string
         tmp.SetText(logFrom);
         LogLine(tmp);
         searchAt = logFrom = nextReturn+1;
      }
      else 
      {
         // If we ran out of buffer space and no carriage returns were detected,
         // then we need to just dump what we have and move on, there's nothing else we can do
         if (wasTruncated)
         {
            tmp.SetText(logFrom);
            LogLine(tmp);
            _buf[0] = '\0';
            _writeTo = searchAt = logFrom = _buf;
         }
         break;
      }
   }

   // And finally, move any remaining incomplete lines back to the beginning of the array, for next time
   if (logFrom > _buf) 
   {
      int slen = strlen(logFrom);
      memmove(_buf, logFrom, slen+1);  // include NUL byte
      _writeTo = &_buf[slen];          // point to our just-moved NUL byte
   }
   else _writeTo = strchr(searchAt, '\0');

   _lastLog = a;
}
 
void LogLineCallback :: Flush()
{
   TCHECKPOINT;

   if (_writeTo > _buf)
   {
      _lastLog.SetText(_buf);
      LogLine(_lastLog);
      _writeTo = _buf;
      _buf[0] = '\0';
   }
}

static NestCount _inLogCallNestCount; // to avoid re-entrancy during logcallback calls
static Hashtable<LogCallbackRef, bool> _logCallbacks;
static DefaultConsoleLogger _dcl;
static DefaultFileLogger _dfl;

status_t LockLog()
{
#ifdef MUSCLE_SINGLE_THREAD_ONLY
   return B_NO_ERROR;
#else
   Mutex * ml = GetGlobalMuscleLock();
   return ml ? ml->Lock() : B_ERROR;
#endif
}

status_t UnlockLog()
{
#ifdef MUSCLE_SINGLE_THREAD_ONLY
   return B_NO_ERROR;
#else
   Mutex * ml = GetGlobalMuscleLock();
   return ml ? ml->Unlock() : B_ERROR;
#endif
}

const char * GetLogLevelName(int ll)
{
   return ((ll>=0)&&(ll<(int) ARRAYITEMS(_logLevelNames))) ? _logLevelNames[ll] : "???";
}

const char * GetLogLevelKeyword(int ll)
{
   return ((ll>=0)&&(ll<(int) ARRAYITEMS(_logLevelKeywords))) ? _logLevelKeywords[ll] : "???";
}

int ParseLogLevelKeyword(const char * keyword)
{
   for (uint32 i=0; i<ARRAYITEMS(_logLevelKeywords); i++) if (strcmp(keyword, _logLevelKeywords[i]) == 0) return i;
   return -1;
}

int GetFileLogLevel()
{
   return _dfl._fileLogLevel;
}

String GetFileLogName()
{
   return _dfl._fileLogName;
}

int GetConsoleLogLevel()
{
   return _dcl._consoleLogLevel;
}

int GetMaxLogLevel()
{
   bool isLogLocked = (LockLog() == B_NO_ERROR);
   int ret = muscleMax(_dcl._consoleLogLevel, _dfl._fileLogLevel);
   if (isLogLocked) (void) UnlockLog();
   return ret;
}

status_t SetFileLogName(const String & logName)
{
   if (LockLog() == B_NO_ERROR)
   {
      _dfl._fileLogName = logName;
      LogTime(MUSCLE_LOG_DEBUG, "File log name set to: %s\n", logName());
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetFileLogLevel(int loglevel)
{
   if (LockLog() == B_NO_ERROR)
   {
      _dfl._fileLogLevel = loglevel;
      LogTime(MUSCLE_LOG_DEBUG, "File logging level set to: %s\n", GetLogLevelName(_dfl._fileLogLevel));
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetConsoleLogLevel(int loglevel)
{
   if (LockLog() == B_NO_ERROR)
   {
      _dcl._consoleLogLevel = loglevel;
      LogTime(MUSCLE_LOG_DEBUG, "Console logging level set to: %s\n", GetLogLevelName(_dcl._consoleLogLevel));
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

#define DO_LOGGING(doCallbacks)                                     \
{                                                                   \
   va_list argList;                                                 \
   va_start(argList, fmt);                                          \
   _dfl.Log(LogCallbackArgs(when, ll, sourceFile, sourceFunction, sourceLine, fmt, &argList)); \
   va_end(argList);                                                 \
   va_start(argList, fmt);                                          \
   _dcl.Log(LogCallbackArgs(when, ll, sourceFile, sourceFunction, sourceLine, fmt, &argList)); \
   va_end(argList);                                                 \
   if (doCallbacks)                                                 \
   {                                                                \
      HashtableIterator<LogCallbackRef, bool> iter(_logCallbacks);  \
      const LogCallbackRef * nextKey;                               \
      while((nextKey = iter.GetNextKey()) != NULL)                  \
      {                                                             \
         if (nextKey->GetItemPointer())                             \
         {                                                          \
            va_start(argList, fmt);                                 \
            nextKey->GetItemPointer()->Log(LogCallbackArgs(when, ll, sourceFile, sourceFunction, sourceLine, fmt, &argList)); \
            va_end(argList);                                        \
         }                                                          \
      }                                                             \
   }                                                                \
}

// Our 34-character alphabet of usable symbols
#define NUM_CHARS_IN_KEY_ALPHABET (sizeof(_keyAlphabet)-1)
static const char _keyAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ2346789";  // 0, 1, and 5 intentionally omitted since they look like O and I, and S
static const uint32 _keySpaceSize = NUM_CHARS_IN_KEY_ALPHABET * NUM_CHARS_IN_KEY_ALPHABET * NUM_CHARS_IN_KEY_ALPHABET * NUM_CHARS_IN_KEY_ALPHABET;

uint32 GenerateSourceCodeLocationKey(const char * fileName, uint32 lineNumber)
{
#ifdef WIN32
   const char * lastSlash = strrchr(fileName, '\\');
#else
   const char * lastSlash = strrchr(fileName, '/');
#endif
   if (lastSlash) fileName = lastSlash+1;

   return ((CStringHashFunc(fileName)+lineNumber)%(_keySpaceSize-1))+1;  // note that 0 is not considered a valid key value!
}

String SourceCodeLocationKeyToString(uint32 key)
{
   if (key == 0) return "";                  // 0 is not a valid key value
   if (key >= _keySpaceSize) return "????";  // values greater than or equal to our key space size are errors

   char buf[5]; buf[4] = '\0';
   for (int32 i=3; i>=0; i--)
   {
      buf[i] = _keyAlphabet[key % NUM_CHARS_IN_KEY_ALPHABET];
      key /= NUM_CHARS_IN_KEY_ALPHABET; 
   }
   return buf;
}

uint32 SourceCodeLocationKeyFromString(const String & ss)
{
   String s = ss.ToUpperCase().Trim();
   if (s.Length() != 4) return 0;  // codes must always be exactly 4 characters long!

   s.Replace('0', 'O');
   s.Replace('1', 'I');
   s.Replace('5', 'S');

   uint32 ret  = 0;
   uint32 base = 1;
   for (int32 i=3; i>=0; i--)
   {
      const char * p = strchr(_keyAlphabet, s[i]);
      if (p == NULL) return 0;  // invalid character!

      int whichChar = (int) (p-_keyAlphabet);
      ret += (whichChar*base);
      base *= NUM_CHARS_IN_KEY_ALPHABET;  // -1 because the NUL terminator doesn't count
   }
   return ret;
}

void GetStandardLogLinePreamble(char * buf, const LogCallbackArgs & a)
{
   struct tm ltm;
   time_t when = a.GetWhen();
   struct tm * temp = muscle_localtime_r(&when, &ltm);
#ifdef MUSCLE_INCLUDE_SOURCE_LOCATION_IN_LOGTIME
   sprintf(buf, "[%c %02i/%02i %02i:%02i:%02i] [%s] ", GetLogLevelName(a.GetLogLevel())[0], temp->tm_mon+1, temp->tm_mday, temp->tm_hour, temp->tm_min, temp->tm_sec, SourceCodeLocationKeyToString(GenerateSourceCodeLocationKey(a.GetSourceFile(), a.GetSourceLineNumber()))());
#else
   sprintf(buf, "[%c %02i/%02i %02i:%02i:%02i] ", GetLogLevelName(a.GetLogLevel())[0], temp->tm_mon+1, temp->tm_mday, temp->tm_hour, temp->tm_min, temp->tm_sec);
#endif
}

#ifdef MUSCLE_INCLUDE_SOURCE_LOCATION_IN_LOGTIME
status_t _LogTime(const char * sourceFile, const char * sourceFunction, int sourceLine, int ll, const char * fmt, ...)
#else
status_t LogTime(int ll, const char * fmt, ...)
#endif
{
#ifndef MUSCLE_INCLUDE_SOURCE_LOCATION_IN_LOGTIME
   static const char * sourceFile = "";
   static const char * sourceFunction = "";
   static const int sourceLine = -1;
#endif

   status_t lockRet = LockLog();
   if (_inLogCallNestCount.IsInBatch() == false)
   {
      _inLogCallNestCount.Increment();
      {
         time_t when = time(NULL);
         char buf[128];

         va_list dummyList;
         va_start(dummyList, fmt);  // not used
         LogCallbackArgs lca(when, ll, sourceFile, sourceFunction, sourceLine, buf, &dummyList);
         GetStandardLogLinePreamble(buf, lca);
         lca.SetText(buf);
         _dfl.Log(lca);
         va_end(dummyList);

         va_start(dummyList, fmt);  // not used
         _dcl.Log(LogCallbackArgs(when, ll, sourceFile, sourceFunction, sourceLine, buf, &dummyList));
         va_end(dummyList);
      
         // Log message to file/stdio and callbacks
         DO_LOGGING((lockRet==B_NO_ERROR));
      }
      _inLogCallNestCount.Decrement();
   }
   if (lockRet == B_NO_ERROR) UnlockLog();

   return lockRet;
}

status_t LogFlush()
{
   TCHECKPOINT;

   if (LockLog() == B_NO_ERROR)
   {
      HashtableIterator<LogCallbackRef, bool> iter(_logCallbacks);
      const LogCallbackRef * next;
      while((next = iter.GetNextKey()) != NULL) if (next->GetItemPointer()) next->GetItemPointer()->Flush();
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR; 
}

status_t LogStackTrace(int ll, uint32 maxDepth)
{
   TCHECKPOINT;

#if defined(MUSCLE_USE_BACKTRACE)
   void *array[MAX_STACK_TRACE_DEPTH];
   size_t size = backtrace(array, muscleMin(maxDepth, MAX_STACK_TRACE_DEPTH));
   char ** strings = backtrace_symbols(array, size);
   if (strings)
   {
      LogTime(ll, "--Stack trace follows (%zd frames):\n", size);
      for (size_t i = 0; i < size; i++) LogTime(ll, "  %s\n", strings[i]);
      LogTime(ll, "--End Stack trace\n");
      free(strings);
      return B_NO_ERROR;
   }
#else
   (void) ll;        // shut the compiler up
   (void) maxDepth;  // shut the compiler up
#endif

   return B_ERROR;  // I don't know how to do this for other systems!
}

status_t Log(int ll, const char * fmt, ...)
{
   // No way to get these, since #define Log() as a macro causes
   // nasty namespace collisions with other methods/functions named Log()
   static const char * sourceFile     = "";
   static const char * sourceFunction = "";
   static const int sourceLine        = -1;

   status_t lockRet = LockLog();
   if (_inLogCallNestCount.IsInBatch() == false)
   {
      _inLogCallNestCount.Increment();
      {
         time_t when = time(NULL);  // don't inline this, ya dummy
         DO_LOGGING((lockRet==B_NO_ERROR));
      }
      _inLogCallNestCount.Decrement();
   }
   if (lockRet == B_NO_ERROR) (void) UnlockLog();
   return lockRet;
}

status_t PutLogCallback(const LogCallbackRef & cb)
{
   status_t ret = B_ERROR;
   if (LockLog() == B_NO_ERROR)
   {
      ret = _logCallbacks.Put(cb, true);
      (void) UnlockLog();
   }
   return ret;
}

status_t ClearLogCallbacks()
{
   status_t ret = B_ERROR;
   if (LockLog() == B_NO_ERROR)
   {
      _logCallbacks.Clear();
      (void) UnlockLog();
   }
   return ret;
}

status_t RemoveLogCallback(const LogCallbackRef & cb)
{
   status_t ret = B_ERROR;
   if (LockLog() == B_NO_ERROR)
   {
      ret = _logCallbacks.Remove(cb);
      (void) UnlockLog();
   }
   return ret;
}

#endif

}; // end namespace muscle
