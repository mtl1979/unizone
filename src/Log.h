#ifndef LOG_H
#define LOG_H

class QString;
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

	void Create(LogType = LogMain);
	void Create(LogType, const QString &);
	void Close();

	void LogString(const char *, bool);
	void LogString(const QString &, bool);

	// Returns true if the log is open and ready for use
	bool InitCheck() const;

private:
	WFile * fFile;
};

#endif
