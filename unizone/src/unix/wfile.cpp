#include <fcntl.h>
#include "wfile.h"

int
WFile::TranslateMode(int mode)
{
	switch (mode)
	{
	case 1: return O_RDONLY;
	case 2: return O_WRONLY | O_CREAT;
	case 3: return O_RDWR;
	case 4: return O_APPEND;
	case 6: return O_APPEND | O_WRONLY;
	case 7: return O_APPEND | O_RDWR;
	case 10: return O_TRUNC | O_WRONLY;
	case 11: return O_TRUNC | O_RDWR;
	default: return -1;
	}
}

