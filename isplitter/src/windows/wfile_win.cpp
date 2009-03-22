#include "wfile.h"
#include "wstring.h"

#include <io.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

#include <QByteArray>

WFile::WFile()
{
	file = -1;
}

WFile::~WFile()
{
	Close();
}

bool
WFile::Open(const WString &name, int mode)
{
	file = _wopen(name.getBuffer(), mode, (mode & _O_CREAT) ? _S_IREAD | _S_IWRITE : 0);
	return (file != -1);
}

bool
WFile::Open(const QString &name, int mode)
{
	WString wname(name);
	wname.replace(L'/', L'\\');
	int fmode = TranslateMode(mode);
	return Open(wname, fmode);
}

void
WFile::Close()
{
	if (file != -1)
	{
		_close(file);
		file = -1;
	}
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
	bool ret = false;
	if (name.getBuffer() != NULL)
	{
		FILE * f = _wfopen(name.getBuffer(), L"r");
		if (f)
		{
			fclose(f);
			ret = true;
		}
	}
	return ret;
}

bool
WFile::Seek(int64 pos)
{
	return (_lseeki64(file, pos, SEEK_SET) == pos);
}

bool
WFile::At(int64 pos)
{
	return (_telli64(file) == pos);
}

int32
WFile::ReadBlock32(uint8 *buf, uint32 size)
{
	return _read(file, (char *) buf, size);
}

int64
WFile::ReadBlock(QByteArray &buf, uint64 size)
{
	return ReadBlock((uint8 *) buf.data(), size);
}

int64
WFile::ReadBlock(uint8 *buf, uint64 size)
{
	if (size > INT_MAX)
	{
		uint8 * b = buf;
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
		if (_read(file, &c, 1) == 1)
		{
			if (c == 13)
			{
				if (_read(file, &c, 1) == 1)
				{
					if (c != 10)
						_lseeki64(file, -1, SEEK_CUR); // Rewind one byte.
				}
				break;
			}
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
WFile::WriteBlock32(const uint8 *buf, uint32 size)
{
	return _write(file, buf, (unsigned int) size);
}

int64
WFile::WriteBlock(const QByteArray &buf, uint64 size)
{
	return WriteBlock((const uint8 *) buf.data(), size);
}

int64
WFile::WriteBlock(const uint8 *buf, uint64 size)
{
	if (size > INT_MAX)
	{
		const uint8 *b = buf;
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
	_commit(file);
}

int64
WFile::Size()
{
	return _filelengthi64(file);
}
