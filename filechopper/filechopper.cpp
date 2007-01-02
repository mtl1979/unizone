// This little utility lets you chop the last <n> bytes off
// of a file easily.  Default is to chop the last 64K off.
// Handy if you've got a big file that BeShare won't resume
// downloading on because the last bit got messed up during
// a disk crash.
//
// Compile thus:  gcc filechopper.cpp
//
// Usage:  filechopper <filename> [numbytestoremove=65536]
//

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#define _topen open
#define _tfopen fopen
#define _chsize ftruncate
#define _tremove remove
#define _tmain main
#define _tprintf printf
#define _tcscat strcat
#define _ttol atol
#define _TCHAR char
#define _T(x) x
#else
#include <io.h>
#include <tchar.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int 
_tmain(int argc, _TCHAR ** argv)
{
   const long defaultChop = 65536;
   if ((argc != 3)&&(argc != 2))
   {
      printf("filechopper removes the last (n) bytes from the file you\n");
      printf("specify, which can be useful if your machine/net_server crashed\n");
      printf("during download and it won't resume now (because it has\n");
      printf("garbage bytes in it from the crash)\n");
      printf("usage:  filechopper <filename> [numbytestoremove=%li]\n", defaultChop);
      exit(0);
   }

   long count = (argc == 3) ? _ttol(argv[2]) : defaultChop;
   FILE * fp = _tfopen(argv[1], _T("r"));
   if (fp)
   {
      fseek(fp, 0, SEEK_END);
      long fileLen = ftell(fp);
      fclose(fp);

      int fd = _topen(argv[1], O_RDWR|O_APPEND); 
      if (fd >= 0)
      {
         long newSize = fileLen - count;
         if (newSize >= 0)
         {
            _chsize(fd, newSize);
            _tprintf(_T("Chopped the last %li bytes off of [%s], new size is [%li]\n"), count, argv[1], newSize);
			_TCHAR * mdfile = _tcscat(argv[1], _T(".md5"));
			if (_tremove(mdfile) != -1)
				_tprintf(_T("Removed md5 hash file %s\n"), mdfile);
         }
         else printf("File is too short to chop that many bytes off!\n");
      }
      else _tprintf(_T("Error, file [%s] couldn't be opened???\n"), argv[1]);
   }
   else _tprintf(_T("Error, file [%s] not found\n"), argv[1]);

   return 0;
}
