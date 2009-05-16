/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */  

#include "reflector/AbstractReflectSession.h"
#include "reflector/AbstractSessionIOPolicy.h"
#include "reflector/ReflectServer.h"
#include "dataio/TCPSocketDataIO.h"
#include "iogateway/MessageIOGateway.h"
#include "system/Mutex.h"
#include "system/SetupSystem.h"

namespace muscle {

static uint32 _sessionIDCounter = 0L;
static uint32 _factoryIDCounter = 0L;

static uint32 GetNextGlobalID(uint32 & counter)
{
   uint32 ret;

   Mutex * ml = GetGlobalMuscleLock();
   MASSERT(ml, "Please instantiate a CompleteSetupSystem object on the stack before creating any session or session-factory objects (at beginning of main() is preferred)\n");

   if (ml->Lock() == B_NO_ERROR) 
   {
      ret = counter++;
      ml->Unlock();
   }
   else
   {
      LogTime(MUSCLE_LOG_CRITICALERROR, "Couldn't lock global muscle lock while assigning new ID!!?!\n");
      ret = counter++;  // do it anyway, I guess
   }
   return ret;
}

ReflectSessionFactory :: ReflectSessionFactory()
{
   TCHECKPOINT;
   _id = GetNextGlobalID(_factoryIDCounter);
}

status_t ProxySessionFactory :: AttachedToServer()
{
   if (ReflectSessionFactory::AttachedToServer() != B_NO_ERROR) return B_ERROR;

   status_t ret = B_NO_ERROR;
   if (_slaveRef())
   {
      _slaveRef()->SetOwner(GetOwner());
      ret = _slaveRef()->AttachedToServer();
      if (ret != B_NO_ERROR) _slaveRef()->SetOwner(NULL);
   }
   return ret;
}

void ProxySessionFactory :: AboutToDetachFromServer()
{
   if (_slaveRef())
   {
      _slaveRef()->AboutToDetachFromServer();
      _slaveRef()->SetOwner(NULL);
   }
   ReflectSessionFactory::AboutToDetachFromServer();
}

AbstractReflectSession ::
AbstractReflectSession() : _sessionID(GetNextGlobalID(_sessionIDCounter)), _connectingAsync(false), _isConnected(false), _lastByteOutputAt(0), _maxInputChunk(MUSCLE_NO_LIMIT), _maxOutputChunk(MUSCLE_NO_LIMIT), _outputStallLimit(MUSCLE_TIME_NEVER), _autoReconnectDelay(MUSCLE_TIME_NEVER), _reconnectTime(MUSCLE_TIME_NEVER), _wasConnected(false)
{
   char buf[64]; sprintf(buf, UINT32_FORMAT_SPEC, _sessionID);
   _idString = buf;
}

AbstractReflectSession ::
~AbstractReflectSession() 
{
   TCHECKPOINT;
   SetInputPolicy(AbstractSessionIOPolicyRef());   // make sure the input policy knows we're going away
   SetOutputPolicy(AbstractSessionIOPolicyRef());  // make sure the output policy knows we're going away
}

const String &
AbstractReflectSession ::
GetHostName() const 
{
   MASSERT(IsAttachedToServer(), "Can't call GetHostName() while not attached to the server");
   return _hostName;
}

uint16
AbstractReflectSession ::
GetPort() const 
{
   MASSERT(IsAttachedToServer(), "Can't call GetPort() while not attached to the server");
   return _ipAddressAndPort.GetPort();
}

const ip_address &
AbstractReflectSession ::
GetLocalInterfaceAddress() const 
{
   MASSERT(IsAttachedToServer(), "Can't call LocalInterfaceAddress() while not attached to the server");
   return _ipAddressAndPort.GetIPAddress();
}

status_t 
AbstractReflectSession ::
AddOutgoingMessage(const MessageRef & ref) 
{
   MASSERT(IsAttachedToServer(), "Can't call AddOutgoingMessage() while not attached to the server");
   return (_gateway()) ? _gateway()->AddOutgoingMessage(ref) : B_ERROR;
}

status_t
AbstractReflectSession ::
Reconnect()
{
   TCHECKPOINT;

   MASSERT(IsAttachedToServer(), "Can't call Reconnect() while not attached to the server");
   if (_gateway())
   {
      _gateway()->SetDataIO(DataIORef());  // get rid of any existing socket first
      _gateway()->Reset();                 // set gateway back to its virgin state
   }

   _isConnected = _wasConnected = _connectingAsync = false;

   bool doTCPConnect = (_asyncConnectDest.GetIPAddress() != invalidIP);
   bool isReady = false;
   ConstSocketRef sock = doTCPConnect ? ConnectAsync(_asyncConnectDest.GetIPAddress(), _asyncConnectDest.GetPort(), isReady) : CreateDefaultSocket();

   // FogBugz #5256:  If ConnectAsync() fails, we want to act as if it succeeded, so that the calling
   //                 code still uses its normal asynchronous-connect-failure code path.  That way the
   //                 caller doesn't have to worry about synchronous failure as a separate case.
   if ((doTCPConnect)&&(sock() == NULL))
   {
      ConstSocketRef tempSockRef;  // tempSockRef represents the closed remote end of the failed connection and is intentionally closed ASAP
      if (CreateConnectedSocketPair(sock, tempSockRef) == B_NO_ERROR) doTCPConnect = false;
   }

   if (sock())
   {
      DataIORef io = CreateDataIO(sock);
      if (io())
      {
         if (_gateway() == NULL)
         {
            _gateway = CreateGateway();
            if (_gateway() == NULL) return B_ERROR;
         }
         _gateway()->SetDataIO(io);
         if (isReady) 
         {
            _isConnected = _wasConnected = true;
            _connectingAsync = false;
            AsyncConnectCompleted();
         }
         else 
         {
            _isConnected     = false;
            _connectingAsync = doTCPConnect;
         }
         _scratchReconnected = true;   // tells ReflectServer not to shut down our new IO!
         return B_NO_ERROR;
      }
   }
   return B_ERROR;
}

ConstSocketRef 
AbstractReflectSession :: 
CreateDefaultSocket()
{
   return ConstSocketRef();  // NULL Ref means run clientless by default
}

DataIORef
AbstractReflectSession ::
CreateDataIO(const ConstSocketRef & socket)
{
   DataIORef dio(newnothrow TCPSocketDataIO(socket, false));
   if (dio() == NULL) WARN_OUT_OF_MEMORY;
   return dio;
}

AbstractMessageIOGatewayRef
AbstractReflectSession ::
CreateGateway()
{
   AbstractMessageIOGateway * gw = newnothrow MessageIOGateway();
   if (gw == NULL) WARN_OUT_OF_MEMORY;
   return AbstractMessageIOGatewayRef(gw);
}

bool
AbstractReflectSession ::
ClientConnectionClosed()
{
   if (_autoReconnectDelay == MUSCLE_TIME_NEVER) return true;  // true == okay to remove this session
   else
   {
      if (_wasConnected) LogTime(MUSCLE_LOG_DEBUG, "%s:  Connection severed, will auto-reconnect in " UINT64_FORMAT_SPEC "ms\n", GetSessionDescriptionString()(), _autoReconnectDelay);
      PlanForReconnect();
      return false;
   }
}

void
AbstractReflectSession ::
BroadcastToAllSessions(const MessageRef & msgRef, void * userData, bool toSelf)
{
   TCHECKPOINT;

   HashtableIterator<const String *, AbstractReflectSessionRef> iter(GetSessions());
   AbstractReflectSessionRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * session = next->GetItemPointer();
      if ((session)&&((toSelf)||(session != this))) session->MessageReceivedFromSession(*this, msgRef, userData);
   }
}

