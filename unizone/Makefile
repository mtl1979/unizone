INCLUDES = -I$(QTDIR)/include -Isrc -Isrc/muscle
GCCOPTFLAGS = $(INCLUDES) -DQT_THREAD_SUPPORT -DNDEBUG -DBETA -DQT_DLL -DMUSCLE_ENABLE_ZLIB_ENCODING -O2
LFLAGS =
LIBS = -L$(QTDIR)/lib -lqt-mt
EXECUTABLES = Unizone
OBJFILES = aboutdlg.o aboutdlgimpl.o botitem.o chattext.o \
colors.o combo.o downloadimpl.o downloadthread.o downloadworker.o \
fileinfo.o filethread.o formatting.o gotourl.o main.o md5.o \
menubar.o moc_aboutdlg.o moc_aboutdlgimpl.o moc_chattext.o moc_combo.o \
moc_downloadimpl.o moc_downloadthread.o moc_htmlview.o moc_netclient.o \
moc_prefs.o moc_prefsimpl.o moc_privatewindow.o moc_privatewindowimpl.o \
moc_winsharewindow.o moc_uploadthread.o netclient.o nicklist.o platform.o \
prefs.o prefsimpl.o privatewindow.o privatewindowimpl.o \
searchitem.o settings.o textevent.o transferitem.o ulistview.o \
uploadthread.o user.o userlistitem.o winshare_network.o \
winshare_parsing.o winshare_slots.o winsharewindow.o Log.o \
htmlview.o channel.o channelinfo.o channelimpl.o \
moc_channel.o wpwevent.o wstring.o \
moc_channelimpl.o moc_menubar.o moc_ulistview.o \
wstatusbar.o winshare_channels.o winshare_search.o \
updateclient.o serverclient.o titanic.o \
moc_updateclient.o moc_serverclient.o \
moc_scanprogress.o scanprogress.o scanprogressimpl.o

QT_OBJFILES = moc_QMessageTransceiverThread.o moc_QAcceptSocketsThread.o \
QMessageTransceiverThread.o QAcceptSocketsThread.o
VPATH = src
QT_VPATH = src/muscle/qtsupport
SRCFILES=$(addprefix $(VPATH)/,$(addsuffix .cpp,$(basename $(OBJFILES))))
QT_SRCFILES=$(addprefix $(QT_VPATH)/,$(addsuffix .cpp,$(basename $(QT_OBJFILES))))
CXXFLAGS = -W -Wno-multichar -Wno-deprecated $(GCCOPTFLAGS)

all : depends $(EXECUTABLES) translations

Unizone : muscle.a $(OBJFILES) $(QT_OBJFILES)
	$(CXX) $(LFLAGS) $(LIBS) -o $@ $(OBJFILES) $(QT_OBJFILES) muscle.a

moc_QMessageTransceiverThread.o : src/muscle/qtsupport/moc_QMessageTransceiverThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

QMessageTransceiverThread.o : src/muscle/qtsupport/QMessageTransceiverThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

moc_QAcceptSocketsThread.o : src/muscle/qtsupport/moc_QAcceptSocketsThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

QAcceptSocketsThread.o : src/muscle/qtsupport/QAcceptSocketsThread.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

linux/muscle.a : linux/Makefile
	$(MAKE) -C linux -f Makefile

muscle.a : linux/muscle.a
	mv -f linux/muscle.a muscle.a

translations : src/Makefile
	$(MAKE) -C src -f Makefile

depends: .depends

.depends: .depends1 .depends2
	cat .depends1 > .depends
	cat .depends2 >> .depends
	rm -f linux/muscle.a
 
