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
		ref()->SetInputPolicy(AbstractSessionIOPolicyRef(new RateLimitSessionIOPolicy(fLimit)));
	}
	return ref;
}