void
AbstractReflectSession ::
BroadcastToAllFactories(const MessageRef & msgRef, void * userData)
{
   TCHECKPOINT;

   HashtableIterator<IPAddressAndPort, ReflectSessionFactoryRef> iter(GetFactories());
   ReflectSessionFactoryRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      ReflectSessionFactory * factory = next->GetItemPointer();
      if (factory) factory->MessageReceivedFromSession(*this, msgRef, userData);
   }
}

void AbstractReflectSession :: AsyncConnectCompleted()
{
   // These sets are mostly redundant, since typically ReflectServer sets these variables
   // directly before calling AsyncConnectCompleted()... but if third party code is calling
   // AsyncConnectCompleted(), then we might as well make sure they are set from that context
   // also.
   _isConnected = _wasConnected = true;
}

void AbstractReflectSession :: SetInputPolicy(const AbstractSessionIOPolicyRef & newRef) {SetPolicyAux(_inputPolicyRef, _maxInputChunk, newRef, true);}
void AbstractReflectSession :: SetOutputPolicy(const AbstractSessionIOPolicyRef & newRef) {SetPolicyAux(_outputPolicyRef, _maxOutputChunk, newRef, true);}
void AbstractReflectSession :: SetPolicyAux(AbstractSessionIOPolicyRef & myRef, uint32 & chunk, const AbstractSessionIOPolicyRef & newRef, bool isInput)
{
   TCHECKPOINT;

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
ReplaceSession(const AbstractReflectSessionRef & replaceMeWithThis)
{
   MASSERT(IsAttachedToServer(), "Can't call ReplaceSession() while not attached to the server");
   return GetOwner()->ReplaceSession(replaceMeWithThis, this);
}

void
AbstractReflectSession ::
EndSession()
{
   MASSERT(IsAttachedToServer(), "Can't call EndSession() while not attached to the server");
   GetOwner()->EndSession(this);
}

bool
AbstractReflectSession ::
DisconnectSession()
{
   MASSERT(IsAttachedToServer(), "Can't call DisconnectSession() while not attached to the server");
   return GetOwner()->DisconnectSession(this);
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
   uint16 port = _ipAddressAndPort.GetPort();

   String ret = GetTypeName();
   ret += " ";
   ret += GetSessionIDString();
   ret += (port>0)?" from [":" to [";
   ret += _hostName;
   char buf[64]; sprintf(buf, ":%u]", (port>0)?port:_asyncConnectDest.GetPort()); ret += buf;
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
   TCHECKPOINT;

   return _gateway() ? _gateway()->DoOutput(maxBytes) : 0;
}

void
ReflectSessionFactory ::
BroadcastToAllSessions(const MessageRef & msgRef, void * userData)
{
   TCHECKPOINT;

   HashtableIterator<const String *, AbstractReflectSessionRef> iter(GetSessions());
   AbstractReflectSessionRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      AbstractReflectSession * session = next->GetItemPointer();
      if (session) session->MessageReceivedFromFactory(*this, msgRef, userData);
   }
}

