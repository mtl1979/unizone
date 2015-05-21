#if _MSC_VER < 1900
#include <windows.h>
#include <winbase.h>
#include <crtdbg.h>
#include <stdio.h>
#include <io.h>

#define _SMALL_BUFSIZ       512

#define _INTERNAL_BUFSIZ	4096

#define FEOFLAG         0x02    /* end of file has been encountered */
#define FTEXT           0x80    /* file handle is in text mode */


#define _IOYOURBUF      0x0100
#define _IOSETVBUF      0x0400
#define _IOFEOF         0x0800
#define _IOFLRTN        0x1000
#define _IOCTRLZ        0x2000
#define _IOCOMMIT       0x4000


#define REG1 register

#define inuse(s)        ((s)->_flag & (_IOREAD|_IOWRT|_IORW))
#define anybuf(s)       ((s)->_flag & (_IOMYBUF|_IONBF|_IOYOURBUF))

void __cdecl _getbuf (FILE *str);

/*
 * Control structure for lowio file handles
 */
typedef struct {
        long osfhnd;    /* underlying OS file HANDLE */
        char osfile;    /* attributes of file (e.g., open in text mode?) */
        char pipech;    /* one char buffer for handles opened on pipes */
#ifdef _MT
        int lockinitflag;
        CRITICAL_SECTION lock;
#endif  /* _MT */
    }   ioinfo;

/*
 * Special, static ioinfo structure used only for more graceful handling
 * of a C file handle value of -1 (results from common errors at the stdio
 * level).
 */
extern _CRTIMP ioinfo __badioinfo;

/*
 * Definition of IOINFO_L2E, the log base 2 of the number of elements in each
 * array of ioinfo structs.
 */
#define IOINFO_L2E          5

/*
 * Definition of IOINFO_ARRAY_ELTS, the number of elements in ioinfo array
 */
#define IOINFO_ARRAY_ELTS   (1 << IOINFO_L2E)

extern _CRTIMP ioinfo * __pioinfo[];

#define _pioinfo(i) ( __pioinfo[(i) >> IOINFO_L2E] + ((i) & (IOINFO_ARRAY_ELTS - \
					1 )) )
#define _pioinfo_safe(i)    ( ((i) != -1) ? _pioinfo(i) : &__badioinfo )
#define _osfile_safe(i)     ( _pioinfo_safe(i)->osfile )

/***
*int _filwbuf(stream) - fill buffer and get first wide character
*
*Purpose:
*       get a buffer if the file doesn't have one, read into it, return first
*       char. try to get a buffer, if a user buffer is not assigned. called
*       only from getc; intended for use only within library. assume no input
*       stream is to remain unbuffered when memory is available unless it is
*       marked _IONBF. at worst, give it a single char buffer. the need for a
*       buffer, no matter how small, becomes evident when we consider the
*       ungetc's necessary in scanf
*
*       [NOTE: Multi-thread - _filwbuf() assumes that the caller has aquired
*       the stream lock, if needed.]
*
*Entry:
*       FILE *stream - stream to read from
*
*Exit:
*       returns first wide character from buffer (next character to be read)
*       returns WEOF if the FILE is actually a string, or not open for reading,
*       or if open for writing or if no more chars to read.
*       all fields in FILE structure may be changed except _file.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _filwbuf (
        FILE *str
        )

{

        REG1 FILE *stream;

        _ASSERTE(str != NULL);

        /* Init pointer to _iob2 entry. */
        stream = str;

        if (!inuse(stream) || stream->_flag & _IOSTRG)
                return(WEOF);

        if (stream->_flag & _IOWRT) {
                stream->_flag |= _IOERR;
                return(WEOF);
        }

        stream->_flag |= _IOREAD;

        /* Get a buffer, if necessary. */

        if (!anybuf(stream))
                _getbuf(stream);
        else
                stream->_ptr = stream->_base;

        stream->_cnt = _read(_fileno(stream), stream->_base, stream->_bufsiz);

        if ((stream->_cnt == 0) || (stream->_cnt == 1) || stream->_cnt == -1) {
                stream->_flag |= stream->_cnt ? _IOERR : _IOEOF;
                stream->_cnt = 0;
                return(WEOF);
        }

        if (  !(stream->_flag & (_IOWRT|_IORW)) &&
              ((_osfile_safe(_fileno(stream)) & (FTEXT|FEOFLAG)) ==
                (FTEXT|FEOFLAG)) )
                stream->_flag |= _IOCTRLZ;
        /* Check for small _bufsiz (_SMALL_BUFSIZ). If it is small and
           if it is our buffer, then this must be the first _filbuf after
           an fseek on a read-access-only stream. Restore _bufsiz to its
           larger value (_INTERNAL_BUFSIZ) so that the next _filbuf call,
           if one is made, will fill the whole buffer. */
        if ( (stream->_bufsiz == _SMALL_BUFSIZ) && (stream->_flag &
              _IOMYBUF) && !(stream->_flag & _IOSETVBUF) )
        {
                stream->_bufsiz = _INTERNAL_BUFSIZ;
        }
        stream->_cnt -= sizeof(wchar_t);
        return (0xffff & *((wchar_t *)(stream->_ptr))++);

}
#endif