#include "looper.h"

WLooper::WLooper(WHandler * handler, WLooper * reply)
	: fHandler(handler), fReply(reply)
{
	fQueue.setAutoDelete(true);
}

void
WLooper::PostEvent(Message * msg)
{
	fLock.lock();
	fQueue.enqueue(msg);
	fLock.unlock();
	fQueueWait.wakeAll();
}

void
WLooper::PostEvent(WLooper * dst, Message * msg)
{
	dst->PostEvent(msg);
}

void
WLooper::run()
{
	if (fReply)
	{
		Message * init = new Message(WLooper::Init);
		if (init)
			fReply->PostEvent(init);
	}
	Message * msg = NULL;
	bool isEmpty = true;

	while (1)
	{
		fQueueWait.wait();
		fLock.lock();
		isEmpty = fQueue.isEmpty();
		fLock.unlock();
		while (!isEmpty)
		{
			fLock.lock();
			msg = fQueue.head();	// only lock when need to access the queue
			fLock.unlock();
			if (msg)
			{
				if (msg->what == WLooper::Quit)
				{
					// ah! they want us to quit, well alright
					fLock.lock();
					fQueue.clear();
					fLock.unlock();
					if (fReply)
						fReply->PostEvent(WLooper::Done);
					return;	// break out of forever loop
				}
				else
				{
					if (fHandler)
						fHandler->Event(msg);
					fLock.lock();
					fQueue.remove();
					fLock.unlock();
				}
			}
            fLock.lock();
			isEmpty = fQueue.isEmpty();
			fLock.unlock();
		}
	}
}
