/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "reflector/ServerComponent.h"
#include "reflector/ReflectServer.h"

namespace muscle {

ServerComponent ::
ServerComponent() : _owner(NULL)
{
   // empty
}

ServerComponent ::
~ServerComponent() 
{
   MASSERT(_owner == NULL, "ServerComponent deleted while still attached to its ReflectServer!  Maybe you didn't call Cleanup() on the ReflectServer object, or didn't forward an AboutToDetachFromServer() call to your superclass's implementation?");
}

status_t 
ServerComponent ::
AttachedToServer()
{
   return B_NO_ERROR;
}

void
ServerComponent ::
AboutToDetachFromServer()
{
   // empty
}

Message & 
ServerComponent ::
GetCentralState() const 
{
   MASSERT(_owner, "Can't call GetCentralState() while not attached to the server");
   return _owner->GetCentralState();
}

HashtableIterator<const char *, AbstractReflectSessionRef>
ServerComponent ::
GetSessions() const
{
   MASSERT(_owner, "Can't call GetSessions() while not attached to the server");
   return _owner->GetSessions(); 
}

uint32
ServerComponent ::
GetNumSessions() const
{
   MASSERT(_owner, "Can't call GetNumSessions() while not attached to the server");
   return _owner->GetNumSessions(); 
}

AbstractReflectSessionRef 
ServerComponent ::
GetSession(const char * id) const
{
   MASSERT(_owner, "Can't call GetSession() while not attached to the server");
   return _owner->GetSession(id);
}

HashtableIterator<uint16, ReflectSessionFactoryRef> 
ServerComponent ::
GetFactories() const 
{
   MASSERT(_owner, "Can't call GetFactories() while not attached to the server");
   return _owner->GetFactories(); 
}

uint32 
ServerComponent ::
GetNumFactories() const 
{
   MASSERT(_owner, "Can't call GetNumFactories() while not attached to the server");
   return _owner->GetNumFactories(); 
}

ReflectSessionFactoryRef 
ServerComponent ::
GetFactory(uint16 port) const
{
   MASSERT(_owner, "Can't call GetFactory() while not attached to the server");
   return _owner->GetFactory(port);
}


status_t
ServerComponent ::
AddNewSession(AbstractReflectSessionRef ref, int socket)
{
   MASSERT(_owner, "Can't call AddNewSession() while not attached to the server");
   return _owner->AddNewSession(ref, socket);
}

status_t
ServerComponent ::
AddNewConnectSession(AbstractReflectSessionRef ref, uint32 ip, uint16 port)
{
   MASSERT(_owner, "Can't call AddNewConnectSession() while not attached to the server");
   return _owner->AddNewConnectSession(ref, ip, port);
}

void
ServerComponent ::
EndServer()
{
   MASSERT(_owner, "Can't call EndServer() while not attached to the server");
   _owner->EndServer();
}

uint64
ServerComponent ::
GetServerUptime() const
{
   MASSERT(_owner, "Can't call GetServerUptime() while not attached to the server");
   return _owner->GetServerUptime();
}

uint32 
ServerComponent ::
GetNumAvailableBytes() const
{
   MASSERT(_owner, "Can't call GetNumAvailableBytes() while not attached to the server");
   return _owner->GetNumAvailableBytes();
}
 
uint32 
ServerComponent ::
GetMaxNumBytes() const
{
   MASSERT(_owner, "Can't call GetMaxNumBytes() while not attached to the server");
   return _owner->GetMaxNumBytes();
}
 
uint32 
ServerComponent ::
GetNumUsedBytes() const
{
   MASSERT(_owner, "Can't call GetNumUsedBytes() while not attached to the server");
   return _owner->GetNumUsedBytes();
}

status_t
ServerComponent ::
PutAcceptFactory(uint16 port, ReflectSessionFactoryRef factoryRef) 
{
   MASSERT(_owner, "Can't call PutAcceptFactory() while not attached to the server");
   return _owner->PutAcceptFactory(port, factoryRef);
}

status_t
ServerComponent ::
RemoveAcceptFactory(uint16 port)
{
   MASSERT(_owner, "Can't call RemoveAcceptFactory() while not attached to the server");
   return _owner->RemoveAcceptFactory(port);
}

void 
ServerComponent ::
MessageReceivedFromSession(AbstractReflectSession &, MessageRef, void *)
{
   // empty
}

void 
ServerComponent ::
MessageReceivedFromFactory(ReflectSessionFactory &, MessageRef, void * )
{
   // empty
}

};  // end namespace muscle
