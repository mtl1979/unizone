#ifndef WDOWNLOADEVENT_H
#define WDOWNLOADEVENT_H

#include "wtransferevent.h"
#include "message/Message.h"

#include <qevent.h>

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

//------------------------------------------------------------------
class WDownloadEvent : public WTransferEvent
{
public:

	WDownloadEvent(int type)
		: WTransferEvent(type) { fSender = NULL; fReceived = 0; }
	virtual ~WDownloadEvent() { }

	WDownloadThread * Sender() const { return fSender; }
	void SetSender(WDownloadThread *sender) { fSender = sender; }

	uint32 Received() const { return fReceived; }
	void SetReceived(const uint32 received) { fReceived = received; }

	enum
	{
		FirstEvent = 'wDeE',		// Place holder for start of event list
		ConnectFailed,				// sent when the connection failed
		ConnectInProgress,		// the connection is being established
		Connected,					// session was connected, negotiating the connection with the remote client
		Disconnected,				// we got disconnected from the peer before the file was completed
		FileDone,					// file completed
		FileFailed, 				// file dl failed due to an error (not a disconnection)
		FileStarted,				// started new file dl
		FileError,					// critical error, file system error
		FileDataReceived,			// received some data
		FileHashing,				// we're md5'ing the file
		ConnectBackRequest, 		// the thread is accepting on a port, send a connect back request to the remote client
		FileQueued, 				// we're queued
		Init,							// is sent to the GUI with the path of the file being downloaded/requested. This filename may change (also may include the user as well)
		UpdateUI,					// this is sent to the GUI with a new session ID/name pair to update for the upload
		FileBlocked, 				// user has been banned for file transfers
		LastEvent					// Place holder for end of event list
	};

private:
	WDownloadThread * fSender;
	uint32 fReceived;
};

#endif
