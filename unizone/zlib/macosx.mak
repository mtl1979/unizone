
C_FILES=adler32.c compress.c crc32.c deflate.c gzio.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c
O_FILES=$(C_FILES:.c=.o)
CC=gcc-4.2

SDK=/Developer/SDKs/MacOSX10.5.sdk

CFLAGS+=-arch x86_64 -arch i386 -arch ppc -arch ppc64  -isysroot $(SDK)

all : libz.a

libz.a : $(O_FILES)
	libtool -static -o libz.a - $(O_FILES)

clean :
	-rm $(O_FILES)
	-rm libz.a
 
