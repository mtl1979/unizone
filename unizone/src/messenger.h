#ifndef UNIMESSENGER_H
#define UNIMESSENGER_H

#include <qobject.h>

#include "system/MessageTransceiverThread.h"
using namespace muscle;

class WMessenger : public QObject
{
	Q_OBJECT
public:
	WMessenger(QObject * parent, const char * name)
		: QObject(parent, name) { if (!name) setName("WMessenger"); }
	~WMessenger() { }
	virtual void SignalOwner() { }
};

class WMessengerThread : public MessageTransceiverThread
{
public:
	WMessengerThread(WMessenger * parent = NULL) { fParent = parent; }
	~WMessengerThread() {}
protected:
	void SignalOwner() { if (fParent) fParent->SignalOwner(); }
private:
	WMessenger *fParent;
};

#endif
