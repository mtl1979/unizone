#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <assert.h>

#if (_MSC_VER == 1200)
typedef WINBASEAPI int (*resetstkoflwproc) ( void );
resetstkoflwproc _resetstkoflwproc = NULL;
HMODULE _crtlib = 0;

void LoadCRT( void )
{
   if (_crtlib == 0)
   {
	   _crtlib = LoadLibrary(TEXT("msvcrt.dll"));
	   _resetstkoflwproc = (resetstkoflwproc) GetProcAddress(_crtlib, "_resetstkoflw");
	   assert(_resetstkoflwproc != 0);
   }
}

void UnloadCRT( void )
{
	FreeLibrary(_crtlib);
}

int _resetstkoflw( void )
{
	return _resetstkoflwproc();
}

#endif


