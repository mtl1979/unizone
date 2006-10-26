#ifndef WTRANSFER_H
#define WTRANSFER_H

namespace WTransfer
{
	// ways to disguise data
	enum 
	{
		MungeModeNone = 0,
		MungeModeXOR,
		MungeModeNum	// amount of modes
	};
	
	// transfer commands
	enum 
	{
		TransferConnectedToPeer = 'tshr',	// um... I don't use this one
		TransferDisconnectedFromPeer,		// nor this one... I have my own events
		TransferFileList,
		TransferFileHeader,
		TransferFileData,
		TransferDeprecated,					// OLD beshare message
		TransferNotifyQueued,
		TransferMD5SendReadDone,			// these are sent by the MD5 looper...
		TransferMD5RecvReadDone,
		TransferCommandPeerID,
		TransferNotifyRejected
	};
};

#endif