.depends1: $(SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $(SRCFILES) > .depends1

.depends2: $(QT_SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $(QT_SRCFILES) > .depends2

.PHONY=translations depends

# stuff for moc

src/moc_aboutdlg.cpp: src/aboutdlg.h
	moc $< -o $@

src/moc_aboutdlgimpl.cpp: src/aboutdlgimpl.h
	moc $< -o $@

src/moc_chattext.cpp: src/chattext.h
	moc $< -o $@

src/moc_combo.cpp: src/combo.h
	moc $< -o $@

src/moc_downloadimpl.cpp: src/downloadimpl.h
	moc $< -o $@

src/moc_downloadthread.cpp: src/downloadthread.h
	moc $< -o $@

src/moc_uploadthread.cpp: src/uploadthread.h
	moc $< -o $@

src/moc_htmlview.cpp: src/htmlview.h
	moc $< -o $@

src/moc_netclient.cpp: src/netclient.h
	moc $< -o $@

src/moc_prefs.cpp: src/prefs.h
	moc $< -o $@

src/moc_prefsimpl.cpp: src/prefsimpl.h
	moc $< -o $@

src/moc_privatewindow.cpp: src/privatewindow.h
	moc $< -o $@

src/moc_privatewindowimpl.cpp: src/privatewindowimpl.h
	moc $< -o $@

src/moc_scanprogress.cpp: src/scanprogress.h
	moc $< -o $@

src/moc_serverclient.cpp: src/serverclient.h
	moc $< -o $@

src/moc_updateclient.cpp: src/updateclient.h
	moc $< -o $@

src/moc_winsharewindow.cpp: src/winsharewindow.h
	moc $< -o $@

src/moc_channel.cpp: src/channel.h
	moc $< -o $@

src/moc_channelimpl.cpp: src/channelimpl.h
	moc $< -o $@

src/moc_menubar.cpp: src/menubar.h
	moc $< -o $@

src/moc_ulistview.cpp: src/ulistview.h
	moc $< -o $@

src/muscle/qtsupport/moc_QAcceptSocketsThread.cpp: src/muscle/qtsupport/QAcceptSocketsThread.h
	moc $< -o $@

src/muscle/qtsupport/moc_QMessageTransceiverThread.cpp: src/muscle/qtsupport/QMessageTransceiverThread.h
	moc $< -o $@

# stuff for uic
src/privatewindow.h: src/privatewindow.ui
	uic src/privatewindow.ui -o src/privatewindow.h

src/privatewindow.cpp: src/privatewindow.h src/privatewindow.ui
	uic -impl privatewindow.h src/privatewindow.ui -o src/privatewindow.cpp

src/prefs.h: src/prefs.ui
	uic src/prefs.ui -o src/prefs.h

src/prefs.cpp: src/prefs.h src/prefs.ui
	uic -impl prefs.h src/prefs.ui -o src/prefs.cpp

src/aboutdlg.h: src/aboutdlg.ui
	uic src/aboutdlg.ui -o src/aboutdlg.h

src/aboutdlg.cpp: src/aboutdlg.h src/aboutdlg.ui
	uic -impl aboutdlg.h src/aboutdlg.ui -o src/aboutdlg.cpp

src/channel.h: src/channel.ui
	uic src/channel.ui -o src/channel.h

src/channel.cpp: src/channel.h src/channel.ui
	uic -impl channel.h src/channel.ui -o src/channel.cpp

src/scanprogress.h: src/scanprogress.ui
	uic src/scanprogress.ui -o src/scanprogress.h

src/scanprogress.cpp: src/scanprogress.h src/scanprogress.ui
	uic -impl scanprogress.h src/scanprogress.ui -o src/scanprogress.cpp

clean :
	rm -f *.o *.a $(EXECUTABLES)
	rm -f src/moc_*.cpp
	rm -f src/muscle/qtsupport/moc_*.cpp
	rm -f src/aboutdlg.h src/aboutdlg.cpp
	rm -f src/channel.h src/channel.cpp
	rm -f src/prefs.h src/prefs.cpp
	rm -f src/privatewindow.h src/privatewindow.cpp
	$(MAKE) -C src -f Makefile clean
	$(MAKE) -C linux -f Makefile clean

distclean : clean
	rm -f .depends .depends1 .depends2

# DO NOT DELETE

include .depends
