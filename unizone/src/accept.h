#ifndef ACCEPT_H
#define ACCEPT_H

#include "system/AcceptSocketsThread.h"
#include "util/SocketHolder.h"
using namespace muscle;

#include <qobject.h>
#include <qevent.h>

#include "messenger.h"

// #define DEFAULT_LISTEN_PORT 7000 
// NOTE: Deprecated by WSettings::GetBasePort()

#define LISTEN_PORT_RANGE 100

class WAcceptThreadEvent : public QCustomEvent
{
public:
	enum { Type = 'aTeT' };
	
	WAcceptThreadEvent(SocketHolderRef ref) : QCustomEvent(Type) { fRef = ref; }
	virtual ~WAcceptThreadEvent() {}
	
	SocketHolderRef Get() const { return fRef; }
	
private:
	SocketHolderRef fRef;
};

class WAcceptorThread : public AcceptSocketsThread
{
public:
	WAcceptorThread(WMessenger * owner) : fOwner(owner) { }
	~WAcceptorThread() { };
protected:
	void SignalOwner() { if (fOwner) fOwner->SignalOwner(); }
private:
	WMessenger * fOwner;
};

class WAcceptThread : public WMessenger
{
public:
	WAcceptThread(QObject * owner) : WMessenger(owner, "WAcceptThread"), fOwner(owner) {ast = new WAcceptorThread(this);}
	virtual ~WAcceptThread() { delete ast;}

	// Forwarders for AcceptSocketsThread()
	status_t SetPort(uint16 port, uint32 optFrom = 0) { return ast->SetPort(port, optFrom); }
	uint16 GetPort() { return ast->GetPort(); }
	status_t StartInternalThread() { return ast->StartInternalThread(); }
	status_t WaitForInternalThreadToExit() { return ast->WaitForInternalThreadToExit(); }
	void ShutdownInternalThread(bool waitForThread = true) { ast->ShutdownInternalThread(waitForThread); }
	
protected:
	virtual void SignalOwner();
	
private:
	QObject * fOwner;
	WAcceptorThread * ast;
};

#endif
