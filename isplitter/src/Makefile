include isplitter.pro

LUPDATE=$(QTDIR)/bin/lupdate
LRELEASE=$(QTDIR)/bin/lrelease

QMFILES = $(addsuffix .qm, $(basename $(TRANSLATIONS)))

all: $(QMFILES)

isplitter.pro : $(SOURCES)
	$(LUPDATE) isplitter.pro
%.ts : isplitter.pro

%.ts2 : %.ts
	sed -f ../../src/fixts.sed $< > $@
%.qm : %.ts2
	mv -f $< $(addsuffix .ts,$(basename $<))
	$(LRELEASE) $(addsuffix .ts,$(basename $<))
 
clean:
	rm -f *.qm *.ts2
