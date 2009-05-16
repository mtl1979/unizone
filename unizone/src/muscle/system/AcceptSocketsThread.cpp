/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#include "dataio/TCPSocketDataIO.h"  // for the fd_set definition (BeOS/PPC)
#include "system/AcceptSocketsThread.h"
#include "util/NetworkUtilityFunctions.h"

namespace muscle {

AcceptSocketsThread :: AcceptSocketsThread()
{
   // empty
}

AcceptSocketsThread :: AcceptSocketsThread(uint16 port, const ip_address & optInterfaceIP)
{
   (void) SetPort(port, optInterfaceIP);
}

AcceptSocketsThread :: ~AcceptSocketsThread()
{
   // empty
}

status_t AcceptSocketsThread :: SetPort(uint16 port, const ip_address & optInterfaceIP)
{
   if (IsInternalThreadRunning() == false)
   {
      _port = 0;
      _acceptSocket = CreateAcceptingSocket(port, 20, &port, optInterfaceIP);
      if (_acceptSocket())
      {
         _port = port;
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

status_t AcceptSocketsThread :: StartInternalThread()
{
   if ((IsInternalThreadRunning() == false)&&(_acceptSocket()))
   {
      _notifySocket = GetInternalThreadWakeupSocket();
      return (_notifySocket.GetFileDescriptor() >= 0) ? Thread::StartInternalThread() : B_ERROR;
   } 
   return B_ERROR;
}

void AcceptSocketsThread :: InternalThreadEntry()
{
   fd_set readSet;
   bool keepGoing = true;
   while(keepGoing)
   {
      int afd = _acceptSocket.GetFileDescriptor();
      int nfd = _notifySocket.GetFileDescriptor();

      FD_ZERO(&readSet);
      FD_SET(afd, &readSet);
      FD_SET(nfd, &readSet);
      if (select(muscleMax(afd, nfd)+1, &readSet, NULL, NULL, NULL) < 0) break;
      if (FD_ISSET(nfd, &readSet))
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
      if (FD_ISSET(afd, &readSet))
      {
         ConstSocketRef newSocket = Accept(_acceptSocket);
         if (newSocket())
         {
            MessageRef msg(GetMessageFromPool(AST_EVENT_NEW_SOCKET_ACCEPTED));
            msg()->AddTag(AST_NAME_SOCKET, CastAwayConstFromRef(newSocket.GetRefCountableRef()));
            (void) SendMessageToOwner(msg);
         }
      }
   }
}

}; // end namespace muscle
