INCLUDES = -I$(QTDIR)/include -Isrc -Isrc/unix -Isrc/muscle \
	-Isrc/muscle/message -Isrc/muscle/iogateway \
	-Isrc/muscle/reflector -Isrc/muscle/regex -Isrc/muscle/util \
	-Isrc/muscle/syslog -Isrc/muscle/system
GCCOPTFLAGS = $(INCLUDES) -DQT_THREAD_SUPPORT \
	-DNDEBUG -DBETA -DQT_DLL -DMUSCLE_ENABLE_ZLIB_ENCODING -O2
ifeq ($(OSTYPE), linux)
GCCOPTFLAGS += -DLINUX -D__USE_LARGEFILE_64
endif
CXXFLAGS = -Wall -W -Wno-multichar -Wno-deprecated $(GCCOPTFLAGS)
LFLAGS =
LIBS = -L$(QTDIR)/lib -lqt-mt
EXECUTABLES = Unizone
SRCFILES = \
	src/aboutdlgimpl.cpp \
	src/botitem.cpp \
	src/channelinfo.cpp \
	src/channelimpl.cpp \
	src/chattext.cpp \
	src/chatwindow.cpp \
	src/colors.cpp \
	src/combo.cpp \
	src/downloadimpl.cpp \
	src/downloadthread.cpp \
	src/downloadworker.cpp \
	src/fileinfo.cpp \
	src/filethread.cpp \
	src/formatting.cpp \
	src/htmlview.cpp \
	src/main.cpp \
	src/md5.cpp \
	src/menubar.cpp \
	src/netclient.cpp \
	src/nicklist.cpp \
	src/parser.cpp \
	src/picviewer.cpp \
	src/picviewerimpl.cpp \
	src/platform.cpp \
	src/prefsimpl.cpp \
	src/privatewindowimpl.cpp \
	src/searchitem.cpp \
	src/serverclient.cpp \
	src/settings.cpp \
	src/textevent.cpp \
	src/titanic.cpp \
	src/transferitem.cpp \
	src/ulistview.cpp \
	src/updateclient.cpp \
	src/uploadthread.cpp \
	src/user.cpp \
	src/userlistitem.cpp \
	src/util.cpp \
	src/version.cpp \
	src/wcrypt.cpp \
	src/werrorevent.cpp \
	src/wfile.cpp \
	src/winshare_channels.cpp \
	src/winshare_lists.cpp \
	src/winshare_network.cpp \
	src/winshare_parsing.cpp \
	src/winshare_search.cpp \
	src/winshare_slots.cpp \
	src/winsharewindow.cpp \
	src/wmessageevent.cpp \
	src/wpwevent.cpp \
	src/wstatusbar.cpp \
	src/wstring.cpp \
	src/wsystemevent.cpp \
	src/wwarningevent.cpp \
	src/Log.cpp \
	src/unix/fileinfo_unix.cpp \
	src/unix/filethread_unix.cpp \
	src/unix/gotourl_unix.cpp \
	src/unix/mimedb.cpp \
	src/unix/wfile_unix.cpp \
	src/unix/wlaunchthread_unix.cpp

MOC_SRCFILES = \
	src/moc_aboutdlg.cpp \
	src/moc_aboutdlgimpl.cpp \
	src/moc_channel.cpp \
	src/moc_channelimpl.cpp \
	src/moc_chattext.cpp \
	src/moc_combo.cpp \
	src/moc_downloadimpl.cpp \
	src/moc_downloadthread.cpp \
	src/moc_formatting.cpp \
	src/moc_htmlview.cpp \
	src/moc_menubar.cpp \
	src/moc_netclient.cpp \
	src/moc_picviewer.cpp \
	src/moc_picviewerimpl.cpp \
	src/moc_prefs.cpp \
	src/moc_prefsimpl.cpp \
	src/moc_privatewindow.cpp \
	src/moc_privatewindowimpl.cpp \
	src/moc_serverclient.cpp \
	src/moc_ulistview.cpp \
	src/moc_updateclient.cpp \
	src/moc_uploadthread.cpp \
	src/moc_winsharewindow.cpp

MUSCLEMOC_SRCFILES = \
	src/muscle/qtsupport/moc_QAcceptSocketsThread.cpp \
	src/muscle/qtsupport/moc_QMessageTransceiverThread.cpp

