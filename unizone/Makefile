INCLUDES = -I$(QTDIR)/include -Isrc -Isrc/muscle \
	-Isrc/muscle/message -Isrc/muscle/besupport \
	-Isrc/muscle/iogateway -Isrc/muscle/reflector \
	-Isrc/muscle/regex -Isrc/muscle/util \
	-Isrc/muscle/syslog -Isrc/muscle/system
GCCOPTFLAGS = $(INCLUDES) -DQT_THREAD_SUPPORT \
	-DNDEBUG -DBETA -DQT_DLL -DMUSCLE_ENABLE_ZLIB_ENCODING -O2
CXXFLAGS = -Wall -W -Wno-multichar -Wno-deprecated $(GCCOPTFLAGS)
LFLAGS =
LIBS = -L$(QTDIR)/lib -lqt-mt
EXECUTABLES = Unizone
OBJFILES = \
	aboutdlg.o aboutdlgimpl.o botitem.o chattext.o colors.o combo.o \
	downloadimpl.o downloadthread.o downloadworker.o fileinfo.o \
	filethread.o formatting.o gotourl.o main.o md5.o menubar.o \
	netclient.o nicklist.o platform.o prefs.o prefsimpl.o \
	privatewindow.o privatewindowimpl.o searchitem.o settings.o \
	textevent.o transferitem.o ulistview.o uploadthread.o user.o \
	userlistitem.o winshare_lists.o winshare_network.o winshare_parsing.o \
	winshare_slots.o winsharewindow.o Log.o htmlview.o channel.o \
	channelinfo.o channelimpl.o wpwevent.o wstring.o wstatusbar.o \
	winshare_channels.o winshare_search.o updateclient.o util.o \
	serverclient.o titanic.o scanprogress.o scanprogressimpl.o \
	QMessageTransceiverThread.o QAcceptSocketsThread.o \
	chatwindow.o wsystemevent.o wwarningevent.o werrorevent.o \
	version.o

MOC_OBJFILES = \
	moc_aboutdlg.o moc_aboutdlgimpl.o moc_chattext.o moc_combo.o \
	moc_downloadimpl.o moc_downloadthread.o moc_formatting.o \
	moc_htmlview.o moc_netclient.o moc_prefs.o moc_prefsimpl.o \
	moc_privatewindow.o moc_privatewindowimpl.o moc_winsharewindow.o \
	moc_uploadthread.o moc_channel.o moc_channelimpl.o moc_menubar.o \
	moc_ulistview.o moc_updateclient.o moc_serverclient.o moc_scanprogress.o \
	moc_QMessageTransceiverThread.o moc_QAcceptSocketsThread.o 

MUSCLE_OBJFILES = \
	AbstractMessageIOGateway.o MessageIOGateway.o \
	PlainTextMessageIOGateway.o RawDataMessageIOGateway.o \
	Message.o AbstractReflectSession.o DumbReflectSession.o \
	FilterSessionFactory.o RateLimitSessionIOPolicy.o \
	ReflectServer.o ServerComponent.o StorageReflectSession.o \
	PathMatcher.o StringMatcher.o SysLog.o AcceptSocketsThread.o \
	GlobalMemoryAllocator.o MessageTransceiverThread.o SetupSystem.o \
	Thread.o MemoryAllocator.o MiscUtilityFunctions.o \
	NetworkUtilityFunctions.o PulseNode.o String.o ByteBuffer.o \
	QueryFilter.o ZLibCodec.o ZLibUtilityFunctions.o

VPATH = src src/muscle/qtsupport \
 	src/muscle/message src/muscle/besupport \
	src/muscle/iogateway src/muscle/reflector \
	src/muscle/regex src/muscle/util src/muscle/syslog \
	src/muscle/system src/muscle/zlib
SRCFILES =		$(addsuffix .cpp,$(basename $(OBJFILES)))
MOC_SRCFILES =		$(addsuffix .cpp,$(basename $(MOC_OBJFILES)))
MUSCLE_SRCFILES = 	$(addsuffix .cpp,$(basename $(MUSCLE_OBJFILES)))

MOC=$(QTDIR)/bin/moc
UIC=$(QTDIR)/bin/uic

all : depends $(EXECUTABLES) translations

Unizone : $(OBJFILES) $(MUSCLE_OBJFILES) $(MOC_OBJFILES)
	$(CXX) $(LFLAGS) $(LIBS) -o $@ $^

moc_QMessageTransceiverThread.o : src/muscle/qtsupport/moc_QMessageTransceiverThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

QMessageTransceiverThread.o : src/muscle/qtsupport/QMessageTransceiverThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

moc_QAcceptSocketsThread.o : src/muscle/qtsupport/moc_QAcceptSocketsThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

QAcceptSocketsThread.o : src/muscle/qtsupport/QAcceptSocketsThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

translations : src/Makefile
	$(MAKE) -C src -f Makefile

