#include "wutil.h"

#include <string.h>

#ifdef __APPLE__
wchar_t *wcsdup(const wchar_t *s)
{
	size_t len = (wcslen(s) + 1) * sizeof(wchar_t);
	wchar_t *n = (wchar_t *) malloc(len);
	if (n == NULL) return NULL;
	
	return (wchar_t *) memcpy(n, (void *) s, len);		
}
#endif
