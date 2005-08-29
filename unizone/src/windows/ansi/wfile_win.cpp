#include "wfile.h"
#include "wstring.h"

#include <io.h>
#include <windows.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

void ConvertFileName(wchar_t *in, int ilen, char * out, int olen)
{
	if (AreFileApisANSI())
		WideCharToMultiByte(CP_ACP, 0, in, ilen, out, olen, NULL, NULL);
	else
		WideCharToMultiByte(CP_OEMCP, 0, in, ilen, out, olen, NULL, NULL);
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
	ConvertFileName(name, wcslen(name), cname, MAX_PATH - 1);
	file = _open(cname, mode, (mode & _O_CREAT) ? _S_IREAD | _S_IWRITE : 0);
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
	ConvertFileName(name, wcslen(name), cname, MAX_PATH - 1);
	FILE * f = fopen(cname, "r");
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
WFile::ReadBlock(void *buf, UINT64 size)
{
	if (size > LONG_MAX)
	{
		char *b = (char *) buf;
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

int
WFile::WriteBlock(const void *buf, UINT64 size)
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
	_commit(file);
}

INT64
WFile::Size()
{
	return _filelengthi64(file);
}
