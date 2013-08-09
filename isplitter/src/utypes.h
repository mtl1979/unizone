#ifndef UTYPES_H
#define UTYPES_H

#include <limits.h>


#if defined(__amd64__) || defined(_M_AMD64)
#define ISPLITTER_64_BIT_PLATFORM 1
#endif

#ifndef int8
typedef signed char		int8;
typedef unsigned char	uint8;
#endif

#ifndef int16
typedef short				int16;
typedef unsigned short	uint16;
#endif

#ifdef ISPLITTER_64_BIT_PLATFORM
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
#  if defined(ISPLITTER_64_BIT_PLATFORM)
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
# if defined(ISPLITTER_64_BIT_PLATFORM)
#  define UINT32_MAX				UINT_MAX
# else
#  define UINT32_MAX				ULONG_MAX
# endif
#endif
