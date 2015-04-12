TARGET = zlib1
TEMPLATE = lib
CONFIG -= qt debug
CONFIG += dll

win32 {
	DEFINES += WINAPI_FAMILY=100
	RC_FILE = win32\\zlib1.rc
	DEF_FILE = win32\\zlib.def
}

SOURCES +=	adler32.c \
			compress.c \
			crc32.c \
			deflate.c \
			gzclose.c \
			gzlib.c \
			gzread.c \
			gzwrite.c \
			infback.c \
			inffast.c \
			inflate.c \
			inftrees.c \
			trees.c \
			uncompr.c \
			zutil.c

HEADERS +=	crc32.h \
			deflate.h \
			inffast.h \
			inffixed.h \
			inflate.h \
			inftrees.h \
			trees.h \
			zconf.h \
			zlib.h \
			zutil.h

target.path = ../..
INSTALLS += target