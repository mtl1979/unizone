# -------------------------------------------------
# Project created by QtCreator 2009-03-15T19:26:47
# -------------------------------------------------
QT -=	core \
		gui
TARGET = zlib1
TEMPLATE = lib
CONFIG += dll

win32 {
	DEFINES += _CRT_SECURE_NO_WARNINGS
	RC_FILE = win32\\zlib1.rc
	DEF_FILE = win32\\zlib.def
	QMAKE_LFLAGS_DLL += /implib:$(DESTDIR)zdll.lib
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
