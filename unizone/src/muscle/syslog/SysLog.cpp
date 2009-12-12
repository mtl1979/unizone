/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include <stdio.h>
#include <stdarg.h>
#include "dataio/FileDataIO.h"
#include "regex/StringMatcher.h"
#include "syslog/LogCallback.h"
#include "system/SetupSystem.h"
#include "system/SystemInfo.h"  // for GetFilePathSeparator()
#include "util/Directory.h"
#include "util/FilePathInfo.h"
#include "util/Hashtable.h"
#include "util/NestCount.h"
#include "util/String.h"
#include "util/StringTokenizer.h"

#if !defined(MUSCLE_INLINE_LOGGING) && defined(MUSCLE_ENABLE_ZLIB_ENCODING)
# include "zlib.h"
#endif

#if defined(__APPLE__)
# include "AvailabilityMacros.h"  // so we can find out if this version of MacOS/X is new enough to include backtrace() and friends
#endif

#if defined(__linux__) || (defined(MAC_OS_X_VERSION_10_5) && defined(MAC_OS_X_VERSION_MAX_ALLOWED) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5))
# include <execinfo.h>
# define MUSCLE_USE_BACKTRACE 1
#endif

namespace muscle {

#ifdef THIS_FUNCTION_IS_NOT_ACTUALLY_USED_I_JUST_KEEP_IT_HERE_SO_I_CAN_QUICKLY_COPY_AND_PASTE_IT_INTO_THIRD_PARTY_CODE_WHEN_NECESSARY_SAYS_JAF
# include <execinfo.h>
void PrintStackTrace()
{
   FILE * optFile = stdout;
   void *array[256];
   size_t size = backtrace(array, 256);
   char ** strings = backtrace_symbols(array, 256);
   if (strings)
   {
      fprintf(optFile, "--Stack trace follows (%zd frames):\n", size);
      for (size_t i = 0; i < size; i++) fprintf(optFile, "  %s\n", strings[i]);
      fprintf(optFile, "--End Stack trace\n");
      free(strings);
   }
   else fprintf(optFile, "PrintStackTrace:  Error, could not generate stack trace!\n");
}
#endif

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
   else fprintf(optFile, "PrintStackTrace:  Error, could not generate stack trace!\n");
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

static NestCount _inLogPreamble;

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
   DefaultFileLogger() : _fileLogLevel(MUSCLE_LOG_NONE), _maxLogFileSize(MUSCLE_NO_LIMIT), _maxNumLogFiles(MUSCLE_NO_LIMIT), _compressionEnabled(false), _logFileOpenAttemptFailed(false)
   {
       // empty
   }

   virtual ~DefaultFileLogger()
   {
      CloseLogFile();
   }

   virtual void Log(const LogCallbackArgs & a)
   {
      if ((a.GetLogLevel() <= _fileLogLevel)&&(EnsureLogFileCreated(a) == B_NO_ERROR))
      {                                         
         vfprintf(_logFile.GetFile(), a.GetText(), *a.GetArgList());
         _logFile.FlushOutput();
         if ((_maxLogFileSize != MUSCLE_NO_LIMIT)&&(_inLogPreamble.IsInBatch() == false))  // wait until we're outside the preamble to avoid breaking up lines too much
         {
            int64 curFileSize = _logFile.GetPosition();
            if ((curFileSize < 0)||(curFileSize >= (int64)_maxLogFileSize))
            {
               uint32 tempStoreSize = _maxLogFileSize;
               _maxLogFileSize = MUSCLE_NO_LIMIT;  // otherwise we'd recurse indefinitely here!
               CloseLogFile();
               _maxLogFileSize = tempStoreSize;

               (void) EnsureLogFileCreated(a);  // force the opening of the new log file right now, so that the open message show up in the right order
            }
         }
      }
   }

   virtual void Flush()
   {
      _logFile.FlushOutput();
   }

