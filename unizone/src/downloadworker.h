#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include "system/MessageTransceiverThread.h"

using namespace muscle;

// subclass ThreadWorkerSessionFactory to do throttling
class WDownloadThreadWorkerSessionFactory : public ThreadWorkerSessionFactory
{
public:
	WDownloadThreadWorkerSessionFactory(int limit);
	AbstractReflectSession * CreateSession(const String &);
	
private:
	int fLimit;
};

#endif