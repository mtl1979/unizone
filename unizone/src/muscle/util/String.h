/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */
/* This class was derived from the String class written by Michael Olivero (mike95@mike95.com) */
 
#ifndef MuscleString_h
#define MuscleString_h

#include <string.h> 
#include <ctype.h> 
#include "support/Flattenable.h"

namespace muscle {

#ifndef SMALL_MUSCLE_STRING_LENGTH
# define SMALL_MUSCLE_STRING_LENGTH 7  // strings shorter than this length can be stored inline, without requiring an extra new[].
#endif

class String;

/** A character string class, similar to Java's java.lang.String */
class String : public Flattenable {
public:
   /** Constructor.
    *  @param str If non-NULL, the initial value for this String.
    *  @param maxLen If specified, the number of characters to place into
    *                this String (not including the NUL terminator byte).
    *                Default is to scan the string to determine the length.
    */
   String(const char * str = NULL, uint32 maxLen = ((uint32)-1));

   /** Copy Constructor. */
   String(const String & str);

   /** Destructor. */
   virtual ~String();

   /** Assignment Operator. */
   String & operator = (char val) {(void) SetCstr(&val, 1); return *this;}

   /** Assignment Operator. 
     * @param val Pointer to the C-style string to copy from.  May be NULL.
     */
   String & operator = (const char * val) {(void) SetCstr(val); return *this;}

   /** Assignment Operator. */
   String & operator = (const String &rhs) {(void) SetCstr(rhs(), rhs.Length()); return *this;}

   /** Append Operator. 
    *  @param rhs A string to append to this string.
    */
   String & operator += (const String &rhs);

   /** Append Operator.
    *  @param ch A character to append to this string.
    */
   String & operator += (const char ch);
   
   /** Remove Operator. 
    *  @param rhs A substring to remove from this string;  the
    *             last instance of the substring will be cut out.
    *             If (rhs) is not found, there is no effect.
    */
   String & operator -= (const String &rhs);

   /** Remove Operator.
    *  @param ch A character to remove from this string;  the last
    *            instance of this char will be cut out.  If (ch) is
    *            not found, there is no effect.
    */
   String & operator -= (const char ch);
   
   /** Append 'Stream' Operator.
    *  @param rhs A String to append to this string.
    *  @return a non const String refrence to 'this' so you can chain appends.
    */
   String & operator << (const String& rhs) {return (*this += rhs);}

   /** Append 'Stream' Operator.
    *  @param rhs A const char* to append to this string.
    *  @return a non const String refrence to 'this' so you can chain appends.
    */
   String & operator << (const char* rhs) {return (*this += rhs);}
   
   /** Append 'Stream' Operator.
    *  @param rhs An int to append to this string.
    *  @return a non const String refrence to 'this' so you can chain appends.
    */
   String & operator << (int rhs);   

   /** Append 'Stream' Operator.
    *  @param rhs A float to append to this string. Formatting is set at 2 decimals of precision.
    *  @return a non const String refrence to 'this' so you can chain appends.
    */
   String & operator << (float rhs);   

   /** Append 'Stream' Operator.
    *  @param rhs A bool to append to this string. Converts to 'true' and 'false' strings appropriately.
    *  @return a non const String refrence to 'this' so you can chain appends.
    */
   String & operator << (bool rhs);

   /** Comparison Operator.  Returns true if the two strings are equal (as determined by strcmp()) */
   bool operator == (const String &rhs) const;

   /** Comparison Operator.  Returns true if the two strings are not equal (as determined by strcmp()) */
   bool operator != (const String &rhs) const {return !(*this == rhs);}

   /** Comparison Operator.  Returns true if this string comes before (rhs) lexically. */
   bool operator < (const String &rhs) const;

   /** Comparison Operator.  Returns true if this string comes after (rhs) lexically. */
   bool operator > (const String &rhs) const;

   /** Comparison Operator.  Returns true if the two strings are equal, or this string comes before (rhs) lexically. */
   bool operator <= (const String &rhs) const;

