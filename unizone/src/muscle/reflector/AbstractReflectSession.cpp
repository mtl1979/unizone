/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */  

#include "reflector/AbstractReflectSession.h"
#include "reflector/AbstractSessionIOPolicy.h"
#include "reflector/ReflectServer.h"
#include "dataio/TCPSocketDataIO.h"
#include "iogateway/MessageIOGateway.h"
#include "system/Mutex.h"
#include "system/SetupSystem.h"

namespace muscle {

static uint32 _idCounter = 0L;

AbstractReflectSession ::
AbstractReflectSession() : _port(0), _connectingAsync(false), _asyncConnectIP(0), _asyncConnectPort(0), _lastByteOutputAt(0), _maxInputChunk(MUSCLE_NO_LIMIT), _maxOutputChunk(MUSCLE_NO_LIMIT), _outputStallLimit(MUSCLE_TIME_NEVER)
{
   Mutex * ml = GetGlobalMuscleLock();
   MASSERT(ml, "Please instantiate a CompleteSetupSystem object on the stack before creating any session objects (at beginning of main() is preferred)") 
   if (ml->Lock() == B_NO_ERROR) 
   {
      sprintf(_idString, "%lu", _idCounter++);  
      ml->Unlock();
   }
   else
   {
      LogTime(MUSCLE_LOG_ERROR, "Couldn't lock counter lock for new session!!?!\n");
      sprintf(_idString, "%lu", _idCounter++);   // do it anyway, I guess
   }
}

AbstractReflectSession ::
~AbstractReflectSession() 
{
   SetInputPolicy(PolicyRef());   // make sure the input policy knows we're going away
   SetOutputPolicy(PolicyRef());  // make sure the output policy knows we're going away
}

const char * 
AbstractReflectSession ::
GetHostName() const 
{
   MASSERT(IsAttachedToServer(), "Can't call GetHostName() while not attached to the server");
   return _hostName.Cstr();
}

uint16
AbstractReflectSession ::
GetPort() const 
{
   MASSERT(IsAttachedToServer(), "Can't call GetPort() while not attached to the server");
   return _port;
}

const char * 
AbstractReflectSession ::
GetSessionIDString() const 
{
   return _idString;
}

status_t 
AbstractReflectSession ::
AddOutgoingMessage(MessageRef ref) 
{
   MASSERT(IsAttachedToServer(), "Can't call AddOutgoingMessage() while not attached to the server");
   return (_gateway()) ? _gateway()->AddOutgoingMessage(ref) : B_ERROR;
}

status_t
AbstractReflectSession ::
Reconnect()
{
   MASSERT(IsAttachedToServer(), "Can't call Reconnect() while not attached to the server");
   if ((_asyncConnectIP > 0)&&(_gateway()))
   {
      _gateway()->SetDataIO(DataIORef());  // get rid of any existing socket first
      _gateway()->Reset();                 // set gateway back to its virgin state
      _connectingAsync = false;  // paranoia

      bool isReady;
      int socket = ConnectAsync(_asyncConnectIP, _asyncConnectPort, isReady);
      if (socket >= 0)
      {
         DataIO * io = CreateDataIO(socket);
         if (io)
         {
            _gateway()->SetDataIO(DataIORef(io, NULL));
            if (isReady) AsyncConnectCompleted();
                    else _connectingAsync = true;
            return B_NO_ERROR;
         }
         else closesocket(socket);
      }
   }
   return B_ERROR;
}

AbstractMessageIOGateway * 
AbstractReflectSession ::
CreateGateway()
{
   AbstractMessageIOGateway * gw = newnothrow MessageIOGateway();
   if (gw == NULL) WARN_OUT_OF_MEMORY;
   return gw;
}

DataIO *
AbstractReflectSession ::
CreateDataIO(int socket)
{
   DataIO * dio = newnothrow TCPSocketDataIO(socket, false);
   if (dio == NULL) WARN_OUT_OF_MEMORY;
   return dio;
}

bool
AbstractReflectSession ::
ClientConnectionClosed()
{
   return true;  // true == okay to remove this session
}

void
AbstractReflectSession ::
BroadcastToAllSessions(MessageRef msgRef, void * userData, bool toSelf)
{
   HashtableIterator<const char *, AbstractReflectSessionRef> iter = GetSessions();
   AbstractReflectSessionRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * session = next->GetItemPointer();
      if ((session)&&((toSelf)||(session != this))) session->MessageReceivedFromSession(*this, msgRef, userData);
   }
}

