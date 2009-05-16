/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "util/SharedFilterSessionFactory.h"
#include "system/SharedMemory.h"
#include "util/NetworkUtilityFunctions.h"
#include "util/MiscUtilityFunctions.h"

namespace muscle {

SharedFilterSessionFactory :: SharedFilterSessionFactory(const ReflectSessionFactoryRef & slaveRef, const String & sharedMemName, bool isGrantList, bool defaultPass) : ProxySessionFactory(slaveRef), _sharedMemName(sharedMemName), _isGrantList(isGrantList), _defaultPass(defaultPass)
{
   // empty
}

SharedFilterSessionFactory :: ~SharedFilterSessionFactory()
{
   // empty
}

AbstractReflectSessionRef SharedFilterSessionFactory :: CreateSession(const String & clientIP, const IPAddressAndPort & iap)
{
   TCHECKPOINT;
   return ((GetSlave()())&&(IsAccessAllowedForIP(_sharedMemName, Inet_AtoN(clientIP()), _isGrantList, _defaultPass))) ? GetSlave()()->CreateSession(clientIP, iap) : AbstractReflectSessionRef();
}

bool SharedFilterSessionFactory :: IsAccessAllowedForIP(const String & sharedMemName, const ip_address & ip, bool isGrantList, bool defaultPass)
{
   bool allowAccess = defaultPass;
   if (ip != invalidIP)
   {
      SharedMemory sm;
      if (sm.SetArea(sharedMemName(), 0, true) == B_NO_ERROR)
      {
         Queue<NetworkInterfaceInfo> ifs;
         bool gotIFs = false;  // we'll demand-allocate them

         allowAccess = !isGrantList;  // if there is a list, you're off it unless you're on it!

         const ip_address * ips = (const ip_address *) sm();
         uint32 numIPs = sm.GetAreaSize()/sizeof(ip_address);
         for (uint32 i=0; i<numIPs; i++)
         {
            ip_address nextIP = ips[i];
            if (nextIP == ip)
            {
               allowAccess = isGrantList;
               break;
            }
            else if (IsStandardLoopbackDeviceAddress(nextIP))
            {
               if (gotIFs == false)
               {
                  (void) GetNetworkInterfaceInfos(ifs);
                  gotIFs = true;
               }

               // Special case for the localhost IP... see if it matches any of our localhost's known IP addresses
               bool matchedLocal = false;
               for (uint32 j=0; j<ifs.GetNumItems(); j++)
               {
                  if (ifs[j].GetLocalAddress() == ip)
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

}; // end namespace muscle
