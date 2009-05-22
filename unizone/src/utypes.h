#ifndef UTYPES_H
#define UTYPES_H

#include <limits.h>

/* We need to implement all typedefs that MUSCLE would define even though we 
	don't need them explicitly in our classes, because derived class might
	include one of MUSCLE headers and their typedefs clash if not excluded
*/

#define MUSCLE_TYPES_PREDEFINED

#ifndef int8
typedef signed char		int8;
typedef unsigned char	uint8;
#endif

#ifndef int16
typedef short				int16;
typedef unsigned short	uint16;
#endif

#ifdef __amd64__
typedef int					int32;
# ifndef _UINT32
#  define _UINT32
typedef unsigned int		uint32;
# endif
#else
typedef long				int32;
# ifndef _UINT32
#  define _UINT32
typedef unsigned long	uint32;
# endif
#endif

#ifndef int64
# ifdef WIN32
	typedef __int64				int64; 
	typedef unsigned __int64	uint64;
# else
#  ifdef __amd64__
	typedef long int		int64;
	typedef unsigned long int	uint64;
#  else
	typedef long long				int64;
	typedef unsigned long long	uint64;
#  endif
# endif
#endif

#ifndef status_t
typedef int32						status_t;
#endif
#endif
#ifdef __amd64__
# ifndef UINT32_MAX
#  define UINT32_MAX				UINT_MAX
# endif
#else
# ifndef UINT32_MAX
#  define UINT32_MAX				ULONG_MAX
# endif
#endif
