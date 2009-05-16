/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleDumbReflectSession_h
#define MuscleDumbReflectSession_h

#include "reflector/AbstractReflectSession.h"

namespace muscle {

/**
 *  This is a factory class that returns new DumbReflectSession objects.
 */
class DumbReflectSessionFactory : public ReflectSessionFactory
{
public:
   virtual AbstractReflectSessionRef CreateSession(const String & clientAddress, const IPAddressAndPort & factoryInfo);
};

/** This class represents a single TCP connection between a muscled server and a client program.  
 * This class implements a simple "reflect-all-messages-to-all-clients"
 * message forwarding strategy, but may be subclassed to perform more complex message routing
 * logic.
 */
class DumbReflectSession : public AbstractReflectSession
{
public:
   /** Default constructor. */
   DumbReflectSession();

   /** Destructor. */
   virtual ~DumbReflectSession();

   virtual void MessageReceivedFromGateway(const MessageRef & msg, void * userData);

   /** Called when a message is sent to us by another session (possibly this one).  
    *  @param from The session that is sending us the message.
    *  @param msg Reference to the message that we are receiving.
    *  @param userData This is a user value whose semantics are defined by the subclass.
    */
   virtual void MessageReceivedFromSession(AbstractReflectSession & from, const MessageRef & msg, void * userData);

   /**
    * Set this true to enable self-reflection:  if true, messages
    * sent by a given client will be reflected back to that client.
    * Default state is false (i.e. messages only go to everyone else)
    * @param reflectToSelf Whether or not our client's own messages should be bounced back to him.
    */
   void SetReflectToSelf(bool reflectToSelf);

   /** Returns true iff our client's own messages will be bounced back to him.  */
   bool GetReflectToSelf() const;

   /** Returns a human-readable label for our session type:  "Dumb Session" */
   virtual const char * GetTypeName() const {return "Dumb Session";}

private:
   bool _reflectToSelf;
};

}; // end namespace muscle

#endif
