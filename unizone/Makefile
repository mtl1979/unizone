GCCOPTFLAGS = -I$(QTDIR)/include -DQT_THREAD_SUPPORT -Isrc -DNDEBUG -DNAME=\"Unizone\" -Isrc/muscle -DBETA -DQT_DLL -DMUSCLE_ENABLE_ZLIB_ENCODING -O2
LFLAGS =
LIBS = -L$(QTDIR)/lib -lqt-mt
EXECUTABLES = Unizone
OBJFILES = aboutdlg.o aboutdlgimpl.o accept.o botitem.o chattext.o \
colors.o combo.o downloadimpl.o downloadthread.o fileinfo.o \
filethread.o formatting.o genericthread.o gotourl.o main.o md5.o \
menubar.o moc_aboutdlg.o moc_aboutdlgimpl.o moc_chattext.o moc_combo.o \
moc_downloadimpl.o moc_genericthread.o moc_htmlview.o moc_netclient.o \
moc_prefs.o moc_prefsimpl.o moc_privatewindow.o moc_privatewindowimpl.o \
moc_winsharewindow.o netclient.o nicklist.o platform.o \
prefs.o prefsimpl.o privatewindow.o privatewindowimpl.o \
searchitem.o settings.o textevent.o transferitem.o ulistview.o \
uploadthread.o user.o userlistitem.o winshare_network.o \
winshare_parsing.o winshare_slots.o winsharewindow.o Log.o \
htmlview.o channel.o channelinfo.o channelimpl.o \
moc_channel.o wpwevent.o \
moc_channelimpl.o moc_menubar.o moc_ulistview.o \
wstatusbar.o winshare_channels.o winshare_search.o
VPATH = src
CXXFLAGS = -W -Wno-multichar -Wno-deprecated $(GCCOPTFLAGS)
TRANSLATIONS = unizone_de.qm unizone_en.qm unizone_es.qm unizone_fi.qm \
unizone_fr.qm unizone_pt.qm unizone_ru.qm unizone_sv.qm 

all : $(EXECUTABLES) translations

Unizone : $(OBJFILES) ./linux/muscle.a
	$(CXX) $(LFLAGS) $(LIBS) -o $@ $(OBJFILES) ./linux/muscle.a

linux/muscle.a : linux/Makefile
	make -C linux -f Makefile

%.ts : src/unizone.pro
	lupdate $<
 
translations : src/unizone.pro $(TRANSLATIONS)

%.qm : %.ts 
	lrelease $<

.PHONY=translations

src/downloadimpl.cpp: src/privatewindow.h src/channel.h

# stuff for moc

src/moc_aboutdlg.cpp: src/aboutdlg.h
	moc src/aboutdlg.h -o src/moc_aboutdlg.cpp

src/moc_aboutdlgimpl.cpp: src/aboutdlgimpl.h
	moc src/aboutdlgimpl.h -o src/moc_aboutdlgimpl.cpp

src/moc_chattext.cpp: src/chattext.h
	moc src/chattext.h -o src/moc_chattext.cpp

src/moc_combo.cpp: src/combo.h
	moc src/combo.h -o src/moc_combo.cpp

src/moc_downloadimpl.cpp: src/downloadimpl.h
	moc src/downloadimpl.h -o src/moc_downloadimpl.cpp

src/moc_genericthread.cpp: src/genericthread.h
	moc src/genericthread.h -o src/moc_genericthread.cpp

src/moc_htmlview.cpp: src/htmlview.h
	moc src/htmlview.h -o src/moc_htmlview.cpp

src/moc_netclient.cpp: src/netclient.h
	moc src/netclient.h -o src/moc_netclient.cpp

src/moc_prefs.cpp: src/prefs.h
	moc src/prefs.h -o src/moc_prefs.cpp

src/moc_prefsimpl.cpp: src/prefsimpl.h
	moc src/prefsimpl.h -o src/moc_prefsimpl.cpp

src/moc_privatewindow.cpp: src/privatewindow.h
	moc src/privatewindow.h -o src/moc_privatewindow.cpp

src/moc_privatewindowimpl.cpp: src/privatewindowimpl.h
	moc src/privatewindowimpl.h -o src/moc_privatewindowimpl.cpp

src/moc_winsharewindow.cpp: src/winsharewindow.h
	moc src/winsharewindow.h -o src/moc_winsharewindow.cpp

src/moc_channel.cpp: src/channel.h
	moc src/channel.h -o src/moc_channel.cpp

src/moc_channelimpl.cpp: src/channelimpl.h
	moc src/channelimpl.h -o src/moc_channelimpl.cpp

src/moc_menubar.cpp: src/menubar.h
	moc src/menubar.h -o src/moc_menubar.cpp

src/moc_ulistview.cpp: src/ulistview.h
	moc src/ulistview.h -o src/moc_ulistview.cpp

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

clean :
	rm -f *.o $(EXECUTABLES)
	rm -f src/*.qm
	rm -f src/moc_*.cpp
	rm -f src/aboutdlg.h src/aboutdlg.cpp
	rm -f src/channel.h src/channel.cpp
	rm -f src/prefs.h src/prefs.cpp
	rm -f src/privatewindow.h src/privatewindow.cpp
	make -C linux -f Makefile clean
