/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleStringMatcher_h
#define MuscleStringMatcher_h

#include <sys/types.h>
#include "util/RefCount.h"
#include "util/String.h"

#ifdef __BEOS__
# if __POWERPC__
#  include "regex/regex/regex.h"  // use included regex if system doesn't provide one
# else
#  include <regex.h>
# endif
#else
# ifdef WIN32
#  include "regex/regex/regex.h"
# else
#  include <regex.h>
# endif
#endif

namespace muscle {

/** A utility class for doing globbing or regular expression matching.  (A thin wrapper around the C regex calls) */
class StringMatcher : public RefCountable
{
public:
   /** Default Constructor. */
   StringMatcher();

   /** A constructor that sets the given expression.  See SetPattern() for argument semantics. */
   StringMatcher(const String & matchString, bool isSimpleFormat = true);
    
   /** Destructor */
   ~StringMatcher();

   /** 
    * Set a new wildcard pattern or regular expression for this StringMatcher to use in future Match() calls.
    *
    * As of v2.01, simple patterns specified here also have a special case:  If you set a pattern of the form 
    * "<x-y>", where x and y are ASCII representations of positive integers, then this StringMatcher will only 
    * match ASCII representations of integers in that range.  So "<19-21>" would match "19", "20", and "21" only.
    * Also, "<-19>" will match integers less than or equal to 19, and "<21->" will match all integers greater
    * than or equal to 21.  "<->" will match everything, same as "*".
    *
    * @param expression The new globbing pattern or regular expression to match with.
    * @param isSimpleFormat If you wish to use the formal regex syntax, 
    *                       instead of the simple syntax, set isSimpleFormat to false.
    * @return B_NO_ERROR on success, B_ERROR on error (e.g. expression wasn't parsable, or out of memory)
    */
   status_t SetPattern(const String & expression, bool isSimpleFormat=true);
    
   /** Returns the pattern String as it was previously passed in to SetPattern() */
   const String & GetPattern() const {return _pattern;}

   /** Returns true iff this StringMatcher's pattern specifies exactly one possible string.
    *  (i.e. the pattern is just plain old text, with no wildcards or other pattern matching logic specified)
    */
   bool IsPatternUnique() const {return (_hasRegexTokens == false)&&(_negate == false)&&(_rangeMin == MUSCLE_NO_LIMIT);}

   /** Returns true iff (string) is matched by the current expression.
    * @param string a string to match against using our current expression.
    * @return true iff (string) matches, false otherwise.
    */
   bool Match(const char * const string) const;
    
   /** If set true, Match() will return the logical opposite of what
     * it would otherwise return; e.g. it will return true only when
     * the given string doesn't match the pattern.
     * Default state is false.  Note that this flag is also set by
     * SetPattern(..., true), based on whether or not the pattern
     * string starts with a tilde.
     */
   void SetNegate(bool negate) {_negate = negate;}

   /** Returns the current state of our negate flag. */
   bool IsNegate() const {return _negate;}

private:
   bool _regExpValid;
   bool _negate;
   bool _hasRegexTokens;
   String _pattern;
   regex_t _regExp;
   uint32 _rangeMin;
   uint32 _rangeMax;   
}; 

typedef Ref<StringMatcher> StringMatcherRef;

/** Returns a point to a singleton ObjectPool that can be used
 *  to minimize the number of StringMatcher allocations and deletions
 *  by recycling the StringMatcher objects 
 */
StringMatcherRef::ItemPool * GetStringMatcherPool();

// Some regular expression utility functions

/** Puts a backslash in front of any char in (str) that is "special" to the regex pattern matching.
 *  @param str The string to check for special regex chars and possibly modify to escape them.
 *  @param optTokens If non-NULL, only characters in this string will be treated as regex tokens
 *                   and escaped.  If left as NULL (the default), then the standard set of regex
 *                   tokens will be escaped.
 */
void EscapeRegexTokens(String & str, const char * optTokens = NULL);

/** Returns true iff any "special" chars are found in (str).
 *  @param str The string to check for special regex chars.
 *  @return True iff any special regex chars were found in (str).
 */
bool HasRegexTokens(const char * str);

/** Returns true iff (c) is a regular expression "special" char as far as StringMatchers are concerned.
 *  @param c an ASCII char
 *  @param isFirstCharInString true iff (c) is the first char in a pattern string.  (important because
 *                                  StringMatchers recognize certain chars as special only if they are the first in the string)
 *  @return true iff (c) is a special regex char.
 */
bool IsRegexToken(char c, bool isFirstCharInString);

/** Given a regular expression, makes it case insensitive by
 *  replacing every occurance of a letter with a upper-lower combo,
 *  e.g. Hello -> [Hh][Ee][Ll][Ll][Oo]
 *  @param str a string to check for letters, and possibly modify to make case-insensitive
 *  @return true iff anything was changed, false if no changes were necessary.
 */
bool MakeRegexCaseInsensitive(String & str);

};  // end namespace muscle


#endif
