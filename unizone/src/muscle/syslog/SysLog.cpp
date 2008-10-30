/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include <stdio.h>
#include <stdarg.h>
#include "syslog/LogCallback.h"
#include "system/SetupSystem.h"
#include "util/Hashtable.h"
#include "util/MiscUtilityFunctions.h"  // for ExitWithoutCleanup()
#include "util/NestCount.h"

#if defined(__APPLE__)
# include "AvailabilityMacros.h"  // so we can find out if this version of MacOS/X is new enough to include backtrace() and friends
#endif

#if defined(__linux__) || (defined(MAC_OS_X_VERSION_10_5) && defined(MAC_OS_X_VERSION_MAX_ALLOWED) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5))
# include <execinfo.h>
# define MUSCLE_USE_BACKTRACE 1
#endif

BEGIN_NAMESPACE(muscle);

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
   (void) maxDepth;  // shut the compiler up
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

   virtual void Log(time_t, int ll, const char * fmt, va_list argList)
   {
      if (ll <= _consoleLogLevel) 
      {
         vprintf(fmt, argList);                  
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

   virtual void Log(time_t when, int ll, const char * fmt, va_list argList)
   {
      if ((ll <= _fileLogLevel)&&(_logFile == NULL))
      {
         struct tm wtm;
         struct tm * now = muscle_localtime_r(&when, &wtm);
         char tb[100];                           
         sprintf(tb, "%02i%02i%02i%02i.log", now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min); 
         _logFile = fopen(tb, "w");              
         if (_logFile) LogTime(MUSCLE_LOG_INFO, "Created log file %s\n", tb);
      }                                          
      if ((ll <= _fileLogLevel)&&(_logFile))
      {                                         
         vfprintf(_logFile, fmt, argList);       
         Flush();
      }
   }

   virtual void Flush()
   {
      if (_logFile) fflush(_logFile);
   }

   int _fileLogLevel;
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

void LogLineCallback :: Log(time_t when, int logLevel, const char * format, va_list argList)
{
   TCHECKPOINT;

   // Generate the new text
#ifdef __MWERKS__
   int bytesAttempted = vsprintf(_writeTo, format, argList);  // BeOS/PPC doesn't know vsnprintf :^P
#elif WIN32
   int bytesAttempted = _vsnprintf(_writeTo, (sizeof(_buf)-1)-(_writeTo-_buf), format, argList);  // the -1 is for the guaranteed NUL terminator
#else   
   int bytesAttempted = vsnprintf(_writeTo, (sizeof(_buf)-1)-(_writeTo-_buf), format, argList);  // the -1 is for the guaranteed NUL terminator
#endif
   bool wasTruncated = (bytesAttempted != (int)strlen(_writeTo));  // do not combine with above line!

   // Log any newly completed lines
   char * logFrom  = _buf;
   char * searchAt = _writeTo;
   while(true)
   {
      char * nextReturn = strchr(searchAt, '\n');
      if (nextReturn)
      {
         *nextReturn = '\0';  // terminate the string
         LogLine(when, logLevel, logFrom);
         searchAt = logFrom = nextReturn+1;
      }
      else 
      {
         // If we ran out of buffer space and no carriage returns were detected,
         // then we need to just dump what we have and move on, there's nothing else we can do
         if (wasTruncated)
         {
            LogLine(when, logLevel, logFrom);
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

   _lastLogWhen = when;
   _lastLogLevel = logLevel;
}
 
void LogLineCallback :: Flush()
{
   TCHECKPOINT;

   if (_writeTo > _buf)
   {
      LogLine(_lastLogWhen, _lastLogLevel, _buf);
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

#define DO_LOGGING(when, doCallbacks)                               \
{                                                                   \
   va_list argList;                                                 \
   va_start(argList, fmt);                                          \
   _dfl.Log(when, ll, fmt, argList);                                \
   va_end(argList);                                                 \
   va_start(argList, fmt);                                          \
   _dcl.Log(when, ll, fmt, argList);                                \
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
            nextKey->GetItemPointer()->Log(when, ll, fmt, argList); \
            va_end(argList);                                        \
         }                                                          \
      }                                                             \
   }                                                                \
}

void GetStandardLogLinePreamble(char * buf, int logLevel, time_t when)
{
   struct tm ltm;
   struct tm * temp = muscle_localtime_r(&when, &ltm);
   sprintf(buf, "[%c %02i/%02i %02i:%02i:%02i] ", GetLogLevelName(logLevel)[0], temp->tm_mon+1, temp->tm_mday, temp->tm_hour, temp->tm_min, temp->tm_sec);
}

status_t LogTime(int ll, const char * fmt, ...)
{
   TCHECKPOINT;

   status_t lockRet = LockLog();
   if (_inLogCallNestCount.IsInBatch() == false)
   {
      _inLogCallNestCount.Increment();
      {
         time_t n = time(NULL);
         char buf[128];
         GetStandardLogLinePreamble(buf, ll, n);

         va_list dummyList;
         va_start(dummyList, fmt);  // not used
         _dfl.Log(n, ll, buf, dummyList);
         va_end(dummyList);
         va_start(dummyList, fmt);  // not used
         _dcl.Log(n, ll, buf, dummyList);
         va_end(dummyList);
      
         // Log message to file/stdio and callbacks
         DO_LOGGING(n, (lockRet==B_NO_ERROR));
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
   TCHECKPOINT;

   status_t lockRet = LockLog();
   if (_inLogCallNestCount.IsInBatch() == false)
   {
      _inLogCallNestCount.Increment();
      {
         time_t n = time(NULL);  // don't inline this, ya dummy
         DO_LOGGING(n, (lockRet==B_NO_ERROR));
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

END_NAMESPACE(muscle);
