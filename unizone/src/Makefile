TRANSLATIONS = unizone_de.qm unizone_en.qm unizone_es.qm unizone_fi.qm \
unizone_fr.qm unizone_pt.qm unizone_ru.qm unizone_sv.qm unizone_1337.qm

all : translations

%.ts : unizone.pro
	lupdate $<
 
translations : unizone.pro $(TRANSLATIONS)

%.qm : %.ts 
	lrelease $<

clean : 
	rm -rf *.qm

.PHONY=translations clean
