ifeq ($(DEBUG), )
  DEBUG=0
endif

ifeq ($(DEBUG), 0)
  DEBUGCFLAGS = -DNDEBUG
else 
  DEBUGCFLAGS = -g -D_DEBUG 
endif

INCLUDES = -I$(QTDIR)/include -Isrc -Isrc/unix -Isrc/muscle \
	-Isrc/muscle/message -Isrc/muscle/iogateway \
	-Isrc/muscle/reflector -Isrc/muscle/regex -Isrc/muscle/util \
	-Isrc/muscle/syslog -Isrc/muscle/system
GCCOPTFLAGS = $(INCLUDES)  \
	-DBETA -DQT_DLL -O2 -fno-strict-aliasing
KERNEL_NAME := $(shell uname -s || true)

ifeq ($(KERNEL_NAME), Linux)
GCCOPTFLAGS += -DLINUX -D__USE_LARGEFILE_64
endif

CFLAGS = $(DEBUGCFLAGS) -Wall -W -Wno-multichar  $(GCCOPTFLAGS)
CXXFLAGS = $(CFLAGS) -Wno-deprecated
LFLAGS =
LIBS = -L$(QTDIR)/lib -lqt-mt
EXECUTABLES = cyr2utf
SRCFILES = cyr2utf.cpp 
OBJFILES =		$(addsuffix .o,$(basename $(SRCFILES)))

all : $(EXECUTABLES)

.PHONY : qtcheck

qtcheck : qtcheck.out 

qtcheck.out : $(QTDIR)/lib/libqt-mt.so 
	@echo -n "Checking Qt version... "
	@[ -f qtcheck.out ] || touch qtcheck.out
	@ls -l $(QTDIR)/lib/libqt-mt.so > qtcheck.tmp
	@cmp -s qtcheck.tmp qtcheck.out || \
	(rm -f $(MOC_SRCFILES) $(MUSCLEMOC_SRCFILES) $(UIC_SRCFILES) $(UIC_HEADERS) )
	@mv -f qtcheck.tmp qtcheck.out
	@sed -n 's/.*so\.\([[0-9]\.]*\)/\1/p' qtcheck.out

$(SRCFILES) : qtcheck.out

cyr2utf : $(OBJFILES) 
	@echo Linking $@ ...
	@$(CXX) $(LFLAGS) $(LIBS) -o $@ $^

%.o : %.cpp
	@echo Compiling $< ...
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : %.c
	@echo Compiling $< ...
	@$(CC) $(CFLAGS) -c -o $@ $<

clean :
	rm -rf *.o $(EXECUTABLES)

distclean : clean
	@rm -f qtcheck.out

# DO NOT DELETE

