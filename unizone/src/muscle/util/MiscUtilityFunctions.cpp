/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#include "util/MiscUtilityFunctions.h"
#include "util/StringTokenizer.h"

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

String GetHumanReadableTimeString(uint64 timeUS)
{
   char buf[256] = ""; 
   if (timeUS > 0)
   {
#ifdef WIN32
      // Borland's localtime() function is buggy, so we'll use the Win32 API instead.
      uint64 winTime = timeUS*10;  // Convert to (100ns units)
      FILETIME fileTime;
      fileTime.dwHighDateTime = (DWORD) ((winTime>>32) & 0xFFFFFFFF);
      fileTime.dwLowDateTime  = (DWORD) ((winTime>> 0) & 0xFFFFFFFF);
      FILETIME localTime;
      if (FileTimeToLocalFileTime(&fileTime, &localTime))
      {
         SYSTEMTIME st;
         if (FileTimeToSystemTime(&localTime, &st)) sprintf(buf, "%02i/%02i/%02i %02i:%02i:%02i", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
      }
#else
   time_t timeS = (time_t) (timeUS/1000000);  // timeS is seconds since 1970
   struct tm * ts = localtime(&timeS);
   if (ts) sprintf(buf, "%02i/%02i/%02i %02i:%02i:%02i", ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday, ts->tm_hour, ts->tm_min, ts->tm_sec);
#endif
   }
   return buf;
}
 
uint64 ParseHumanReadableTimeString(const String & s)
{
   StringTokenizer tok(s(), "/: ");
   const char * year   = tok();
   const char * month  = tok();
   const char * day    = tok();
   const char * hour   = tok();
   const char * minute = tok();
   const char * second = tok();
   if ((month)&&(day)&&(year)&&(hour)&&(minute)&&(second))
   {
      struct tm temp;
      temp.tm_sec  = atoi(second);
      temp.tm_min  = atoi(minute);
      temp.tm_hour = atoi(hour);
      temp.tm_mday = atoi(day);
      temp.tm_mon  = atoi(month)-1;
      temp.tm_year = atoi(year)-1900;
      time_t timeS = mktime(&temp);
      return ((uint64)timeS)*1000000;
   }
   else return 0;
}

};  // end namespace muscle
