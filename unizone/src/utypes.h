#ifndef UTYPES_H
#define UTYPES_H

#include <limits.h>
#if defined(_MSC_VER) && _MSC_VER >= 1900
#include <stdint.h>
#endif

/* We need to implement all typedefs that MUSCLE would define even though we
	don't need them explicitly in our classes, because derived class might
	include one of MUSCLE headers and their typedefs clash if not excluded
*/

#define MUSCLE_TYPES_PREDEFINED

#if defined(__amd64__) || defined(_M_AMD64)
#define MUSCLE_64_BIT_PLATFORM 1
#endif

#ifndef int8
typedef signed char		int8;
typedef unsigned char	uint8;
#endif

#ifndef int16
typedef short				int16;
typedef unsigned short	uint16;
#endif

#if defined(MUSCLE_64_BIT_PLATFORM)
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
#  if defined(MUSCLE_64_BIT_PLATFORM)
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

#ifndef UINT32_MAX
# if defined(MUSCLE_64_BIT_PLATFORM)
#  define UINT32_MAX				UINT_MAX
# else
#  define UINT32_MAX				ULONG_MAX
# endif
#endif
