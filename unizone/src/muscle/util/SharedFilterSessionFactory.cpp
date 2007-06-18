/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "util/SharedFilterSessionFactory.h"
#include "system/SharedMemory.h"
#include "util/NetworkUtilityFunctions.h"
#include "util/MiscUtilityFunctions.h"

BEGIN_NAMESPACE(muscle);

SharedFilterSessionFactory :: SharedFilterSessionFactory(const ReflectSessionFactoryRef & slaveRef, const String & sharedMemName, bool isGrantList, bool defaultPass) : _slaveRef(slaveRef), _sharedMemName(sharedMemName), _isGrantList(isGrantList), _defaultPass(defaultPass)
{
   // empty
}

SharedFilterSessionFactory :: ~SharedFilterSessionFactory()
{
   // empty
}

AbstractReflectSession * SharedFilterSessionFactory:: CreateSession(const String & clientHostIP)
{
   TCHECKPOINT;
   return ((_slaveRef())&&(IsAccessAllowedForIP(_sharedMemName, Inet_AtoN(clientHostIP()), _isGrantList, _defaultPass))) ? _slaveRef()->CreateSession(clientHostIP) : NULL;
}

bool SharedFilterSessionFactory :: IsAccessAllowedForIP(const String & sharedMemName, uint32 ip, bool isGrantList, bool defaultPass)
{
   bool allowAccess = defaultPass;
   if (ip > 0)
   {
      SharedMemory sm;
      if (sm.SetArea(sharedMemName(), 0, true) == B_NO_ERROR)
      {
         allowAccess = !isGrantList;  // if there is a list, you're off it unless you're on it!

         const uint32 * ips = (const uint32 *) sm();
         uint32 numIPs = sm.GetAreaSize()/sizeof(uint32);
         for (uint32 i=0; i<numIPs; i++)
         {
            uint32 nextIP = ips[i];
            if (nextIP == ip)
            {
               allowAccess = isGrantList;
               break;
            }
            else if (nextIP == localhostIP)
            {
               // Special case for the localhost IP... see if it matches any of our localhost's known IP addresses
               bool matchedLocal = false;
               uint32 nextLocalIP;
               for (uint32 j=0; (nextLocalIP=GetLocalIPAddress(j))>0; j++)
               {
                  if (nextLocalIP == ip)
                  {
                     allowAccess = isGrantList;
                     matchedLocal = true;
                     break;
                  }
               }
               if (matchedLocal) break;
            }
         }
         sm.UnlockArea(); 
      }
   }
   return allowAccess;
}

END_NAMESPACE(muscle);
