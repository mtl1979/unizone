/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include <stdio.h>
#include "regex/StringMatcher.h"
#include "util/String.h"

BEGIN_NAMESPACE(muscle);

static StringMatcherRef::ItemPool _stringMatcherPool;
StringMatcherRef::ItemPool * GetStringMatcherPool() {return &_stringMatcherPool;}

StringMatcher::StringMatcher() : _regExpValid(false), _negate(false), _hasRegexTokens(false), _rangeMin(MUSCLE_NO_LIMIT), _rangeMax(MUSCLE_NO_LIMIT)
{
   // empty
} 

StringMatcher :: StringMatcher(const String & str, bool simple) : _regExpValid(false), _negate(false)
{
   SetPattern(str, simple);
}

StringMatcher :: ~StringMatcher()
{
   if (_regExpValid) regfree(&_regExp);
}

status_t StringMatcher::SetPattern(const String & s, bool isSimple) 
{
   TCHECKPOINT;

   _pattern = s;
   const char * str = _pattern();
   _hasRegexTokens = HasRegexTokens(str);

   String regexPattern;
   _rangeMin = _rangeMax = MUSCLE_NO_LIMIT;  // default to regular matching mode
   if (isSimple)
   {
      // Special case:  if the first char is a tilde, ignore it, but set the _negate flag.
      if (str[0] == '~')
      {
         _negate = true;
         str++;
      }
      else _negate = false;

      // Special case for strings of form e.g. "<15-23>", which is interpreted to
      // match integers in the range 15-23, inclusive.  Yeah, it's an ungraceful
      // hack, but also quite useful since regex won't do that in general.
      if (str[0] == '<')
      {
         const char * dash = strchr(str+1, '-');
         if (dash)
         {
            const char * nulByte = strchr(dash+1, '\0');
            if (*(nulByte-1) == '>')
            {
               _rangeMin = (str[1]  == '-') ? 0 : (uint32)atoi(str+1);
               _rangeMax = (dash[1] == '>') ? MUSCLE_NO_LIMIT : ((dash[1] == '-') ? 0 : (uint32)atoi(dash+1));
            }
         }
      }

      if (_rangeMin == MUSCLE_NO_LIMIT)
      {
         if ((str[0] == '\\')&&(str[1] == '<')) str++;  // special case escape of initial < for "\<15-23>"

         regexPattern = "^(";

         bool escapeMode = false;
         for (const char * ptr = str; *ptr != '\0'; ptr++)
         {
            char c = *ptr;

            if (escapeMode) escapeMode = false;
            else
            {
               switch(c)
               {
                  case ',':  c = '|';              break;  // commas are treated as union-bars
                  case '.':  regexPattern += '\\'; break;  // dots are considered literals, so escape those
                  case '*':  regexPattern += '.';  break;  // hmmm.
                  case '?':  c = '.';              break;  // question marks mean any-single-char
                  case '\\': escapeMode = true;    break;  // don't transform the next character!
               }
            }
            regexPattern += c;
         }
         if (escapeMode) regexPattern += '\\';  // just in case the user left a trailing backslash
         regexPattern += ")$";
      }
   }

   // Free the old regular expression, if any
   if (_regExpValid)    
   {
     regfree(&_regExp);
     _regExpValid = false;
   }

   // And compile the new one
   if (_rangeMin == MUSCLE_NO_LIMIT)
   {
      _regExpValid = (regcomp(&_regExp, (regexPattern.Length() > 0) ? regexPattern.Cstr() : str, REG_EXTENDED) == 0);
      return _regExpValid ? B_NO_ERROR : B_ERROR;
   }
   else return B_NO_ERROR;  // for range queries, we don't need a valid regex
}


bool StringMatcher::Match(const char * const str) const
{
   TCHECKPOINT;

   bool ret = false;  // pessimistic default

   if (_rangeMin == MUSCLE_NO_LIMIT)
   {
      if (_regExpValid) ret = (regexec(&_regExp, str, 0, NULL, 0) != REG_NOMATCH);
   }
   else if ((str[0] >= '0')&&(str[0] <= '9'))
   {
      ret = muscleInRange((uint32) atoi(str), _rangeMin, _rangeMax);
   }

   return _negate ? (!ret) : ret;
}


bool IsRegexToken(char c, bool isFirstCharInString)
{
   switch(c)
   {
      // muscle 2.50:  fixed to match exactly the chars specified in muscle/regex/regex/regcomp.c
      case '[': case ']': case '*': case '?': case '\\': case ',': case '|': case '(': case ')':
      case '=': case '^': case '+': case '$': case '{':  case '}': case ':': case '-':
        return true;

      case '<': case '~':   // these chars are only special if they are the first character in the string
         return isFirstCharInString; 
 
      default:
         return false;
   }
}

void EscapeRegexTokens(String & s, const char * optTokens)
{
   TCHECKPOINT;

   const char * str = s.Cstr();

   String ret;

   bool isFirst = true;
   while(*str)
   {
     if (optTokens ? (strchr(optTokens, *str) != NULL) : IsRegexToken(*str, isFirst)) ret += '\\';
     isFirst = false;
     ret += *str;
     str++;
   }
   s = ret;
}

void RemoveEscapeChars(String & s)
{
   uint32 len = s.Length();
   String ret; (void) ret.Prealloc(len);
   bool lastWasEscape = false;
   for (uint32 i=0; i<len; i++)
   {
      char c = s[i];
      bool isEscape = (c == '\\');
      if ((lastWasEscape)||(isEscape == false)) ret += c;
      lastWasEscape = ((isEscape)&&(lastWasEscape == false));
   }
   s = ret;
}

bool HasRegexTokens(const char * str)
{
   bool isFirst = true;
   while(*str)
   {
      if (IsRegexToken(*str, isFirst)) return true;
      else 
      {
         str++;
         isFirst = false;
      }
   }
   return false;
}

bool MakeRegexCaseInsensitive(String & str)
{
   TCHECKPOINT;

   bool changed = false;
   String ret;
   for (uint32 i=0; i<str.Length(); i++)
   {
     char next = str[i];
     if ((next >= 'A')&&(next <= 'Z'))
     {
        char buf[5];
        sprintf(buf, "[%c%c]", next, next+('a'-'A'));
        ret += buf;
        changed = true;
     }
     else if ((next >= 'a')&&(next <= 'z'))
     {
        char buf[5];
        sprintf(buf, "[%c%c]", next, next+('A'-'a'));
        ret += buf;
        changed = true;
     }
     else ret += next;
   }
   if (changed) str = ret;
   return changed;
}

END_NAMESPACE(muscle);
