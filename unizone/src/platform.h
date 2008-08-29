#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
# ifndef WIN32
#  define WIN32
# endif
#endif

#ifdef WIN32
#pragma warning(disable: 4786)

#include <windows.h>

#if defined(BUILD_WIN98)

#if !defined(FLASHWINFO) // Microsoft Visual C++ 6.0 doesn't define this!!!

#undef BUILD_WIN98
#pragma message ("platform.h: BUILD_WIN98 is not supported on your compiler, undefining BUILD_WIN98!")

#endif // !defined(FLASHWINFO)  

#endif // defined(BUILD_WIN98)

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
#elif defined(_MSC_VER) && defined(_M_IX86)

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

#define HAVE_LRINT
#endif

#ifndef HAVE_LRINT
#define lrint(x) ((int) x)
#define llrint(x) ((int64) x)
#define lrintf(x) ((int) x)
#endif

#if defined(__APPLE__)
# include <string.h>
#else
# if !defined(ssize_t)
typedef int ssize_t;
# endif
#endif

#endif // PLATFORM_H
