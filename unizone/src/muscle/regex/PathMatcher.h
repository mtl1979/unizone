/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MusclePathMatcher_h
#define MusclePathMatcher_h

#include "util/Queue.h"
#include "regex/StringMatcher.h"
#include "message/Message.h"

namespace muscle {

/** Type for a reference to a StringMatcher object. */
typedef Ref<StringMatcher> StringMatcherRef;
 
/** Just a reference-countable list of references to StringMatcher objects */
class StringMatcherQueue : public Queue<StringMatcherRef>, public RefCountable
{
public:
   /** Constructor.
     * @param initialSize passed on to the Queue constructor.
     */
   StringMatcherQueue(uint32 initialSize = SMALL_QUEUE_SIZE) : Queue<StringMatcherRef>(initialSize) {/* empty */}
};

/** Type for a reference to a queue of StringMatcher objects. */
typedef Ref<StringMatcherQueue> StringMatcherQueueRef;

/** This class is used to do efficient regex-pattern-matching of one or more query strings (e.g. "/.*./.*./j*remy/fries*") 
  * against various path strings (e.g. "/12.18.240.15/123/jeremy/friesner").  A given path string is said to 'match'
  * if it matches at least one of the query strings added to this object.
  */
class PathMatcher : public RefCountable
{
public:
   /** Default Constructor.  Creates a PathMatcher with no query strings in it */
   PathMatcher();

   /** Destructor */
   ~PathMatcher();

   /** Removes all path nodes from this object */
   void Clear();

   /** Parses the given query string (e.g. "/12.18.240.15/1234/beshare/j*") to this PathMatcher's set of query strings.
    *  @param wildpath a string of form "/x/y/z/...", representing a pattern-matching function.
    *  @param optStringMatcherPool     If non-NULL, an ObjectPool to recycle StringMatcher objects.  (optional, for optimization only)
    *  @param optQueuePool             If non-NULL, an ObjectPool to recycle Queue<StringMatcherQueueRef> objects.  (optional, for optimization only)
    *  @return B_NO_ERROR on success, B_ERROR if out of memory.
    */
   status_t AddPathString(const String & path, StringMatcherRef::ItemPool * optStringMatcherPool = NULL, StringMatcherQueueRef::ItemPool * optQueuePool = NULL);

   /** Adds all of (matcher)'s StringMatchers to this matcher */
   void AddPathsFromMatcher(const PathMatcher & matcher);

   /** Adds zero or more wild paths to this matcher based on the contents of a string field in a Message.
    *  @param fieldName the name of a string field to look for node path expressions in.
    *  @param msg the Message to look for node path expressions in
    *  @param optPrependIfNoLeadingSlash If non-NULL, a '/' and this string will be prepended to any found path string that doesn't start with a '/' character.
    *  @param optStringMatcherPool     If non-NULL, an ObjectPool to recycle StringMatcher objects.  (optional, for optimization only)
    *  @param optQueuePool             If non-NULL, an ObjectPool to recycle Queue<StringMatcherQueueRef> objects.  (optional, for optimization only)
    *  @return B_NO_ERROR on success, or B_ERROR if out of memory.
    */
   status_t AddPathsFromMessage(const char * fieldName, const Message & msg, const char * optPrependIfNoLeadingSlash, StringMatcherRef::ItemPool * optStringMatcherPool = NULL, StringMatcherQueueRef::ItemPool * optQueuePool = NULL);

   /** Removes the given path string and its associated StringMatchers from this matcher.
    *  @param wildpath the path string to remove
    *  @return B_NO_ERROR if the given path string was found and removed, or B_ERROR if it couldn't be found.
    */
   status_t RemovePathString(const String & wildpath);

   /**
    * Returns true iff the given fully qualified path string matches our query.
    * @param path the path string to check to see if it matches
    */
   bool MatchesPath(const char * path) const;
    
   /** Returns true iff the given path string is in our string set */
   bool ContainsPathString(const String & w) const;

   /**
    * Utility method.
    * If (w) starts with '/', this method will remove the slash.
    * If it doesn't start with a '/', and (optPrepend) is non-NULL,
    * this method will prepend (optPrepend+"/") to (w).
    * @param w A string to inspect, and potentially modify.
    * @param optPrepend If non-NULL, a string to potentially prepend to (w)
    */
   void AdjustStringPrefix(String & w, const char * optPrepend) const;

   /** Returns the number of paths present in this PathMatcher.  */
   uint32 GetNumPaths() const {return _parsers.GetNumItems();}

   /** Returns the nth parser-queue 
    *  @param which Index of the string you want.
    *  @return the matcher-queue of the given path string, or NULL on failure (index out of range)
    */
   const StringMatcherQueue * GetParserQueue(uint32 which) const {return (which < _parsers.GetNumItems()) ? _parsers[which].GetItemPointer() : NULL;}

   /** Returns the nth path string.
    *  @param which Index of the string you want.  MUST be between 0 and GetNumPaths()-1, inclusive! 
    */
   const String & GetPathString(uint32 which) const {return _pathStrings[which];}

private:
   Queue<StringMatcherQueueRef> _parsers;
   Queue<String> _pathStrings;
};


/** Returns a pointer into (path) after the (depth)'th '/' char
 *  @param depth the depth in the path to search for (0 == root, 1 == first level, 2 == second, etc.)
 *  @param path the path string (e.g. "/x/y/z/...") to search through
 *  @result a pointer into (path), or NULL on failure.
 */
const char * GetPathClause(int depth, const char * path);

/** As above, but returns a String object for just the given clause,
 *  instead of the entire remainder of the string.  This version is 
 *  somewhat less efficient, but easier to user.
 *  @param depth the depth in the path to search for (0 == root, 1 == first level, 2 == second, etc.)
 *  @param path the path string (e.g. "/x/y/z/...") to search through
 *  @result The string that is the (nth) item in the path, or "" if the depth is invalid.
 */
String GetPathClauseString(int depth, const char * path);

/** Returns the depth of the given path.  (path) should start with a '/'
 *  @param path a string starting with '/'
 *  @return the number of clauses in (path).  (a.k.a the 'depth' of (path))
 */
int GetPathDepth(const char * path);

};  // end namespace muscle

#endif
