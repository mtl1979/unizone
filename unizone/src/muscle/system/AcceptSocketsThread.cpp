/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "dataio/TCPSocketDataIO.h"  // for the fd_set definition (BeOS/PPC)
#include "system/AcceptSocketsThread.h"
#include "util/NetworkUtilityFunctions.h"
#include "util/SocketHolder.h"

BEGIN_NAMESPACE(muscle);

AcceptSocketsThread :: AcceptSocketsThread() : _acceptSocket(-1)
{
   // empty
}

AcceptSocketsThread :: AcceptSocketsThread(uint16 port, uint32 optFrom) : _acceptSocket(-1)
{
   (void) SetPort(port, optFrom);
}

AcceptSocketsThread :: ~AcceptSocketsThread()
{
   CloseSocket(_acceptSocket);
}

status_t AcceptSocketsThread :: SetPort(uint16 port, uint32 optFrom)
{
   if (IsInternalThreadRunning() == false)
   {
      CloseSocket(_acceptSocket);
      _port = 0;
      _acceptSocket = CreateAcceptingSocket(port, 20, &port, optFrom);
      if (_acceptSocket >= 0)
      {
         _port = port;
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t AcceptSocketsThread :: StartInternalThread()
{
   if ((IsInternalThreadRunning() == false)&&(_acceptSocket >= 0))
   {
      _notifySocket = GetInternalThreadWakeupSocket();
      return (_notifySocket >= 0) ? Thread::StartInternalThread() : B_ERROR;
   } 
   return B_ERROR;
}

void AcceptSocketsThread :: InternalThreadEntry()
{
   fd_set readSet;
   bool keepGoing = true;
   while(keepGoing)
   {
      FD_ZERO(&readSet);
      FD_SET(_acceptSocket, &readSet);
      FD_SET(_notifySocket, &readSet);
      if (select(muscleMax(_acceptSocket, _notifySocket)+1, &readSet, NULL, NULL, NULL) < 0) break;
      if (FD_ISSET(_notifySocket, &readSet))
      {
         MessageRef msgRef;
         int32 numLeft;
         while((numLeft = WaitForNextMessageFromOwner(msgRef, 0)) >= 0)
         {
            if (MessageReceivedFromOwner(msgRef, numLeft) != B_NO_ERROR)
            { 
               keepGoing = false;
               break;
            }
         }
      }
      if (FD_ISSET(_acceptSocket, &readSet))
      {
         int newSocket = Accept(_acceptSocket);
         if (newSocket >= 0)
         {
            SocketHolderRef ref(newnothrow SocketHolder(newSocket), NULL);
            MessageRef msg(GetMessageFromPool(AST_EVENT_NEW_SOCKET_ACCEPTED));
            msg()->AddTag(AST_NAME_SOCKET, ref.GetGeneric());
            (void) SendMessageToOwner(msg);
         }
      }
   }
}

END_NAMESPACE(muscle);
