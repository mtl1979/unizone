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

%.qm : %.ts 
	$(LRELEASE) $<

clean : 
	rm -f unizone_unix.pro 
	rm -f *.qm

.PHONY=translations clean
