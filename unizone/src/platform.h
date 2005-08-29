#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qstring.h>
#include "wstring.h"
#include "util/String.h"

using muscle::String;

#ifdef WIN32

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

#if defined(lrint)
#  define HAVE_LRINT
#elif defined(_MSC_VER) && defined(_X86_)

	// http://mega-nerd.com/FPcast/float_cast.h

	/*	Win32 doesn't seem to have these functions. 
	**	Therefore implement inline versions of these functions here.
	*/
	
	__inline long int 
	lrint (double flt)
	{	
		int intgr;

		_asm
		{	
			fld flt
			fistp intgr
		} ;
			
		return intgr ;
	} 

	__inline int64
	llrint (double flt)
	{
		int64 intgr;

		_asm
		{	
			fld flt
			fistp intgr
		} ;

		return intgr ;
	}

	__inline long int 
	lrintf (float flt)
	{	
		int intgr;

		_asm
		{	
			fld flt
			fistp intgr
		} ;
			
		return intgr ;
	}
#define HAVE_LRINT
#endif

#ifndef HAVE_LRINT
#define lrint(x) ((int) x)
#define llrint(x) ((int64) x)
#define lrintf(x) ((int) x)
#endif

#if !defined(ssize_t)
typedef int ssize_t;
#endif

#endif // PLATFORM_H