void
ReflectSessionFactory ::
BroadcastToAllFactories(const MessageRef & msgRef, void * userData, bool toSelf)
{
   TCHECKPOINT;

   HashtableIterator<IPAddressAndPort, ReflectSessionFactoryRef> iter(GetFactories());
   ReflectSessionFactoryRef * next;
   while((next = iter.GetNextValue()) != NULL)
   {
      ReflectSessionFactory * factory = next->GetItemPointer();
      if ((factory)&&((toSelf)||(factory != this))) factory->MessageReceivedFromFactory(*this, msgRef, userData);
   }
}

const ConstSocketRef &
AbstractReflectSession ::
GetSessionSelectSocket() const
{
   const AbstractMessageIOGateway * gw = GetGateway()();
   if (gw)
   {
      const DataIORef & io = gw->GetDataIO();
      if (io()) return io()->GetSelectSocket();
   }
   return GetNullSocket();
}

void 
AbstractReflectSession ::
PlanForReconnect()
{
   _reconnectTime = (_autoReconnectDelay == MUSCLE_TIME_NEVER) ? MUSCLE_TIME_NEVER : (GetRunTime64()+_autoReconnectDelay);
   InvalidatePulseTime();
}

uint64 
AbstractReflectSession :: 
GetPulseTime(const PulseArgs &)
{
   return _reconnectTime;
}

void
AbstractReflectSession :: 
Pulse(const PulseArgs & args)
{
   PulseNode::Pulse(args);
   if (args.GetCallbackTime() >= _reconnectTime)
   {
      if (_autoReconnectDelay == MUSCLE_TIME_NEVER) _reconnectTime = MUSCLE_TIME_NEVER;
      else
      {
         // FogBugz #3810
         if (_wasConnected) LogTime(MUSCLE_LOG_DEBUG, "%s is attempting to auto-reconnect...\n", GetSessionDescriptionString()());
         _reconnectTime = MUSCLE_TIME_NEVER;
         if (Reconnect() != B_NO_ERROR)
         {
            LogTime(MUSCLE_LOG_DEBUG, "%s: Couldn't auto-reconnect, will try again later...\n", GetSessionDescriptionString()());
            PlanForReconnect();  // okay, we'll try again later!
         }
      }
   }
}


}; // end namespace muscle
