#include "downloadworker.h"
#include "reflector/RateLimitSessionIOPolicy.h"

WDownloadThreadWorkerSessionFactory::WDownloadThreadWorkerSessionFactory(int limit)
{
	fLimit = limit;
}

AbstractReflectSession *
WDownloadThreadWorkerSessionFactory::CreateSession(const String & s)
{
	AbstractReflectSession * ref = ThreadWorkerSessionFactory::CreateSession(s);
	if (ref && fLimit != 0)
	{
		ref->SetInputPolicy(PolicyRef(new RateLimitSessionIOPolicy(fLimit)));
	}
	return ref;
}

