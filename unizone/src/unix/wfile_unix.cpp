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
WFile::ReadBlock(void *buf, INT64 size)
{
	if (size > LONG_MAX)
	{
		char * b = (char *) buf;
		int numbytes = 0;
		while (size > 0)
		{
			int nb = read(file, b, (size > LONG_MAX) ? LONG_MAX : size);
			if (nb == 0)
				break;
			numbytes += nb;
			size -= nb;
			b += nb;
		}
		return numbytes;
	}
	return read(file, buf, size);
}

int 
WFile::ReadLine(char *buf, int size)
{
	int numbytes = 0;
	while (size-- > 0)
	{
		char c;
		if (read(file, &c, 1) == 1)
		{
			numbytes++;
			*buf++ = c;
			if (c == 10)
				break;
		}
		else
			break;
	}
	*buf = 0;
	return numbytes;
}

int
WFile::WriteBlock(const void *buf, INT64 size)
{
	if (size > LONG_MAX)
	{
		const char *b = (const char *) buf;
		int numbytes = 0;
		while (size > 0)
		{
			int nb = write(file, b, (size > LONG_MAX) ? LONG_MAX : size);
			if (nb == 0)
				break;
			numbytes += nb;
			size -= nb;
			b += nb;
		}
		return numbytes;
	}
	return write(file, buf, size);
}

void
WFile::Flush()
{
	(void) fsync(file);
}

UINT64
WFile::Size()
{
	INT64 pos = lseek64(file, 0, SEEK_CUR);
	INT64 length = lseek64(file, 0, SEEK_END);
	(void) lseek64(file, pos, SEEK_SET);
	return length;
}
