#include <fcntl.h>
#include "wfile.h"

int
WFile::TranslateMode(int mode)
{
	switch (mode)
	{
	case 1: case 65:
		return _O_RDONLY | _O_BINARY;
	case 2: case 66:
		return _O_WRONLY | _O_CREAT | _O_BINARY;
	case 3: case 67:
		return _O_RDWR | _O_BINARY;
	case 4: case 68:
		return _O_APPEND | _O_BINARY;
	case 6: case 70:
		return _O_APPEND | _O_WRONLY | _O_BINARY;
	case 7: case 71:
		return _O_APPEND | _O_RDWR | _O_BINARY;
	case 10: case 74:
		return _O_TRUNC | _O_WRONLY | _O_BINARY;
	case 11: case 75:
		return _O_TRUNC | _O_RDWR | _O_BINARY;
	default: return -1;
	}
}

