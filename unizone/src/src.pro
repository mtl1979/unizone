TARGET = Unizone
DEBUG_SOURCES =
# uncomment and set following define to base directory of openssl to enable SSL support
#SSLDIR = c:\\build\\openssl-1.0.1h

#We can't rely on having "debug" in CONFIG if we set "release" as default target
CONFIG(debug, debug|release) {
	DEFINES		 += _DEBUG
	DEBUG_SOURCES = debugimpl.cpp
}

CONFIG += precompile_header
CONFIG -= debug

#Source files for Unizone
SOURCES1 =	aboutdlgimpl.cpp \
			botitem.cpp \
			channelimpl.cpp \
			channelinfo.cpp \
			channels.cpp \
			chatevent.cpp \
			chattext.cpp \
			chatwindow.cpp \
			combo.cpp \
			downloadimpl.cpp \
			downloadqueue.cpp \
			downloadthread.cpp \
			downloadworker.cpp \
			events.cpp \
			fileinfo.cpp \
			filethread.cpp \
			htmlview.cpp \
			listthread.cpp \
			listutil.cpp \
			main.cpp \
			md5.cpp \
			menubar.cpp \
			messageutil.cpp \
			netclient.cpp \
			nicklist.cpp \
			picviewerimpl.cpp \
			platform.cpp \
			prefsimpl.cpp \
			privatewindowimpl.cpp \
			regex_new.cpp \
			regex_old.cpp \
			resolver.cpp \
			resolver4.cpp \
			resolver6.cpp \
			resolverthread.cpp \
			search.cpp \
			searchitem.cpp \
			serverclient.cpp \
			settings.cpp \
			textevent.cpp \
			titanic.cpp \
			tokenizer.cpp \
			transferitem.cpp \
			uenv.cpp \
			ulistview.cpp \
			updateclient.cpp \
			uploadimpl.cpp \
			uploadthread.cpp \
			user.cpp \
			userlistitem.cpp \
			util.cpp \
			version.cpp \
			wcrypt.cpp \
			werrorevent.cpp \
			winshare_lists.cpp \
			winshare_parsing.cpp \
			winshare_network.cpp \
			winshare_slots.cpp \
			winsharewindow.cpp \
			wmessageevent.cpp \
			wpwevent.cpp \
			wstatusbar.cpp \
			wstring.cpp \
			wsystemevent.cpp \
			wwarningevent.cpp \
			Log.cpp

SOURCES1 +=	$$DEBUG_SOURCES

#Source files for MUSCLE
SOURCES2 =  muscle/qtsupport/QMessageTransceiverThread.cpp \
			muscle/qtsupport/QAcceptSocketsThread.cpp \
			muscle/iogateway/AbstractMessageIOGateway.cpp \
			muscle/iogateway/MessageIOGateway.cpp \
			muscle/iogateway/PlainTextMessageIOGateway.cpp \
			muscle/iogateway/RawDataMessageIOGateway.cpp \
			muscle/message/Message.cpp \
			muscle/reflector/AbstractReflectSession.cpp \
			muscle/reflector/DataNode.cpp \
			muscle/reflector/DumbReflectSession.cpp \
			muscle/reflector/FilterSessionFactory.cpp \
			muscle/reflector/RateLimitSessionIOPolicy.cpp \
			muscle/reflector/ReflectServer.cpp \
			muscle/reflector/ServerComponent.cpp \
			muscle/reflector/SignalHandlerSession.cpp \
			muscle/reflector/StorageReflectSession.cpp \
			muscle/regex/PathMatcher.cpp \
			muscle/regex/QueryFilter.cpp \
			muscle/regex/StringMatcher.cpp \
			muscle/syslog/SysLog.cpp \
			muscle/system/AcceptSocketsThread.cpp \
			muscle/system/GlobalMemoryAllocator.cpp \
			muscle/system/MessageTransceiverThread.cpp \
			muscle/system/SetupSystem.cpp \
			muscle/system/SignalMultiplexer.cpp \
			muscle/system/SystemInfo.cpp \
			muscle/system/Thread.cpp \
			muscle/util/ByteBuffer.cpp \
			muscle/util/Directory.cpp \
			muscle/util/FilePathInfo.cpp \
			muscle/util/MemoryAllocator.cpp \
			muscle/util/MiscUtilityFunctions.cpp \
			muscle/util/NetworkUtilityFunctions.cpp \
			muscle/util/PulseNode.cpp \
			muscle/util/SocketMultiplexer.cpp \
			muscle/util/String.cpp \
			muscle/zlib/ZLibCodec.cpp \
			muscle/zlib/ZLibUtilityFunctions.cpp

FORMS = 	aboutdlg.ui \
			picviewer.ui \
			prefs.ui \
			scanprogress.ui

