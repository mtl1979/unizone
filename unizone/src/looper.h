#ifndef LOOPER_H
#define LOOPER_H

#include <qthread.h>
#include <qqueue.h>
#include <message/Message.h>
using namespace muscle;

// All classes that can handle events MUST derive form this simple class
class WHandler
{
public:
	virtual ~WHandler() {}

	// reimplement THIS method to handle all events that get to you.
	// this uses MUSCLE's Message for events
	// WLooper will post to this method
	virtual void Event(Message *) = 0;
};

// My attempt to resurrect the famous BLooper
class WLooper : public QThread
{
public:
	WLooper(WHandler * handler, WLooper * reply = NULL);
	virtual ~WLooper() {}

	enum
	{
		Init = 'lOpI',	// the looper sends this message the other looper passed in the constructor after initialization is successful (after it is start()'ed)
		Quit,	// send this message to the looper and it will quit
		Done	// you are free to delete the handler after your looper get's this message
	};

	void PostEvent(int what) { PostEvent(new Message(what)); }
	// Message is owned by WLooper once it is passed to this method
	void PostEvent(Message *);
	static void PostEvent(WLooper *, Message *);		// ah... interthread communication :)
														// this is identical to calling looper->PostEvent(Message*)

protected:
	virtual void run();

private:
	WHandler * fHandler;
	WLooper * fReply;	// we send our messages here
	QMutex fLock;
	QWaitCondition fQueueWait;
	QQueue<Message> fQueue;
};

#endif
