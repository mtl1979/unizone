/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "util/MiscUtilityFunctions.h"

namespace muscle {

status_t ParseArg(const String & a, Message & addTo)
{
   String argName = a;

   // Remove any commenting after the hash mark
   int32 hashIdx = argName.IndexOf("#");
   if (hashIdx >= 0) argName = argName.Substring(0, hashIdx);
   argName = argName.Trim();

   // Remove any initial dashes
   while(argName.StartsWith("-")) argName = argName.Substring(1);

   int equalsAt = argName.IndexOf('=');
   String argValue;
   if (equalsAt >= 0)
   {
      argValue = argName.Substring(equalsAt+1).Trim();
      argName  = argName.Substring(0, equalsAt).Trim();
   }
   return (argName.Length() > 0) ? addTo.AddString(argName.ToLowerCase()(), argValue) : B_NO_ERROR;
}

status_t ParseArgs(int argc, char ** argv, Message & addTo)
{
   for (int i=0; i<argc; i++) if (ParseArg(argv[i], addTo) != B_NO_ERROR) return B_ERROR;
   return B_NO_ERROR;
}

status_t ParseFile(FILE * fpIn, Message & addTo)
{
   const int bufSize = 2048;
   char * buf = newnothrow char[bufSize];
   if (buf)
   {
      status_t ret = B_NO_ERROR;
      while(fgets(buf, bufSize, fpIn))
      {
         if (ParseArg(String(buf), addTo) != B_NO_ERROR)
         {
            ret = B_ERROR;
            break;
         }
      }
      delete [] buf;
      return ret;
   }
   else
   {
      WARN_OUT_OF_MEMORY;
      return B_ERROR;
   }
}

/** Gotta define this myself, since atoll() isn't standard. :^( 
  * Note that this implementation doesn't handle negative numbers!
  */
uint64 Atoull(const char * str)
{
   uint64 base = 1;
   uint64 ret  = 0;

   // Move to the last digit in the number
   const char * s = str;
   while ((*s >= '0')&&(*s <= '9')) s++;

   // Then iterate back to the beginning, tabulating as we go
   while((--s >= str)&&(*s >= '0')&&(*s <= '9')) 
   {
      ret  += base * ((uint64)(*s-'0'));
      base *= (uint64)10;
   }
   return ret;
}

};  // end namespace muscle

