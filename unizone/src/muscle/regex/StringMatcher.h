/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleStringMatcher_h
#define MuscleStringMatcher_h

#include <sys/types.h>
#include "util/RefCount.h"

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

class String;

////////////////////////////////////////////////////////////////////////////
//
// NOTE:  This class is based on the psStringMatcher v1.3 class 
//        developed by Lars Jørgen Aas <larsa@tihlde.hist.no> for the
//        Prodigal Software File Requester.  Used by permission.
//
////////////////////////////////////////////////////////////////////////////


/** A utility class for doing globbing or regular expression matching.  (A thin wrapper around the C regex calls) */
class StringMatcher : public RefCountable
{
public:
   /** Default Constructor. */
   StringMatcher();

   /** A constructor that sets the given expression.  See SetPattern() for argument semantics. */
   StringMatcher(const char * matchString, bool isSimpleFormat = true);
    
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
   status_t SetPattern(const char * expression, bool isSimpleFormat=true);
    
   /** Returns true iff (string) is matched by the current expression.
    * @param string a string to match against using our current expression.
    * @return true iff (string) matches, false otherwise.
    */
   bool Match(const char * const string) const;
    
   /** If set true, Match() will return the logical opposite of what
     * it would otherwise return; e.g. it will return true only when
     * the given string doesn't match the pattern.
     * Default state is false.  Note that this flag can also be set
     * or unset by calling SetPattern(..., true).
     */
   void SetNegate(bool negate) {_negate = negate;}

   /** Returns the current state of our negate flag. */
   bool IsNegate() const {return _negate;}

private:
   bool _regExpValid;
   bool _negate;
   regex_t _regExp;
   uint32 _rangeMin;
   uint32 _rangeMax;   
}; 

// Some regular expression utility functions

/** Puts a backslash in front of any char in (str) that is "special" to the regex pattern matching.
 *  @param str The string to check for special regex chars and possibly modify to escape them.
 */
void EscapeRegexTokens(String & str);

/** Returns true iff any "special" chars are found in (str).
 *  @param str The string to check for special regex chars.
 *  @return True iff any special regex chars were found in (str).
 */
bool HasRegexTokens(const char * str);

/** Returns true iff (c) is a regular expression "special" char.
 *  @param c an ASCII char
 *  @param isFirstCharInString true iff (c) is the first char in a pattern string.
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
