#include "wutil.h"

wchar_t *wcslwr(wchar_t *s)
{
	wchar_t *b = s;
	while (*b != 0)
	{
		*b = towlower( *b );
		b++;
	}
	return s;
}
