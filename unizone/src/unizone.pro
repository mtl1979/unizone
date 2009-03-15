SOURCES =	aboutdlgimpl.cpp \
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
                formatting.cpp \
                htmlview.cpp \
                listthread.cpp \
                main.cpp \
                md5.cpp \
                menubar.cpp \
                netclient.cpp \
                nicklist.cpp \
                picviewerimpl.cpp \
                platform.cpp \
                prefsimpl.cpp \
                privatewindowimpl.cpp \
                resolver.cpp \
                resolverthread.cpp \
                search.cpp \
                searchitem.cpp \
                serverclient.cpp \
                settings.cpp \
                textevent.cpp \
                titanic.cpp \
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
                Log.cpp \
                muscle/qtsupport/QMessageTransceiverThread.cpp \
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
                muscle/reflector/StorageReflectSession.cpp \
                muscle/regex/PathMatcher.cpp \
                muscle/regex/QueryFilter.cpp \
                muscle/regex/StringMatcher.cpp \
                muscle/syslog/SysLog.cpp \
                muscle/system/AcceptSocketsThread.cpp \
                muscle/system/GlobalMemoryAllocator.cpp \
                muscle/system/MessageTransceiverThread.cpp \
                muscle/system/SetupSystem.cpp \
                muscle/system/SystemInfo.cpp \
                muscle/system/Thread.cpp \
                muscle/util/ByteBuffer.cpp \
                muscle/util/MemoryAllocator.cpp \
                muscle/util/MiscUtilityFunctions.cpp \
                muscle/util/NetworkUtilityFunctions.cpp \
                muscle/util/PulseNode.cpp \
                muscle/util/String.cpp \
                muscle/zlib/ZLibCodec.cpp \
                muscle/zlib/ZLibUtilityFunctions.cpp

FORMS = 	aboutdlg.ui \
                channel.ui \
                picviewer.ui \
                prefs.ui \
                privatewindow.ui \
                scanprogress.ui

HEADERS =       aboutdlgimpl.h \
                channelimpl.h \
                channels.h \
                chattext.h \
                combo.h \
                downloadimpl.h \
                downloadthread.h \
                filethread.h \
                formatting.h \
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
                ulistview.h \
                updateclient.h \
                uploadimpl.h \
                uploadthread.h \
                winsharewindow.h \
                muscle/qtsupport/QMessageTransceiverThread.h \
                muscle/qtsupport/QAcceptSocketsThread.h


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

CODEC =		utf-8

DEFINES += MUSCLE_ENABLE_ZLIB_ENCODING _CRT_SECURE_NO_WARNINGS

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    SOURCES += debugimpl.cpp
    win32:LIBS += -L../regex/debug
} else {
    win32:LIBS += -L../regex/release
}
win32 {
    DEFINES += WIN32_LEAN_AND_MEAN UNICODE
    LIBS += ole32.lib shlwapi.lib user32.lib ws2_32.lib winmm.lib iphlpapi.lib shell32.lib advapi32.lib regex.lib
    SOURCES +=   scanprogressimpl.cpp \
                 windows/_filwbuf.c \
                 windows/_getbuf.c \
                 windows/vwsscanf.c \
                 windows/fileinfo_win.cpp \
                 windows/filethread_win.cpp \
                 windows/gotourl_win.cpp \
                 windows/wfile.cpp \
                 windows/wfile_win.cpp \
                 windows/wlaunchthread_win.cpp \
                 windows/wutil_msvc.cpp
	RC_FILE =  icon.rc
}

unix {
    SOURCES +=    unix/fileinfo_unix.cpp \
                  unix/filethread_unix.cpp \
                  unix/gotourl_unix.cpp \
                  unix/mimedb.cpp \
                  unix/wcslwr.c \
                  unix/wcsupr.c \
                  unix/wfile.cpp \
                  unix/wfile_unix.cpp \
                  unix/wlaunchthread_unix.cpp \
                  unix/wutil_unix.cpp
}

QT += qt3support

INCLUDEPATH += muscle ../regex
