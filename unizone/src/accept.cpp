#include <qapplication.h>

#include "accept.h"

void
WAcceptThread::SignalOwner()
{
	MessageRef next;
	while (ast->GetNextReplyFromInternalThread(next) >= 0)
	{
		switch (next()->what)
		{
			case AST_EVENT_NEW_SOCKET_ACCEPTED:
			{
				GenericRef tag;
				if (next()->FindTag(AST_NAME_SOCKET, tag) == B_OK)
				{
					SocketHolderRef sref;
					sref.SetFromGeneric(tag);
					if (sref())
					{
						WAcceptThreadEvent *e = new WAcceptThreadEvent(sref);
						if (e)
							QThread::postEvent(fOwner, e);
					}
				}
				break;
			}
		}
	}
}