MUSCLE_SRCFILES = \
	src/muscle/qtsupport/QMessageTransceiverThread.cpp \
	src/muscle/qtsupport/QAcceptSocketsThread.cpp \
	src/muscle/iogateway/AbstractMessageIOGateway.cpp \
	src/muscle/iogateway/MessageIOGateway.cpp \
	src/muscle/iogateway/PlainTextMessageIOGateway.cpp \
	src/muscle/iogateway/RawDataMessageIOGateway.cpp \
	src/muscle/message/Message.cpp \
	src/muscle/reflector/AbstractReflectSession.cpp \
	src/muscle/reflector/DumbReflectSession.cpp \
	src/muscle/reflector/FilterSessionFactory.cpp \
	src/muscle/reflector/RateLimitSessionIOPolicy.cpp \
	src/muscle/reflector/ReflectServer.cpp \
	src/muscle/reflector/ServerComponent.cpp \
	src/muscle/reflector/StorageReflectSession.cpp \
	src/muscle/regex/PathMatcher.cpp \
	src/muscle/regex/QueryFilter.cpp \
	src/muscle/regex/StringMatcher.cpp \
	src/muscle/syslog/SysLog.cpp \
	src/muscle/system/AcceptSocketsThread.cpp \
	src/muscle/system/GlobalMemoryAllocator.cpp \
	src/muscle/system/MessageTransceiverThread.cpp \
	src/muscle/system/SetupSystem.cpp \
	src/muscle/system/Thread.cpp \
	src/muscle/util/ByteBuffer.cpp \
	src/muscle/util/MemoryAllocator.cpp \
	src/muscle/util/MiscUtilityFunctions.cpp \
	src/muscle/util/NetworkUtilityFunctions.cpp \
	src/muscle/util/PulseNode.cpp \
	src/muscle/util/String.cpp \
	src/muscle/zlib/ZLibCodec.cpp \
	src/muscle/zlib/ZLibUtilityFunctions.cpp

VPATH = src src/unix src/muscle/qtsupport src/muscle/message \
	src/muscle/iogateway src/muscle/reflector \
	src/muscle/regex src/muscle/util src/muscle/syslog \
	src/muscle/system src/muscle/zlib

UIC_FILES = aboutdlg.ui channel.ui picviewer.ui prefs.ui privatewindow.ui
UIC_HEADERS  = $(addprefix src/,$(addsuffix .h,$(basename $(UIC_FILES))))
UIC_SRCFILES = $(addprefix src/,$(addsuffix .cpp,$(basename $(UIC_FILES))))
UIC_OBJFILES = $(addprefix src/,$(addsuffix .o,$(basename $(UIC_FILES))))

OBJFILES =		$(addsuffix .o,$(basename $(SRCFILES)))
MOC_OBJFILES =		$(addsuffix .o,$(basename $(MOC_SRCFILES)))
MUSCLEMOC_OBJFILES =	$(addsuffix .o,$(basename $(MUSCLEMOC_SRCFILES)))
MUSCLE_OBJFILES = 	$(addsuffix .o,$(basename $(MUSCLE_SRCFILES)))


MOC=$(QTDIR)/bin/moc
UIC=$(QTDIR)/bin/uic

all : qtcheck $(EXECUTABLES) translations

qtcheck.out : 
	@touch qtcheck.out

qtcheck : $(QTDIR)/lib/libqt-mt.so qtcheck.out
	@echo -n "Checking Qt version... "
	@ls -l $(QTDIR)/lib/libqt-mt.so > qtcheck.tmp
	@cmp -s qtcheck.tmp qtcheck.out || \
	(rm -f $(MOC_SRCFILES) $(MUSCLEMOC_SRCFILES) $(UIC_SRCFILES) $(UIC_HEADERS) )
	@mv -f qtcheck.tmp qtcheck.out
	@sed -n 's/.*so\.\([[0-9]\.]*\)/\1/p' qtcheck.out

Unizone : $(OBJFILES) $(MUSCLE_OBJFILES) $(MOC_OBJFILES) $(MUSCLEMOC_OBJFILES) $(UIC_OBJFILES)
	@echo Linking $@ ...
	@$(CXX) $(LFLAGS) $(LIBS) -o $@ $^

%.o : %.cpp
	@echo Compiling $< ...
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

# moc_QMessageTransceiverThread.o : src/muscle/qtsupport/moc_QMessageTransceiverThread.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<

# QMessageTransceiverThread.o : src/muscle/qtsupport/QMessageTransceiverThread.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<

# moc_QAcceptSocketsThread.o : src/muscle/qtsupport/moc_QAcceptSocketsThread.cpp
# 	$(CXX) $(CXXFLAGS) -c -o $@ $<

# QAcceptSocketsThread.o : src/muscle/qtsupport/QAcceptSocketsThread.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<

translations : src/Makefile
	$(MAKE) -C src -f Makefile

depends: .depends

.depends: .depends1 .depends2 .depends3 .depends4 .depends5
	@cat .depends1 > .depends
	@cat .depends2 >> .depends
	@cat .depends3 >> .depends
	@cat .depends4 >> .depends
	@cat .depends5 >> .depends
 
.depends1: $(UIC_SRCFILES)
	@echo Building dependencies \(1/5\) ...
	@$(CXX) $(CXXFLAGS) -MM $^ > .depends1

