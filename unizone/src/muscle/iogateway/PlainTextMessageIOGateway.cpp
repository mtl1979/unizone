/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include "iogateway/PlainTextMessageIOGateway.h"

namespace muscle {

PlainTextMessageIOGateway ::
PlainTextMessageIOGateway() : _eolString("\r\n"), _prevCharWasCarriageReturn(false), _flushPartialIncomingLines(false)
{
   // empty
}

PlainTextMessageIOGateway ::
~PlainTextMessageIOGateway()
{
   // empty
}

int32
PlainTextMessageIOGateway ::
DoOutputImplementation(uint32 maxBytes)
{
   TCHECKPOINT;

   const Message * msg = _currentSendingMessage();
   if (msg == NULL)
   {
      // try to get the next message from our queue
      (void) GetOutgoingMessageQueue().RemoveHead(_currentSendingMessage);
      msg = _currentSendingMessage();
      _currentSendLineIndex = _currentSendOffset = -1;
   }

   if (msg)
   {
      if ((_currentSendOffset < 0)||(_currentSendOffset >= (int32)_currentSendText.Length()))
      {
         // Try to get the next line of text from our message
         if (msg->FindString(PR_NAME_TEXT_LINE, ++_currentSendLineIndex, _currentSendText) == B_NO_ERROR)
         {
            _currentSendOffset = 0;
            _currentSendText += _eolString;
         }
         else
         {
            _currentSendingMessage.Reset();  // no more text available?  Go to the next message then.
            return DoOutputImplementation(maxBytes);
         }
      }
      if ((msg)&&(_currentSendOffset >= 0)&&(_currentSendOffset < (int32)_currentSendText.Length()))
      {
         // Send as much as we can of the current text line
         const char * bytes = _currentSendText.Cstr() + _currentSendOffset;
         int32 bytesWritten = GetDataIO()()->Write(bytes, muscleMin(_currentSendText.Length()-_currentSendOffset, maxBytes));
              if (bytesWritten < 0) return -1;
         else if (bytesWritten > 0)
         {
            _currentSendOffset += bytesWritten;
            int32 subRet = DoOutputImplementation(maxBytes-bytesWritten);
            return (subRet >= 0) ? subRet+bytesWritten : -1;
         }
      }
   }
   return 0;
}

MessageRef
PlainTextMessageIOGateway ::
AddIncomingText(const MessageRef & inMsg, const char * s)
{
   MessageRef ret = inMsg;
   if (ret() == NULL) ret = GetMessageFromPool(PR_COMMAND_TEXT_STRINGS);
   if (ret())
   {
      if (_incomingText.HasChars())
      {
         (void) ret()->AddString(PR_NAME_TEXT_LINE, _incomingText.Append(s));
         _incomingText.Clear();
      }
      else (void) ret()->AddString(PR_NAME_TEXT_LINE, s);
   }
   return ret;
}

int32
PlainTextMessageIOGateway ::
DoInputImplementation(AbstractGatewayMessageReceiver & receiver, uint32 maxBytes)
{
   TCHECKPOINT;

   int32 ret = 0;
   const int tempBufSize = 2048;
   char buf[tempBufSize];
   int32 bytesRead = GetDataIO()()->Read(buf, muscleMin(maxBytes, (uint32)(sizeof(buf)-1)));
   if (bytesRead < 0)
   {
      FlushInput(receiver);
      return -1;
   }
   if (bytesRead > 0)
   {
      ret += bytesRead;
      buf[bytesRead] = '\0';

      MessageRef inMsg;  // demand-allocated
      int32 beginAt = 0;
      for (int32 i=0; i<bytesRead; i++)
      {
         char nextChar = buf[i];
         if ((nextChar == '\r')||(nextChar == '\n'))
         {
            buf[i] = '\0';  // terminate the string here
            if ((nextChar == '\r')||(_prevCharWasCarriageReturn == false)) inMsg = AddIncomingText(inMsg, &buf[beginAt]);
            beginAt = i+1;
         }
         _prevCharWasCarriageReturn = (nextChar == '\r');
      }
      if (beginAt < bytesRead)
      {
         if (_flushPartialIncomingLines) inMsg = AddIncomingText(inMsg, &buf[beginAt]);
                                    else _incomingText += &buf[beginAt];
      }
      if (inMsg()) receiver.CallMessageReceivedFromGateway(inMsg);
   }
   return ret;
}

void 
PlainTextMessageIOGateway :: FlushInput(AbstractGatewayMessageReceiver & receiver)
{
   if (_incomingText.HasChars())
   {
      MessageRef inMsg = GetMessageFromPool(PR_COMMAND_TEXT_STRINGS);
      if ((inMsg())&&(inMsg()->AddString(PR_NAME_TEXT_LINE, _incomingText) == B_NO_ERROR))
      {
         _incomingText.Clear();
         receiver.CallMessageReceivedFromGateway(inMsg);
      }
   }
}

bool
PlainTextMessageIOGateway ::
HasBytesToOutput() const
{
   return ((_currentSendingMessage() != NULL)||(GetOutgoingMessageQueue().HasItems()));
}

void
PlainTextMessageIOGateway ::
Reset()
{
   TCHECKPOINT;

   AbstractMessageIOGateway::Reset();
   _currentSendingMessage.Reset();
   _currentSendText.Clear();
   _prevCharWasCarriageReturn = false;
   _incomingText.Clear();
}

}; // end namespace muscle