   /** Comparison Operator.  Returns true if the two strings are equal, or this string comes after (rhs) lexically. */
   bool operator >= (const String &rhs) const;

   /** Array Operator.  Used to get easy access to the characters that make up this string.
    *  @param index Index of the character to return.  Be sure to only use valid indices!
    */
   char operator [] (uint32 index) const {verifyIndex(index); return _buffer[index];}

   /** Array Operator.  Used to get easy access to the characters that make up this string.
    *  @param index Index of the character to set.  Be sure to only use valid indices!
    */
   char & operator [] (uint32 index) {verifyIndex(index); return _buffer[index];}

   /** Returns the character at the (index)'th position in the string.
    *  @param index A value between 0 and (Length()-1), inclusive.
    *  @return A character value.
    */
   char CharAt(uint32 index) const {return operator[](index);}
 
   /** Compares this string to another string using strcmp() */
   int CompareTo(const String &anotherString) const {return strcmp(Cstr(), anotherString.Cstr());}

   /** Returns a C-style pointer to our held character string. */
   const char * Cstr() const {return _buffer ? _buffer : "";}

   /** Convenience synonym for Cstr(). */
   const char * operator()() const {return Cstr();}  

   /** Sets our state from the given C-style string.
     * @param str The new string to copy from.  If maxLen is negative, this may be NULL.
     * @param maxLen If set, the number of characters to copy (not including the NUL
     *               terminator byte).  By default, the number of characters is determined
     *               automatically by scanning the string.
     */
   status_t SetCstr(const char * str, uint32 maxLen = ((uint32)-1));

   /** Returns true iff this string ends with (suffix) */
   bool EndsWith(const String &suffix) const;

   /** Returns true iff this string is equal to (string), as determined by strcmp(). */
   bool Equals(const String & str) const {return (*this == str);}

   /** Returns the first index of (ch) in this string starting at or after (fromIndex), or -1 if not found. */
   int IndexOf(char ch, uint32 fromIndex = 0) const;

   /** Returns the first index of substring (str) in this string starting at or after (fromIndex), or -1 if not found. */
   int IndexOf(const String &str, uint32 fromIndex = 0) const;

   /** Returns the last index of (ch) in this string starting at or after (fromIndex), or -1 if not found. */
   int LastIndexOf(char ch, uint32 fromIndex = 0) const;

   /** Returns the last index of substring (str) in this string */
   int LastIndexOf(const String &str) const;

   /** Returns the last index of substring (str) in this string starting at or after (fromIndex), or -1 if not found. */
   int LastIndexOf(const String &str, uint32 fromIndex) const;

   /** Returns the number of characters in the string (not including the terminating NUL byte) */
   uint32 Length() const {return _length;}

   /** Returns the number of instances of (c) in this string. */
   uint32 GetNumInstancesOf(char ch) const;

   /** Returns the number of instances of (substring) in this string. */
   uint32 GetNumInstancesOf(const String & substring) const;

   /** Returns true iff this string starts with (prefix) */
   bool StartsWith(const String &prefix) const;

   /** Returns true iff this string starts with the first (tooffset) characters of (prefix) */
   bool StartsWith(const String &prefix, uint32 toffset) const; 

   /** Returns a string that consists of (str) prepended to this string.  Does not modify the String it is called on. */
   String Prepend(const String & str) const {String ret = str; ret += *this; return ret;}

   /** Returns a string that consists of (str) appended to this string.  Does not modify the String it is called on. */
   String Append(const String & str) const {String ret = *this; ret += str; return ret;}

   /** Returns a string that consists of only the last part of this string, starting with index (beginIndex).  Does not modify the string it is called on. */
   String Substring(uint32 beginIndex) const; 

   /** Returns a string that consists of only the characters in this string from range (beginIndex) to (endIndex-1).  Does not modify the string it is called on. */
   String Substring(uint32 beginIndex, uint32 endIndex) const; 

