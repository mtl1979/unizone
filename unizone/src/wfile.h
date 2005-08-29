#ifndef WFILE_H
#define WFILE_H

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#ifndef INT64
# ifdef WIN32
#  define INT64 __int64
#  define UINT64 unsigned __int64
# else
#  define INT64 long long
#  define UINT64 unsigned long long
# endif
#endif

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
	int ReadBlock(void *, INT64);
	int WriteBlock(const void *, INT64);

	int ReadLine(char *, int);

	bool Seek(INT64);
	bool At(INT64);
	bool AtEnd() {return At(Size());}
	INT64 Size();
private:
	int TranslateMode(int);
	int file;
};
#endif
