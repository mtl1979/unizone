// Wrapper class to allocate/deallocate wide string buffers on demand

#ifdef WIN32
#  include <windows.h>
#endif

class QString;

// RedHat Linux 8.x doesn't seem to define __LINUX__

#ifdef linux
#  if !defined(__LINUX__)
#    define __LINUX__
#  endif
#endif

// For Unicode support on Linux
#if defined(__LINUX__)
#include <wchar.h>
#endif

// Converts array of wchar_t to QString
QString wideCharToQString(const wchar_t *wide);

// Converts QString to pointer to array of wchar_t, pointer must be deleted when not needed anymore
wchar_t *qStringToWideChar(const QString &str);

class WString
{
public:
	WString();
	WString(const wchar_t *);
	WString(const QString &);
	~WString();

	operator wchar_t *() const { return buffer; };
	QString toQString() const;
	WString &operator=(const wchar_t *);
	WString &operator=(const WString &);
	WString &operator=(const QString &);

	bool operator!=(const wchar_t *);
	bool operator!=(const WString &);
	bool operator!=(const QString &);

	bool operator==(const wchar_t *);
	bool operator==(const WString &);
	bool operator==(const QString &);

private:
	wchar_t *buffer;
	void free();						// Free internal buffer
};

