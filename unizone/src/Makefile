TRANSLATIONS = unizone_de.qm unizone_en.qm unizone_es.qm unizone_fi.qm \
unizone_fr.qm unizone_pt.qm unizone_ru.qm unizone_sv.qm unizone_1337.qm
LUPDATE=$(QTDIR)/bin/lupdate
LRELEASE=$(QTDIR)/bin/lrelease
all : translations

%.ts : unizone.pro
	$(LUPDATE) $<
 
translations : unizone.pro $(TRANSLATIONS)

%.qm : %.ts 
	$(LRELEASE) $<

clean : 
	rm -rf *.qm

.PHONY=translations clean
