#ifndef WSTRING_H
#define WSTRING_H

// Wrapper class to allocate/deallocate wide string buffers on demand

#ifdef WIN32
#  include <windows.h>
#else
// For Unicode support on Linux or FreeBSD???
#  include <wchar.h>
#  include <wctype.h>
#  include <stdlib.h>
#endif

#ifdef __APPLE__
wchar_t *wcsdup(const wchar_t *);
#endif

#include <platform.h>

class QString;

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

	WString lower() const;
	WString upper() const;
	WString reverse() const;

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

	mutable char * utfbuf;
	mutable int utflen;
};
#endif