   uint32 AddPreExistingLogFiles(const String & filePattern)
   {
      String dirPart, filePart;
      int32 lastSlash = filePattern.LastIndexOf(GetFilePathSeparator());   
      if (lastSlash >= 0)
      {
         dirPart = filePattern.Substring(0, lastSlash);
         filePart = filePattern.Substring(lastSlash+1);
      }
      else 
      {
         dirPart  = ".";
         filePart = filePattern;
      }

      Hashtable<String, uint64> pathToTime;
      if (filePart.HasChars())
      {
         StringMatcher sm(filePart);

         Directory d(dirPart());
         if (d.IsValid())
         {
            const char * nextName; 
            while((nextName = d.GetCurrentFileName()) != NULL)
            {
               String fn = nextName;
               if (sm.Match(fn))
               {
                  String fullPath = dirPart+GetFilePathSeparator()+fn;
                  FilePathInfo fpi(fullPath());
                  if ((fpi.Exists())&&(fpi.IsRegularFile())) pathToTime.Put(fullPath, fpi.GetCreationTime());
               }
               d++;
            }
         }

         // Now we sort by creation time...
         pathToTime.SortByValue();

         // And add the results to our _oldFileNames queue.  That way when the log file is opened, the oldest files will be deleted (if appropriate)
         for (HashtableIterator<String, uint64> iter(pathToTime); iter.HasMoreKeys(); iter++) (void) _oldLogFileNames.AddTail(iter.GetKey());
      }
      return pathToTime.GetNumItems();
   }

   int _fileLogLevel;
   String _prototypeLogFileName;
   uint32 _maxLogFileSize;
   uint32 _maxNumLogFiles;
   bool _compressionEnabled;

private:
   status_t EnsureLogFileCreated(const LogCallbackArgs & a)
   {
      if ((_logFile.GetFile() == NULL)&&(_logFileOpenAttemptFailed == false))
      {
         String logFileName = _prototypeLogFileName;
         if (logFileName.IsEmpty()) logFileName = "%f.log";

         HumanReadableTimeValues hrtv; (void) GetHumanReadableTimeValues(((uint64)a.GetWhen())*1000000, hrtv);
         logFileName = hrtv.ExpandTokens(logFileName);

         _logFile.SetFile(fopen(logFileName(), "w"));
         if (_logFile.GetFile() != NULL) 
         {
            _activeLogFileName = logFileName;
            LogTime(MUSCLE_LOG_DEBUG, "Created Log file [%s]\n", _activeLogFileName());

            while(_oldLogFileNames.GetNumItems() >= _maxNumLogFiles)
            {
               const char * c = _oldLogFileNames.Head()();
                    if (remove(c) == 0)  LogTime(MUSCLE_LOG_DEBUG, "Deleted old Log file [%s]\n", c);
               else if (errno != ENOENT) LogTime(MUSCLE_LOG_ERROR, "Error deleting old Log file [%s]\n", c);
               _oldLogFileNames.RemoveHead();
            }
         }
         else 
         {
            _activeLogFileName.Clear();
            _logFileOpenAttemptFailed = true;  // avoid an indefinite number of log-failed messages
            LogTime(MUSCLE_LOG_ERROR, "Failed to open Log file [%s], logging to file is now disabled.\n", logFileName());
         }
      }
      return (_logFile.GetFile() != NULL) ? B_NO_ERROR : B_ERROR;
   }

   void CloseLogFile()
   {
      if (_logFile.GetFile())
      {
         LogTime(MUSCLE_LOG_DEBUG, "Closing Log file [%s]\n", _activeLogFileName());
         String oldFileName = _activeLogFileName;  // default file to delete later, will be changed if/when we've made the .gz file
         _activeLogFileName.Clear();   // do this first to avoid reentrancy issues
         _logFile.Shutdown();

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
         if (_compressionEnabled)
         {
            FileDataIO inIO(fopen(oldFileName(), "rb"));
            if (inIO.GetFile() != NULL)
            {
               String gzName = oldFileName + ".gz";
               gzFile gzOut = gzopen(gzName(), "wb9"); // 9 for maximum compression
               if (gzOut != Z_NULL)
               {
                  bool ok = true;
                  while(1)
                  {
                     char buf[128*1024];
                     int32 bytesRead = inIO.Read(buf, sizeof(buf));
                     if (bytesRead < 0) break;  // EOF

                     int bytesWritten = gzwrite(gzOut, buf, bytesRead);
                     if (bytesWritten <= 0)
                     {
                        ok = false;  // write error, oh dear
                        break;
                     }
                  } 
                  gzclose(gzOut);

                  if (ok)
                  {
                     inIO.Shutdown();
                     if (remove(oldFileName()) != 0) LogTime(MUSCLE_LOG_ERROR, "Error deleting log file [%s] after compressing it to [%s]!\n", oldFileName(), gzName());
                     oldFileName = gzName;
                  }
                  else 
                  {
                     if (remove(gzName()) != 0) LogTime(MUSCLE_LOG_ERROR, "Error deleting gzip'd log file [%s] after compression failed!\n", gzName());
                  }
               }
               else LogTime(MUSCLE_LOG_ERROR, "Could not open compressed Log file [%s]!\n", gzName());
            }
            else LogTime(MUSCLE_LOG_ERROR, "Could not reopen Log file [%s] to compress it!\n", oldFileName());
         }
#endif
         if (_maxNumLogFiles != MUSCLE_NO_LIMIT) (void) _oldLogFileNames.AddTail(oldFileName);  // so we can delete it later
      }
   }

