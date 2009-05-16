/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.TXT file for details. */

#ifndef MuscleLogCallback_h
#define MuscleLogCallback_h

#include "syslog/SysLog.h"
#include "util/RefCount.h"

namespace muscle {

/** This class encapsulates the information that is sent to the Log() and LogLine() callback methods of the LogCallback and LogLineCallback classes.  By putting all the information into a class object, we only have to push one parameter onto the stack with each call instead of many. */
class LogCallbackArgs
{
public:
   /** Default Constructor */
   LogCallbackArgs() : _when(0), _logLevel(MUSCLE_LOG_INFO), _sourceFile(""), _sourceFunction(""), _sourceLine(0), _text("") {/* empty */}

   /** Constructor 
     * @param when Timestamp for this log message, in (seconds past 1970) format.
     * @param logLevel The MUSCLE_LOG_* severity level of this log message
     * @param sourceFile The name of the source code file that contains the LogLine() call that generated this callback.
     *                   Note that this parameter will only be valid if -DMUSCLE_INCLUDE_SOURCE_CODE_LOCATION_IN_LOGTIME
     *                   was defined when muscle was compiled.  Otherwise this value may be passed as "".
     * @param sourceFunction The name of the source code function that contains the LogLine() call that generated this callback.
     *                   Note that this parameter will only be valid if -DMUSCLE_INCLUDE_SOURCE_CODE_LOCATION_IN_LOGTIME
     *                   was defined when muscle was compiled.  Otherwise this value may be passed as "".
     * @param sourceLine The line number of the LogLine() call that generated this callback.
     *                   Note that this parameter will only be valid if -DMUSCLE_INCLUDE_SOURCE_CODE_LOCATION_IN_LOGTIME
     *                   was defined when muscle was compiled.  Otherwise this value may be passed as -1.
     * @param text The format text if this object is being passed in a Log() callback.  If this object is being passed
     *             in a LogLine() callback, this will be the verbatim text of the line.
     * @param argList In a Log() callback, this is a pointer to a va_list object that can be used to expand (text).
     *                In a LogLine() callback, this value will be NULL.
     */
   LogCallbackArgs(const time_t & when, int logLevel, const char * sourceFile, const char * sourceFunction, int sourceLine, const char * text, va_list * argList) : _when(when), _logLevel(logLevel), _sourceFile(sourceFile), _sourceFunction(sourceFunction), _sourceLine(sourceLine), _text(text), _argList(argList) {/* empty */}

   /** Returns the timestamp indicating when this message was generated, in (seconds since 1970) format. */
   const time_t & GetWhen() const {return _when;}

   /** Returns the MUSCLE_LOG_* severity level of this log message. */
   int GetLogLevel() const {return _logLevel;}

   /** Returns the name of the source code file that contains the LogLine() call that generated this callback, or "" if it's not available.  */
   const char * GetSourceFile() const {return _sourceFile;}

  /** Returns the the name of the source code function that contains the LogLine() call that generated this callback,
    * or "" if it's not available.
    */
   const char * GetSourceFunction() const {return _sourceFunction;}

   /** Returns the line number of the LogLine() call that generated this callback, or -1 if it's not available. */
   int GetSourceLineNumber() const {return _sourceLine;}

   /** Returns the format text if this object is being passed in a Log() callback.  If this object is being passed
     * in a LogLine() callback, this will be the verbatim text of the line.
     */
   const char * GetText() const {return _text;}

   /** Returns the Log() callback, this is a pointer to a va_list object that can be used to expand (text).
     * In a LogLine() callback, this value will be NULL..
     */
   va_list * GetArgList() const {return _argList;}
 
   /** Set the timestamp associated with the Log message
     * @param when A time value (seconds since 1970)
     */
   void SetWhen(const time_t & when) {_when = when;}

   /** Set the MUSCLE_LOG_* severity level of this Log message 
     * param ll A MUSCLE_LOG_* value
     */
   void SetLogLevel(int ll) {_logLevel = ll;}

   /** Set the source file name of this Log Message.
     * @param sf A source file name string.  Note that this string will not be copied and therefore must remain valid!
     */
   void SetSourceFile(const char * sf) {_sourceFile = sf;}

   /** Set the source function name of this Log Message.
     * @param sf A source function name string.  Note that this string will not be copied and therefore must remain valid!
     */
   void SetSourceFunction(const char * sf) {_sourceFunction = sf;}

   /** Set the source line number of this Log Message.
     * @param sourceLine A source line number, or -1 to indicate invalid.
     */
   void SetSourceLineNumber(int sourceLine) {_sourceLine = sourceLine;}

   /** Set the text string of this Log Message.
     * @param txt A text string.  Note that this string will not be copied and therefore must remain valid!
     */
   void SetText(const char * txt) {_text = txt;}

   /** Sets the pointer to a va_list that can be used to expand our text.
     * @param va Pointer to a va_list, or NULL if there is none.  Noe that this object is not copied and therefore must remain valid!
     */
   void SetArgList(va_list * va) {_argList = va;}

private:
   time_t _when;
   int _logLevel;
   const char * _sourceFile;
   const char * _sourceFunction;
   int _sourceLine;
   const char * _text;
   va_list * _argList; };

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
    *  @param a LogCallbackArgs object containing all the arguments to this method.
    */
   virtual void Log(const LogCallbackArgs & a) = 0;

   /** Callback method.  When this method is called, the callback should flush any
     * held buffers out.  (i.e. call fflush() or whatever)
     */
   virtual void Flush() = 0;
};
DECLARE_REFTYPES(LogCallback);

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
   virtual void Log(const LogCallbackArgs & a);

   /** Implemented to call LogLine() when appropriate */
   virtual void Flush();
   
protected:
   /** This will be called whenever a fully-formed line of log text is ready.
     * implement it to do whatever you like with the text.
     * @param a The log callback arguments.  The (format) string
     *          in this case will always be a literal string that can be printed verbatim.
     */ 
   virtual void LogLine(const LogCallbackArgs & a) = 0;

private:
   LogCallbackArgs _lastLog; // stored for use by Flush()
   char * _writeTo;     // points to the next spot in (_buf) to sprintf() into
   char _buf[2048];     // where we assemble our text
};

/** Add a custom LogCallback object to the global log callbacks set.
 *  @param cbRef Reference to a LogCallback object. 
 */
status_t PutLogCallback(const LogCallbackRef & cbRef);

/** Removes the given callback from our list.  
 *  @param cbRef Reference of the LogCallback to remove from the callback list.
 *  @returns B_NO_ERROR on success, or B_ERROR if the given callback wasn't found, or the lock couldn't be locked.
 */
status_t RemoveLogCallback(const LogCallbackRef & cbRef);

/** Removes all log callbacks from the callback set
 *  @returns B_NO_ERROR on success, or B_ERROR if the log lock couldn't be locked for some reason.
 */ 
status_t ClearLogCallbacks();

}; // end namespace muscle

#endif
