#include "wfile.h"
#include "wstring.h"

#include <unistd.h>
#include <fcntl.h>

WFile::WFile()
{
	file = -1;
}

WFile::~WFile()
{
	if (file)
		Close();
}

bool
WFile::Open(const WString &name, int mode)
{
	file = open64((const char *) name, mode);
	return (file != -1);
}

bool
WFile::Open(const QString &name, int mode)
{
	WString wname(name);
	wname.replace(L'\\', L'/');
	int fmode = TranslateMode(mode);
	return Open(wname, fmode);
}

void
WFile::Close()
{
	close(file);
	file = -1;
}

bool
WFile::Exists(const QString &name)
{
	WString wname(name);
	wname.replace(L'\\', L'/');
	return WFile::Exists(wname);
};

bool
WFile::Exists(const WString &name)
{
	int ret = access((const char *) name, F_OK);
	return (ret == 0);
}

bool
WFile::Seek(INT64 pos)
{
	return (lseek64(file, pos, SEEK_SET) == pos);
}

bool
WFile::At(INT64 pos)
{
	return (lseek64(file, 0, SEEK_CUR) == pos);
}

int
WFile::ReadBlock(void *buf, int size)
{
	return read(file, buf, size);
}

int
WFile::WriteBlock(const void *buf, int size)
{
	return write(file, buf, size);
}

void
WFile::Flush()
{
}

UINT64
WFile::Size()
{
	INT64 pos = lseek64(file, 0, SEEK_CUR);
	INT64 length = lseek64(file, 0, SEEK_END);
	(void) lseek64(file, pos, SEEK_SET);
	return length;
}