void
AbstractReflectSession ::
BroadcastToAllFactories(MessageRef msgRef, void * userData)
{
   HashtableIterator<uint16, ReflectSessionFactoryRef> iter = GetFactories();
   ReflectSessionFactoryRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      ReflectSessionFactory * factory = next->GetItemPointer();
      if (factory) factory->MessageReceivedFromSession(*this, msgRef, userData);
   }
}

void AbstractReflectSession :: AsyncConnectCompleted()
{
   // empty
}

void AbstractReflectSession :: SetInputPolicy(PolicyRef newRef) {SetPolicyAux(_inputPolicyRef, _maxInputChunk, newRef, true);}
void AbstractReflectSession :: SetOutputPolicy(PolicyRef newRef) {SetPolicyAux(_outputPolicyRef, _maxOutputChunk, newRef, true);}
void AbstractReflectSession :: SetPolicyAux(PolicyRef & myRef, uint32 & chunk, PolicyRef newRef, bool isInput)
{
   if (newRef != myRef)
   {
      PolicyHolder ph(this, isInput);
      if (myRef()) myRef()->PolicyHolderRemoved(ph);
      myRef = newRef;
      chunk = myRef() ? 0 : MUSCLE_NO_LIMIT;  // sensible default to use until my policy gets its say about what we should do
      if (myRef()) myRef()->PolicyHolderAdded(ph);
   }
}

status_t
AbstractReflectSession ::
ReplaceSession(AbstractReflectSessionRef replaceMeWithThis)
{
   MASSERT(IsAttachedToServer(), "Can't call ReplaceSession() while not attached to the server");
   return _owner->ReplaceSession(replaceMeWithThis, this);
}

void
AbstractReflectSession ::
EndSession()
{
   MASSERT(IsAttachedToServer(), "Can't call EndSession() while not attached to the server");
   _owner->EndSession(this);
}

void
AbstractReflectSession ::
DisconnectSession()
{
   MASSERT(IsAttachedToServer(), "Can't call DisconnectSession() while not attached to the server");
   _owner->DisconnectSession(this);
}

String
AbstractReflectSession ::
GetDefaultHostName() const
{
   return "<unknown>";
}

String
AbstractReflectSession ::
GetSessionDescriptionString() const
{
   String ret = GetTypeName();
   ret += " ";
   ret += GetSessionIDString();
   ret += (_port>0)?" from [":" to [";
   ret += _hostName;
   ret += ':';
   char buf[64]; sprintf(buf, "%u]", (_port>0)?_port:_asyncConnectPort); ret += buf;
   return ret;
}


bool
AbstractReflectSession ::
HasBytesToOutput() const
{
   return _gateway() ? _gateway()->HasBytesToOutput() : false;
}

bool 
AbstractReflectSession :: 
IsReadyForInput() const
{
   return _gateway() ? _gateway()->IsReadyForInput() : false;
}

int32
AbstractReflectSession ::
DoInput(AbstractGatewayMessageReceiver & receiver, uint32 maxBytes)
{
   return _gateway() ? _gateway()->DoInput(receiver, maxBytes) : 0;
}

int32 
AbstractReflectSession ::
DoOutput(uint32 maxBytes)
{
   return _gateway() ? _gateway()->DoOutput(maxBytes) : 0;
}

ReflectSessionFactory ::
ReflectSessionFactory() : _port(0), _socket(-1)
{
   // empty
}

ReflectSessionFactory ::
~ReflectSessionFactory()
{
   // empty
}

void
ReflectSessionFactory ::
BroadcastToAllSessions(MessageRef msgRef, void * userData)
{
   HashtableIterator<const char *, AbstractReflectSessionRef> iter = GetSessions();
   AbstractReflectSessionRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * session = next->GetItemPointer();
      if (session) session->MessageReceivedFromFactory(*this, msgRef, userData);
   }
}

void
ReflectSessionFactory ::
BroadcastToAllFactories(MessageRef msgRef, void * userData, bool toSelf)
{
   HashtableIterator<uint16, ReflectSessionFactoryRef> iter = GetFactories();
   ReflectSessionFactoryRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      ReflectSessionFactory * factory = next->GetItemPointer();
      if ((factory)&&((toSelf)||(factory != this))) factory->MessageReceivedFromFactory(*this, msgRef, userData);
   }
}

};  // end namespace muscle
