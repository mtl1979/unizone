/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.TXT file for details. */

#ifndef MuscleLogCallback_h
#define MuscleLogCallback_h

#include "syslog/SysLog.h"
#include "util/RefCount.h"

BEGIN_NAMESPACE(muscle);

/** Callback object that can be added with PutLogCallback() 
 *  Whenever a log message is generated, all added LogCallback
 *  objects will have their Log() methods called.  All log callbacks
 *  are synchronized via a global lock, hence they will be thread safe.
 */
class LogCallback : public RefCountable
{
public:
   /** Default constructor */
   LogCallback() {/* empty */}

   /** Destructor, to keep C++ honest */
   virtual ~LogCallback() {/* empty */}

   /** Callback method.  Called whenever a message is logged with Log() or LogTime().
    *  @param when Timestamp of when the message was logged
    *  @param logLevel the MUSCLE_LOG_* value the message was logged with.
    *  @param format the printf-style format string
    *  @param argList the varargs argument list 
    */
   virtual void Log(time_t when, int logLevel, const char * format, va_list argList) = 0;

   /** Callback method.  When this method is called, the callback should flush any
     * held buffers out.  (i.e. call fflush() or whatever)
     */
   virtual void Flush() = 0;
};

typedef Ref<LogCallback> LogCallbackRef;

/** Specialization of LogCallback that parses the Log() calls
 *  into nicely formatted lines of text, then calls LogLine()
 *  to hand them to your code.  Easier than having lots of classes
 *  that all have to do this themselves.  Assumes that all log
 *  lines will be less than 2048 characters long.
 */
class LogLineCallback : public LogCallback
{
public:
   /** Constructor */
   LogLineCallback();

   /** Destructor */
   virtual ~LogLineCallback();

   /** Implemented to call LogLine() when appropriate */
   virtual void Log(time_t when, int logLevel, const char * format, va_list argList);

   /** Implemented to call LogLine() when appropriate */
   virtual void Flush();
   
protected:
   /** This will be called whenever a fully-formed line of log text is ready.
     * implement it to do whatever you like with the text.
     * @param when Timestamp of when the message was logged
     * @param logLevel the MUSCLE_LOG_* value the message was logged with
     * @param line the text of the log message
     */ 
   virtual void LogLine(time_t when, int logLevel, const char * line) = 0;

private:
   time_t _lastLogWhen; // stored for use by Flush()
   int _lastLogLevel;   // stored for use by Flush()
   char * _writeTo;     // points to the next spot in (_buf) to sprintf() into
   char _buf[2048];     // where we assemble our text
};

/** Add a custom LogCallback object to the global log callbacks set.
 *  @param cbRef Reference to a LogCallback object. 
 */
status_t PutLogCallback(LogCallbackRef cbRef);

/** Removes the given callback from our list.  
 *  @param cbRef Reference of the LogCallback to remove from the callback list.
 *  @returns B_NO_ERROR on success, or B_ERROR if the given callback wasn't found, or the lock couldn't be locked.
 */
status_t RemoveLogCallback(LogCallbackRef cbRef);

/** Removes all log callbacks from the callback set
 *  @returns B_NO_ERROR on success, or B_ERROR if the log lock couldn't be locked for some reason.
 */ 
status_t ClearLogCallbacks();

END_NAMESPACE(muscle);

#endif
