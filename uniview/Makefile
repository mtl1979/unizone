INCLUDES = -I$(QTDIR)/include -I. 
GCCOPTFLAGS = $(INCLUDES) -DQT_THREAD_SUPPORT -DNDEBUG -DBETA -DQT_DLL -O2
LFLAGS =
LIBS = -L$(QTDIR)/lib -lqt-mt
EXECUTABLES = Uniview
OBJFILES = uniview.o uniwindow.o htmlview.o combo.o textevent.o \
wstring.o moc_htmlview.o moc_combo.o moc_uniwindow.o

SRCFILES=$(addsuffix .cpp,$(basename $(OBJFILES)))
CXXFLAGS = -W -Wno-multichar -Wno-deprecated $(GCCOPTFLAGS)

all : depends $(EXECUTABLES)

Uniview : $(OBJFILES) 
	$(CXX) $(LFLAGS) $(LIBS) -o $@ $(OBJFILES)

#moc_htmlview.o : moc_htmlview.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<

#moc_combo.o : moc_combo.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<

#moc_uniwindow.o : moc_uniwindow.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<

depends: .depends

.depends: $(SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $(SRCFILES) > .depends

.PHONY=depends

# stuff for moc

moc_htmlview.cpp: htmlview.h
	moc $< -o $@

moc_combo.cpp: combo.h
	moc $< -o $@

moc_uniwindow.cpp: uniwindow.h
	moc $< -o $@

clean :
	rm -f *.o $(EXECUTABLES)
	rm -f moc_*.cpp

# DO NOT DELETE

include .depends
