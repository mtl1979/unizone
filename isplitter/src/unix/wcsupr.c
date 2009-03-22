#include "wutil.h"

wchar_t *wcsupr(wchar_t *s)
{
	wchar_t *b = s;
	while (*b != 0)
	{
		*b = towupper( *b );
		b++;
	}
	return s;
}
