#ifndef WSTRING_H
#define WSTRING_H

// Wrapper class to allocate/deallocate wide string buffers on demand

#ifdef WIN32
#  include <windows.h>
#endif

class QString;

// RedHat Linux 8.x doesn't seem to define __LINUX__

#if defined(linux)
#  if !defined(__LINUX__)
#    define __LINUX__
#  endif
#endif

// For Unicode support on Linux or FreeBSD???
#if defined(__LINUX__) || defined(__FreeBSD__) || defined(__QNX__)
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
	WString(const char *); // UTF-8
	~WString();

#ifdef WIN32
	WString lower() const;
	WString upper() const;
	WString reverse() const;
#endif

	void replace(wchar_t, wchar_t);
	operator wchar_t *() const { return buffer; };
	operator const char *() const;
	wchar_t *getBuffer() const { return buffer; };
	int length() const; 
	QString toQString() const;

	WString &operator=(const wchar_t *);
	WString &operator=(const WString &);
	WString &operator=(const QString &);

	WString &operator+=(const wchar_t *);
	WString &operator+=(const WString &);
	WString &operator+=(const QString &);

	bool operator!=(const wchar_t *);
	bool operator!=(const WString &);
	bool operator!=(const QString &);

	bool operator==(const wchar_t *);
	bool operator==(const WString &);
	bool operator==(const QString &);

protected:
	void setBuffer(wchar_t *buf);

private:
	wchar_t *buffer;
	void free();						// Free internal buffer
};

#endif
