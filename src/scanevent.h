#ifndef SCANEVENT_H
#define SCANEVENT_H

#include <qevent.h>

class QString;

class ScanEvent : public QCustomEvent
{
public:
	enum Type
	{
		ScanDirectory = 1935893618, // scdr
		ScanFile,
		ScannedDirs,
		ScannedFiles,
		DirsLeft,
		Reset,
		Show,
		Hide
	};
	ScanEvent(Type t, QString data) : QCustomEvent((int) t), fText(data) {}
	ScanEvent(Type t, int data) : QCustomEvent((int) t) , fNumber(data) {}
	ScanEvent(Type t) : QCustomEvent((int) t) {}
	~ScanEvent() {}
	QString text() { return fText; }
	int number() { return fNumber; }
private:
	QString fText;
	int fNumber;
};
#endif
