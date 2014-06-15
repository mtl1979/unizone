# -------------------------------------------------
# Project created by QtCreator 2009-03-15T19:26:47
# -------------------------------------------------
TARGET = regex
TEMPLATE = lib
CONFIG -= qt
CONFIG += dll

DEFINES += REGEX_DLL
win32: DEFINES += _CRT_SECURE_NO_WARNINGS

SOURCES +=  regcomp.c \
            regerror.c \
            regexec.c \
            regfree.c
HEADERS +=  regex.h \
            regex2.h \
            utils.h
