/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MusclePlainTextMessageIOGateway_h
#define MusclePlainTextMessageIOGateway_h

#include "iogateway/AbstractMessageIOGateway.h"

BEGIN_NAMESPACE(muscle);

/** This is the name of the string field used to hold text lines. */
#define PR_NAME_TEXT_LINE "tl"

/** The 'what' code that will be found in incoming Messages. */
#define PR_COMMAND_TEXT_STRINGS 1886681204 // 'ptxt'

/** 
 * This gateway translates lines of text (separated by \r, \n, or \r\n) into
 * Messages.  It can be used for "telnet-style" net interactions.
 * Incoming and outgoing messages may have one or more strings in their PR_NAME_TEXT_LINE field.
 * Each of these strings represents a line of text (separator chars not included)
 */
class PlainTextMessageIOGateway : public AbstractMessageIOGateway
{
public:
   /** Default constructor */
   PlainTextMessageIOGateway();

   /** Destructor */
   virtual ~PlainTextMessageIOGateway();

   virtual bool HasBytesToOutput() const;
   virtual void Reset();

   /** Set the end-of-line string to be attached to outgoing text lines.
    * Default value is "\r\n".
    */
   virtual void SetOutgoingEndOfLineString(const char * str) {_eolString = str;}

protected:
   virtual int32 DoOutputImplementation(uint32 maxBytes = MUSCLE_NO_LIMIT);
   virtual int32 DoInputImplementation(AbstractGatewayMessageReceiver & receiver, uint32 maxBytes = MUSCLE_NO_LIMIT);

private:
   MessageRef _currentSendingMessage;
   String _currentSendText;
   String _eolString;
   int32 _currentSendLineIndex;
   int32 _currentSendOffset;
   bool _prevCharWasCarriageReturn;
   String _incomingText;
};

END_NAMESPACE(muscle);

#endif
