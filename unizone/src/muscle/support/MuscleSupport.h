/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

/******************************************************************************
/
/     File:     MuscleSupport.h
/
/     Description:  Standard types, macros, etc, for MUSCLE.
/                   Many of them are suspiciously BeOS-like.  ;^)
/
*******************************************************************************/

#ifndef MuscleSupport_h
#define MuscleSupport_h

#define MUSCLE_VERSION_STRING "2.43"

// Just declare the muscle namespace as existing.
// If we ever decide to make the muscle namespace a superset
// of another namespace, we would add a 'using namespace' line here.
namespace muscle
{
   // empty
};

// MIPS CPUs (e.g. on SGIs) can't stand non-aligned word reads, so we'll
// accomodate them by using memcpy() instead.
#ifdef MIPS
# define MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT
#endif

// Borland C++ builder also runs under Win32, but it doesn't set this flag
// So we'd better set it ourselves.
#ifdef __BORLANDC__
# ifndef WIN32
#  define WIN32 1
# endif
#endif

// VC++ can't handle this stuff, it's too lame
#ifdef WIN32
# define UNISTD_H_NOT_AVAILABLE
# define NEW_H_NOT_AVAILABLE
#endif

#ifndef UNISTD_H_NOT_AVAILABLE
# include <unistd.h>
#endif

#ifndef NEW_H_NOT_AVAILABLE
# include <new>
using std::bad_alloc;
using std::nothrow_t;
using std::nothrow;
using std::new_handler;
using std::set_new_handler;
#else
# define MUSCLE_AVOID_NEWNOTHROW
#endif

#define WARN_OUT_OF_MEMORY muscle::LogTime(muscle::MUSCLE_LOG_CRITICALERROR, "ERROR--OUT OF MEMORY!  (%s:%i)\n",__FILE__,__LINE__)

#ifndef newnothrow
# ifdef MUSCLE_AVOID_NEWNOTHROW
#  define newnothrow new
# else
#  define newnothrow new (nothrow) 
# endif
#endif

#ifdef __BEOS__
# include <kernel/debugger.h>
# define MCRASH_IMPL debugger("muscle assertion failure")
#else
# define MCRASH_IMPL *((uint32*)NULL) = 0x666
#endif

#define MCRASH(msg) {muscle::LogTime(muscle::MUSCLE_LOG_CRITICALERROR, "ASSERTION FAILED: (%s:%i) %s\n", __FILE__,__LINE__,msg); muscle::LogStackTrace(MUSCLE_LOG_CRITICALERROR); MCRASH_IMPL;}
#define MASSERT(x,msg) {if(!(x)) MCRASH(msg)}
#define MCHECKPOINT muscle::LogTime(muscle::MUSCLE_LOG_WARNING, "Reached checkpoint at %s:%i\n", __FILE__, __LINE__)

#define UNLESS(x) if(!(x))
#define ARRAYITEMS(x) (sizeof(x)/sizeof(x[0]))  // returns # of items in array

typedef void * muscleVoidPointer;  // it's a bit easier, syntax-wise, to use this type than (void *) directly in some cases.

#ifdef __BEOS__
# include <support/Errors.h>
# include <support/ByteOrder.h>  // might as well use the real thing (and avoid complaints about duplication)
# include <support/SupportDefs.h>
# include <support/TypeConstants.h>
# ifdef BONE
#  define closesocket close
# else
#  define BEOS_OLD_NETSERVER
# endif
#else
# define B_ERROR    -1
# define B_NO_ERROR 0
# define B_OK       B_NO_ERROR
# ifndef WIN32
#  define closesocket close
# endif
# ifdef __ATHEOS__
#  include </ainc/atheos/types.h>
# else
#  ifndef MUSCLE_TYPES_PREDEFINED  /* certain (ahem) projects already set these themselves... */
#   define true                     1
#   define false                    0
    typedef signed char             int8;
    typedef unsigned char           uint8;
    typedef short                   int16;
    typedef unsigned short          uint16;
    typedef long                    int32;
    typedef unsigned long           uint32;
