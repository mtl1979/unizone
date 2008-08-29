#include "wfile.h"
#include "wstring.h"

#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

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
	file = -1;
	if (name.getBuffer() != NULL)
#ifdef __APPLE__
		file = open((const char *) name, mode, (mode & O_CREAT) ? S_IRUSR | S_IWUSR : 0);
#else
		file = open64((const char *) name, mode, (mode & O_CREAT) ? S_IRUSR | S_IWUSR : 0);
#endif
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
	int ret = -1;
	if (name.getBuffer() != NULL)
		ret = access((const char *) name, F_OK);
	return (ret == 0);
}

bool
WFile::Seek(int64 pos)
{
#ifdef __APPLE__
	return (lseek(file, pos, SEEK_SET) == pos);
#else
	return (lseek64(file, pos, SEEK_SET) == pos);
#endif
}

bool
WFile::At(int64 pos)
{
#ifdef __APPLE__
	return (lseek(file, 0, SEEK_CUR) == pos);
#else
	return (lseek64(file, 0, SEEK_CUR) == pos);
#endif
}

int32
WFile::ReadBlock32(void *buf, uint32 size)
{
	return read(file, buf, size);
}

int64
WFile::ReadBlock(void *buf, uint64 size)
{
	if (size > INT_MAX)
	{
		char * b = (char *) buf;
		int64 numbytes = 0;
		while (size > 0)
		{
			int nb = ReadBlock32(b, (size > INT_MAX) ? INT_MAX : (unsigned int) size);
			if (nb == 0)
				break;
			numbytes += nb;
			size -= nb;
			b += nb;
		}
		return numbytes;
	}
	return ReadBlock32(buf, (unsigned int) size);
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
			if (c == 10)
				break;
			else
			{
				numbytes++;
				*buf++ = c;
			}
		}
		else
			break;
	}
	*buf = 0;
	return numbytes;
}

int32
WFile::WriteBlock32(const void *buf, uint32 size)
{
	return write(file, buf, (unsigned int) size);
}

int64
WFile::WriteBlock(const void *buf, uint64 size)
{
	if (size > INT_MAX)
	{
		const char *b = (const char *) buf;
		int64 numbytes = 0;
		while (size > 0)
		{
			int nb = WriteBlock32(b, (size > INT_MAX) ? INT_MAX : (unsigned int) size);
			if (nb == 0)
				break;
			numbytes += nb;
			size -= nb;
			b += nb;
		}
		return numbytes;
	}
	return WriteBlock32(buf, (unsigned int) size);
}

void
WFile::Flush()
{
	(void) fsync(file);
}

int64
WFile::Size()
{
#ifdef __APPLE__
	int64 pos = lseek(file, 0, SEEK_CUR);
	int64 length = lseek(file, 0, SEEK_END);
	(void) lseek(file, pos, SEEK_SET);
#else
	int64 pos = lseek64(file, 0, SEEK_CUR);
	int64 length = lseek64(file, 0, SEEK_END);
	(void) lseek64(file, pos, SEEK_SET);
#endif
	return length;
}
