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
moc_search.o moc_winsharewindow.o netclient.o nicklist.o platform.o \
prefs.o prefsimpl.o privatewindow.o privatewindowimpl.o search.o \
searchitem.o settings.o textevent.o transferitem.o ulistview.o \
uploadthread.o user.o userlistitem.o winshare_network.o \
winshare_parsing.o winshare_slots.o winsharewindow.o Log.o \
htmlview.o channel.o channels.o channelinfo.o channelimpl.o \
channelsimpl.o moc_channel.o moc_channels.o \
moc_channelimpl.o moc_channelsimpl.o moc_menubar.o moc_ulistview.o
VPATH = src
CXXFLAGS = -W -Wno-multichar -Wno-deprecated $(GCCOPTFLAGS)

all : $(EXECUTABLES)

Unizone : $(OBJFILES)
	$(CXX) $(LFLAGS) $(LIBS) -o $@ $(OBJFILES) ./linux/muscle.a

# stuff for moc
.PHONY=moc uic
moc:
	moc src/aboutdlg.h -o src/moc_aboutdlg.cpp
	moc src/aboutdlgimpl.h -o src/moc_aboutdlgimpl.cpp
	moc src/chattext.h -o src/moc_chattext.cpp
	moc src/combo.h -o src/moc_combo.cpp
	moc src/downloadimpl.h -o src/moc_downloadimpl.cpp
	moc src/genericthread.h -o src/moc_genericthread.cpp
	moc src/htmlview.h -o src/moc_htmlview.cpp
	moc src/netclient.h -o src/moc_netclient.cpp
	moc src/prefs.h -o src/moc_prefs.cpp
	moc src/prefsimpl.h -o src/moc_prefsimpl.cpp
	moc src/privatewindow.h -o src/moc_privatewindow.cpp
	moc src/privatewindowimpl.h -o src/moc_privatewindowimpl.cpp
	moc src/search.h -o src/moc_search.cpp
	moc src/winsharewindow.h -o src/moc_winsharewindow.cpp
	moc src/channel.h -o src/moc_channel.cpp
	moc src/channelimpl.h -o src/moc_channelimpl.cpp
	moc src/channels.h -o src/moc_channels.cpp
	moc src/channelsimpl.h -o src/moc_channelsimpl.cpp
	moc src/menubar.h -o src/moc_menubar.cpp
	moc src/ulistview.h -o src/moc_ulistview.cpp

# stuff for uic
uic:
	uic src/privatewindow.ui -o src/privatewindow.h
	uic -impl privatewindow.h src/privatewindow.ui -o src/privatewindow.cpp
	uic src/prefs.ui -o src/prefs.h
	uic -impl prefs.h src/prefs.ui -o src/prefs.cpp
	uic src/aboutdlg.ui -o src/aboutdlg.h
	uic -impl aboutdlg.h src/aboutdlg.ui -o src/aboutdlg.cpp
	uic src/channel.ui -o src/channel.h
	uic -impl channel.h src/channel.ui -o src/channel.cpp
	uic src/channels.ui -o src/channels.h
	uic -impl channels.h src/channels.ui -o src/channels.cpp
clean :
	rm -f *.o $(EXECUTABLES)
