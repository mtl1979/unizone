# TRANSLATIONS = unizone_de.qm unizone_en.qm unizone_es.qm unizone_fi.qm \
# unizone_fr.qm unizone_pt.qm unizone_ru.qm unizone_sv.qm 
# unizone_1337.qm
LUPDATE=$(QTDIR)/bin/lupdate
LRELEASE=$(QTDIR)/bin/lrelease
all : translations

-include unizone.pro

QMFILES = $(addsuffix .qm,$(basename $(TRANSLATIONS)))

aboutdlg.cpp : aboutdlg.ui
	@$(MAKE) -C .. src/aboutdlg.cpp

channel.cpp : channel.ui
	@$(MAKE) -C .. src/channel.cpp

picviewer.cpp : picviewer.ui
	@$(MAKE) -C .. src/picviewer.cpp

prefs.cpp : prefs.ui
	@$(MAKE) -C .. src/prefs.cpp

privatewindow.cpp : privatewindow.ui
	@$(MAKE) -C .. src/privatewindow.cpp

scanprogress.cpp : scanprogress.ui
	@$(MAKE) -C .. src/scanprogress.cpp

unizone.pro : $(SOURCES) $(HEADERS)
	@$(LUPDATE) $@	

%.ts : unizone.pro
 
translations : $(QMFILES)

%.qm : %.ts
	@echo Creating $@ ... 
	@$(LRELEASE) $(addsuffix .ts,$(basename $<))

clean : 
	@rm -f *.qm

distclean: clean
	@rm -f unizone_unix.pro 

.PHONY=translations clean
