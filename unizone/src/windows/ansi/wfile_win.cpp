#include "wfile.h"
#include "wstring.h"

#include <io.h>

WFile::WFile()
{
	file = NULL;
}

WFile::~WFile()
{
	if (file)
		Close();
}

bool
WFile::Open(const WString &name, int mode)
{
	file = _open((const char *) name, mode);
	return (file != NULL);
}

bool
WFile::Open(const QString &name, int mode)
{
	WString wname(name);
	int fmode = TranslateMode(mode);
	return Open(wname, fmode);
}

void
WFile::Close()
{
	_close(file);
	file = NULL;
}

bool
WFile::Exists(const QString &name)
{
	WString wname(name);
	wname.replace(L'/', L'\\');
	return WFile::Exists(wname);
};

bool
WFile::Exists(const WString &name)
{
	FILE * f = fopen((const char *) name, "r");
	bool ret = false;
	if (f)
	{
		fclose(f);
		ret = true;
	}
	return ret;
}

bool
WFile::Seek(INT64 pos)
{
	return (_lseeki64(file, pos, SEEK_SET) == pos);
}

bool
WFile::At(INT64 pos)
{
	return (_telli64(file) == pos);
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
	_commit(file);
}

INT64
WFile::Size()
{
	return _filelengthi64(file);
}
