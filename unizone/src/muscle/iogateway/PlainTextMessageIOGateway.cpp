/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "iogateway/PlainTextMessageIOGateway.h"

namespace muscle {

PlainTextMessageIOGateway ::
PlainTextMessageIOGateway() : _eolString("\r\n"), _prevCharWasCarriageReturn(false)
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
DoOutput(uint32 maxBytes)
{
   const Message * msg = _currentSendingMessage.GetItemPointer();
   if (msg == NULL) 
   {
      // try to get the next message from our queue
      (void) GetOutgoingMessageQueue().RemoveHead(_currentSendingMessage);
      msg = _currentSendingMessage.GetItemPointer();
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
            return DoOutput(maxBytes);
         }
      }
      if ((msg)&&(_currentSendOffset >= 0)&&(_currentSendOffset < (int32)_currentSendText.Length()))
      {
         // Send as much as we can of the current text line
         const char * bytes = _currentSendText.Cstr() + _currentSendOffset;
         int32 bytesWritten = GetDataIO()->Write(bytes, muscleMin(_currentSendText.Length()-_currentSendOffset, maxBytes));
         if (bytesWritten < 0) 
         {
            FlushPendingInput();
            return -1;
         }
         else if (bytesWritten > 0)
         {
            _currentSendOffset += bytesWritten;
            int32 subRet = DoOutput(maxBytes-bytesWritten);
            return (subRet >= 0) ? subRet+bytesWritten : -1;
         }
      }
   }
   return 0;
}


void
PlainTextMessageIOGateway ::
FlushPendingInput()
{
   if (_incomingText.Length() > 0)
   {
      MessageRef inMsg = GetMessageFromPool(PR_COMMAND_TEXT_STRINGS);
      if (inMsg())
      {
         (void) inMsg()->AddString(PR_NAME_TEXT_LINE, _incomingText);
         _incomingText = "";
         (void) GetIncomingMessageQueue().AddTail(inMsg);
      }
   }
}

int32 
PlainTextMessageIOGateway ::
DoInput(uint32 maxBytes)
{
   int32 ret = 0;
   const int tempBufSize = 2048;
   char buf[tempBufSize];
   int32 bytesRead = GetDataIO()->Read(buf, muscleMin(maxBytes, (uint32)(sizeof(buf)-1)));
   if (bytesRead < 0) 
   {
      FlushPendingInput();
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
            if ((nextChar == '\r')||(_prevCharWasCarriageReturn == false)) 
            {
               if (inMsg() == NULL) inMsg = GetMessageFromPool(PR_COMMAND_TEXT_STRINGS);
               if (inMsg())
               {
                  if (_incomingText.Length() > 0) 
                  {
                     (void) inMsg()->AddString(PR_NAME_TEXT_LINE, _incomingText.Append(&buf[beginAt]));
                     _incomingText = "";
                  }
                  else (void) inMsg()->AddString(PR_NAME_TEXT_LINE, &buf[beginAt]);
               }
            }
            beginAt = i+1;
         }
         _prevCharWasCarriageReturn = (nextChar == '\r');
      }
      if (beginAt < bytesRead) _incomingText += &buf[beginAt];
      if (inMsg()) (void) GetIncomingMessageQueue().AddTail(inMsg);
   }
   return ret;
}

bool 
PlainTextMessageIOGateway ::
HasBytesToOutput() const
{
   return ((_currentSendingMessage.GetItemPointer() != NULL)||(GetOutgoingMessageQueue().GetNumItems() > 0));
}

};  // end namespace muscle
