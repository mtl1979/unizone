#ifndef UNIMESSENGER_H
#define UNIMESSENGER_H
#include "system/MessageTransceiverThread.h"
using namespace muscle;

class WMessenger
{
public:
	virtual void SignalOwner() { };
};

class WMessengerThread : public MessageTransceiverThread
{
public:
	WMessengerThread(WMessenger * parent = NULL) { fParent = parent; }
	~WMessengerThread() {};
protected:
	void SignalOwner() { if (fParent) fParent->SignalOwner(); };
private:
	WMessenger *fParent;
};

#endif
