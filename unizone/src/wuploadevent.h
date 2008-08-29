#ifndef WUPLOADEVENT_H
#define WUPLOADEVENT_H

#include "wtransferevent.h"
#include "message/Message.h"

#include <qevent.h>

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

//------------------------------------------------------------------
class WUploadEvent : public WTransferEvent
{
public:
//	enum { Type = 'wUeE' };

	WUploadEvent(int type)
		: WTransferEvent(type) { fSender = NULL; fSent = 0; }

	virtual ~WUploadEvent() { }

	WUploadThread * Sender() const { return fSender; }
	void SetSender(WUploadThread *sender) { fSender = sender; }

	uint64 Sent() const { return fSent; }
	void SetSent(const uint64 sent) { fSent = sent; }

	enum
	{
		FirstEvent = 'wUeE',		// Placeholder for start of event list
		ConnectFailed = 'uEcF', // sent when the connection failed
		ConnectInProgress,		// the connection is being established
		Connected,					// session was connected, negotiating the connection with the remote client
		Disconnected,				// we got disconnected from the peer before the file was completed
		FileDone,					// file completed
		FileStarted,				// started new file dl
		FileError,					// critical error, file system error
		FileDataSent,				// sent some data
		FileHashing,				// we're md5'ing the file
		FileQueued, 				// we're queued
		Init,							// is sent to the GUI with the path of the file being downloaded/requested. This filename may change (also may include the user as well)
		UpdateUI,					// this is sent to the GUI with a new session ID/name pair to update for the upload
		FileBlocked, 				// user has been banned for file transfers
		LastEvent					// Placeholder for end of event list
	};

private:
	WUploadThread * fSender;
	uint64 fSent;
};

#endif
