#ifndef MENUBAR_H
#define MENUBAR_H

#include <qmenubar.h>
#include <qpopupmenu.h>

class MenuBar : public QMenuBar
{
	Q_OBJECT
public:
	MenuBar(QWidget * parent = NULL);
	~MenuBar();

	QPopupMenu * File() const { return fFile; }

private:

	/* File menu */
	QPopupMenu * fFile;
};

#endif


