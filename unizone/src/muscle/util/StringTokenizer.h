/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleStringTokenizer_h
#define MuscleStringTokenizer_h

#include "support/MuscleSupport.h"

namespace muscle {

/** String tokenizer class, similar to Java's java.util.StringTokenizer */
class StringTokenizer
{
public:
   /** Initializes the StringTokenizer to parse (tokenizeMe), which 
    *  should be a string of tokens (e.g. words) separated by any 
    *  of the characters specified in (separators)
    *  @param tokenizeMe the string to break up into 'words'.
    *  @param separators ASCII string representing a list of characters to interpret a word separators.
    *                    Defaults to ", \t" (where "\t" is of course the tab character)
    */
   StringTokenizer(const char * tokenizeMe, const char * separators = ", \t")
   {
      int tlen = strlen(tokenizeMe);
      int slen = strlen(separators);
 
      char * temp = newnothrow_array(char, slen+1+tlen+1);
      if (temp)
      {
         strcpy(temp, separators);
         _seps = temp;
         _next = temp + slen + 1;
         strcpy(_next, tokenizeMe);
      }
      else 
      {
         _seps = _next = NULL;
         WARN_OUT_OF_MEMORY;
      }
 
      _alloced = true;
   }

   /** This Constructor is the same as above, only with this one you allow
    *  the StringTokenizer to operator directly on (tokenizeMe) rather
    *  than making it allocate a copy first.  (it's a bit more efficient)
    *  @param junk Ignored; it's only here to disambiguate the two constructors.
    *  @param tokenizeMe The string to tokenize.  This string will get munged!
    *  @param separators ASCII string representing a list of characters to interpret a word separators.
    *                    Defaults to ", \t" (where "\t" is of course the tab character)
    */
   StringTokenizer(bool junk, char * tokenizeMe, const char * separators = ", \t")
   {
      (void) junk;
      _next = tokenizeMe;
      _seps = separators;
      _alloced = false;
   }

   /** Destructor */
   ~StringTokenizer()
   {
      // must cast to (char *) or VC++ complains :^P
      if (_alloced) delete [] ((char *)_seps);
   }

   /** Returns the next token in the parsed string, or NULL if there are no more tokens left */
   char * GetNextToken()
   {
      if (_seps)
      {
         // Move until first non-sep char
         while((*_next)&&(strchr(_seps, *_next) != NULL)) _next++;
         if (*_next)
         {
            char * ret = _next;

            // Move until next sep-char
            while((*_next)&&(strchr(_seps, *_next) == NULL)) _next++;
            if (*_next) 
            {
               *_next = '\0';
               _next++;
            }
            return ret;
         }
      }
      return NULL;
   }

   /** Convenience synonym for GetNextToken() */
   char * operator()() {return GetNextToken();}
 
   /** Returns the remainder of the string, starting with the next token,
    *  or NULL if there are no more tokens in the string.
    *  Doesn't affect the next return value of GetNextToken(), though.
    */
   char * GetRemainderOfString()
   {
      if (_seps)
      {
         // Move until first non-sep char
         while((*_next)&&(strchr(_seps, *_next) != NULL)) _next++;
         return (*_next) ? _next : NULL;  // and return from there
      }
      return NULL;
   }

private:
   StringTokenizer(const StringTokenizer &);   // unimplemented on purpose
   StringTokenizer & operator = (const StringTokenizer &);  // unimplemented on purpose

   bool _alloced;
   const char * _seps;
   char * _next;
};

}; // end namespace muscle

#endif
