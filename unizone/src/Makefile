# TRANSLATIONS = unizone_de.qm unizone_en.qm unizone_es.qm unizone_fi.qm \
# unizone_fr.qm unizone_pt.qm unizone_ru.qm unizone_sv.qm 
# unizone_1337.qm
LUPDATE=$(QTDIR)/bin/lupdate
LRELEASE=$(QTDIR)/bin/lrelease
all : translations

-include unizone_unix.pro

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

unizone_unix.pro : unizone.pro $(SOURCES) $(HEADERS)
	@sed 's/scanprogress.cpp//' < unizone.pro > $@
	@$(LUPDATE) $@	

%.ts : unizone_unix.pro
 
translations : $(QMFILES)

%.ts2 : %.ts
	@sed -f fixts.sed $< > $@

%.qm : %.ts2
	@echo Creating $@ ... 
	@mv -f $< $(addsuffix .ts,$(basename $<))
	@$(LRELEASE) $(addsuffix .ts,$(basename $<))
clean : 
	@rm -f *.qm *.ts2

distclean: clean
	@rm -f unizone_unix.pro 

.PHONY=translations clean
