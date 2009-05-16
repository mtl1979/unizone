/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

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

const Hashtable<const String *, AbstractReflectSessionRef> &
ServerComponent ::
GetSessions() const
{
   MASSERT(_owner, "Can't call GetSessions() while not attached to the server");
   return _owner->GetSessions(); 
}

AbstractReflectSessionRef 
ServerComponent ::
GetSession(uint32 id) const
{
   MASSERT(_owner, "Can't call GetSession() while not attached to the server");
   return _owner->GetSession(id);
}

AbstractReflectSessionRef 
ServerComponent ::
GetSession(const String & id) const
{
   MASSERT(_owner, "Can't call GetSession() while not attached to the server");
   return _owner->GetSession(id);
}

const Hashtable<IPAddressAndPort, ReflectSessionFactoryRef> &
ServerComponent ::
GetFactories() const 
{
   MASSERT(_owner, "Can't call GetFactories() while not attached to the server");
   return _owner->GetFactories(); 
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
AddNewSession(const AbstractReflectSessionRef & ref, const ConstSocketRef & socket)
{
   MASSERT(_owner, "Can't call AddNewSession() while not attached to the server");
   return _owner->AddNewSession(ref, socket);
}

status_t
ServerComponent ::
AddNewConnectSession(const AbstractReflectSessionRef & ref, const ip_address & ip, uint16 port, uint64 autoReconnectDelay)
{
   MASSERT(_owner, "Can't call AddNewConnectSession() while not attached to the server");
   return _owner->AddNewConnectSession(ref, ip, port, autoReconnectDelay);
}

status_t
ServerComponent ::
AddNewDormantConnectSession(const AbstractReflectSessionRef & ref, const ip_address & ip, uint16 port, uint64 autoReconnectDelay)
{
   MASSERT(_owner, "Can't call AddNewDormantConnectSession() while not attached to the server");
   return _owner->AddNewDormantConnectSession(ref, ip, port, autoReconnectDelay);
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

uint64
ServerComponent ::
GetServerSessionID() const
{
   MASSERT(_owner, "Can't call GetServerSessionID() while not attached to the server");
   return _owner->GetServerSessionID();
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
PutAcceptFactory(uint16 port, const ReflectSessionFactoryRef & factoryRef, const ip_address & optInterfaceIP, uint16 * optRetPort)
{
   MASSERT(_owner, "Can't call PutAcceptFactory() while not attached to the server");
   return _owner->PutAcceptFactory(port, factoryRef, optInterfaceIP, optRetPort);
}

status_t
ServerComponent ::
RemoveAcceptFactory(uint16 port, const ip_address & optInterfaceIP)
{
   MASSERT(_owner, "Can't call RemoveAcceptFactory() while not attached to the server");
   return _owner->RemoveAcceptFactory(port, optInterfaceIP);
}

void 
ServerComponent ::
MessageReceivedFromSession(AbstractReflectSession &, const MessageRef &, void *)
{
   // empty
}

void 
ServerComponent ::
MessageReceivedFromFactory(ReflectSessionFactory &, const MessageRef &, void * )
{
   // empty
}

}; // end namespace muscle
