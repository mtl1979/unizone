include isplitter.pro

LUPDATE=$(QTDIR)/bin/lupdate
LRELEASE=$(QTDIR)/bin/lrelease

QMFILES = $(addsuffix .qm, $(basename $(TRANSLATIONS)))

all: $(QMFILES)

isplitter.pro : $(SOURCES)
	$(LUPDATE) isplitter.pro
%.ts : isplitter.pro

%.qm : %.ts
	$(LRELEASE) $(addsuffix .ts,$(basename $<))

clean:
	rm -f *.qm
