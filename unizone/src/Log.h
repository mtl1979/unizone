#ifndef LOG_H
#define LOG_H

#include <qfile.h>

class WLog
{
public:
	// Constructor. Does nothing.
	WLog();
	// Destructor. If current log is open, it closes it.
	~WLog();

	void Create(bool = false);
	void Close();

	void LogString(const char *);
	void LogString(QString);

	// Returns true if the log is open and ready for use
	bool InitCheck() const;

private:
	QFile * fFile;
};

#endif
