#ifndef WGENERICEVENT_H
#define WGENERICEVENT_H

#include "message/Message.h"

#include <qevent.h>

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

//------------------------------------------------------------------
class WGenericEvent : public QCustomEvent
{
public:
	enum { Type = 'wGeE' };

	// This event stores a MessageRef
	WGenericEvent(MessageRef msg) 
		: QCustomEvent(Type) { fMsg = msg; }
	virtual ~WGenericEvent() { }

	MessageRef Msg() const { return fMsg; }
	void SetMsg(MessageRef m) { fMsg = m; }

	enum
	{
		ConnectFailed = 'gEcF',	// sent when the connection failed
		ConnectInProgress,		// the connection is being established
		Connected,				// session was connected, negotiating the connection with the remote client
		Disconnected,			// we got disconnected from the peer before the file was completed
		FileDone,				// file completed
		FileFailed,				// file dl failed due to an error (not a disconnection)
		FileStarted,			// started new file dl
        FileError,				// critical error, file system error
		FileDataReceived,		// received some data
		FileHashing,			// we're md5'ing the file
		ConnectBackRequest,		// the thread is accepting on a port, send a connect back request to the remote client
		FileQueued,				// we're queued
		Init,					// is sent to the GUI with the path of the file being downloaded/requested. This filename may change (also may include the user as well)
		UpdateUI,				// this is sent to the GUI with a new session ID/name pair to update for the upload
		FileBlocked				// user has been banned for file transfers
	};

private:
	MessageRef fMsg;
};

#endif