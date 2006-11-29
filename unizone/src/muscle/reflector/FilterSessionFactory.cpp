/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "reflector/FilterSessionFactory.h"
#include "reflector/StorageReflectConstants.h"

BEGIN_NAMESPACE(muscle);

FilterSessionFactory :: FilterSessionFactory(const ReflectSessionFactoryRef & slaveRef, uint32 msph, uint32 tms) : _slaveRef(slaveRef), _tempLogFor(NULL), _maxSessionsPerHost(msph), _totalMaxSessions(tms)
{
   // empty
}

FilterSessionFactory :: ~FilterSessionFactory()
{
   // empty
}

AbstractReflectSession * FilterSessionFactory:: CreateSession(const String & clientHostIP)
{
   TCHECKPOINT;

   if (GetNumSessions() >= _totalMaxSessions)
   {
      LogTime(MUSCLE_LOG_DEBUG, "Connection from [%s] refused (all %lu sessions slots are in use).\n", clientHostIP(), _totalMaxSessions);
      return NULL;
   }
      
   if (_maxSessionsPerHost != MUSCLE_NO_LIMIT)
   {
      uint32 count = 0;
      AbstractReflectSessionRef next;
      HashtableIterator<const char *, AbstractReflectSessionRef> iter = GetSessions();
      while(iter.GetNextValue(next) == B_NO_ERROR) 
      {
         if ((next())&&(strcmp(next()->GetHostName(), clientHostIP()) == 0)&&(++count >= _maxSessionsPerHost))
         {
            LogTime(MUSCLE_LOG_DEBUG, "Connection from [%s] refused (host already has %lu sessions open).\n", clientHostIP(), _maxSessionsPerHost);
            return NULL;
         }
      }
   }

   AbstractReflectSession * ret = NULL;
   if (_slaveRef())
   {
      // If we have any requires, then this IP must match at least one of them!
      if (_requires.GetNumItems() > 0)
      {
         bool matched = false;
         HashtableIterator<String, StringMatcherRef> iter(_requires);
         StringMatcherRef next;
         while(iter.GetNextValue(next) == B_NO_ERROR)
         {
            if (next()->Match(clientHostIP()))
            {
               matched = true;
               break;
            }
         }
         if (matched == false) 
         {
            LogTime(MUSCLE_LOG_DEBUG, "Connection from [%s] doesn't match any require pattern, access denied.\n", clientHostIP());
            return NULL;
         }
      }

      // This IP must *not* match any of our bans!
      HashtableIterator<String, StringMatcherRef> iter(_bans);
      StringMatcherRef next;
      while(iter.GetNextValue(next) == B_NO_ERROR)
      {
         const String * key = iter.GetNextKey();
         if (next()->Match(clientHostIP()))
         {
            LogTime(MUSCLE_LOG_DEBUG, "Connection from [%s] matches ban pattern [%s], access denied.\n", clientHostIP(), key->Cstr());
            return NULL;
         }
      }

      // Okay, he passes.  We'll let our slave create a session for him.
      ret = _slaveRef()->CreateSession(clientHostIP);
      if (ret) 
      {
         if (_inputPolicyRef())  ret->SetInputPolicy(_inputPolicyRef);
         if (_outputPolicyRef()) ret->SetOutputPolicy(_outputPolicyRef);
      }
   }
   return ret;
}

void FilterSessionFactory :: MessageReceivedFromSession(AbstractReflectSession & from, const MessageRef & msgRef, void *)
{
   TCHECKPOINT;

   const Message * msg = msgRef();
   if (msg)
   {
      _tempLogFor = &from;
      const char * s;
      for (int b=0; (msg->FindString(PR_NAME_KEYS, b, &s) == B_NO_ERROR); b++)
      {
         switch(msg->what)
         {
            case PR_COMMAND_ADDBANS:        PutBanPattern(s);                 break;
            case PR_COMMAND_ADDREQUIRES:    PutRequirePattern(s);             break;
            case PR_COMMAND_REMOVEBANS:     RemoveMatchingBanPatterns(s);     break;                    
            case PR_COMMAND_REMOVEREQUIRES: RemoveMatchingRequirePatterns(s); break;                    
         }
      }
      _tempLogFor = NULL;
   }
}
   
