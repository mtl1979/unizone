#include <fcntl.h>
#include "wfile.h"

int
WFile::TranslateMode(int mode)
{
	switch (mode)
	{
	case 1: return _O_RDONLY;
	case 2: return _O_WRONLY | _O_CREAT;
	case 3: return _O_RDWR;
	case 4: return _O_APPEND;
	case 6: return _O_APPEND | _O_WRONLY;
	case 7: return _O_APPEND | _O_RDWR;
	case 10: return _O_TRUNC | _O_WRONLY;
	case 11: return _O_TRUNC | _O_RDWR;
	default: return -1;
	}
}

