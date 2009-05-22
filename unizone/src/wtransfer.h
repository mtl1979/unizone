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
		// um... I don't use this one
		TransferConnectedToPeer = 1953720434,	// tshr
		// nor this one... I have my own events
		TransferDisconnectedFromPeer,		
		TransferFileList,
		TransferFileHeader,
		TransferFileData,
		// OLD BeShare message
		TransferDeprecated,			
		TransferNotifyQueued,
		// these are sent by the MD5 looper...
		TransferMD5SendReadDone,		
		TransferMD5RecvReadDone,
		//
		TransferCommandPeerID,
		TransferNotifyRejected
	};
};

#endif
