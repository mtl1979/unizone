GCCOPTFLAGS =
KERNEL_NAME := $(shell uname -s || true)

ifeq ($(KERNEL_NAME), Linux)
GCCOPTFLAGS += -DLINUX -D__USE_LARGEFILE_64
endif

all: musclecopy
MUSCLEDIR = ../unizone/src/muscle
UICFILES = \
	status.ui 
UIC_SRCFILES = \
	status.cpp
UIC_HEADERS = \
	status.h
SRCFILES = \
	downloadthread.cpp \
	genericthread.cpp \
	main.cpp \
	md5.cpp \
	statusimpl.cpp

COMMON_SRCFILES = \
	../unizone/src/unix/wcslwr.c \
	../unizone/src/unix/wcsupr.c \
	../unizone/src/unix/wfile.cpp \
	../unizone/src/unix/wfile_unix.cpp \
	../unizone/src/unix/wutil_unix.cpp \
	../unizone/src/wstring.cpp

MOC_SRCFILES = \
	moc_genericthread.cpp \
	moc_status.cpp \
	moc_statusimpl.cpp

MUSCLE_SRCFILES = \
	$(MUSCLEDIR)/iogateway/AbstractMessageIOGateway.cpp \
	$(MUSCLEDIR)/iogateway/MessageIOGateway.cpp \
	$(MUSCLEDIR)/message/Message.cpp \
	$(MUSCLEDIR)/reflector/AbstractReflectSession.cpp \
	$(MUSCLEDIR)/reflector/DataNode.cpp \
	$(MUSCLEDIR)/reflector/DumbReflectSession.cpp \
	$(MUSCLEDIR)/reflector/RateLimitSessionIOPolicy.cpp \
	$(MUSCLEDIR)/reflector/ReflectServer.cpp \
	$(MUSCLEDIR)/reflector/ServerComponent.cpp \
	$(MUSCLEDIR)/reflector/StorageReflectSession.cpp \
	$(MUSCLEDIR)/regex/PathMatcher.cpp \
	$(MUSCLEDIR)/regex/QueryFilter.cpp \
	$(MUSCLEDIR)/regex/StringMatcher.cpp \
	$(MUSCLEDIR)/syslog/SysLog.cpp \
	$(MUSCLEDIR)/system/MessageTransceiverThread.cpp \
	$(MUSCLEDIR)/system/SetupSystem.cpp \
	$(MUSCLEDIR)/system/Thread.cpp \
	$(MUSCLEDIR)/util/ByteBuffer.cpp \
	$(MUSCLEDIR)/util/MiscUtilityFunctions.cpp \
	$(MUSCLEDIR)/util/NetworkUtilityFunctions.cpp \
	$(MUSCLEDIR)/util/PulseNode.cpp \
	$(MUSCLEDIR)/util/String.cpp \
	$(MUSCLEDIR)/zlib/ZLibCodec.cpp

LIBS = -L$(QTDIR)/lib -lqt-mt
INCLUDES = -I$(QTDIR)/include -I$(MUSCLEDIR) -I. -I../unizone/src
CXXFLAGS = $(INCLUDES) -DQT_THREAD_SUPPORT -DMUSCLE_ENABLE_ZLIB_ENCODING -Wall -W -Wno-multichar -Wno-deprecated $(GCCOPTFLAGS)
CFLAGS = $(INCLUDES) -Wall -W -Wno-multichar $(GCCOPTFLAGS)

UIC_OBJFILES = $(addsuffix .o,$(basename $(UIC_SRCFILES)))
MOC_OBJFILES = $(addsuffix .o,$(basename $(MOC_SRCFILES)))
OBJFILES = $(addsuffix .o,$(basename $(SRCFILES)))
MUSCLE_OBJFILES = $(addsuffix .o,$(basename $(MUSCLE_SRCFILES)))
COMMON_OBJFILES = $(addsuffix .o,$(basename $(COMMON_SRCFILES)))

MOC=$(QTDIR)/bin/moc
UIC=$(QTDIR)/bin/uic

musclecopy: $(OBJFILES) $(UIC_OBJFILES) $(MOC_OBJFILES) $(MUSCLE_OBJFILES) $(COMMON_OBJFILES)
	$(CXX) $(LIBS) -o $@ $^

moc_genericthread.cpp : genericthread.h
	$(MOC) $< -o $@

moc_status.cpp : status.h
	$(MOC) $< -o $@

moc_statusimpl.cpp : statusimpl.h
	$(MOC) $< -o $@

status.h : status.ui
	$(UIC) status.ui -o status.h

status.cpp : status.h status.ui	
	$(UIC) -impl status.h status.ui -o status.cpp

depends: .depends

.depends: $(UIC_SRCFILES) $(MOC_SRCFILES) $(SRCFILES) $(MUSCLE_SRCFILES) $(COMMON_SRCFILES)
	$(CXX) $(CXXFLAGS) -MM $^ > .depends

clean:
	rm -f musclecopy
	rm -f $(OBJFILES) $(MUSCLE_OBJFILES)
	rm -f $(COMMON_OBJFILES)
	rm -f $(UIC_SRCFILES) $(UIC_HEADERS) $(UIC_OBJFILES)
	rm -f $(MOC_SRCFILES) $(MOC_OBJFILES)

distclean: clean
	rm -f .depends
-include .depends
