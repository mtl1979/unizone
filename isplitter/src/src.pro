unix:TARGET=isplitter
win32:TARGET=ImageSplitter
DEBUG_SOURCES =

#We can't rely on having "debug" in CONFIG if we set "release" as default target
CONFIG(debug, debug|release) {
	DEFINES		 += _DEBUG
	DEBUG_SOURCES = debugimpl.cpp
}
CONFIG -= debug

SOURCES =	main.cpp \
			mainwindowimpl.cpp \
			menubar.cpp \
			previewimpl.cpp \
			uenv.cpp \
			util.cpp \
			wstring.cpp


SOURCES +=	$$DEBUG_SOURCES

FORMS =		mainwindow.ui

HEADERS =	menubar.h \
			mainwindowimpl.h \
			previewimpl.h

TRANSLATIONS =	isplitter_en.ts \
				isplitter_fi.ts

CODEC =		UTF-8


win32 {
	DEFINES += _CRT_SECURE_NO_WARNINGS
	LIBS += shlwapi.lib
	SOURCES +=	windows/wfile.cpp \
				windows/wfile_win.cpp \
				windows/wutil_msvc.cpp \
				windows/vswscanf.c \
				windows/_filwbuf.c \
				windows/_getbuf.c
	RC_FILE =	isplitter.rc

	!isEmpty(DEBUG_SOURCES) {
		qtlibs.files = $$[QT_INSTALL_LIBS]\\Qt3Supportd4.dll \
					   $$[QT_INSTALL_LIBS]\\QtCored4.dll \
					   $$[QT_INSTALL_LIBS]\\QtGuid4.dll \
					   $$[QT_INSTALL_LIBS]\\QtNetworkd4.dll \
					   $$[QT_INSTALL_LIBS]\\QtSqld4.dll \
					   $$[QT_INSTALL_LIBS]\\QtSvgd4.dll \
					   $$[QT_INSTALL_LIBS]\\QtXmld4.dll
	} else {
		qtlibs.files = $$[QT_INSTALL_LIBS]\\Qt3Support4.dll \
					   $$[QT_INSTALL_LIBS]\\QtCore4.dll \
					   $$[QT_INSTALL_LIBS]\\QtGui4.dll \
					   $$[QT_INSTALL_LIBS]\\QtNetwork4.dll \
					   $$[QT_INSTALL_LIBS]\\QtSql4.dll \
					   $$[QT_INSTALL_LIBS]\\QtSvg4.dll \
					   $$[QT_INSTALL_LIBS]\\QtXml4.dll
	}
	qtlibs.path = ../..
	qtlibs.CONFIG += recursive
	INSTALLS += qtlibs

	qtimageformats.path = ../../plugins/imageformats
	qtimageformats.files = $$[QT_INSTALL_PLUGINS]\\imageformats\\qsvg4.dll \
					  $$[QT_INSTALL_PLUGINS]\\imageformats\\qgif4.dll \
					  $$[QT_INSTALL_PLUGINS]\\imageformats\\qico4.dll \
					  $$[QT_INSTALL_PLUGINS]\\imageformats\\qjpeg4.dll \
					  $$[QT_INSTALL_PLUGINS]\\imageformats\\qmng4.dll \
					  $$[QT_INSTALL_PLUGINS]\\imageformats\\qtga4.dll \
					  $$[QT_INSTALL_PLUGINS]\\imageformats\\qtiff4.dll

	qtimageformats.CONFIG += recursive
	INSTALLS += qtimageformats
}


unix {
	SOURCES +=	unix/wfile.cpp \
				unix/wfile_unix.cpp \
				unix/wutil_unix.cpp \
				unix/wcsdup.c \
				unix/wcslwr.c \
				unix/wcsupr.c
}


QT += qt3support



target.path = ../..
INSTALLS += target

!equals(TEMPLATE, vcapp) {
isEmpty(QMAKE_LUPDATE) {
	win32:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]\\lupdate.exe
	else:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/lupdate
}

updatets.input = TRANSLATIONS
updatets.depends = $$SOURCES $$FORMS src.pro
updatets.output = ${QMAKE_FILE_IN}
updatets.commands = $$QMAKE_LUPDATE src.pro
updatets.CONFIG += no_link no_clean explicit_dependencies recursive target_predeps

QMAKE_EXTRA_COMPILERS += updatets

isEmpty(QMAKE_LRELEASE) {
	win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
	else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link recursive target_predeps

QMAKE_EXTRA_COMPILERS += updateqm

translations.files = $$TRANSLATIONS
translations.files ~= s/\\.ts/.qm/g
translations.path = ../../translations
translations.CONFIG += recursive
INSTALLS += translations
}