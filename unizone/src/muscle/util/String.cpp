/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "util/String.h"
#include <stdarg.h>

namespace muscle {

String::String(const char * str, int32 maxLen) : Flattenable(), _buffer(NULL), _bufferLen(0), _length(0)
{
   SetCstr(str, maxLen);
}

String::String(const String & str) : Flattenable(), _buffer(NULL), _bufferLen(0), _length(0)
{
   *this = str;
}

status_t
String::SetCstr(const char * str, int32 maxLen)
{
   if (maxLen < 0) maxLen = str ? strlen(str) : 0;
   if (maxLen > 0)
   {
      if (str[maxLen-1] != '\0') maxLen++;  // make room to add the NUL byte if necessary
      if (EnsureBufferSize(maxLen, false) != B_NO_ERROR) return B_ERROR;
      memcpy(_buffer, str, maxLen-1);
      _buffer[maxLen-1] = '\0';
      _length = maxLen-1;
   }
   else
   {
      if (_buffer) _buffer[0] = '\0';
      _length = 0;
   }
   return B_NO_ERROR;
}

String &
String::operator+=(const char aChar)
{
   if (EnsureBufferSize(Length()+2, true) == B_NO_ERROR)
   {
      _buffer[_length++] = aChar;
      _buffer[_length]   = '\0';
   }
   return *this;
}

String &
String::operator-=(const char aChar)
{
   int idx = LastIndexOf(aChar);
   if (idx >= 0)
   {
      String temp = Substring(idx+1);
      (*this) = this->Substring(0, idx);
      (*this) += temp;
   }
   return *this;
}

String &
String::operator+=(const String &other)
{
   uint32 otherLen = other.Length();
   if ((otherLen > 0)&&(EnsureBufferSize(Length() + otherLen + 1, true) == B_NO_ERROR))
   {
      strcpy(&_buffer[_length], other.Cstr());
      _length += otherLen;
   }
   return *this;
}

String &
String::operator-=(const String &other)
{
   if (*this == other) *this = "";
   else
   {
      int idx = LastIndexOf(other);
      if (idx >= 0)
      {
         String temp = Substring(idx+other.Length());
         (*this) = this->Substring(0, idx);
         (*this) += temp;
      }
   }
   return *this;
}

String &
String::operator<<(int rhs)
{
   char buff[64];
   sprintf(buff, "%d", rhs);
   return *this << buff;
}

String &
String::operator<<(float rhs)
{
   char buff[64];
   sprintf(buff, "%.2f", rhs);
   return *this << buff;
}

String &
String::operator<<(bool rhs)
{
   const char* val = rhs ? "true" : "false"; 
   return *this << val;
}
 

bool
String::operator==(const String &rhs) const
{
   return (this == &rhs) ? true : ((Length() == rhs.Length()) ? (strcmp(Cstr(), rhs.Cstr()) == 0) : false);
}

bool
String::operator<(const String &rhs) const
{
   return (this == &rhs) ? false : (strcmp(Cstr(), rhs.Cstr()) < 0);
}

bool
String::operator>(const String &rhs) const
{
   return (this == &rhs) ? false : (strcmp(Cstr(), rhs.Cstr()) > 0);
}

bool
String::operator<=(const String &rhs) const
{
   return (this == &rhs) ? true : (strcmp(Cstr(), rhs.Cstr()) <= 0);
}

bool
String::operator>=(const String & rhs) const
{
   return (this == &rhs) ? true : (strcmp(Cstr(), rhs.Cstr()) >= 0);
}

bool
String::EndsWith(const String &s2) const
{ 
   return (Length() < s2.Length()) ? false : (strcmp(Cstr()+(Length()-s2.Length()), s2.Cstr()) == 0); 
}

void
String::Reverse()
{
   if ((_buffer)&&(Length() > 0))
   {
      uint32 from = 0;
      uint32 to = Length()-1;
      while(from<to) muscleSwap(_buffer[from++], _buffer[to--]);
   }
}

status_t
String::Replace(char findChar, char replaceChar)
{
   status_t ret = B_ERROR;
   if ((_buffer)&&(findChar != replaceChar))
   {
      char * c = _buffer;
      while(*c)
      {
         if (*c == findChar) 
         {
            *c = replaceChar;
            ret = B_NO_ERROR;
         }
         c++;
      }
   }
   return ret;
}
   
status_t
String::Replace(const String& match, const String& replace)
{
   status_t ret = B_ERROR;

   if (match != replace)
   {
      String temp(*this);
      String newString;
 
      int loc; 
      while ((loc = temp.IndexOf(match)) != -1)
      {
         ret = B_NO_ERROR;
         newString += temp.Substring(0, loc);
         newString += replace;
         temp = temp.Substring(loc + match.Length());
      } 
      if (ret == B_NO_ERROR) 
      {
         newString += temp;
         *this = newString;
      }
   }
   return ret;
}

int
String::IndexOf(char ch, uint32 fromIndex) const
{
   const char * temp = (fromIndex < Length()) ? strchr(Cstr()+fromIndex, ch) : NULL; 
   return temp ? (temp - Cstr()) : -1; 
}

int
String::IndexOf(const String &s2, uint32 fromIndex) const
{
   const char *theFind = (fromIndex < Length()) ? strstr(Cstr()+fromIndex, s2.Cstr()) : NULL;
   return theFind ? (theFind - Cstr()) : -1;
}

int
String::LastIndexOf(char ch, uint32 fromIndex) const
{
   const char * lio = (fromIndex < Length()) ? strrchr(Cstr()+fromIndex, ch) : NULL;
   return lio ? (lio - Cstr()) : -1;
}

int
String::LastIndexOf(const String &s2) const
{
   return (s2.Length() <= Length()) ? LastIndexOf(s2, Length() - s2.Length()) : -1;
}

int
String::LastIndexOf(const String &s2, uint32 fromIndex) const
{
   if (s2.Length() == 0) return Length()-1;
   if (fromIndex >= Length()) return -1;
   for (int i=fromIndex; i>=0; i--) if (strncmp(Cstr()+i, s2.Cstr(), s2.Length()) == 0) return i;
   return -1;
}

bool
String::StartsWith(const String &s2) const
{
   return (Length() < s2.Length()) ? false : StartsWith(s2, 0); 
} 
 
bool 
String::StartsWith(const String &s2, uint32 offset) const 
{
   return (offset > Length() - s2.Length()) ? false : (strncmp(Cstr()+offset, s2.Cstr(), s2.Length()) == 0); 
}

String
String::Substring(uint32 left) const
{
   return Substring(left, Length());
}

String
String::Substring(uint32 left, uint32 right) const
{
   MASSERT(left <= right, "Invalid Substring range");
   MASSERT(right <= Length(), "Index out of bounds");

   String ret;
   if (ret.EnsureBufferSize(right-left+1, true) == B_NO_ERROR)
   {
      const char * c = Cstr();
      for (uint32 i=left; i<right; i++) ret += c[i];
   }
   return ret;
}

String
String::Substring(const String & markerString) const
{
   int idx = LastIndexOf(markerString);
   return (idx >= 0) ? Substring(idx+markerString.Length()) : *this;
}

String
String::Substring(uint32 left, const String & markerString) const
{
   int idx = IndexOf(markerString, left);
   return (idx >= 0) ? Substring(left, idx) : *this;
}

String
String::ToLowerCase() const
{
   String ret(_buffer);
   if (ret._buffer) for (uint32 i=0; i<ret.Length(); i++) ret._buffer[i] = (char)tolower(ret._buffer[i]);
   return ret;
}

String
String::ToUpperCase() const
{
   String ret(_buffer);
   if (ret._buffer) for (uint32 i=0; i<ret.Length(); i++) ret._buffer[i] = (char)toupper(ret._buffer[i]);
   return ret;
}
 
String 
String::Trim() const 
{ 
   int32 len = (int32) Length();
   const char * s = Cstr();
   int32 startIdx; for (startIdx = 0;     startIdx<len;    startIdx++) if (!IsSpaceChar(s[startIdx])) break; 
   int32 endIdx;   for (endIdx   = len-1; endIdx>startIdx; endIdx--)   if (!IsSpaceChar(s[endIdx]))   break; 
   return Substring((uint32)startIdx, (uint32)(endIdx+1)); 
}

uint32 String :: FlattenedSize() const
{
   return Length()+1;   
}

void String :: Flatten(uint8 *buffer) const
{
   strcpy((char *)buffer, Cstr());
}

status_t String :: Unflatten(const uint8 *buf, uint32 size)
{
   return SetCstr((const char *)buf, (int32)size);
}

uint32 String :: HashCode() const
{
   return CStringHashFunc(Cstr());
}

uint32 String :: GetNumInstancesOf(char ch) const
{
   uint32 ret = 0;
   for (const char * s = Cstr(); (*s != '\0'); s++) if (*s == ch) ret++; 
   return ret;
}

uint32 String :: GetNumInstancesOf(const String & substring) const
{
   uint32 ret = 0;
   if (substring.Length() > 0)
   {
      uint32 lastIdx = 0;
      int32 idx;
      while((idx = IndexOf(substring, lastIdx)) >= 0)
      {
         ret++;
         lastIdx = idx + substring.Length();
      }
   }
   return ret;
}

// This method tries to ensure that at least (newBufLen) chars
// are available for storing data in.  (requestedBufLen) should include
// the terminating NUL.  If (retainValue) is true, the current string value
// will be retained; otherwise it should be set right after this call
// returns...
status_t String::EnsureBufferSize(uint32 requestedBufLen, bool retainValue)
{
   if ((requestedBufLen > 0)&&((_buffer == NULL)||(requestedBufLen > _bufferLen)))
   {
      // If we're doing an initial allocation, just allocate the bytes requested.
      // If it's a re-allocation, allocate more than requested as it's more likely
      // to happen yet another time...
      uint32 newBufLen = (_buffer == NULL) ? requestedBufLen : (requestedBufLen * 2);
      char * newBuf = (requestedBufLen <= sizeof(_smallBuffer)) ? _smallBuffer : newnothrow char[newBufLen]; 
      if (newBuf == _smallBuffer) newBufLen = sizeof(_smallBuffer);
      if (newBuf)
      {
         if ((retainValue)&&(_buffer)&&(newBuf != _buffer)) strncpy(newBuf, _buffer, newBufLen);
         if (_buffer != _smallBuffer) delete [] _buffer;
         _buffer = newBuf;
         _bufferLen = newBufLen;
      }
      else 
      {
         WARN_OUT_OF_MEMORY;
         return B_ERROR;
      }
   }
   return B_NO_ERROR;
}

/*--- ElfHash --------------------------------------------------- 
 *  The published hash algorithm used in the UNIX ELF format 
 *  for object files. Accepts a pointer to a string to be hashed 
 *  and returns an unsigned long. 
 *  jaf:  stolen from: http://www.ddj.com/articles/1996/9604/9604b/9604b.htm?topic=algorithms
 *-------------------------------------------------------------*/ 
uint32 CStringHashFunc(const char * n)
{
    uint32 h = 0, g; 
    const unsigned char * name = (const unsigned char *) n;
    while (*name) 
    { 
        h = (h << 4) + *name++; 
        if ((g = h & 0xF0000000) != 0) h ^= g >> 24; 
        h &= ~g; 
    } 
    return h; 
}

int CStringCompareFunc(const char * const & s1, const char * const & s2, void *)
{
    return strcmp(s1, s2);
}

int StringCompareFunc(const String & s1, const String & s2, void *)
{
   return muscleCompare(s1, s2);
}

};  // end namespace muscle
