#include "accept.h"

#include <qapplication.h>

void
WAcceptThread::SignalOwner()
{
	MessageRef next;
	while (GetNextReplyFromInternalThread(next) >= 0)
	{
		switch (next()->what)
		{
			case AST_EVENT_NEW_SOCKET_ACCEPTED:
			{
				GenericRef tag;
				if (next()->FindTag(AST_NAME_SOCKET, tag) == B_OK)
				{
					SocketHolderRef sref(tag, NULL);
					if (sref())
					{
						WAcceptThreadEvent *e = new WAcceptThreadEvent(sref);
						if (e)
							QApplication::postEvent(fOwner, e);
					}
				}
				break;
			}
		}
	}
}