depends: .depends

.depends: .depends1 .depends2 .depends3
	cat .depends1 > .depends
	cat .depends2 >> .depends
	cat .depends3 >> .depends
 
.depends1: $(SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $^ > .depends1

.depends2: $(MOC_SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $^ > .depends2

.depends3: $(MUSCLE_SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $^ > .depends3

.PHONY=translations depends

# stuff for moc

src/moc_aboutdlg.cpp: src/aboutdlg.h
	$(MOC) $< -o $@

src/moc_aboutdlgimpl.cpp: src/aboutdlgimpl.h
	$(MOC) $< -o $@

src/moc_chattext.cpp: src/chattext.h
	$(MOC) $< -o $@

src/moc_combo.cpp: src/combo.h
	$(MOC) $< -o $@

src/moc_downloadimpl.cpp: src/downloadimpl.h
	$(MOC) $< -o $@

src/moc_downloadthread.cpp: src/downloadthread.h
	$(MOC) $< -o $@

src/moc_uploadthread.cpp: src/uploadthread.h
	$(MOC) $< -o $@

src/moc_htmlview.cpp: src/htmlview.h
	$(MOC) $< -o $@

src/moc_netclient.cpp: src/netclient.h
	$(MOC) $< -o $@

src/moc_prefs.cpp: src/prefs.h
	$(MOC) $< -o $@

src/moc_prefsimpl.cpp: src/prefsimpl.h
	$(MOC) $< -o $@

src/moc_privatewindow.cpp: src/privatewindow.h
	$(MOC) $< -o $@

src/moc_privatewindowimpl.cpp: src/privatewindowimpl.h
	$(MOC) $< -o $@

src/moc_scanprogress.cpp: src/scanprogress.h
	$(MOC) $< -o $@

src/moc_serverclient.cpp: src/serverclient.h
	$(MOC) $< -o $@

src/moc_updateclient.cpp: src/updateclient.h
	$(MOC) $< -o $@

src/moc_winsharewindow.cpp: src/winsharewindow.h
	$(MOC) $< -o $@

src/moc_channel.cpp: src/channel.h
	$(MOC) $< -o $@

src/moc_channelimpl.cpp: src/channelimpl.h
	$(MOC) $< -o $@

src/moc_menubar.cpp: src/menubar.h
	$(MOC) $< -o $@

src/moc_ulistview.cpp: src/ulistview.h
	$(MOC) $< -o $@

src/moc_formatting.cpp: src/formatting.h
	$(MOC) $< -o $@

src/muscle/qtsupport/moc_QAcceptSocketsThread.cpp: src/muscle/qtsupport/QAcceptSocketsThread.h
	$(MOC) $< -o $@

src/muscle/qtsupport/moc_QMessageTransceiverThread.cpp: src/muscle/qtsupport/QMessageTransceiverThread.h
	$(MOC) $< -o $@

# stuff for uic
src/privatewindow.h: src/privatewindow.ui
	$(UIC) src/privatewindow.ui -o src/privatewindow.h

src/privatewindow.cpp: src/privatewindow.h src/privatewindow.ui
	$(UIC) -impl privatewindow.h src/privatewindow.ui -o src/privatewindow.cpp

src/prefs.h: src/prefs.ui
	$(UIC) src/prefs.ui -o src/prefs.h

src/prefs.cpp: src/prefs.h src/prefs.ui
	$(UIC) -impl prefs.h src/prefs.ui -o src/prefs.cpp

src/aboutdlg.h: src/aboutdlg.ui
	$(UIC) src/aboutdlg.ui -o src/aboutdlg.h

src/aboutdlg.cpp: src/aboutdlg.h src/aboutdlg.ui
	$(UIC) -impl aboutdlg.h src/aboutdlg.ui -o src/aboutdlg.cpp

src/channel.h: src/channel.ui
	$(UIC) src/channel.ui -o src/channel.h

src/channel.cpp: src/channel.h src/channel.ui
	$(UIC) -impl channel.h src/channel.ui -o src/channel.cpp

src/scanprogress.h: src/scanprogress.ui
	$(UIC) src/scanprogress.ui -o src/scanprogress.h

src/scanprogress.cpp: src/scanprogress.h src/scanprogress.ui
	$(UIC) -impl scanprogress.h src/scanprogress.ui -o src/scanprogress.cpp

clean :
	rm -f *.o *.a $(EXECUTABLES)
	rm -f src/moc_*.cpp
	rm -f src/muscle/qtsupport/moc_*.cpp
	rm -f src/aboutdlg.h src/aboutdlg.cpp
	rm -f src/channel.h src/channel.cpp
	rm -f src/prefs.h src/prefs.cpp
	rm -f src/privatewindow.h src/privatewindow.cpp
	$(MAKE) -C src -f Makefile clean

distclean : clean
	rm -f .depends .depends1 .depends2 .depends3

# DO NOT DELETE

-include .depends
