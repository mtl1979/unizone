#ifndef WTRANSFEREVENT_H
#define WTRANSFEREVENT_H

#include <qevent.h>

#ifdef WIN32
#pragma warning(disable: 4786)
#endif


class WTransferEvent : public QCustomEvent
{
public:
	WTransferEvent(int type)
		: QCustomEvent(type)
	{
		fFile = QString::null;
		fSession = QString::null;
		fError = QString::null;
		fPort = 0;
		fTime = (uint64) -1;
		fStart = -1;
		fSize = -1;
		fOffset = -1;
		fDone = false;
		fFailed = false;
	}

	virtual ~WTransferEvent() {}

	QString File() const { return fFile; }
	void SetFile(const QString &file) { fFile = file; }

	QString Session() const { return fSession; }
	void SetSession(const QString &session) { fSession = session; }

	QString Error() const {return fError; }
	void SetError(const QString &error) { fError = error; }

	uint32 Port() const { return fPort; }
	void SetPort(const uint32 port) { fPort = port; }

	uint64 Time() const { return fTime; }
	void SetTime(const uint64 time) { fTime = time; }

	int64 Start() const { return fStart; }
	void SetStart(const int64 start) { fStart = start; }

	int64 Size() const { return fSize; }
	void SetSize(const int64 size) { fSize = size; }

	int64 Offset() const { return fOffset; }
	void SetOffset(const int64 offset) { fOffset = offset; }

	bool Done() const { return fDone; }
	void SetDone(const bool done) { fDone = done; }

	bool Failed() const { return fFailed; }
	void SetFailed(const bool failed) { fFailed = failed; }

private:
	QString fFile, fSession, fError;
	uint32 fPort;
	uint64 fTime;
	int64 fStart, fSize, fOffset;
	bool fDone, fFailed;

};

#endif
