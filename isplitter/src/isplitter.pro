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
CODEC = utf-8
QT += qt3support

HEADERS += menubar.h \
    mainwindowimpl.h \
    previewimpl.h

win32 {
	OTHER_FILES +=	isplitter.rc
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
			unix/wcsdup.cpp \
    			unix/wcslwr.cpp \
    			unix/wcsupr.cpp
}

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    SOURCES += debugimpl.cpp
}