   /** Returns a string that consists of only the last part of this string, starting with the first character after the last instance of (markerString).  
    *  If (markerString) is not found in the string, then this entire String is returned.
    *  For example, String("this is a test").Substring("is a") returns " test".
    *  Does not modify the string it is called on. 
    */
   String Substring(const String & markerString) const; 

   /** Returns a string that consists of only the characters in the string from range (beginIndex) until the character just before
    *  the first character in (markerString).  If (markerString) is not found, then the entire substring starting at (beginIndex) is returned.
    *  For example, String("this is a test").Substring(1, "is a") returns "his ".
    *  Does not modify the string it is called on. 
    */
   String Substring(uint32 beginIndex, const String & markerString) const; 

   /** Returns an all lower-case version of this string.  Does not modify the string it is called on. */
   String ToLowerCase() const; 

   /** Returns an all upper-case version of this string.  Does not modify the string it is called on. */
   String ToUpperCase() const; 

   /** Returns an version of this string that has all leading and trailing whitespace removed.  Does not modify the string it is called on. */
   String Trim() const;  

   /** Like CompareTo(), but case insensitive. */
   int CompareToIgnoreCase(const String &s) const             {return ToLowerCase().CompareTo(s.ToLowerCase());}

   /** Like EndsWith(), but case insensitive. */
   bool EndsWithIgnoreCase(const String &s) const             {return ToLowerCase().EndsWith(s.ToLowerCase());}

   /** Like Equals(), but case insensitive. */
   bool EqualsIgnoreCase(const String &s) const               {return ToLowerCase().Equals(s.ToLowerCase());}

   /** Like IndexOf(), but case insensitive. */
   int IndexOfIgnoreCase(const String &s) const               {return ToLowerCase().IndexOf(s.ToLowerCase());}

   /** Like IndexOf(), but case insensitive. */
   int IndexOfIgnoreCase(const String &s, uint32 f) const     {return ToLowerCase().IndexOf(s.ToLowerCase(),f);}

   /** Like IndexOf(), but case insensitive. */
   int IndexOfIgnoreCase(char ch) const                       {return ToLowerCase().IndexOf((char)tolower(ch));}

   /** Like IndexOf(), but case insensitive. */
   int IndexOfIgnoreCase(char ch, uint32 f) const             {return ToLowerCase().IndexOf((char)tolower(ch),f);}

   /** Like LastIndexOf(), but case insensitive. */
   int LastIndexOfIgnoreCase(const String &s) const           {return ToLowerCase().LastIndexOf(s.ToLowerCase());}

   /** Like LastIndexOf(), but case insensitive. */
   int LastIndexOfIgnoreCase(const String &s, uint32 f) const {return ToLowerCase().LastIndexOf(s.ToLowerCase(),f);}

   /** Like LastIndexOf(), but case insensitive. */
   int LastIndexOfIgnoreCase(char ch) const                   {return ToLowerCase().LastIndexOf((char)tolower(ch));}

   /** Like LastIndexOf(), but case insensitive. */
   int LastIndexOfIgnoreCase(char ch, uint32 f) const         {return ToLowerCase().LastIndexOf((char)tolower(ch),f);}

   /** Like StartsWith(), but case insensitive. */
   bool StartsWithIgnoreCase(const String &s) const           {return ToLowerCase().StartsWith(s.ToLowerCase());}

   /** Like StartsWith(), but case insensitive. */
   bool StartsWithIgnoreCase(const String &s, uint32 o) const {return ToLowerCase().StartsWith(s.ToLowerCase(),o);}

   /** Returns a hash code for this string */
   uint32 HashCode() const;

   /** Replaces all instances of (oldChar) in this string with (newChar).
     * @returns B_NO_ERROR on success, or B_ERROR if no changes were necessary.
     */
   status_t Replace(char oldChar, char newChar); 

   /** Replaces all instances of (match) in this string with (replace).
     * @returns B_NO_ERROR on success, or B_ERROR if no changes were necessary.
     */
   status_t Replace(const String& match, const String& replace); 
 
