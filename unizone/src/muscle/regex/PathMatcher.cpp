/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "regex/PathMatcher.h"
#include "util/StringTokenizer.h"

namespace muscle {

PathMatcher ::
PathMatcher()
{
   // empty
}

PathMatcher ::
~PathMatcher()
{
   // empty
}

void 
PathMatcher :: 
Clear()
{
   _parsers.Clear();
   _pathStrings.Clear();
}

void
PathMatcher ::
AdjustStringPrefix(String & path, const char * optPrepend) const
{
   if (path.Length() > 0)
   {
           if (path[0] == '/') path = path.Substring(1);
      else if (optPrepend)     
      {
         String temp(optPrepend);  // gcc/BeOS chokes on more compact code than this :^P
         temp += '/';
         temp += path;
         path = temp;
      }
   }
}

status_t 
PathMatcher ::
AddPathString(const String & path, StringMatcherRef::ItemPool * optStringMatcherPool, StringMatcherQueueRef::ItemPool * optQueuePool)
{
   if (path.Length() > 0) 
   {
      StringMatcherQueue * newQ = optQueuePool ? optQueuePool->ObtainObject() : newnothrow StringMatcherQueue;
      if (newQ == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
      StringMatcherQueueRef qRef(newQ, optQueuePool);

      String temp;
      int lastSlashPos = -1;
      int slashPos = 0;
      while(slashPos >= 0)
      {
         slashPos = path.IndexOf('/', lastSlashPos+1);
         temp = path.Substring(lastSlashPos+1, (slashPos >= 0) ? slashPos : path.Length());
         StringMatcher * sm = NULL;  // default, same as "*"
         if (strcmp(temp.Cstr(), "*"))
         {
            sm = optStringMatcherPool ? optStringMatcherPool->ObtainObject() : newnothrow StringMatcher;
            if (sm == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}
            sm->SetPattern(temp.Cstr());
         }
         (void) newQ->AddTail(StringMatcherRef(sm, optStringMatcherPool));
         lastSlashPos = slashPos;
      }
      (void) _parsers.AddTail(qRef);
      (void) _pathStrings.AddTail(path);
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t 
PathMatcher ::
RemovePathString(const String & path)
{
   int which = _pathStrings.IndexOf(path);
   if (which >= 0) 
   {
      _parsers.RemoveItemAt(which);
      _pathStrings.RemoveItemAt(which);
      return B_NO_ERROR;
   }
   return B_ERROR;
}

bool
PathMatcher ::
ContainsPathString(const String & w) const
{
   return (_pathStrings.IndexOf(w) >= 0);
}

status_t 
PathMatcher ::
AddPathsFromMessage(const char * fieldName, const Message & msg, const char * prependIfNoLeadingSlash, StringMatcherRef::ItemPool * optStringMatcherPool, StringMatcherQueueRef::ItemPool * optQueuePool)
{
   status_t ret = B_NO_ERROR;
   String str;
   for (int i=0; msg.FindString(fieldName, i, str) == B_NO_ERROR; i++) 
   {
      AdjustStringPrefix(str, prependIfNoLeadingSlash);
      if (AddPathString(str, optStringMatcherPool, optQueuePool) != B_NO_ERROR) ret = B_ERROR;
   }
   return ret;
}

void 
PathMatcher ::
AddPathsFromMatcher(const PathMatcher & matcher)
{
   for (int i=matcher._parsers.GetNumItems()-1; i>=0; i--)
   {
      (void) _pathStrings.AddTail(*matcher._pathStrings.GetItemAt(i));
      (void) _parsers.AddTail(*matcher._parsers.GetItemAt(i));
   }
}

bool
PathMatcher :: 
MatchesPath(const char * path) const
{
   uint32 numClauses = GetPathDepth(path);
   for (int i=_parsers.GetNumItems()-1; i>=0; i--)
   {
      const StringMatcherQueue * nextSubscription = _parsers.GetItemAt(i)->GetItemPointer();
      if ((nextSubscription)&&(nextSubscription->GetNumItems() == numClauses))
      {
         bool matched = true;  // default
         StringTokenizer tok(path, (const char *)"/");
         for (uint32 j=0; j<numClauses; j++)
         {
            const char * nextToken = tok.GetNextToken();
            const StringMatcher * nextMatcher = nextSubscription->GetItemAt(j)->GetItemPointer();
            if ((nextToken == NULL)||((nextMatcher)&&(nextMatcher->Match(nextToken) == false))) 
            {
               matched = false;
               break;
            }
         }
         if (matched) return true;
      }
   }
   return false;
}

// Returns a pointer into (path) after the (depth)'th '/' char
const char * GetPathClause(int depth, const char * path)
{
   for (int i=0; i<depth; i++)
   {
      const char * nextSlash = strchr(path, '/');
      if (nextSlash == NULL) 
      {
         path = NULL;
         break;
      }
      path = nextSlash + 1;
   }
   return path;          
}

String GetPathClauseString(int depth, const char * path)
{
   String ret;
   const char * str = GetPathClause(depth, path);
   if (str)
   {
      ret = str;
      ret = ret.Substring(0, "/");
   }
   return ret;
}


int GetPathDepth(const char * path)
{
   if ((path[0] == '/')&&(path[1] == '\0')) return 0;

   int depth = 0;
   while(true)
   {
      const char * nextSlash = strchr(path, '/');
      if (nextSlash == NULL) break;
      depth++;
      path = nextSlash+1;
   }
   return depth;
}


};  // end namespace muscle
