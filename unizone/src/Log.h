#ifndef LOG_H
#define LOG_H

#include <qstring.h>
class WFile;

class WLog
{
public:

	enum LogType {
		LogMain,
		LogPrivate,
		LogChannel
	};
	// Constructor. Does nothing.
	WLog();
	// Destructor. If current log is open, it closes it.
	~WLog();

	void Create(LogType = LogMain, const QString & = QString::null);
	void Close();

	void LogString(const char *);
	void LogString(const QString &);

	// Returns true if the log is open and ready for use
	bool InitCheck() const;

private:
	WFile * fFile;
};

#endif
