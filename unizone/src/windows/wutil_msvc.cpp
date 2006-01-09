#include <qstring.h>

#include "wutil.h"

#if defined(_X86_)
void __declspec(naked) 
wcopy (wchar_t *dest, const wchar_t *src, size_t len)
{
	_asm
	{
			push ebp;
			mov ebp, esp;
			push ebx;
			mov ecx, len;
			jecxz c3;
		
			mov eax, src;
			and eax, 3;
			jnz c2a;
			mov eax, dest;
			and eax, 3;
			jnz c2a;
			mov edx, dest;
			mov ebx, src; 
			bt ecx, 0;
			jnc c1;
			dec ecx;
			mov eax, [ebx+ecx*2];
			and eax, 0x0000FFFF;
			mov [edx+ecx*2], eax;
			jecxz c4;
			dec ecx;
			jmp c1a;
c1:			xor ax, ax;
			mov [edx+ecx*2], ax;
			dec ecx;
c1a:		bt ecx,0;
			jc c2b;
			mov ax, [ebx+ecx*2];
			mov [edx+ecx*2], ax;
			jecxz c4;
			
			jmp c2d;
c2a:		mov edx, dest;
			mov ebx, src;
c2b:		dec ecx;
c2c:		mov eax, [ebx+ecx*2];
			mov [edx+ecx*2], eax;
			jecxz c4;
			
c2d:		sub ecx, 2;
			jmp c2c;
c3:			// NULL string
			mov edx, dest;
			mov [edx], cx;
c4:			pop ebx;
			pop ebp;
			ret;
	}
}

void __declspec(naked) 
wreplace(wchar_t *buffer, wchar_t win, wchar_t wout)
{
	_asm
	{
		push ebp;
		mov ebp, esp;
		push bx;
		mov eax, buffer;
		and eax, eax;
		jz e;
		mov bx, win;
		mov cx, wout;
l:		mov dx, [eax];
		and dx, dx;
		jz e;
		xor dx, bx;
		jnz next;
		mov [eax], cx;
next:	add eax, 2;
		jmp l;
e:		pop bx;
		pop ebp;
		ret;
	}
}

void __declspec(naked)
wcat(wchar_t *dest, const wchar_t*src, size_t pos)
{
	__asm
	{
		push ebp;
		mov ebp, esp;
		push bx
		mov eax, src;
		mov edx, pos;
		shl edx, 1;
		add edx, dest;			// dest += pos;
s1:		mov bx, [eax];
		and bx, bx;
		jz se
		add eax, 2;				// src++;
		mov [edx], bx;			// *dest = *src;
		add edx, 2;				// dest++;
		jmp s1;
se:		xor bx,bx;				// terminate dest
		mov [edx], bx;
		pop bx;
		pop ebp;
		ret;
	}
}

void __declspec(naked)
wreverse(wchar_t *dest, const wchar_t *src, ssize_t len)
{
	_asm
	{
		push ebp;
		mov ebp, esp;
		push bx;
		mov eax, src;			// eax[ecx] = &src[len - 1]
		sub eax, 2;
		mov edx, dest;
		mov ecx, len;
		jecxz re;				// len == 0?
r1:		mov bx, [eax+ecx*2]
		and bx, bx;
		jz re;
		mov [edx], bx;			// *dest = *src;
		add edx, 2;				// dest++;
		dec ecx;				// len--; src--;
		jecxz re;				// len == 0?
		jmp r1;
re:		xor bx, bx				// dest[len] = 0;
		mov [edx], bx
		pop bx;
		pop ebp;
		ret;
	}
}
#else
#error Unsupported platform!
#endif

/*
*
*  Conversion functions
*
*/

QString 
wideCharToQString(const wchar_t *wide)
{
    QString result;
    result.setUnicodeCodes(wide, lstrlenW(wide));
    return result;
}

wchar_t *
qStringToWideChar(const QString &str)
{
   	if (str.isNull())
	{
       	return NULL;
	}
	
	wchar_t *result = new wchar_t[str.length() + 1];
	if (result)
	{
		wcopy(result, (uint16 *) str.unicode(), str.length());
		return result;
	}
	else
	{
		return NULL;
	}
}

