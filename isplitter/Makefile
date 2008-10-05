all: isplitter translations

translations: src/isplitter.pro
	make -C src

UICFILES = \
	src/mainwindow.ui
UIC_HEADERS = \
	src/mainwindow.h
SRCFILES = \
	src/main.cpp \
	src/mainwindowimpl.cpp \
	src/menubar.cpp \
	src/previewimpl.cpp \
	src/util.cpp \
	../unizone/src/uenv.cpp

MOC_SRCFILES = \
	src/moc_mainwindowimpl.cpp \
	src/moc_menubar.cpp \
	src/moc_previewimpl.cpp

QTINC = $(QTDIR)/include
QTLIB ?= /usr/lib
LIBS = -L$(QTLIB) -lQtCore -lQtGui -lQt3Support
INCLUDES = -I$(QTINC) -I$(QTINC)/QtCore -I$(QTINC)/QtGui -I$(QTINC)/Qt3Support \
	-I../unizone/src -I../unizone/src/muscle -Isrc -I.
CXXFLAGS = $(INCLUDES) -DMUSCLE_SINGLE_THREAD_ONLY -DQT3_SUPPORT -DQT_CORE_LIB
VPATH = src

MOC_OBJFILES = $(addsuffix .o,$(basename $(MOC_SRCFILES)))
OBJFILES = $(addsuffix .o,$(basename $(SRCFILES)))

MOC=$(QTDIR)/bin/moc
UIC=$(QTDIR)/bin/uic

isplitter: $(OBJFILES) $(MOC_OBJFILES)
	$(CXX) $(LIBS) -o $@ $^

src/moc_mainwindowimpl.cpp : src/mainwindowimpl.h
	$(MOC) $< -o $@

src/moc_menubar.cpp : src/menubar.h
	$(MOC) $< -o $@

src/moc_previewimpl.cpp : src/previewimpl.h
	$(MOC) $< -o $@

src/mainwindow.h : src/mainwindow.ui
	$(UIC) -o src/mainwindow.h src/mainwindow.ui

src/mainwindowimpl.cpp : src/mainwindow.h

depends: .depends

.depends: $(MOC_SRCFILES) $(SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $^ > .depends

clean:
	$(MAKE) -C src clean
	rm -f isplitter
	rm -f $(OBJFILES)
	rm -f $(UIC_HEADERS)
	rm -f $(MOC_SRCFILES) $(MOC_OBJFILES)

distclean: clean
	rm -f .depends
-include .depends
