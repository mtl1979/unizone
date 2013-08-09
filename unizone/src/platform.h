#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
# ifndef WIN32
#  define WIN32
# endif
#endif

#ifdef WIN32

#include <windows.h>

// Flash window specified by fWinHandle
void WFlashWindow(HWND fWinHandle);
#endif // WIN32

// RedHat Linux 8.x doesn't seem to define __LINUX__

#if defined(linux) || defined(LINUX)
#  if !defined(__LINUX__)
#    define __LINUX__
#  endif
#endif

#include	<math.h>
#include	<time.h>

#ifdef WIN32
inline struct tm * gmtime_r(const time_t *clock, struct tm *result)
{
	memcpy(result, gmtime(clock), sizeof(struct tm));
	return result;
}

inline struct tm * localtime_r(const time_t *clock, struct tm *result)
{
	memcpy(result, localtime(clock), sizeof(struct tm));
	return result;
}
#endif

#if defined(lrint)
#  define HAVE_LRINT
#elif defined(_MSC_VER)
# ifdef _M_AMD64
#  include <emmintrin.h>

#  define lrint(dbl) _mm_cvttsd_si32(_mm_set_sd(dbl))
#  define llrint(dbl) _mm_cvttsd_si64x(_mm_set_sd(dbl))
#  define lrintf(flt) _mm_cvttss_si32(_mm_set_ss(flt)) 
#  define HAVE_LRINT
# elif defined(_M_IX86)
	// http://mega-nerd.com/FPcast/float_cast.h

	/*	Win32 doesn't seem to have these functions. 
	**	Therefore implement inline versions of these functions here.
	*/
	
	inline __declspec(naked) int 
	lrint (double flt)
	{	
		int intgr;

		_asm
		{	
			push ebp
			mov ebp, esp
			fld flt
			fistp intgr
			mov eax, intgr
			pop ebp
			ret
		} ;
	} 

	inline __declspec(naked) __int64
	llrint (double flt)
	{
		__int64 intgr;

		_asm
		{	
			push ebp
			mov ebp, esp
			fld flt
			fistp intgr
			mov edx, DWORD PTR intgr
			mov eax, DWORD PTR intgr + 4
			pop ebp
			ret
		} ;
	}

	inline __declspec(naked) int 
	lrintf (float flt)
	{	
		int intgr;

		_asm
		{	
			push ebp
			mov ebp, esp
			fld flt
			fistp intgr
			mov eax, intgr
			pop ebp
			ret
		} ;
	}
# define HAVE_LRINT
# endif
#endif

#ifndef HAVE_LRINT
#define lrint(x) ((int) x)
#define llrint(x) ((int64) x)
#define lrintf(x) ((int) x)
#endif

#if defined(__APPLE__)
# include <string.h>
#elif defined(__LINUX__)
# include <unistd.h>
#else
# if !defined(ssize_t)
#  if defined(_MSC_VER) && defined(_M_AMD64)
typedef __int64 ssize_t;
#  elif defined(__amd64__)
typedef long ssize_t;
#  else
typedef int ssize_t;
#  endif
# endif
#endif

#endif // PLATFORM_H
