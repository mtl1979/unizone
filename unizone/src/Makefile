# TRANSLATIONS = unizone_de.qm unizone_en.qm unizone_es.qm unizone_fi.qm \
# unizone_fr.qm unizone_pt.qm unizone_ru.qm unizone_sv.qm 
# unizone_1337.qm
LUPDATE=$(QTDIR)/bin/lupdate
LRELEASE=$(QTDIR)/bin/lrelease
all : translations

include unizone.pro

QMFILES = $(addsuffix .qm,$(basename $(TRANSLATIONS)))

unizone.pro : $(SOURCES) $(HEADERS)
	$(LUPDATE) $@	

%.ts : unizone.pro
 
translations : $(QMFILES)

%.qm : %.ts 
	$(LRELEASE) $<

clean : 
	rm -rf *.qm

.PHONY=translations clean
