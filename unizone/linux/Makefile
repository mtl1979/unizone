GCCOPTFLAGS = -I../src/muscle/message -I../src/muscle/besupport -I../src/muscle/iogateway -I../src/muscle/reflector -I../src/muscle/regex -I../src/muscle/util -I../src/muscle/syslog -I../src/muscle/system -I../src/muscle -DQT_THREAD_SUPPORT -I$(QTDIR)/include -O2
LFLAGS =
LIBS =
EXECUTABLES =
OBJFILES = AbstractMessageIOGateway.o MessageIOGateway.o PlainTextMessageIOGateway.o RawDataMessageIOGateway.o Message.o AbstractReflectSession.o DumbReflectSession.o FilterSessionFactory.o RateLimitSessionIOPolicy.o ReflectServer.o ServerComponent.o StorageReflectSession.o PathMatcher.o StringMatcher.o SysLog.o AcceptSocketsThread.o GlobalMemoryAllocator.o MessageTransceiverThread.o SetupSystem.o Thread.o MemoryAllocator.o MiscUtilityFunctions.o NetworkUtilityFunctions.o PulseNode.o String.o
VPATH = ../src/muscle/message ../src/muscle/besupport ../src/muscle/iogateway ../src/muscle/reflector ../src/muscle/regex ../src/muscle/util ../src/muscle/syslog ../src/muscle/system
CXXFLAGS = -Wall -W -Wno-multichar $(GCCOPTFLAGS)
AR = ar r
all : muscle.a

muscle.a : $(OBJFILES)
	$(AR) $@ $^

clean:
	rm -f *.o muscle.a
file:/usr/share/doc/HTML/index.html
