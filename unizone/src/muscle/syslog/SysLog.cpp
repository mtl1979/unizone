/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "syslog/LogCallback.h"
#include "system/Mutex.h"
#include "util/Hashtable.h"

namespace muscle {

// VC++ can't handle partial template specialization, so we'll do it explicitly here
#ifdef _MSC_VER
template <> class HashFunctor<Ref<LogCallback> >
{
public:
   uint32 operator() (const Ref<LogCallback> x) const {return (uint32)x();}
};
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
         struct tm * now = localtime(&when);
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
}

LogLineCallback :: ~LogLineCallback() 
{
   // empty
}

void LogLineCallback :: Log(time_t when, int logLevel, const char * format, va_list argList)
{
   // Generate the new text
   vsprintf(_writeTo, format, argList);
   _writeTo = strchr(_writeTo, '\0');
   if (_writeTo >= _buf+sizeof(_buf))
   { 
      printf("LogLineCallback: buffer overflow detected!!!");
      exit(10);
   }

   // Log any newly completed lines
   char * startAt = _buf;
   while(true)
   {
      char * nextReturn = strchr(startAt, '\n');
      if (nextReturn)
      {
         *nextReturn = '\0';  // terminate the string
         LogLine(when, logLevel, startAt);
         startAt = nextReturn+1;
      }
      else break;
   }
   
   // And finally, move any partial lines back to the beginning of the array
   if (startAt > _buf) 
   {
      memmove(_buf, startAt, strlen(startAt)+1);
      _writeTo = strchr(_buf, '\0');
   }
   
   _lastLogWhen = when;
   _lastLogLevel = logLevel;
}
 
void LogLineCallback :: Flush()
{
   if (_writeTo > _buf)
   {
      LogLine(_lastLogWhen, _lastLogLevel, _buf);
      _writeTo = _buf;
      _buf[0] = '\0';
   }
}

static Hashtable<LogCallbackRef, bool> _logCallbacks;
static DefaultConsoleLogger _dcl;
static DefaultFileLogger _dfl;

Mutex * _muscleLogLock = NULL;  // must be set up by a ThreadSetupSystem!

static status_t LockLog()
{
#ifdef MUSCLE_SINGLE_THREAD_ONLY
   return B_NO_ERROR;
#else
   if (_muscleLogLock == NULL)
   {
      printf("Please instantiate a CompleteSetupSystem object on the stack before doing any logging (at beginning of main() is preferred)");
      exit(10);
   }
   return _muscleLogLock->Lock();
#endif
}

static status_t UnlockLog()
{
#ifdef MUSCLE_SINGLE_THREAD_ONLY
   return B_NO_ERROR;
#else
   return _muscleLogLock->Unlock();
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
   (void) LockLog();
   int ret = _dfl._fileLogLevel;
   (void) UnlockLog();
   return ret;
}

int GetConsoleLogLevel()
{
   (void) LockLog();
   int ret = _dcl._consoleLogLevel;
   (void) UnlockLog();
   return ret;
}

int GetMaxLogLevel()
{
   (void) LockLog();
   int ret = (_dcl._consoleLogLevel > _dfl._fileLogLevel) ? _dcl._consoleLogLevel : _dfl._fileLogLevel;
   (void) UnlockLog();
   return ret;
}

status_t SetFileLogLevel(int loglevel)
{
   if (LockLog() == B_NO_ERROR)
   {
      _dfl._fileLogLevel = loglevel;
      LogTime(MUSCLE_LOG_INFO, "File logging level set to: %s\n", GetLogLevelName(_dfl._fileLogLevel));
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
      LogTime(MUSCLE_LOG_INFO, "Console logging level set to: %s\n", GetLogLevelName(_dcl._consoleLogLevel));
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

#define DO_LOGGING(when)                                                       \
{                                                                              \
   va_list argList;                                                            \
   va_start(argList, fmt);                                                     \
   _dfl.Log(when, ll, fmt, argList);                                           \
   va_end(argList);                                                            \
   va_start(argList, fmt);                                                     \
   _dcl.Log(when, ll, fmt, argList);                                           \
   va_end(argList);                                                            \
   HashtableIterator<LogCallbackRef, bool> iter = _logCallbacks.GetIterator(); \
   const LogCallbackRef * nextKey;                                             \
   while((nextKey = iter.GetNextKey()) != NULL)                                \
   {                                                                           \
      if (nextKey->GetItemPointer())                                           \
      {                                                                        \
         va_start(argList, fmt);                                               \
         nextKey->GetItemPointer()->Log(when, ll, fmt, argList);               \
         va_end(argList);                                                      \
      }                                                                        \
   }                                                                           \
}

void GetStandardLogLinePreamble(char * buf, int logLevel, time_t when)
{
   struct tm * temp = localtime(&when);
   sprintf(buf, "[%c %02i/%02i %02i:%02i:%02i] ", GetLogLevelName(logLevel)[0], temp->tm_mon+1, temp->tm_mday, temp->tm_hour, temp->tm_min, temp->tm_sec);
}

status_t LogTime(int ll, const char * fmt, ...)
{
   if (LockLog() == B_NO_ERROR)
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
      DO_LOGGING(n);
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t LogFlush()
{
   if (LockLog() == B_NO_ERROR)
   {
      HashtableIterator<LogCallbackRef, bool> iter = _logCallbacks.GetIterator();
      const LogCallbackRef * next;
      while((next = iter.GetNextKey()) != NULL) if (next->GetItemPointer()) next->GetItemPointer()->Flush();
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR; 
}

status_t Log(int ll, const char * fmt, ...)
{
   if (LockLog() == B_NO_ERROR)
   {
      time_t n = time(NULL);  // don't inline this, ya dummy
      DO_LOGGING(n);
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t PutLogCallback(LogCallbackRef cb)
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

status_t RemoveLogCallback(LogCallbackRef cb)
{
   status_t ret = B_ERROR;
   if (LockLog() == B_NO_ERROR)
   {
      ret = _logCallbacks.Remove(cb);
      UnlockLog();
   }
   return ret;
}

};  // end namespace muscle
