#include "wfile.h"
#include "wstring.h"

#include <io.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

#include <QByteArray>
#include <QIODevice>

WFile::WFile()
{
	file = -1;
	filename = WString();
}

WFile::~WFile()
{
	Close();
}

bool
WFile::Open(const WString &name, int mode)
{
	filename = name;
#if	__STDC_WANT_SECURE_LIB__
	errno_t err = _wsopen_s(&file, name.getBuffer(), mode, (mode & QIODevice::WriteOnly) ? _SH_DENYRW : _SH_DENYWR, (mode & _O_CREAT) ? S_IREAD | _S_IWRITE : 0);
	return (err == 0);
#else
	file = _wopen(name.getBuffer(), mode, (mode & _O_CREAT) ? _S_IREAD | _S_IWRITE : 0);
	return (file != -1);
#endif
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
		filename = WString();
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
		FILE * f;
#if	__STDC_WANT_SECURE_LIB__
		errno_t err = _wfopen_s(&f, name.getBuffer(), L"r");
		if (err == 0)
#else
		f = _wfopen(name.getBuffer(), L"r");
		if (f)
#endif
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
WFile::ReadBlock32(uint8 *buf, size_t size)
{
	return _read(file, buf, (unsigned int) size);
}

int64
WFile::ReadBlock(QByteArray &buf, uint64 size)
{
	return ReadBlock((uint8 *)buf.data(), size);
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
WFile::WriteBlock32(const uint8 *buf, size_t size)
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
