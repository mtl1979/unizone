#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include "system/MessageTransceiverThread.h"

using namespace muscle;

// subclass ThreadWorkerSessionFactory to do throttling
class WDownloadThreadWorkerSessionFactory : public ThreadWorkerSessionFactory
{
public:
	WDownloadThreadWorkerSessionFactory(int limit);
	ThreadWorkerSessionRef CreateThreadWorkerSession(const String &, const IPAddressAndPort &);

private:
	int fLimit;
};

#endif
