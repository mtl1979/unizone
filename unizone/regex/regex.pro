TARGET = regex
TEMPLATE = lib
CONFIG -= qt debug
CONFIG += dll

DEFINES += REGEX_DLL POSIX_MISTAKE
win32: DEFINES += _CRT_SECURE_NO_WARNINGS

SOURCES +=  regcomp.c \
            regerror.c \
            regexec.c \
            regfree.c
HEADERS +=  regex.h \
            regex2.h \
            utils.h

target.path = ../..
INSTALLS += target