status_t FilterSessionFactory :: PutBanPattern(const char * banPattern)
{
   TCHECKPOINT;

   if (_bans.ContainsKey(banPattern)) return B_NO_ERROR;
   StringMatcherRef newMatcherRef(newnothrow StringMatcher(banPattern));
   if (newMatcherRef())
   {
      if (_bans.Put(banPattern, newMatcherRef) == B_NO_ERROR) 
      {
         if (_tempLogFor) LogTime(MUSCLE_LOG_DEBUG, "Session [%s/%s] is banning [%s] on port %u\n", _tempLogFor->GetHostName(), _tempLogFor->GetSessionIDString(), banPattern, _tempLogFor->GetPort());
         return B_NO_ERROR;
      }
   }
   else WARN_OUT_OF_MEMORY;

   return B_ERROR;
}

status_t FilterSessionFactory :: PutRequirePattern(const char * requirePattern)
{
   TCHECKPOINT;

   if (_requires.ContainsKey(requirePattern)) return B_NO_ERROR;
   StringMatcherRef newMatcherRef(newnothrow StringMatcher(requirePattern));
   if (newMatcherRef())
   {
      if (_requires.Put(requirePattern, newMatcherRef) == B_NO_ERROR) 
      {
         if (_tempLogFor) LogTime(MUSCLE_LOG_DEBUG, "Session [%s/%s] is requiring [%s] on port %u\n", _tempLogFor->GetHostName(), _tempLogFor->GetSessionIDString(), requirePattern, _tempLogFor->GetPort());
         return B_NO_ERROR;
      }
   }
   else WARN_OUT_OF_MEMORY;

   return B_ERROR;
}

status_t FilterSessionFactory :: RemoveBanPattern(const char * banPattern)
{
   if (_bans.Remove(banPattern) == B_NO_ERROR)
   {
      if (_tempLogFor) LogTime(MUSCLE_LOG_DEBUG, "Session [%s/%s] is removing ban [%s] on port %u\n", _tempLogFor->GetHostName(), _tempLogFor->GetSessionIDString(), banPattern, _tempLogFor->GetPort());
      return B_NO_ERROR;
   }
   return B_ERROR;
}

status_t FilterSessionFactory :: RemoveRequirePattern(const char * requirePattern)
{
   if (_requires.Remove(requirePattern) == B_NO_ERROR)
   {
      if (_tempLogFor) LogTime(MUSCLE_LOG_DEBUG, "Session [%s/%s] is removing requirement [%s] on port %u\n", _tempLogFor->GetHostName(), _tempLogFor->GetSessionIDString(), requirePattern, _tempLogFor->GetPort());
      return B_NO_ERROR;
   }
   return B_ERROR;
}

void FilterSessionFactory :: RemoveMatchingBanPatterns(const char * exp)
{
   StringMatcher sm(exp);
   HashtableIterator<String, StringMatcherRef> iter(_bans);
   String next;  // important to make this copy to avoid dangling pointer trouble!
   while(iter.GetNextKey(next) == B_NO_ERROR) if (sm.Match(next())) RemoveBanPattern(next());
}


void FilterSessionFactory :: RemoveMatchingRequirePatterns(const char * exp)
{
   StringMatcher sm(exp);
   HashtableIterator<String, StringMatcherRef> iter(_requires);
   String next;  // important to make this copy to avoid dangling pointer trouble!
   while(iter.GetNextKey(next) == B_NO_ERROR) if (sm.Match(next())) RemoveRequirePattern(next());
}

END_NAMESPACE(muscle);
