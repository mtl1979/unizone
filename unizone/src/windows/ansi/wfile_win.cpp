#include "wfile.h"
#include "wstring.h"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

void ConvertFileName(wchar_t *in, int ilen, char * out, int olen)
{
	int len;
	if (AreFileApisANSI())
		len = WideCharToMultiByte(CP_ACP, 0, in, ilen, out, olen, NULL, NULL);
	else
		len = WideCharToMultiByte(CP_OEMCP, 0, in, ilen, out, olen, NULL, NULL);
	out[len] = 0;
}

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
	char cname[MAX_PATH];
	file = -1;
	if (name.getBuffer() != NULL)
	{
		ConvertFileName(name, wcslen(name), cname, MAX_PATH - 1);
		file = _open(cname, mode, (mode & _O_CREAT) ? _S_IREAD | _S_IWRITE : 0);
	}
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
	_close(file);
	file = -1;
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
	char cname[MAX_PATH];
	bool ret = false;
	if (name.getBuffer() != NULL)
	{
		ConvertFileName(name, wcslen(name), cname, MAX_PATH - 1);
		FILE * f = fopen(cname, "r");
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
WFile::ReadBlock32(void *buf, uint32 size)
{
	return read(file, buf, size);
}

int64
WFile::ReadBlock(void *buf, uint64 size)
{
	if (size > INT_MAX)
	{
		char *b = (char *) buf;
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
			if (c == 13)
			{
				if (read(file, &c, 1) == 1)
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
	_commit(file);
}

int64
WFile::Size()
{
	return _filelengthi64(file);
}