#   if defined(WIN32) && !defined(__GNUWIN32__)
     typedef __int64                int64;
     typedef unsigned __int64       uint64;
#   else
     typedef long long              int64;
     typedef unsigned long long     uint64;
#   endif
    typedef unsigned char           uchar;
    typedef unsigned short          unichar;                   
    typedef int32                   status_t;
#  endif  /* !MUSCLE_TYPES_PREDEFINED */
# endif  /* !__ATHEOS__*/
#endif  /* __BEOS__*/

#define MAKETYPE(x) ((((unsigned long)(x[0])) << 24) | \
                     (((unsigned long)(x[1])) << 16) | \
                     (((unsigned long)(x[2])) <<  8) | \
                     (((unsigned long)(x[3])) <<  0))

#ifndef __BEOS__
//:Be-style message-field type codes.
// I've calculated the integer equivalents for these codes
// because gcc whines like a little girl about the four-byte
// constants when compiling under Linux --jaf
enum {
   B_ANY_TYPE     = 1095653716, // 'ANYT',  // wild card
   B_BOOL_TYPE    = 1112493900, // 'BOOL',
   B_DOUBLE_TYPE  = 1145195589, // 'DBLE',
   B_FLOAT_TYPE   = 1179406164, // 'FLOT',
   B_INT64_TYPE   = 1280069191, // 'LLNG',
   B_INT32_TYPE   = 1280265799, // 'LONG',
   B_INT16_TYPE   = 1397248596, // 'SHRT',
   B_INT8_TYPE    = 1113150533, // 'BYTE',
   B_MESSAGE_TYPE = 1297303367, // 'MSGG',
   B_POINTER_TYPE = 1347310674, // 'PNTR',
   B_POINT_TYPE   = 1112559188, // 'BPNT',
   B_RECT_TYPE    = 1380270932, // 'RECT',
   B_STRING_TYPE  = 1129534546, // 'CSTR',
   B_OBJECT_TYPE  = 1330664530, // 'OPTR',  // used for flattened objects
   B_RAW_TYPE     = 1380013908, // 'RAWT',  // used for raw byte arrays
   B_MIME_TYPE    = 1296649541  // 'MIME',  // new for v1.44
};
#endif

// This constant is used in various places to mean 'as much as you want'
#define MUSCLE_NO_LIMIT ((uint32)-1)

// Constants that aren't actual BeOS field types, but I'll continue 
// using the naming convention, just for consistency
enum {
   B_TAG_TYPE     = 1297367367  // 'MTAG'  // new for v2.00; for in-mem-only tags
};

/** A handy little method to swap the bytes of any int-style datatype around */
template<typename T> inline T muscleSwapBytes(T swapMe)
{
   T retVal;
   const uint8 * readFrom = (const uint8 *) &swapMe;
   uint8 * writeTo  = ((uint8 *) &retVal)+sizeof(retVal);
   for (uint32 i=0; i<sizeof(swapMe); i++) {*(--writeTo) = *(readFrom++);}
   return retVal;
}

/** This template safely copies a value in from an untyped byte buffer to a typed value. 
  * (Make sure MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT is defined if you are on a CPU
  *  that doesn't like non-word-aligned data reads and writes)
  */
template<typename T> inline void muscleCopyIn(T & dest, const void * source) 
{
#ifdef MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT
   memcpy(&dest, source, sizeof(dest));
#else
   dest = *((const T*)source);
#endif
}

/** This template safely copies a value in from a typed value to an untyped byte buffer.
  * (Make sure MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT is defined if you are on a CPU
  *  that doesn't like non-word-aligned data reads and writes)
  */
template<typename T> inline void muscleCopyOut(void * dest, const T & source) 
{
#ifdef MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT
   memcpy(dest, &source, sizeof(source));
#else
   *((T*)dest) = source;
#endif
}

