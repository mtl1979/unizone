#ifndef MENUBAR_H
#define MENUBAR_H

#include <qmenubar.h>
#include <q3popupmenu.h>

class MenuBar : public QMenuBar
{
	Q_OBJECT
public:
	MenuBar(QWidget * parent = NULL);
	~MenuBar();

	Q3PopupMenu * File() const { return fFile; }

private:

	/* File menu */
	Q3PopupMenu * fFile;
};

#endif