HEADERS =	aboutdlgimpl.h \
			channelimpl.h \
			channels.h \
			chattext.h \
			chatwindow.h \
			combo.h \
			downloadimpl.h \
			downloadthread.h \
			filethread.h \
			htmlview.h \
			listthread.h \
			menubar.h \
			netclient.h \
			picviewerimpl.h \
			prefsimpl.h \
			privatewindowimpl.h \
			resolverthread.h \
			scanprogressimpl.h \
			search.h \
			serverclient.h \
			tokenizer.h \
			ulistview.h \
			updateclient.h \
			uploadimpl.h \
			uploadthread.h \
			user.h \
			util.h \
			winsharewindow.h \
			muscle/qtsupport/QMessageTransceiverThread.h \
			muscle/qtsupport/QAcceptSocketsThread.h

RESOURCES    = unizone.qrc

TRANSLATIONS =  unizone_en.ts \
				unizone_fi.ts \
				unizone_fr.ts \
				unizone_de.ts \
				unizone_kr.ts \
				unizone_pt.ts \
				unizone_ru.ts \
				unizone_es.ts \
				unizone_sv.ts \
				unizone_1337.ts

PRECOMPILED_HEADER = unizone_pch.h

CODEC =		UTF-8

DEFINES += MUSCLE_ENABLE_ZLIB_ENCODING _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS
!isEmpty(SSL_DIR) {
	DEFINES += MUSCLE_ENABLE_SSL
	SOURCES2 +=	muscle/dataio/SSLSocketDataIO.cpp \
			    muscle/iogateway/SSLSocketAdapterGateway.cpp
	win32 {
		LIBS += -L$$SSLDIR\\out32dll libeay32.lib ssleay32.lib
		SOURCES2 += $$SSLDIR\\inc32\\openssl\\applink.c
	} else: LIBS += -L$$SSLDIR/lib -llibeay32 -lssleay32
}

win32 {
	DEFINES += WIN32_LEAN_AND_MEAN UNICODE REGEX_USEDLL
	LIBS += ole32.lib shlwapi.lib user32.lib ws2_32.lib winmm.lib iphlpapi.lib shell32.lib advapi32.lib version.lib regex.lib
	!contains(CONFIG, zlib):LIBS += zlib1.lib
	SOURCES1 += scanprogressimpl.cpp \
			   windows/_filwbuf.c \
			   windows/_getbuf.c \
			   windows/vswscanf.c \
			   windows/fileinfo_win.cpp \
			   windows/filethread_win.cpp \
			   windows/gotourl_win.cpp \
			   windows/wfile.cpp \
			   windows/wfile_win.cpp \
			   windows/wlaunchthread_win.cpp \
			   windows/wutil_msvc.cpp
	RC_FILE =  icon.rc
	!isEmpty(DEBUG_SOURCES) {
		LIBS += -L..\\regex\\debug 
		!contains(CONFIG, zlib):LIBS += -L..\\zlib\\debug
		qtlibs.files = $$[QT_INSTALL_LIBS]\\Qt3Supportd4.dll \
					   $$[QT_INSTALL_LIBS]\\QtCored4.dll \
					   $$[QT_INSTALL_LIBS]\\QtGuid4.dll \
					   $$[QT_INSTALL_LIBS]\\QtNetworkd4.dll \
					   $$[QT_INSTALL_LIBS]\\QtSqld4.dll \
					   $$[QT_INSTALL_LIBS]\\QtXmld4.dll
	} else {
		LIBS += -L..\\regex\\release 
		!contains(CONFIG, zlib):LIBS += -L..\\zlib\\release
		qtlibs.files = $$[QT_INSTALL_LIBS]\\Qt3Support4.dll \
					   $$[QT_INSTALL_LIBS]\\QtCore4.dll \
					   $$[QT_INSTALL_LIBS]\\QtGui4.dll \
					   $$[QT_INSTALL_LIBS]\\QtNetwork4.dll \
					   $$[QT_INSTALL_LIBS]\\QtSql4.dll \
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
	SOURCES1 += unix/fileinfo_unix.cpp \
			    unix/filethread_unix.cpp \
			    unix/gotourl_unix.cpp \
			    unix/mimedb.cpp \
			    unix/wcslwr.c \
			    unix/wcsupr.c \
			    unix/wfile.cpp \
			    unix/wfile_unix.cpp \
			    unix/wlaunchthread_unix.cpp \
			    unix/wutil_unix.cpp
	!contains(CONFIG, zlib):LIBS += -lz
}

SOURCES  =  $$SOURCES1 $$SOURCES2

QT += qt3support

INCLUDEPATH += muscle ../regex ../zlib


target.path = ../..
INSTALLS += target

isEmpty(QMAKE_LUPDATE) {
	win32:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]\\lupdate.exe
	else:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/lupdate
}

updatets.input = TRANSLATIONS 
updatets.depends = $$SOURCES1 $$FORMS src.pro
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
