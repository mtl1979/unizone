all: isplitter translations

translations: src/isplitter.pro
	make -C src

UICFILES = \
	src/mainwindow.ui \
	src/preview.ui
UIC_SRCFILES = \
	src/mainwindow.cpp \
	src/preview.cpp
UIC_HEADERS = \
	src/mainwindow.h \
	src/preview.h
SRCFILES = \
	src/main.cpp \
	src/mainwindowimpl.cpp \
	src/menubar.cpp \
	src/previewimpl.cpp

MOC_SRCFILES = \
	src/moc_mainwindow.cpp \
	src/moc_mainwindowimpl.cpp \
	src/moc_menubar.cpp \
	src/moc_preview.cpp \
	src/moc_previewimpl.cpp

LIBS = -L$(QTDIR)/lib -lqt-mt
INCLUDES = -I$(QTDIR)/include -Isrc -I.
CXXFLAGS = $(INCLUDES)
VPATH = src

UIC_OBJFILES = $(addsuffix .o,$(basename $(UIC_SRCFILES)))
MOC_OBJFILES = $(addsuffix .o,$(basename $(MOC_SRCFILES)))
OBJFILES = $(addsuffix .o,$(basename $(SRCFILES)))

MOC=$(QTDIR)/bin/moc
UIC=$(QTDIR)/bin/uic

isplitter: $(OBJFILES) $(UIC_OBJFILES) $(MOC_OBJFILES)
	$(CXX) $(LIBS) -o $@ $^

src/moc_mainwindow.cpp : src/mainwindow.h
	$(MOC) $< -o $@

src/moc_mainwindowimpl.cpp : src/mainwindowimpl.h
	$(MOC) $< -o $@

src/moc_menubar.cpp : src/menubar.h
	$(MOC) $< -o $@

src/moc_preview.cpp : src/preview.h
	$(MOC) $< -o $@

src/moc_previewimpl.cpp : src/previewimpl.h
	$(MOC) $< -o $@

src/mainwindow.h : src/mainwindow.ui
	$(UIC) src/mainwindow.ui -o src/mainwindow.h

src/mainwindow.cpp : src/mainwindow.h src/mainwindow.ui	
	$(UIC) -impl src/mainwindow.h src/mainwindow.ui -o src/mainwindow.cpp

src/preview.h : src/preview.ui
	$(UIC) src/preview.ui -o src/preview.h

src/preview.cpp : src/preview.h src/preview.ui
	$(UIC) -impl src/preview.h src/preview.ui -o src/preview.cpp

depends: .depends

.depends: $(UIC_SRCFILES) $(MOC_SRCFILES) $(SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $^ > .depends

clean:
	$(MAKE) -C src clean
	rm -f isplitter
	rm -f $(OBJFILES) 
	rm -f $(UIC_SRCFILES) $(UIC_HEADERS) $(UIC_OBJFILES)
	rm -f $(MOC_SRCFILES) $(MOC_OBJFILES)

distclean: clean
	rm -f .depends
-include .depends
