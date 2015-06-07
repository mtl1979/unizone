#include <stdio.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>


int
main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <input file>\n", argv[0]);
		return -1;
	}
	else
	{
		wchar_t *out;
		char *test;
		int wlen, tlen;
		wlen = MultiByteToWideChar(51949, 0, argv[1], (int) strlen(argv[1]), NULL, 0);
		out = new wchar_t[wlen + 1];
		if (out)
		{
			(void) MultiByteToWideChar(51949, 0, argv[1], (int) strlen(argv[1]), out, wlen);
		}
		else
		{
			printf("Out of memory!\n");
			return -2;
		}
		tlen = WideCharToMultiByte(51949, 0, out, wlen, NULL, 0, NULL, NULL);
		test = new char[tlen + 1];
		if (test)
		{
			(void) WideCharToMultiByte(51949, 0, out, wlen, test, tlen, NULL, NULL);
		}
		else
		{
			delete [] out;
			printf("Out of memory!\n");
			return -2;
		}
		if (strcmp(argv[1], test) != 0)
		{
			printf("Decoder error!\n");
			delete [] out;
			delete [] test;
			return -2;
		}
		// test that filenames aren't identical
		bool same;
		if (tlen == wlen)
		{
			bool same = true;
			for (int x = 0; x < tlen; x++)
			{
				if ((wchar_t) argv[1][x] != out[x])
				{
					same = false;
					break;
				}
			}
		}
		else
			same = false;
		
		if (!same)
		{
			FILE *fin, *fout;
			errno_t err;
			__int64 sz = 0;
			char buf[1024];
			err = fopen_s(&fin, argv[1], "rb");
			if (err != 0)
			{
				printf("Error opening input file!\n");
				return -3;
			}
			err = _wfopen_s(&fout, out, L"wb");
			if (err != 0)
			{
				fclose(fin);
				printf("Error opening output file!\n");
				return -3;
			}
#ifdef WIN32
			struct _stati64 s;
			_stati64(argv[1], &s);
#else
			struct stat64 s;
			stat64(argv[1], &s);
#endif
			sz = s.st_size;
			size_t r;
			__int64 numbytes = 0;
			while ((r = fread(buf, 1, 1024, fin)) > 0)
			{
				fwrite(buf, 1, r, fout);
				numbytes += r;
			}
			printf("Read %Li bytes.\n", sz);
			printf("Wrote %Li bytes.\n", numbytes);
			fclose(fin);
			fclose(fout);
			if (sz == numbytes)
			{
				remove(argv[1]);
			}		
			else
			{
				_wremove(out);
			}
		}
		delete [] out;
		delete [] test;
	}
	return 0;
}