   /** Reverses the order of all characters in the string, so that e.g. "string" becomes "gnirts" */
   void Reverse();

   /** Part of the Flattenable interface.
    *  @return false
    */
   virtual bool IsFixedSize() const {return false;}

   /** Part of the Flattenable interface.
    *  @return B_STRING_TYPE
    */
   virtual uint32 TypeCode() const {return B_STRING_TYPE;}

   /** Part of the Flattenable interface.
    *  @return Length()+1  (the +1 is for the terminating NUL byte)
    */
   virtual uint32 FlattenedSize() const;

   /** Part of the Flattenable interface.  Flattens our string into (buffer).
    *  @param buffer A byte array to receive the flattened version of this string.
    *                There must be at least FlattenedSize() bytes available in this array.
    *  The clever secret here is that a flattened String is just a C-style
    *  null-terminated character array, and can be used interchangably as such.
    */
   virtual void Flatten(uint8 *buffer) const;

   /** Unflattens a String from (buf).
    *  @param buf an array of (size) bytes.
    *  @size the number of bytes in (buf).
    *  @return B_NO_ERROR (never fails!)
    */
   virtual status_t Unflatten(const uint8 *buf, uint32 size);

   /** Makes sure that we have pre-allocated enough space for a string at least (numChars)
    *  ASCII characters long.  If not, this method will try to allocate the space.
    *  @param numChars How much space to pre-allocate, in ASCII characters.
    *  @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory).
    */ 
   status_t Prealloc(uint32 newBufLen) {return EnsureBufferSize(newBufLen+1, true);}

private:
   bool IsSpaceChar(char c) const {return ((c==' ')||(c=='\t')||(c=='\r')||(c=='\n'));}
   status_t EnsureBufferSize(uint32 newBufLen, bool retainValue);
   char _smallBuffer[SMALL_MUSCLE_STRING_LENGTH+1];  // store very small strings in-line, to avoid dynamic allocation
   char * _buffer;            // Stores the chars.  May point at (_smallBuffer), or a dynamically allocated buffer
   uint32 _bufferLen;         // Number of bytes pointed to by (_buffer)
   uint32 _length;            // cached strlen(_buffer)

   void verifyIndex(uint32 index) const {MASSERT(index < _length, "Index Out Of Bounds Exception");}
};

/** A nice hashing function for use with (const char *)'s */
uint32 CStringHashFunc(const char * str); 

template <class T> class HashFunctor;

template <>
class HashFunctor<String>
{
public:
   uint32 operator () (const String & x) const {return CStringHashFunc(x());}
};

template <>
class HashFunctor<const char *>
{
public:
   uint32 operator () (const char * x) const {return CStringHashFunc(x);}
};

/** A function for comparing (const String &)'s -- calls the String operators */
int StringCompareFunc(const String &, const String &, void *);

/** A function for comparing (const char *)'s -- calls strcmp() */
int CStringCompareFunc(const char * const &, const char * const &, void *);

inline String operator+(const String & lhs, const String &rhs)  {String ret(lhs); ret += rhs; return ret;}
inline String operator+(const String & lhs, const char *rhs)    {String ret(lhs); ret += rhs; return ret;}
inline String operator+(const char * lhs,   const String & rhs) {String ret(lhs); ret += rhs; return ret;}
inline String operator+(const String & lhs, char rhs)           {String ret(lhs); ret += rhs; return ret;}
inline String operator-(const String & lhs, const String &rhs)  {String ret(lhs); ret -= rhs; return ret;}
inline String operator-(const String & lhs, const char *rhs)    {String ret(lhs); ret -= rhs; return ret;}
inline String operator-(const char *lhs,    const String &rhs)  {String ret(lhs); ret -= rhs; return ret;}
inline String operator-(const String & lhs, char rhs)           {String ret(lhs); ret -= rhs; return ret;}

};  // end namespace muscle

#endif
