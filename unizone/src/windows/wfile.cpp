#include <fcntl.h>
#include "wfile.h"

int
WFile::TranslateMode(int mode)
{
	switch (mode)
	{
	case 1: return _O_RDONLY | _O_BINARY;
	case 2: return _O_WRONLY | _O_CREAT | _O_BINARY;
	case 3: return _O_RDWR | _O_BINARY;
	case 4: return _O_APPEND | _O_BINARY;
	case 6: return _O_APPEND | _O_WRONLY | _O_BINARY;
	case 7: return _O_APPEND | _O_RDWR | _O_BINARY;
	case 10: return _O_TRUNC | _O_WRONLY | _O_BINARY;
	case 11: return _O_TRUNC | _O_RDWR | _O_BINARY;
	default: return -1;
	}
}

