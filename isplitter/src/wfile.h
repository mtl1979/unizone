#ifndef WFILE_H
#define WFILE_H

#ifdef WIN32
#include <windows.h>
#endif

#include "utypes.h"

class WString;
class QString;

class WFile
{
public:
	WFile();
	~WFile();

	bool Open(const WString &, int);
	bool Open(const QString &, int);
	void Close();
	static bool Exists(const QString &);
	static bool Exists(const WString &);

	void Flush();
	int64 ReadBlock(void *, uint64);
	int64 WriteBlock(const void *, uint64);

	int32 ReadBlock32(void *, uint32);	// For backwards compatibility with functions that can't handle 64-bit values
	int32 WriteBlock32(const void *, uint32);

	int ReadLine(char *, int);

	bool Seek(int64);
	bool At(int64);
	bool AtEnd() {return At(Size());}
	int64 Size();
private:
	int TranslateMode(int);
	int file;
};
#endif