.depends2: $(SRCFILES)
	@echo Building dependencies \(2/5\) ...
	@$(CXX) $(CXXFLAGS) -MM $^ > .depends2

.depends3: $(MOC_SRCFILES)
	@echo Building dependencies \(3/5\) ...
	@$(CXX) $(CXXFLAGS) -MM $^ > .depends3

.depends4: $(MUSCLE_SRCFILES)
	@echo Building dependencies \(4/5\) ...
	@$(CXX) $(CXXFLAGS) -MM $^ > .depends4

.depends5: $(MUSCLEMOC_SRCFILES)
	@echo Building dependencies \(5/5\) ...
	@$(CXX) $(CXXFLAGS) -MM $^ > .depends5

.PHONY=translations depends

# stuff for moc

src/moc_aboutdlg.cpp: src/aboutdlg.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_aboutdlgimpl.cpp: src/aboutdlgimpl.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_chattext.cpp: src/chattext.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_combo.cpp: src/combo.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_downloadimpl.cpp: src/downloadimpl.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_downloadthread.cpp: src/downloadthread.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_uploadthread.cpp: src/uploadthread.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_htmlview.cpp: src/htmlview.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_netclient.cpp: src/netclient.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_picviewer.cpp: src/picviewer.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_picviewerimpl.cpp: src/picviewerimpl.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_prefs.cpp: src/prefs.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_prefsimpl.cpp: src/prefsimpl.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_privatewindow.cpp: src/privatewindow.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_privatewindowimpl.cpp: src/privatewindowimpl.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_serverclient.cpp: src/serverclient.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_updateclient.cpp: src/updateclient.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_winsharewindow.cpp: src/winsharewindow.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_channel.cpp: src/channel.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_channelimpl.cpp: src/channelimpl.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_menubar.cpp: src/menubar.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_ulistview.cpp: src/ulistview.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/moc_formatting.cpp: src/formatting.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/muscle/qtsupport/moc_QAcceptSocketsThread.cpp: src/muscle/qtsupport/QAcceptSocketsThread.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

src/muscle/qtsupport/moc_QMessageTransceiverThread.cpp: src/muscle/qtsupport/QMessageTransceiverThread.h
	@echo Creating $@ ...
	@$(MOC) $< -o $@

# stuff for uic
src/picviewer.h: src/picviewer.ui
	@echo Creating $@ ...
	@$(UIC) src/picviewer.ui -o src/picviewer.h

src/picviewer.cpp: src/picviewer.h src/picviewer.ui
	@echo Creating $@ ...
	@$(UIC) -impl picviewer.h src/picviewer.ui -o src/picviewer.cpp	

src/privatewindow.h: src/privatewindow.ui
	@echo Creating $@ ...
	@$(UIC) src/privatewindow.ui -o src/privatewindow.h

src/privatewindow.cpp: src/privatewindow.h src/privatewindow.ui
	@echo Creating $@ ...
	@$(UIC) -impl privatewindow.h src/privatewindow.ui -o src/privatewindow.cpp

src/prefs.h: src/prefs.ui
	@echo Creating $@ ...
	@$(UIC) src/prefs.ui -o src/prefs.h

src/prefs.cpp: src/prefs.h src/prefs.ui
	@echo Creating $@ ...
	@$(UIC) -impl prefs.h src/prefs.ui -o src/prefs.cpp

src/aboutdlg.h: src/aboutdlg.ui
	@echo Creating $@ ...
	@$(UIC) src/aboutdlg.ui -o src/aboutdlg.h

src/aboutdlg.cpp: src/aboutdlg.h src/aboutdlg.ui
	@echo Creating $@ ...
	@$(UIC) -impl aboutdlg.h src/aboutdlg.ui -o src/aboutdlg.cpp

src/channel.h: src/channel.ui
	@echo Creating $@ ...
	@$(UIC) src/channel.ui -o src/channel.h

src/channel.cpp: src/channel.h src/channel.ui
	@echo Creating $@ ...
	@$(UIC) -impl channel.h src/channel.ui -o src/channel.cpp

clean-aux:
	@rm -f *.o *.a $(EXECUTABLES)
	@rm -f $(OBJFILES) $(MUSCLE_OBJFILES)
	@rm -f $(MOC_SRCFILES) $(MOC_OBJFILES)
	@rm -f $(MUSCLEMOC_SRCFILES) $(MUSCLEMOC_OBJFILES)
	@rm -f $(UIC_SRCFILES) $(UIC_OBJFILES)
	@rm -f $(UIC_HEADERS)
	@rm -f core.*
clean :
	@$(MAKE) -C src -f Makefile clean
	@$(MAKE) clean-aux

distclean :
	@$(MAKE) -C src -f Makefile distclean
	@$(MAKE) clean-aux
	@rm -f .depends .depends[1-5] 

# DO NOT DELETE

-include .depends
