#ifdef WIN32
#pragma warning (disable: 4512)
#endif

#include "downloadworker.h"
#include "reflector/RateLimitSessionIOPolicy.h"

WDownloadThreadWorkerSessionFactory::WDownloadThreadWorkerSessionFactory(int limit)
{
	fLimit = limit;
}

ThreadWorkerSessionRef
WDownloadThreadWorkerSessionFactory::CreateThreadWorkerSession(const String &, const IPAddressAndPort &)
{
	ThreadWorkerSessionRef ref(new ThreadWorkerSession());
	if (ref() && fLimit != 0)
	{
		ref()->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(fLimit)));
	}
	return ref;
}