/** Swaps the two arguments */
template<typename T> inline void muscleSwap(T& p1, T& p2) {T t = p1; p1 = p2; p2 = t;}

/** Returns the smaller of the two arguments */
template<typename T> inline const T & muscleMin(const T& p1, const T& p2) {return (p1 < p2) ? p1 : p2;}

/** Returns the larger of the two arguments */
template<typename T> inline const T & muscleMax(const T& p1, const T& p2) {return (p1 < p2) ? p2 : p1;}

/** Returns the value nearest to (v) that is still in the range [lo, hi]. */
template<typename T> inline const T & muscleClamp(const T & v, const T& lo, const T& hi) {return (v < lo) ? lo : ((v > hi) ? hi : v);}

/** Returns true iff (v) is in the range [lo,hi]. */
template<typename T> inline bool muscleInRange(const T & v, const T& lo, const T& hi) {return ((v >= lo)&&(v <= hi));}

/** Returns -1 if arg1 is larger, or 1 if arg2 is larger, or 0 if they are equal. */
template<typename T> inline int muscleCompare(const T & arg1, const T & arg2) {return (arg1>arg2) ? 1 : ((arg1<arg2) ? -1 : 0);}

/** Returns the absolute value of (arg) */
template<typename T> inline T muscleAbs(const T & arg) {return (arg<0)?(-arg):arg;}

/** Rounds the given float to the nearest integer value. */
inline int muscleRintf(float f) {return (f>=0.0f) ? ((int)(f+0.5f)) : -((int)((-f)+0.5f));}

