unix:TARGET=../isplitter
win32:TARGET="Image Splitter"

SOURCES = main.cpp \
    mainwindowimpl.cpp \
    menubar.cpp \
    previewimpl.cpp \
    util.cpp \
    wstring.cpp \
    uenv.cpp

FORMS = mainwindow.ui
TRANSLATIONS = isplitter_en.ts \
    isplitter_fi.ts
CODEC = UTF-8
QT += qt3support

HEADERS += menubar.h \
    mainwindowimpl.h \
    previewimpl.h

win32 {
	RC_FILE =	isplitter.rc
	SOURCES +=		windows/wfile.cpp \
				windows/wfile_win.cpp \
				windows/wutil_msvc.cpp \
    				windows/vwsscanf.c \
    				windows/_filwbuf.c \
    				windows/_getbuf.c
	LIBS += shlwapi.lib
	DEFINES += _CRT_SECURE_NO_WARNINGS
}

unix {
	SOURCES +=	unix/wfile.cpp \
			unix/wfile_unix.cpp \
			unix/wutil_unix.cpp \
			unix/wcsdup.c \
    			unix/wcslwr.c \
    			unix/wcsupr.c
}

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    SOURCES += debugimpl.cpp
}

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link

QMAKE_EXTRA_COMPILERS += updateqm

PRE_TARGETDEPS += compiler_updateqm_make_all