   String _activeLogFileName;
   FileDataIO _logFile;
   bool _logFileOpenAttemptFailed;
   Queue<String> _oldLogFileNames;
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
   return _dfl._prototypeLogFileName;
}

uint32 GetFileLogMaximumSize()
{
   return _dfl._maxLogFileSize;
}

uint32 GetMaxNumLogFiles()
{
   return _dfl._maxNumLogFiles;
}

bool GetFileLogCompressionEnabled()
{
   return _dfl._compressionEnabled;
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
      _dfl._prototypeLogFileName = logName;
      LogTime(MUSCLE_LOG_DEBUG, "File log name set to: %s\n", logName());
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetOldLogFilesPattern(const String & pattern)
{
   if (LockLog() == B_NO_ERROR)
   {
      uint32 numAdded = _dfl.AddPreExistingLogFiles(pattern);
      LogTime(MUSCLE_LOG_DEBUG, "Old Log Files pattern set to: [%s] ("UINT32_FORMAT_SPEC" files matched)\n", pattern(), numAdded);
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetFileLogMaximumSize(uint32 maxSizeBytes)
{
   if (LockLog() == B_NO_ERROR)
   {
      _dfl._maxLogFileSize = maxSizeBytes;
      if (maxSizeBytes == MUSCLE_NO_LIMIT) LogTime(MUSCLE_LOG_DEBUG, "File log maximum size set to: (unlimited).\n");
                                      else LogTime(MUSCLE_LOG_DEBUG, "File log maximum size set to: "UINT32_FORMAT_SPEC" bytes.\n", maxSizeBytes);
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetMaxNumLogFiles(uint32 maxNumLogFiles)
{
   if (LockLog() == B_NO_ERROR)
   {
      _dfl._maxNumLogFiles = maxNumLogFiles;
      if (maxNumLogFiles == MUSCLE_NO_LIMIT) LogTime(MUSCLE_LOG_DEBUG, "Maximum number of log files set to: (unlimited).\n");
                                        else LogTime(MUSCLE_LOG_DEBUG, "Maximum number of log files to: "UINT32_FORMAT_SPEC"\n", maxNumLogFiles);
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t SetFileLogCompressionEnabled(bool enable)
{
#ifdef MUSCLE_ENABLE_ZLIB_ENCODING
   if (LockLog() == B_NO_ERROR)
   {
      _dfl._compressionEnabled = enable;
      LogTime(MUSCLE_LOG_DEBUG, "File log compression %s.\n", enable?"enabled":"disabled");
      (void) UnlockLog();
      return B_NO_ERROR;
   }
   else return B_ERROR;
#else
   if (enable) 
   {
      LogTime(MUSCLE_LOG_CRITICALERROR, "Can not enable log file compression, MUSCLE was compiled without MUSCLE_ENABLE_ZLIB_ENCODING specified!\n");
      return B_ERROR;
   }
   else return B_NO_ERROR;
#endif
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

// Our 27-character alphabet of usable symbols
#define NUM_CHARS_IN_KEY_ALPHABET (sizeof(_keyAlphabet)-1)  // -1 because the NUL terminator doesn't count
static const char _keyAlphabet[] = "23456789BCDFGHJKMNPRSTVWXYZ";  // FogBugz #5808: vowels and some numerals omitted to avoid ambiguity and inadvertent swearing
static const uint32 _keySpaceSize = NUM_CHARS_IN_KEY_ALPHABET * NUM_CHARS_IN_KEY_ALPHABET * NUM_CHARS_IN_KEY_ALPHABET * NUM_CHARS_IN_KEY_ALPHABET;

uint32 GenerateSourceCodeLocationKey(const char * fileName, uint32 lineNumber)
{
#ifdef WIN32
   const char * lastSlash = strrchr(fileName, '\\');
#else
   const char * lastSlash = strrchr(fileName, '/');
#endif
   if (lastSlash) fileName = lastSlash+1;

   return ((CalculateHashCode(fileName,strlen(fileName))+lineNumber)%(_keySpaceSize-1))+1;  // note that 0 is not considered a valid key value!
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
      base *= NUM_CHARS_IN_KEY_ALPHABET;
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

#define DO_LOGGING_CALLBACK(cb) \
{                               \
   va_list argList;             \
   va_start(argList, fmt);      \
   cb.Log(LogCallbackArgs(when, ll, sourceFile, sourceFunction, sourceLine, fmt, &argList)); \
   va_end(argList);             \
}

#define DO_LOGGING_CALLBACKS for (HashtableIterator<LogCallbackRef, bool> iter(_logCallbacks); iter.HasMoreKeys(); iter++) if (iter.GetKey()()) DO_LOGGING_CALLBACK((*iter.GetKey()()));

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
   {
      // First, log the preamble
      time_t when = time(NULL);
      char buf[128];

      // First, send to the log file
      {
         NestCountGuard g(_inLogPreamble);  // must be inside the braces!
         va_list dummyList;
         va_start(dummyList, fmt);  // not used
         LogCallbackArgs lca(when, ll, sourceFile, sourceFunction, sourceLine, buf, &dummyList);
         GetStandardLogLinePreamble(buf, lca);
         lca.SetText(buf);
         _dfl.Log(lca);
         va_end(dummyList);
      }
      DO_LOGGING_CALLBACK(_dfl);
     
      // Then, send to the display
      {
         NestCountGuard g(_inLogPreamble);  // must be inside the braces!
         va_list dummyList;
         va_start(dummyList, fmt);  // not used
         _dcl.Log(LogCallbackArgs(when, ll, sourceFile, sourceFunction, sourceLine, buf, &dummyList));
         va_end(dummyList);
      }
      DO_LOGGING_CALLBACK(_dcl);  // must be outside of the braces!
   
      // Then log the actual message as supplied by the user
      if (lockRet == B_NO_ERROR) DO_LOGGING_CALLBACKS;
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
   {
      time_t when = time(NULL);  // don't inline this, ya dummy
      DO_LOGGING_CALLBACK(_dfl);
      DO_LOGGING_CALLBACK(_dcl);
      if (lockRet == B_NO_ERROR) DO_LOGGING_CALLBACKS;
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

status_t GetHumanReadableTimeValues(uint64 timeUS, HumanReadableTimeValues & v, uint32 timeType)
{
   TCHECKPOINT;

   if (timeUS == MUSCLE_TIME_NEVER) return B_ERROR;

   int microsLeft = (int)(timeUS%1000000);

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
      v = HumanReadableTimeValues(st.wYear, st.wMonth-1, st.wDay-1, st.wDayOfWeek, st.wHour, st.wMinute, st.wSecond, microsLeft);
      return B_NO_ERROR;
   }
#else
   time_t timeS = (time_t) (timeUS/1000000);  // timeS is seconds since 1970
   struct tm * ts = (timeType == MUSCLE_TIMEZONE_UTC) ? localtime(&timeS) : gmtime(&timeS);  // only convert if it isn't already local
   if (ts) 
   {
      v = HumanReadableTimeValues(ts->tm_year+1900, ts->tm_mon, ts->tm_mday-1, ts->tm_wday, ts->tm_hour, ts->tm_min, ts->tm_sec, microsLeft);
      return B_NO_ERROR;
   }
#endif

   return B_ERROR;
}

String HumanReadableTimeValues :: ExpandTokens(const String & origString) const
{
   if (origString.IndexOf('%') < 0) return origString;

   String newString = origString;
   (void) newString.Replace("%%", "%");  // do this first!
   (void) newString.Replace("%T", "%Q %D %Y %h:%m:%s");
   (void) newString.Replace("%t", "%Y/%M/%D %h:%m:%s");
   (void) newString.Replace("%f", "%Y-%M-%D_%hh%mm%s");

   static const char * _daysOfWeek[]   = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
   static const char * _monthsOfYear[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

   (void) newString.Replace("%Y", String("%1").Arg((int32)GetYear()));
   (void) newString.Replace("%M", String("%1").Arg((int32)(GetMonth()+1),      "%02i"));
   (void) newString.Replace("%Q", String("%1").Arg(_monthsOfYear[muscleClamp(GetMonth(), 0, (int)(ARRAYITEMS(_monthsOfYear)-1))]));
   (void) newString.Replace("%D", String("%1").Arg((int32)(GetDayOfMonth()+1), "%02i"));
   (void) newString.Replace("%d", String("%1").Arg((int32)(GetDayOfMonth()+1), "%02i"));
   (void) newString.Replace("%W", String("%1").Arg((int32)(GetDayOfWeek()+1),  "%02i"));
   (void) newString.Replace("%w", String("%1").Arg((int32)(GetDayOfWeek()+1),  "%02i"));
   (void) newString.Replace("%q", String("%1").Arg(_daysOfWeek[muscleClamp(GetDayOfWeek(), 0, (int)(ARRAYITEMS(_daysOfWeek)-1))]));
   (void) newString.Replace("%h", String("%1").Arg((int32)GetHour(),           "%02i"));
   (void) newString.Replace("%m", String("%1").Arg((int32)GetMinute(),         "%02i"));
   (void) newString.Replace("%s", String("%1").Arg((int32)GetSecond(),         "%02i"));
   (void) newString.Replace("%x", String("%1").Arg((int32)GetMicrosecond(),    "%06i"));

   uint32 r1 = rand();
   uint32 r2 = rand();
   char buf[64]; sprintf(buf, UINT64_FORMAT_SPEC, (((uint64)r1)<<32)|((uint64)r2));
   (void) newString.Replace("%r", buf);

   return newString;
}

String GetHumanReadableTimeString(uint64 timeUS, uint32 timeType)
{
   TCHECKPOINT;

   if (timeUS == MUSCLE_TIME_NEVER) return ("(never)");
   else
   {
      HumanReadableTimeValues v;
      if (GetHumanReadableTimeValues(timeUS, v, timeType) == B_NO_ERROR)
      {
         char buf[256];
         sprintf(buf, "%02i/%02i/%02i %02i:%02i:%02i", v.GetYear(), v.GetMonth()+1, v.GetDayOfMonth()+1, v.GetHour(), v.GetMinute(), v.GetSecond());
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
# if defined(__BORLANDC__) || defined(MUSCLE_USING_OLD_MICROSOFT_COMPILER)
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

enum {
   TIME_UNIT_MICROSECOND,
   TIME_UNIT_MILLISECOND,
   TIME_UNIT_SECOND,
   TIME_UNIT_MINUTE,
   TIME_UNIT_HOUR,
   TIME_UNIT_DAY,
   TIME_UNIT_WEEK,
   TIME_UNIT_MONTH,
   TIME_UNIT_YEAR,
   NUM_TIME_UNITS
};

static const uint64 MICROS_PER_SECOND = 1000000;
static const uint64 MICROS_PER_DAY    = 24*60*60*MICROS_PER_SECOND;

static const uint64 _timeUnits[NUM_TIME_UNITS] = {
   1,                       // micros -> micros
   1000,                    // millis -> micros
   MICROS_PER_SECOND,       // secs   -> micros
   60*MICROS_PER_SECOND,    // mins   -> micros
   60*60*MICROS_PER_SECOND, // hours  -> micros
   MICROS_PER_DAY,          // days   -> micros
   7*MICROS_PER_DAY,        // weeks  -> micros
   30*MICROS_PER_DAY,       // months -> micros (well, sort of -- we assume a month is always 30  days, which isn't really true)
   365*MICROS_PER_DAY       // years  -> micros (well, sort of -- we assume a years is always 365 days, which isn't really true)
};
static const char * _timeUnitNames[NUM_TIME_UNITS] = {
   "microsecond",
   "millisecond",
   "second",
   "minute",
   "hour",
   "day",
   "week",
   "month",
   "year",
};

uint64 ParseHumanReadableTimeIntervalString(const String & s)
{
   if ((s.EqualsIgnoreCase("forever"))||(s.EqualsIgnoreCase("never"))||(s.StartsWithIgnoreCase("inf"))) return MUSCLE_TIME_NEVER;

   /** Find first digit */
   const char * d = s();
   while((*d)&&(isdigit(*d) == false)) d++;
   if (*d == '\0') return 0;

   /** Find first letter */
   const char * l = s();
   while((*l)&&(isalpha(*l) == false)) l++;
   if (*l == '\0') l = "s";  // default to seconds

   const uint64 _oneSecond = 1000000;
   uint64 multiplier = _oneSecond;   // default units is seconds
   String tmp(l); tmp = tmp.ToLowerCase();
        if ((tmp.StartsWith("us"))||(tmp.StartsWith("micro"))) multiplier = _timeUnits[TIME_UNIT_MICROSECOND];
   else if ((tmp.StartsWith("ms"))||(tmp.StartsWith("milli"))) multiplier = _timeUnits[TIME_UNIT_MILLISECOND];
   else if (tmp.StartsWith("mo"))                              multiplier = _timeUnits[TIME_UNIT_MONTH];
   else if (tmp.StartsWith("s"))                               multiplier = _timeUnits[TIME_UNIT_SECOND];
   else if (tmp.StartsWith("m"))                               multiplier = _timeUnits[TIME_UNIT_MINUTE];
   else if (tmp.StartsWith("h"))                               multiplier = _timeUnits[TIME_UNIT_HOUR];
   else if (tmp.StartsWith("d"))                               multiplier = _timeUnits[TIME_UNIT_DAY];
   else if (tmp.StartsWith("w"))                               multiplier = _timeUnits[TIME_UNIT_WEEK];
   else if (tmp.StartsWith("y"))                               multiplier = _timeUnits[TIME_UNIT_YEAR];

   const char * afterLetters = l;
   while((*afterLetters)&&((*afterLetters==',')||(isalpha(*afterLetters)||(isspace(*afterLetters))))) afterLetters++;

   uint64 ret = Atoull(d)*multiplier;
   if (*afterLetters) ret += ParseHumanReadableTimeIntervalString(afterLetters);
   return ret;
}

String GetHumanReadableTimeIntervalString(uint64 intervalUS, uint32 maxClauses, uint64 minPrecision, bool * optRetIsAccurate)
{
   // Find the largest unit that is still smaller than (micros)
   uint32 whichUnit = TIME_UNIT_MICROSECOND;
   for (uint32 i=0; i<NUM_TIME_UNITS; i++) 
   {
      if (_timeUnits[whichUnit] < intervalUS) whichUnit++;
                                         else break;
   }
   if ((whichUnit >= NUM_TIME_UNITS)||((whichUnit > 0)&&(_timeUnits[whichUnit] > intervalUS))) whichUnit--;

   uint64 numUnits = intervalUS/_timeUnits[whichUnit];
   char buf[256]; sprintf(buf, UINT64_FORMAT_SPEC" %s%s", numUnits, _timeUnitNames[whichUnit], (numUnits==1)?"":"s");
   String ret = buf;
  
   uint64 leftover = intervalUS%_timeUnits[whichUnit];
   if (leftover > 0)
   {
      if ((leftover > minPrecision)&&(maxClauses > 1)) ret += GetHumanReadableTimeIntervalString(leftover, maxClauses-1, minPrecision, optRetIsAccurate).Prepend(", ");
                                                  else if (optRetIsAccurate) *optRetIsAccurate = false;
   }
   else if (optRetIsAccurate) *optRetIsAccurate = true;

   return ret;
}

}; // end namespace muscle
