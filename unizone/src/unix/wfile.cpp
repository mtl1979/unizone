#include <fcntl.h>
#include "wfile.h"

int
WFile::TranslateMode(int mode)
{
	switch (mode)
	{
	case 1: case 65:
		return O_RDONLY;
	case 2: case 66:
		return O_WRONLY | O_CREAT;
	case 3: case 67:
		return O_RDWR;
	case 4: case 68:
		return O_APPEND;
	case 6: case 70:
		return O_APPEND | O_WRONLY;
	case 7: case 71:
		return O_APPEND | O_RDWR;
	case 10: case 74:
		return O_TRUNC | O_WRONLY;
	case 11: case 75:
		return O_TRUNC | O_RDWR;
	default: return -1;
	}
}

