/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "reflector/DumbReflectSession.h"

namespace muscle {

// This is a callback function that may be passed to the ServerProcessLoop() function.
// It creates and returns a new DumbReflectSession object.
AbstractReflectSessionRef DumbReflectSessionFactory :: CreateSession(const String &, const IPAddressAndPort &)
{
   AbstractReflectSession * ret = newnothrow DumbReflectSession;
   if (ret == NULL) WARN_OUT_OF_MEMORY;
   return AbstractReflectSessionRef(ret);
}

DumbReflectSession :: 
DumbReflectSession() : _reflectToSelf(false) 
{
   // empty
}

DumbReflectSession :: 
~DumbReflectSession()
{
   // empty
}

// Called when a new message is received from our IO gateway.  We forward it on to all our server-side neighbors.
void 
DumbReflectSession :: 
MessageReceivedFromGateway(const MessageRef & msgRef, void *) 
{
   BroadcastToAllSessions(msgRef, NULL, _reflectToSelf);
}

// Called when a new message is sent to us by one of our server-side neighbors.  We forward it on to our client.
void 
DumbReflectSession :: 
MessageReceivedFromSession(AbstractReflectSession & /*from*/, const MessageRef & msg, void * /*userData*/) 
{
   (void) AddOutgoingMessage(msg);
}

// Set this true to enable self-reflection:  if true, messages
// sent by our client will be reflected back to our client as well as everyone else.
// Default state is false (i.e. messages only go to everyone else)
void 
DumbReflectSession :: 
SetReflectToSelf(bool reflectToSelf) 
{
   _reflectToSelf = reflectToSelf;
}

// Accessor
bool 
DumbReflectSession :: 
GetReflectToSelf() const 
{
   return _reflectToSelf;
}

}; // end namespace muscle