#ifndef __BEOS__
# if defined(__CYGWIN__) || defined(_M_IX86) || defined(__GNUWIN32__) // Cygwin is for Windows on x86, hence little endian
#  define LITTLE_ENDIAN 1234
#  define BIG_ENDIAN    4321
#  define BYTE_ORDER LITTLE_ENDIAN  
# elif defined(__FreeBSD__) || defined(__APPLE__)
#  include <machine/endian.h>
# else
#  include <endian.h>  // (non-standard) POSIX-ish include, defines BYTE_ORDER as LITTLE_ENDIAN or BIG_ENDIAN
# endif
# define SWAP_DOUBLE(arg)   muscleSwapBytes((double)(arg))
# define SWAP_FLOAT(arg)    muscleSwapBytes((float)(arg))
# define SWAP_INT64(arg)    muscleSwapBytes((uint64)(arg))
# define SWAP_INT32(arg)    muscleSwapBytes((uint32)(arg))
# define SWAP_INT16(arg)    muscleSwapBytes((uint16)(arg))
# if BYTE_ORDER == LITTLE_ENDIAN
#  define B_HOST_IS_LENDIAN 1
#  define B_HOST_IS_BENDIAN 0
#  define B_HOST_TO_LENDIAN_DOUBLE(arg) ((double)(arg))
#  define B_HOST_TO_LENDIAN_FLOAT(arg)  ((float)(arg))
#  define B_HOST_TO_LENDIAN_INT64(arg)  ((uint64)(arg))
#  define B_HOST_TO_LENDIAN_INT32(arg)  ((uint32)(arg))
#  define B_HOST_TO_LENDIAN_INT16(arg)  ((uint16)(arg))
#  define B_HOST_TO_BENDIAN_DOUBLE(arg) SWAP_DOUBLE(arg)
#  define B_HOST_TO_BENDIAN_FLOAT(arg)  SWAP_FLOAT(arg)
#  define B_HOST_TO_BENDIAN_INT64(arg)  SWAP_INT64(arg)
#  define B_HOST_TO_BENDIAN_INT32(arg)  SWAP_INT32(arg)
#  define B_HOST_TO_BENDIAN_INT16(arg)  SWAP_INT16(arg)
#  define B_LENDIAN_TO_HOST_DOUBLE(arg) ((double)(arg))
#  define B_LENDIAN_TO_HOST_FLOAT(arg)  ((float)(arg))
#  define B_LENDIAN_TO_HOST_INT64(arg)  ((uint64)(arg))
#  define B_LENDIAN_TO_HOST_INT32(arg)  ((uint32)(arg))
#  define B_LENDIAN_TO_HOST_INT16(arg)  ((uint16)(arg))
#  define B_BENDIAN_TO_HOST_DOUBLE(arg) SWAP_DOUBLE(arg)
#  define B_BENDIAN_TO_HOST_FLOAT(arg)  SWAP_FLOAT(arg)
#  define B_BENDIAN_TO_HOST_INT64(arg)  SWAP_INT64(arg)
#  define B_BENDIAN_TO_HOST_INT32(arg)  SWAP_INT32(arg)
#  define B_BENDIAN_TO_HOST_INT16(arg)  SWAP_INT16(arg)
# else /* LITTLE_ENDIAN */
#  define B_HOST_IS_LENDIAN 0
#  define B_HOST_IS_BENDIAN 1
#  define B_HOST_TO_LENDIAN_DOUBLE(arg) SWAP_DOUBLE(arg)
#  define B_HOST_TO_LENDIAN_FLOAT(arg)  SWAP_FLOAT(arg)
#  define B_HOST_TO_LENDIAN_INT64(arg)  SWAP_INT64(arg)
#  define B_HOST_TO_LENDIAN_INT32(arg)  SWAP_INT32(arg)
#  define B_HOST_TO_LENDIAN_INT16(arg)  SWAP_INT16(arg)
#  define B_HOST_TO_BENDIAN_DOUBLE(arg) ((double)(arg))
#  define B_HOST_TO_BENDIAN_FLOAT(arg)  ((float)(arg))
#  define B_HOST_TO_BENDIAN_INT64(arg)  ((uint64)(arg))
#  define B_HOST_TO_BENDIAN_INT32(arg)  ((uint32)(arg))
#  define B_HOST_TO_BENDIAN_INT16(arg)  ((uint16)(arg))
#  define B_LENDIAN_TO_HOST_DOUBLE(arg) SWAP_DOUBLE(arg)
#  define B_LENDIAN_TO_HOST_FLOAT(arg)  SWAP_FLOAT(arg)
#  define B_LENDIAN_TO_HOST_INT64(arg)  SWAP_INT64(arg)
#  define B_LENDIAN_TO_HOST_INT32(arg)  SWAP_INT32(arg)
#  define B_LENDIAN_TO_HOST_INT16(arg)  SWAP_INT16(arg)
#  define B_BENDIAN_TO_HOST_DOUBLE(arg) ((double)(arg))
#  define B_BENDIAN_TO_HOST_FLOAT(arg)  ((float)(arg))
#  define B_BENDIAN_TO_HOST_INT64(arg)  ((uint64)(arg))
#  define B_BENDIAN_TO_HOST_INT32(arg)  ((uint32)(arg))
#  define B_BENDIAN_TO_HOST_INT16(arg)  ((uint16)(arg))
# endif /* !LITTLE_ENDIAN */
#endif /* !__BEOS__ */

//:Macro to turn a type code into a string representation.
// (typecode) is the type code to get the string for
// (buf) is a (char *) to hold the output string; it must be >= 5 bytes long.
#define MakePrettyTypeCodeString(typecode, buf)                     \
   {                                                                \
      uint32 __bigEndian = B_HOST_TO_BENDIAN_INT32(typecode);       \
      memcpy(buf, (const char *)&__bigEndian, sizeof(__bigEndian)); \
      buf[sizeof(__bigEndian)] = '\0';                              \
   }           

#include <stdio.h>
#include <stdlib.h>
#include "syslog/SysLog.h"  // for LogTime()

#endif /* _MUSCLE_SUPPORT_H */
