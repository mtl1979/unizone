# TRANSLATIONS = unizone_de.qm unizone_en.qm unizone_es.qm unizone_fi.qm \
# unizone_fr.qm unizone_pt.qm unizone_ru.qm unizone_sv.qm 
# unizone_1337.qm
LUPDATE=$(QTDIR)/bin/lupdate
LRELEASE=$(QTDIR)/bin/lrelease
all : translations

-include unizone_unix.pro

QMFILES = $(addsuffix .qm,$(basename $(TRANSLATIONS)))

unizone_unix.pro : unizone.pro $(SOURCES) $(HEADERS)
	sed 's/scanprogress.cpp//' < unizone.pro > $@
	$(LUPDATE) $@	

%.ts : unizone_unix.pro
 
translations : $(QMFILES)

%.ts2 : %.ts
	sed -f fixts.sed $< > $@

%.qm : %.ts2 
	mv -f $< $(addsuffix .ts,$(basename $<))
	$(LRELEASE) $(addsuffix .ts,$(basename $<))
clean : 
	rm -f *.qm *.ts2

distclean: clean
	rm -f unizone_unix.pro 

.PHONY=translations clean
